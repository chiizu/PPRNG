/*
  Copyright (C) 2011 chiizu
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
           (!m_criteria.ivs.shouldCheckMax ||
            ivs.worseThanOrEqual(m_criteria.ivs.max));
  }

  bool CheckHiddenPower(const IVs &ivs) const
  {
    if (m_criteria.ivs.hiddenType == Element::UNKNOWN)
    {
      return true;
    }
    
    if ((m_criteria.ivs.hiddenType == Element::ANY) ||
        (m_criteria.ivs.hiddenType == ivs.HiddenType()))
    {
      return ivs.HiddenPower() >= m_criteria.ivs.minHiddenPower;
    }
    
    return false;
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

}

uint64_t HashedSeedSearcher::Criteria::ExpectedNumberOfResults() const
{
  uint64_t  numSeeds = seedParameters.NumberOfSeeds();
  
  uint64_t  numIVFrames = ivFrame.max - ivFrame.min + 1;
  
  IVs  maxIVs = ivs.shouldCheckMax ? ivs.max : IVs(0x7FFF7FFF);
  
  uint64_t  numIVs = IVs::CalculateNumberOfCombinations(ivs.min, ivs.max);
  
  uint64_t  numResults = numIVFrames * numSeeds * numIVs /
                           (32 * 32 * 32 * 32 * 32 * 32);
  
  if (ivs.hiddenType != Element::UNKNOWN)
  {
    numResults = IVs::AdjustExpectedResultsForHiddenPower
      (numResults, ivs.min, ivs.max, ivs.hiddenType, ivs.minHiddenPower);
  }
  
  return numResults;
}

void HashedSeedSearcher::Search
  (const Criteria &criteria, const ResultCallback &resultHandler,
   const SearchRunner::ProgressCallback &progressHandler)
{
  HashedSeedGenerator    seedGenerator(criteria.seedParameters);
  FrameGeneratorFactory  frameGenFactory(criteria.ivs.isRoamer ?
                                         HashedIVFrameGenerator::Roamer :
                                         HashedIVFrameGenerator::Normal);
  
  SeedFrameSearcher<FrameGeneratorFactory>  seedSearcher(frameGenFactory,
                                                         criteria.ivFrame);
  FrameChecker           frameChecker(criteria);
  SearchRunner           searcher;
  
  searcher.SearchThreaded(seedGenerator, seedSearcher, frameChecker,
                          resultHandler, progressHandler);
}

}
