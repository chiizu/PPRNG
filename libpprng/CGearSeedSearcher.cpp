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


#include "CGearSeedSearcher.h"
#include "SeedSearcher.h"

namespace pprng
{

namespace
{

struct FrameChecker
{
  FrameChecker(const CGearSeedSearcher::Criteria &criteria)
    : m_criteria(criteria)
  {}
  
  bool operator()(const CGearIVFrame &frame) const
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
    if (m_criteria.ivs.hiddenType == Element::NONE)
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
  
  const CGearSeedSearcher::Criteria  &m_criteria;
};

struct FrameGeneratorFactory
{
  typedef CGearIVFrameGenerator  FrameGenerator;
  
  FrameGeneratorFactory(CGearIVFrameGenerator::FrameType frameType)
    : m_frameType(frameType)
  {}
  
  CGearIVFrameGenerator operator()(uint32_t seed) const
  {
    return CGearIVFrameGenerator(seed, m_frameType);
  }
  
  const CGearIVFrameGenerator::FrameType m_frameType;
};

}

uint64_t CGearSeedSearcher::Criteria::ExpectedNumberOfResults() const
{
  uint64_t  delays = maxDelay - minDelay + 1;
  
  uint64_t  numSeeds = delays * 256 * 24;
  
  uint64_t  numFrames = frameRange.max - frameRange.min + 1;
  
  IVs  maxIVs = ivs.shouldCheckMax ? ivs.max : IVs(0x7FFF7FFF);
  
  uint64_t  numIVs = IVs::CalculateNumberOfCombinations(ivs.min, ivs.max);
  
  uint64_t  numResults = numFrames * numSeeds * numIVs /
                           (32 * 32 * 32 * 32 * 32 * 32);
  
  if (ivs.hiddenType != Element::NONE)
  {
    numResults = IVs::AdjustExpectedResultsForHiddenPower
      (numResults, ivs.min, ivs.max, ivs.hiddenType, ivs.minHiddenPower);
  }
  
  return numResults;
}

void CGearSeedSearcher::Search
  (const Criteria &criteria, const ResultCallback &resultHandler,
   const SearchRunner::ProgressCallback &progressHandler)
{
  CGearSeedGenerator        seedGenerator(criteria.minDelay, criteria.maxDelay,
                                          criteria.macAddressLow);
  
  FrameGeneratorFactory     frameGenFactory(criteria.ivs.isRoamer ?
                                            CGearIVFrameGenerator::Roamer :
                                            CGearIVFrameGenerator::Normal);
  
  SeedFrameSearcher<FrameGeneratorFactory>  seedSearcher(frameGenFactory,
                                                         criteria.frameRange);
  FrameChecker              frameChecker(criteria);
  
  SearchRunner              searcher;
  
  searcher.Search(seedGenerator, seedSearcher, frameChecker,
                  resultHandler, progressHandler);
}

}
