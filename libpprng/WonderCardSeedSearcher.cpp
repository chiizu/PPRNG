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


#include "WonderCardSeedSearcher.h"
#include "SeedGenerator.h"

namespace pprng
{

namespace
{

struct FrameChecker
{
  FrameChecker(const WonderCardSeedSearcher::Criteria &criteria)
    : m_criteria(criteria)
  {}
  
  bool operator()(const WonderCardFrame &frame) const
  {
    return CheckNature(frame.nature) && CheckAbility(frame.pid.Gen5Ability()) &&
           CheckGender(frame.pid.GenderValue()) && CheckIVs(frame.ivs) &&
           CheckHiddenPower(frame.ivs);
  }
  
  bool CheckNature(Nature::Type nature) const
  {
    return (m_criteria.nature == Nature::ANY) ||
           (m_criteria.nature == Nature::UNKNOWN) ||
           (m_criteria.nature == nature);
  }
  
  bool CheckAbility(uint32_t ability) const
  {
    return (m_criteria.ability > 1) || (m_criteria.ability == ability);
  }
  
  bool CheckGender(uint32_t genderValue) const
  {
    return Gender::GenderValueMatches(genderValue,
                                      m_criteria.gender,
                                      m_criteria.genderRatio);
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
  
  const WonderCardSeedSearcher::Criteria  &m_criteria;
};

struct FrameGeneratorFactory
{
  FrameGeneratorFactory(const WonderCardSeedSearcher::Criteria &criteria)
    : m_criteria(criteria)
  {}
  
  WonderCardFrameGenerator operator()(const HashedSeed &seed) const
  {
    return WonderCardFrameGenerator(seed, m_criteria.startFromLowestFrame,
                                    m_criteria.ivSkip, m_criteria.pidSkip,
                                    m_criteria.natureSkip,
                                    m_criteria.canBeShiny,
                                    m_criteria.tid, m_criteria.sid);
  }
  
  const WonderCardSeedSearcher::Criteria  &m_criteria;
};

}

uint64_t WonderCardSeedSearcher::Criteria::ExpectedNumberOfResults() const
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
  
  uint64_t  numIVs = IVs::CalculateNumberOfCombinations(minIVs, maxIVs);
  
  uint64_t  natureDivisor = (nature != Nature::ANY) ? 25 : 1;
  uint64_t  abilityDivisor = (ability > 1) ? 1 : 2;
  
  uint64_t  numResults = numSeeds * numFrames * numIVs /
                         (32 * 32 * 32 * 32 * 32 * 32 *
                          natureDivisor * abilityDivisor);
  
  if (hiddenType != Element::UNKNOWN)
  {
    numResults = IVs::AdjustExpectedResultsForHiddenPower
      (numResults, minIVs, maxIVs, hiddenType, minHiddenPower);
  }
  
  return numResults;
}

void WonderCardSeedSearcher::Search(const Criteria &criteria,
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
  
  FrameChecker              frameChecker(criteria);
  
  // slightly hacky...
  SearcherType::FrameRange  frameRange(criteria.startFromLowestFrame ? 1 :
                                       criteria.minFrame,
                                       criteria.maxFrame);
  
  SearcherType              searcher;
  
  searcher.SearchThreaded(seedGenerator, FrameGeneratorFactory(criteria),
                          frameRange, frameChecker,
                          resultHandler, progressHandler);
}

}
