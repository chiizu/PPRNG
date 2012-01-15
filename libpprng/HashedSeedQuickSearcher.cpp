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


#include "HashedSeedQuickSearcher.h"
#include "HashedSeedSearcher.h"

#include <map>
#include <boost/unordered_map.hpp>

namespace pprng
{

namespace
{

typedef uint32_t IVSeedRow[3];

typedef boost::unordered_map<uint32_t, std::map<uint32_t, uint32_t> > IVSeedMap;

IVSeedMap MakeMap(IVSeedRow *data, uint32_t rowCount)
{
  IVSeedMap  result;
  
  for (uint32_t i = 0; i < rowCount; ++i)
  {
    result[data[i][0]][data[i][1]] = data[i][2];
  }
  
  return result;
}

// macro for parsing data file
#define DEFINE_IV_SEED(SEED, FRAME, IVWORD)   { SEED, FRAME, IVWORD },

// macro for defining the seed map
#define DEFINE_IV_SEED_MAP(NAME, DATASOURCE) \
IVSeedMap  NAME = MakeMap(DATASOURCE, sizeof(DATASOURCE) / sizeof(IVSeedRow))

// normal sets
IVSeedRow  PerfectIVsData[] = {
#include "data/normal_enc/perfect.txt"
};

DEFINE_IV_SEED_MAP(PerfectIVs, PerfectIVsData);

IVSeedRow  PhysIVsData[] = {
#include "data/normal_enc/phys.txt"
};

DEFINE_IV_SEED_MAP(PhysIVs, PhysIVsData);

IVSeedRow  SpecIVsData[] = {
#include "data/normal_enc/spec.txt"
};

DEFINE_IV_SEED_MAP(SpecIVs, SpecIVsData);

IVSeedRow  HpIVsData[] = {
#include "data/normal_enc/bug.txt"
#include "data/normal_enc/dark.txt"
#include "data/normal_enc/dragon.txt"
#include "data/normal_enc/electric.txt"
#include "data/normal_enc/fighting.txt"
#include "data/normal_enc/fire.txt"
#include "data/normal_enc/flying.txt"
#include "data/normal_enc/ghost.txt"
#include "data/normal_enc/grass.txt"
#include "data/normal_enc/ground.txt"
#include "data/normal_enc/ice.txt"
#include "data/normal_enc/poison.txt"
#include "data/normal_enc/psychic.txt"
#include "data/normal_enc/rock.txt"
#include "data/normal_enc/steel.txt"
#include "data/normal_enc/water.txt"
};

DEFINE_IV_SEED_MAP(HpIVs, HpIVsData);


// trick room sets
IVSeedRow  PerfectTrickIVsData[] = {
#include "data/normal_enc/perfect_trick.txt"
};

DEFINE_IV_SEED_MAP(PerfectTrickIVs, PerfectTrickIVsData);

IVSeedRow  PhysTrickIVsData[] = {
#include "data/normal_enc/phys_trick.txt"
};

DEFINE_IV_SEED_MAP(PhysTrickIVs, PhysTrickIVsData);

IVSeedRow  SpecTrickIVsData[] = {
#include "data/normal_enc/spec_trick.txt"
};

DEFINE_IV_SEED_MAP(SpecTrickIVs, SpecTrickIVsData);

IVSeedRow  HpTrickIVsData[] = {
#include "data/normal_enc/bug_trick.txt"
#include "data/normal_enc/dark_trick.txt"
#include "data/normal_enc/dragon_trick.txt"
#include "data/normal_enc/electric_trick.txt"
#include "data/normal_enc/fighting_trick.txt"
#include "data/normal_enc/fire_trick.txt"
#include "data/normal_enc/flying_trick.txt"
#include "data/normal_enc/ghost_trick.txt"
#include "data/normal_enc/grass_trick.txt"
#include "data/normal_enc/ground_trick.txt"
#include "data/normal_enc/ice_trick.txt"
#include "data/normal_enc/poison_trick.txt"
#include "data/normal_enc/psychic_trick.txt"
#include "data/normal_enc/rock_trick.txt"
#include "data/normal_enc/steel_trick.txt"
#include "data/normal_enc/water_trick.txt"
};

DEFINE_IV_SEED_MAP(HpTrickIVs, HpTrickIVsData);


// roamer normal sets
IVSeedRow  RoamerPerfectIVsData[] = {
#include "data/roamer_enc/perfect.txt"
};

DEFINE_IV_SEED_MAP(RoamerPerfectIVs, RoamerPerfectIVsData);

IVSeedRow  RoamerPhysIVsData[] = {
#include "data/roamer_enc/phys.txt"
};

DEFINE_IV_SEED_MAP(RoamerPhysIVs, RoamerPhysIVsData);

IVSeedRow  RoamerSpecIVsData[] = {
#include "data/roamer_enc/spec.txt"
};

DEFINE_IV_SEED_MAP(RoamerSpecIVs, RoamerSpecIVsData);

IVSeedRow  RoamerHpIVsData[] = {
#include "data/roamer_enc/bug.txt"
#include "data/roamer_enc/dark.txt"
#include "data/roamer_enc/dragon.txt"
#include "data/roamer_enc/electric.txt"
#include "data/roamer_enc/fighting.txt"
#include "data/roamer_enc/fire.txt"
#include "data/roamer_enc/flying.txt"
#include "data/roamer_enc/ghost.txt"
#include "data/roamer_enc/grass.txt"
#include "data/roamer_enc/ground.txt"
#include "data/roamer_enc/ice.txt"
#include "data/roamer_enc/poison.txt"
#include "data/roamer_enc/psychic.txt"
#include "data/roamer_enc/rock.txt"
#include "data/roamer_enc/steel.txt"
#include "data/roamer_enc/water.txt"
};

DEFINE_IV_SEED_MAP(RoamerHpIVs, RoamerHpIVsData);


// roamer trick room sets
IVSeedRow  RoamerPerfectTrickIVsData[] = {
#include "data/roamer_enc/perfect_trick.txt"
};

DEFINE_IV_SEED_MAP(RoamerPerfectTrickIVs, RoamerPerfectTrickIVsData);

IVSeedRow  RoamerPhysTrickIVsData[] = {
#include "data/roamer_enc/phys_trick.txt"
};

DEFINE_IV_SEED_MAP(RoamerPhysTrickIVs, RoamerPhysTrickIVsData);

IVSeedRow  RoamerSpecTrickIVsData[] = {
#include "data/roamer_enc/spec_trick.txt"
};

DEFINE_IV_SEED_MAP(RoamerSpecTrickIVs, RoamerSpecTrickIVsData);

IVSeedRow  RoamerHpTrickIVsData[] = {
#include "data/roamer_enc/bug_trick.txt"
#include "data/roamer_enc/dark_trick.txt"
#include "data/roamer_enc/dragon_trick.txt"
#include "data/roamer_enc/electric_trick.txt"
#include "data/roamer_enc/fighting_trick.txt"
#include "data/roamer_enc/fire_trick.txt"
#include "data/roamer_enc/flying_trick.txt"
#include "data/roamer_enc/ghost_trick.txt"
#include "data/roamer_enc/grass_trick.txt"
#include "data/roamer_enc/ground_trick.txt"
#include "data/roamer_enc/ice_trick.txt"
#include "data/roamer_enc/poison_trick.txt"
#include "data/roamer_enc/psychic_trick.txt"
#include "data/roamer_enc/rock_trick.txt"
#include "data/roamer_enc/steel_trick.txt"
#include "data/roamer_enc/water_trick.txt"
};

DEFINE_IV_SEED_MAP(RoamerHpTrickIVs, RoamerHpTrickIVsData);


const IVSeedMap& GetIVSeedMap(IVPattern::Type pattern, bool isRoamer)
{
  switch (pattern)
  {
  case IVPattern::HEX_FLAWLESS:
    return isRoamer ? RoamerPerfectIVs : PerfectIVs;
    break;
    
  case IVPattern::PHYSICAL_FLAWLESS:
    return isRoamer ? RoamerPhysIVs : PhysIVs;
    break;
    
  case IVPattern::SPECIAL_FLAWLESS:
    return isRoamer ? RoamerSpecIVs : SpecIVs;
    break;
    
  case IVPattern::SPECIAL_HIDDEN_POWER_FLAWLESS:
    return isRoamer ? RoamerHpIVs : HpIVs;
    break;
    
  case IVPattern::HEX_FLAWLESS_TRICK:
    return isRoamer ? RoamerPerfectTrickIVs : PerfectTrickIVs;
    break;
    
  case IVPattern::PHYSICAL_FLAWLESS_TRICK:
    return isRoamer ? RoamerPhysTrickIVs : PhysTrickIVs;
    break;
    
  case IVPattern::SPECIAL_FLAWLESS_TRICK:
    return isRoamer ? RoamerSpecTrickIVs : SpecTrickIVs;
    break;
    
  case IVPattern::SPECIAL_HIDDEN_POWER_FLAWLESS_TRICK:
    return isRoamer ? RoamerHpTrickIVs : HpTrickIVs;
    break;
  
  case IVPattern::CUSTOM:
  default:
    throw;
    break;
  }
}

struct FrameChecker
{
  FrameChecker(const HashedSeedQuickSearcher::Criteria &criteria)
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
  
  const HashedSeedQuickSearcher::Criteria  &m_criteria;
};

struct SeedSearcher
{
  typedef HashedIVFrame  ResultType;
  
  SeedSearcher(const IVSeedMap &seedMap,
               const SearchCriteria::FrameRange &frameRange)
    : m_seedMap(seedMap), m_frameRange(frameRange)
  {}
  
  void Search(const HashedSeed &seed, const FrameChecker &frameChecker,
              const HashedSeedQuickSearcher::ResultCallback &resultHandler)
  {
    uint32_t  ivSeed = seed.rawSeed >> 32;
    
    IVSeedMap::const_iterator  i = m_seedMap.find(ivSeed);
    if (i != m_seedMap.end())
    {
      std::map<uint32_t, uint32_t>::const_iterator  j = i->second.begin();
      std::map<uint32_t, uint32_t>::const_iterator  end = i->second.end();
      
      while ((j != end) && (j->first < m_frameRange.min))
        ++j;
      
      HashedIVFrame  result(seed);
      while ((j != end) && (j->first <= m_frameRange.max))
      {
        result.number = j->first;
        result.ivs = j->second;
        
        if (frameChecker(result))
          resultHandler(result);
        
        ++j;
      }
    }
  }
  
  const IVSeedMap                   &m_seedMap;
  const SearchCriteria::FrameRange  &m_frameRange;
};

}

uint64_t HashedSeedQuickSearcher::Criteria::ExpectedNumberOfResults() const
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

void HashedSeedQuickSearcher::Search
  (const Criteria &criteria, const ResultCallback &resultHandler,
   const SearchRunner::ProgressCallback &progressHandler)
{
  if ((criteria.ivPattern == IVPattern::CUSTOM) ||
      (criteria.ivFrame.min > 6) || (criteria.ivFrame.max > 6))
  {
    HashedSeedSearcher::Criteria  slowCriteria;
    
    slowCriteria.seedParameters = criteria.seedParameters;
    slowCriteria.ivs = criteria.ivs;
    slowCriteria.ivFrame = criteria.ivFrame;
    
    HashedSeedSearcher  searcher;
  
    searcher.Search(slowCriteria, resultHandler, progressHandler);
  }
  else
  {
    HashedSeedGenerator  seedGenerator(criteria.seedParameters);
    SeedSearcher         seedSearcher(GetIVSeedMap(criteria.ivPattern,
                                                   criteria.ivs.isRoamer),
                                      criteria.ivFrame);
    
    FrameChecker         frameChecker(criteria);
    SearchRunner         searcher;
    
    searcher.SearchThreaded(seedGenerator, seedSearcher, frameChecker,
                            resultHandler, progressHandler);
  }
}

}
