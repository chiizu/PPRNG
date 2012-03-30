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


#include "TrainerIDSearcher.h"
#include "SeedSearcher.h"

namespace pprng
{

namespace
{

struct FrameChecker
{
  FrameChecker(const TrainerIDSearcher::Criteria &criteria)
    : m_criteria(criteria),
      m_eggPID((uint64_t(criteria.shinyPID.word) * 0xFFFFFFFFULL) >> 32)
  {}
  
  bool operator()(const Gen5TrainerIDFrame &frame) const
  {
    if (m_criteria.hasTID && (frame.tid != m_criteria.tid))
      return false;
    
    if (m_criteria.hasShinyPID &&
        ((m_criteria.giftShiny && !frame.giftShiny) ||
         (m_criteria.wildShiny && !frame.wildShiny) ||
         (m_criteria.eggShiny && !frame.eggShiny)))
      return false;
    
    return true;
  }
  
  const TrainerIDSearcher::Criteria  &m_criteria;
  const PID                          m_eggPID;
};

struct FrameGeneratorFactory
{
  typedef Gen5TrainerIDFrameGenerator  FrameGenerator;
  
  FrameGeneratorFactory(const PID &shinyPID)
    : m_ShinyPID(shinyPID)
  {}
  
  Gen5TrainerIDFrameGenerator operator()(const HashedSeed &seed) const
  {
    return Gen5TrainerIDFrameGenerator(seed, m_ShinyPID);
  }
  
  const PID m_ShinyPID;
};

}

uint64_t TrainerIDSearcher::Criteria::ExpectedNumberOfResults() const
{
  uint64_t  numSeeds = seedParameters.NumberOfSeeds();
  
  uint64_t  numFrames = frame.max - frame.min + 1;
  
  uint64_t  tidDivisor = hasTID ? 65536 : 1;
  uint64_t  shinyDivisor =
    (hasShinyPID && (wildShiny || giftShiny || eggShiny)) ? 8192 : 1;
  
  return numFrames * numSeeds / (tidDivisor * shinyDivisor);
}

void TrainerIDSearcher::Search
  (const Criteria &criteria, const ResultCallback &resultHandler,
   const SearchRunner::ProgressCallback &progressHandler)
{
  HashedSeedGenerator    seedGenerator(criteria.seedParameters);
  FrameGeneratorFactory  frameGeneratorFactory(criteria.shinyPID);
  
  SeedFrameSearcher<FrameGeneratorFactory>  seedSearcher(frameGeneratorFactory,
                                                         criteria.frame);
  
  FrameChecker  frameChecker(criteria);
  
  SearchRunner  searcher;
  
  searcher.Search(seedGenerator, seedSearcher, frameChecker,
                  resultHandler, progressHandler);
}

}
