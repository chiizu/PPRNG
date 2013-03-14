/*
  Copyright (C) 2011-2012 chiizu
  chiizu.pprng@gmail.com
  
  This file is part of libpprng.
  
  libpprng is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  libpprng is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with libpprng.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "HashedSeedSearcher.h"
#include "SeedSearcher.h"

#include "IVSeedCache.h"

namespace pprng
{

namespace
{

struct FrameChecker
{
  FrameChecker(const HashedSeedSearcher::Criteria &criteria)
    : m_criteria(criteria)
  {}
  
  bool operator()(const HashedIVFrame &frame) const
  {
    return CheckIVs(frame.ivs) && CheckHiddenPower(frame.ivs);
  }
  
  bool CheckIVs(const IVs &ivs) const
  {
    return ivs.betterThanOrEqual(m_criteria.ivs.min) &&
           (m_criteria.ivs.max.isMax() ||
            ivs.worseThanOrEqual(m_criteria.ivs.max));
  }

  bool CheckHiddenPower(const IVs &ivs) const
  {
    return m_criteria.ivs.CheckHiddenPower(ivs.HiddenType(), ivs.HiddenPower());
  }
  
  const HashedSeedSearcher::Criteria  &m_criteria;
};

struct FrameGeneratorFactory
{
  typedef HashedIVFrameGenerator  FrameGenerator;
  
  FrameGeneratorFactory(HashedIVFrameGenerator::FrameType frameType)
    : m_frameType(frameType)
  {}
  
  HashedIVFrameGenerator operator()(const HashedSeed &seed) const
  {
    return HashedIVFrameGenerator(seed, m_frameType);
  }
  
  const HashedIVFrameGenerator::FrameType m_frameType;
};

struct SeedHashSearcher
{
  typedef HashedIVFrame  ResultType;
  
  SeedHashSearcher(const IVSeedHash &seedHash,
                   const SearchCriteria::FrameRange &frameRange,
                   bool isBlack2White2)
    : m_seedHash(seedHash), m_frameRange(frameRange),
      m_frameOffset(isBlack2White2 ? 2 : 0)
  {}
  
  void Search(const HashedSeed &seed, const FrameChecker &frameChecker,
              const HashedSeedSearcher::ResultCallback &resultHandler)
  {
    uint32_t  ivSeed = seed.rawSeed >> 32;
    
    IVSeedHash::const_iterator  i = m_seedHash.find(ivSeed);
    if (i != m_seedHash.end())
    {
      std::vector<SeedData>::const_iterator  j = i->second.begin();
      std::vector<SeedData>::const_iterator  end = i->second.end();
      
      uint32_t  limit = m_frameRange.min + m_frameOffset;
      while ((j != end) && (j->frame < limit))
        ++j;
      
      limit = m_frameRange.max + m_frameOffset;
      HashedIVFrame  result(seed);
      while ((j != end) && (j->frame <= limit))
      {
        result.number = j->frame - m_frameOffset;
        result.ivs = j->ivWord;
        
        if (frameChecker(result))
          resultHandler(result);
        
        ++j;
      }
    }
  }
  
  const IVSeedHash                  &m_seedHash;
  const SearchCriteria::FrameRange  &m_frameRange;
  const uint32_t                    m_frameOffset;
};

struct SeedMapSearcher
{
  typedef HashedIVFrame  ResultType;
  
  SeedMapSearcher(const IVSeedMap &seedMap,
                  const SearchCriteria::FrameRange &frameRange,
                  bool isBlack2White2)
    : m_seedMap(seedMap), m_frameRange(frameRange),
      m_frameOffset(isBlack2White2 ? 2 : 0)
  {}
  
  void Search(const HashedSeed &seed, const FrameChecker &frameChecker,
              const HashedSeedSearcher::ResultCallback &resultHandler)
  {
    uint32_t  ivSeed = seed.rawSeed >> 32;
    uint32_t  limit = m_frameRange.min + m_frameOffset;
    
    IVSeedMap::const_iterator  i = m_seedMap.lower_bound(ivSeed);
    IVSeedMap::const_iterator  end = m_seedMap.upper_bound(ivSeed);
    while ((i != end) && (i->second.frame < limit))
      ++i;
    
    limit = m_frameRange.max + m_frameOffset;
    HashedIVFrame  result(seed);
    while ((i != end) && (i->second.frame <= limit))
    {
      result.number = i->second.frame - m_frameOffset;
      result.ivs = i->second.ivWord;
      
      if (frameChecker(result))
        resultHandler(result);
      
      ++i;
    }
  }
  
  const IVSeedMap                   &m_seedMap;
  const SearchCriteria::FrameRange  &m_frameRange;
  const uint32_t                    m_frameOffset;
};

}

uint64_t HashedSeedSearcher::Criteria::ExpectedNumberOfResults() const
{
  uint64_t  numSeeds = seedParameters.NumberOfSeeds();
  
  uint64_t  numIVFrames = ivFrame.max - ivFrame.min + 1;
  
  uint64_t  numIVs = IVs::CalculateNumberOfCombinations(ivs.min, ivs.max);
  
  uint64_t  numResults = numIVFrames * numSeeds * numIVs /
                           (32 * 32 * 32 * 32 * 32 * 32);
  
  numResults = IVs::AdjustExpectedResultsForHiddenPower
    (numResults, ivs.min, ivs.max, ivs.hiddenTypeMask, ivs.minHiddenPower);
  
  return numResults;
}

void HashedSeedSearcher::Search
  (const Criteria &criteria, const ResultCallback &resultHandler,
   const SearchRunner::ProgressCallback &progressHandler)
{
  HashedSeedGenerator  seedGenerator(criteria.seedParameters);
  FrameChecker         frameChecker(criteria);
  SearchRunner         searcher;
  
  IVPattern::Type  ivPattern = criteria.ivs.GetPattern();
  bool             isBlack2White2 =
    Game::IsBlack2White2(criteria.seedParameters.version);
  uint32_t         offset = isBlack2White2 ? 2 :0;
  
  if ((ivPattern == IVPattern::CUSTOM) ||
      (criteria.ivFrame.min > (IVSeedMapMaxFrame - offset)) ||
      (criteria.ivFrame.max > (IVSeedMapMaxFrame - offset)) ||
      (criteria.ivs.isRoamer && isBlack2White2))
  {
    FrameGeneratorFactory  frameGenFactory(criteria.ivs.isRoamer ?
                                         HashedIVFrameGenerator::Roamer :
                                         HashedIVFrameGenerator::Normal);
  
    SeedFrameSearcher<FrameGeneratorFactory>  seedSearcher(frameGenFactory,
                                                           criteria.ivFrame);
    
    searcher.SearchThreaded(seedGenerator, seedSearcher, frameChecker,
                            resultHandler, progressHandler);
  }
  else if ((criteria.ivFrame.min > (IVSeedHashMaxFrame - offset)) ||
           (criteria.ivFrame.max > (IVSeedHashMaxFrame - offset)))
  {
    SeedMapSearcher  seedSearcher(GetIVSeedMap(ivPattern),
                                  criteria.ivFrame,
                                  isBlack2White2);
    searcher.SearchThreaded(seedGenerator, seedSearcher, frameChecker,
                            resultHandler, progressHandler);
  }
  else
  {
    SeedHashSearcher  seedSearcher(GetIVSeedHash(ivPattern,
                                                 criteria.ivs.isRoamer),
                                   criteria.ivFrame,
                                   isBlack2White2);
    searcher.SearchThreaded(seedGenerator, seedSearcher, frameChecker,
                            resultHandler, progressHandler);
  }
}

}
