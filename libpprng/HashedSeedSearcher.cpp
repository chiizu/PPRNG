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
#include "SeedGenerator.h"

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
    return ivs.betterThanOrEqual(m_criteria.minIVs) &&
           (!m_criteria.shouldCheckMaxIVs ||
            ivs.worseThanOrEqual(m_criteria.maxIVs));
  }

  bool CheckHiddenPower(const IVs &ivs) const
  {
    if (m_criteria.hiddenType == Element::UNKNOWN)
    {
      return true;
    }
    
    if ((m_criteria.hiddenType == Element::ANY) ||
        (m_criteria.hiddenType == ivs.HiddenType()))
    {
      return ivs.HiddenPower() >= m_criteria.minHiddenPower;
    }
    
    return false;
  }
  
  const HashedSeedSearcher::Criteria  &m_criteria;
};

struct FrameGeneratorFactory
{
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

uint64_t HashedSeedSearcher::Criteria::ExpectedNumberOfResults()
{
  uint64_t  seconds = (toTime - fromTime).total_seconds() + 1;
  uint64_t  keyCombos = buttonPresses.size();
  uint64_t  timer0Values = (timer0High - timer0Low) + 1;
  uint64_t  vcountValues = (vcountHigh - vcountLow) + 1;
  uint64_t  vframeValues = (vframeHigh - vframeLow) + 1;
  
  uint64_t  numSeeds =
    seconds * keyCombos * timer0Values * vcountValues * vframeValues;
  
  uint64_t  numFrames = maxFrame - minFrame + 1;
  
  IVs  maxIVs = shouldCheckMaxIVs ? this->maxIVs : IVs(0x7FFF7FFF);
  
  uint32_t  numIVs = (maxIVs.hp() - minIVs.hp() + 1) *
                     (maxIVs.at() - minIVs.at() + 1) *
                     (maxIVs.df() - minIVs.df() + 1) *
                     (maxIVs.sa() - minIVs.sa() + 1) *
                     (maxIVs.sd() - minIVs.sd() + 1) *
                     (maxIVs.sp() - minIVs.sp() + 1);
  return numFrames * numSeeds * numIVs / (32 * 32 * 32 * 32 * 32 * 32);
}

void HashedSeedSearcher::Search(const Criteria &criteria,
                                  const ResultCallback &resultHandler,
                                  const ProgressCallback &progressHandler)
{
  HashedSeedGenerator   seedGenerator(criteria.version,
                                      criteria.macAddressLow,
                                      criteria.macAddressHigh,
                                      criteria.timer0Low, criteria.timer0High,
                                      criteria.vcountLow, criteria.vcountHigh,
                                      criteria.vframeLow, criteria.vframeHigh,
                                      criteria.fromTime, criteria.toTime,
                                      criteria.buttonPresses);
  
  FrameGeneratorFactory   frameGenFactory(criteria.isRoamer ?
                                          HashedIVFrameGenerator::Roamer :
                                          HashedIVFrameGenerator::Normal);
  
  FrameChecker              frameChecker(criteria);
  SearcherType::FrameRange  frameRange(criteria.minFrame, criteria.maxFrame);
  
  SearcherType              searcher;
  
  searcher.Search(seedGenerator, frameGenFactory,
                  frameRange, frameChecker,
                  resultHandler, progressHandler);
}

}
