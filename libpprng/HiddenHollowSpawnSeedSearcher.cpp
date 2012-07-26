/*
  Copyright (C) 2012 chiizu
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


#include "HiddenHollowSpawnSeedSearcher.h"
#include "SeedSearcher.h"

namespace pprng
{

namespace
{

struct FrameChecker
{
  FrameChecker(const HiddenHollowSpawnSeedSearcher::Criteria &criteria)
    : m_criteria(criteria)
  {}
  
  bool operator()(const HiddenHollowSpawnFrame &frame) const
  {
    return frame.isSpawn && (frame.group == m_criteria.group) &&
      (frame.slot == m_criteria.slot) &&
      ((m_criteria.slot >= 3) ||
       ((m_criteria.gender == Gender::FEMALE) ?
         (frame.genderPercentage < m_criteria.femalePercentage) :
        (m_criteria.gender == Gender::MALE) ?
         (frame.genderPercentage >= m_criteria.femalePercentage) :
         true));
  }
  
  const HiddenHollowSpawnSeedSearcher::Criteria  &m_criteria;
};

struct FrameGeneratorFactory
{
  typedef HiddenHollowSpawnFrameGenerator  FrameGenerator;
  
  FrameGeneratorFactory(bool memoryLinkUsed)
    : m_memoryLinkUsed(memoryLinkUsed)
  {}
  
  HiddenHollowSpawnFrameGenerator operator()(const HashedSeed &seed) const
  {
    return HiddenHollowSpawnFrameGenerator(seed, m_memoryLinkUsed);
  }
  
  bool m_memoryLinkUsed;
};

static const uint32_t SlotPercent[] = { 1, 4, 15, 1, 4, 15, 25, 1, 4, 15, 25 };

}

uint64_t HiddenHollowSpawnSeedSearcher::Criteria::ExpectedNumberOfResults() const
{
  uint64_t  numSeeds = seedParameters.NumberOfSeeds();
  
  uint64_t  numFrames = frame.max - frame.min + 1;
  
  uint64_t  slotPercent = SlotPercent[slot];
  
  uint64_t  genderNumerator = 1, genderDenominator = 1;
  if (slot < 3)
  {
    switch (gender)
    {
    case Gender::FEMALE:
      genderNumerator = femalePercentage;
      genderDenominator = 100;
      break;
      
    case Gender::MALE:
      genderNumerator = 100 - femalePercentage;
      genderDenominator = 100;
      break;
      
    default:
      break;
    }
  }
  
  uint64_t  numResults =
    numSeeds * numFrames * 5 * slotPercent * genderNumerator /
    (100 * 4 * 100 * genderDenominator);
  
  return numResults;
}

void HiddenHollowSpawnSeedSearcher::Search
  (const Criteria &criteria, const ResultCallback &resultHandler,
   const SearchRunner::ProgressCallback &progressHandler)
{
  HashedSeedGenerator    seedGenerator(criteria.seedParameters);
  FrameGeneratorFactory  frameGeneratorFactory(criteria.memoryLinkUsed);
  
  SeedFrameSearcher<FrameGeneratorFactory>  seedSearcher(frameGeneratorFactory,
                                                         criteria.frame);
  
  FrameChecker  frameChecker(criteria);
  
  SearchRunner  searcher;
  
  searcher.SearchThreaded(seedGenerator, seedSearcher, frameChecker,
                          resultHandler, progressHandler);
}

}
