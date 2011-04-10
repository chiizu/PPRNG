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


#include "SIDSearcher.h"
#include "SeedGenerator.h"

namespace pprng
{

namespace
{

struct FrameChecker
{
  FrameChecker(const SIDSearcher::Criteria &criteria)
    : m_criteria(criteria)
  {}
  
  bool operator()(const Gen5TrainerIDFrame &frame) const
  {
    return frame.tid == m_criteria.tid;
  }
  
  const SIDSearcher::Criteria  &m_criteria;
};

struct FrameGeneratorFactory
{
  Gen5TrainerIDFrameGenerator operator()(const HashedSeed &seed) const
  {
    return Gen5TrainerIDFrameGenerator(seed);
  }
};

}

uint64_t SIDSearcher::Criteria::ExpectedNumberOfResults()
{
  uint64_t  seconds = 24 * 60 * 60;
  uint64_t  keyCombos = 1;
  uint64_t  timer0Values = (timer0High - timer0Low) + 1;
  uint64_t  vcountValues = (vcountHigh - vcountLow) + 1;
  uint64_t  vframeValues = (vframeHigh - vframeLow) + 1;
  
  uint64_t  numSeeds =
    seconds * keyCombos * timer0Values * vcountValues * vframeValues;
  
  uint64_t  numFrames = maxFrame - minFrame + 1;
  
  return numFrames * numSeeds / 65536;
}

void SIDSearcher::Search(const Criteria &criteria,
                         const ResultCallback &resultHandler,
                         const ProgressCallback &progressHandler)
{
  using namespace boost::posix_time;
  using namespace boost::gregorian;
  
  ptime  from(criteria.startDate, criteria.startTime);
  ptime  to(criteria.startDate, criteria.endTime);
  
  Button::List  keyList;
  keyList.push_back(0);
  
  HashedSeedGenerator   seedGenerator(criteria.version,
                                        criteria.macAddressLow,
                                        criteria.macAddressHigh,
                                        criteria.timer0Low, criteria.timer0High,
                                        criteria.vcountLow, criteria.vcountHigh,
                                        criteria.vframeLow, criteria.vframeHigh,
                                        from, to,
                                        keyList);
  
  FrameChecker              frameChecker(criteria);
  SearcherType::FrameRange  frameRange(criteria.minFrame, criteria.maxFrame);
  
  SearcherType              searcher;
  
  searcher.Search(seedGenerator, FrameGeneratorFactory(),
                  frameRange, frameChecker,
                  resultHandler, progressHandler);
}

}
