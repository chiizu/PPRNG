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


#include "Gen4TIDSearcher.h"
#include "SeedSearcher.h"
#include "SearchCriteria.h"

namespace pprng
{

namespace
{

struct FrameChecker
{
  FrameChecker(const Gen4TIDSearcher::Criteria &criteria)
    : m_criteria(criteria)
  {}
  
  bool operator()(const Gen4TrainerIDFrame &frame) const
  {
    return (!m_criteria.considerTID || (frame.tid == m_criteria.tid)) &&
           (!m_criteria.considerSID || (frame.sid == m_criteria.sid)) &&
           (!m_criteria.considerPID ||
            m_criteria.pid.IsShiny(frame.tid, frame.sid));
  }
  
  const Gen4TIDSearcher::Criteria  &m_criteria;
};

struct SeedSearcher
{
  void Search(const uint32_t seed, const FrameChecker &checker,
              const Gen4TIDSearcher::ResultCallback &resultHandler)
  {
    Gen4TrainerIDFrameGenerator  frameGen(seed);
    frameGen.AdvanceFrame();
    
    // can only hit the first frame
    Gen4TrainerIDFrame  frame = frameGen.CurrentFrame();
    
    if (checker(frame))
      resultHandler(frame);
  }
};

}

uint64_t Gen4TIDSearcher::Criteria::ExpectedNumberOfResults() const
{
  uint64_t  delays = maxDelay - minDelay + 1;
  
  uint64_t  numSeeds = delays * 256 * 24;
  
  uint64_t  tidDivisor = considerTID ? 65536 : 1;
  uint64_t  sidDivisor = considerSID ? 65536 : 1;
  uint64_t  pidDivisor = considerPID ? 8192 : 1;
  
  uint64_t  numResults = numSeeds / (tidDivisor * sidDivisor * pidDivisor);
  
  return numResults;
}

void Gen4TIDSearcher::Search
  (const Criteria &criteria, const ResultCallback &resultHandler,
   const SearchRunner::ProgressCallback &progressHandler)
{
  TimeSeedGenerator  seedGenerator(criteria.minDelay, criteria.maxDelay);
  SeedSearcher       seedSearcher;
  FrameChecker       frameChecker(criteria);
  SearchRunner       searcher;
  
  searcher.Search(seedGenerator, seedSearcher, frameChecker,
                  resultHandler, progressHandler);
}

}
