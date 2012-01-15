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
#include "SeedSearcher.h"

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
    return (m_criteria.pid.nature == Nature::ANY) ||
           (m_criteria.pid.nature == Nature::UNKNOWN) ||
           (m_criteria.pid.nature == nature);
  }
  
  bool CheckAbility(uint32_t ability) const
  {
    return (m_criteria.pid.ability == Ability::ANY) ||
           (m_criteria.pid.ability == ability);
  }
  
  bool CheckGender(uint32_t genderValue) const
  {
    return Gender::GenderValueMatches(genderValue,
                                      m_criteria.pid.gender,
                                      m_criteria.pid.genderRatio);
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
  
  const WonderCardSeedSearcher::Criteria  &m_criteria;
};

struct FrameGeneratorFactory
{
  typedef WonderCardFrameGenerator  FrameGenerator;
  
  FrameGeneratorFactory(const WonderCardSeedSearcher::Criteria &criteria)
    : m_criteria(criteria)
  {}
  
  WonderCardFrameGenerator operator()(const HashedSeed &seed) const
  {
    return WonderCardFrameGenerator(seed, m_criteria.frameParameters);
  }
  
  const WonderCardSeedSearcher::Criteria  &m_criteria;
};

}

uint64_t WonderCardSeedSearcher::Criteria::ExpectedNumberOfResults() const
{
  uint64_t  numSeeds = seedParameters.NumberOfSeeds();
  
  uint64_t  numFrames = frame.max - frame.min + 1;
  
  IVs  maxIVs = ivs.shouldCheckMax ? ivs.max : IVs(0x7FFF7FFF);
  
  uint64_t  numIVs = IVs::CalculateNumberOfCombinations(ivs.min, ivs.max);
  
  uint64_t  natureDivisor = (pid.nature != Nature::ANY) ? 25 : 1;
  uint64_t  abilityDivisor = (pid.ability > 1) ? 1 : 2;
  
  uint64_t  numResults = numSeeds * numFrames * numIVs /
                         (32 * 32 * 32 * 32 * 32 * 32 *
                          natureDivisor * abilityDivisor);
  
  if (ivs.hiddenType != Element::UNKNOWN)
  {
    numResults = IVs::AdjustExpectedResultsForHiddenPower
      (numResults, ivs.min, ivs.max, ivs.hiddenType, ivs.minHiddenPower);
  }
  
  return numResults;
}

void WonderCardSeedSearcher::Search
  (const Criteria &criteria, const ResultCallback &resultHandler,
   const SearchRunner::ProgressCallback &progressHandler)
{
  HashedSeedGenerator         seedGenerator(criteria.seedParameters);
  FrameGeneratorFactory       frameGeneratorFactory(criteria);
  
  // slightly hacky...
  SearchCriteria::FrameRange  frameRange
    (criteria.frameParameters.startFromLowestFrame ? 1 : criteria.frame.min,
     criteria.frame.max);
  
  SeedFrameSearcher<FrameGeneratorFactory>  seedSearcher(frameGeneratorFactory,
                                                         frameRange);
  
  FrameChecker                frameChecker(criteria);
  
  SearchRunner                searcher;
  
  searcher.SearchThreaded(seedGenerator, seedSearcher, frameChecker,
                          resultHandler, progressHandler);
}

}
