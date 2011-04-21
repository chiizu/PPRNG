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


#include "TrainerIDSearcher.h"
#include "SeedGenerator.h"

namespace pprng
{

namespace
{

struct FrameChecker
{
  FrameChecker(const TrainerIDSearcher::Criteria &criteria)
    : m_criteria(criteria),
      m_pidHighLowBitsDiffer((criteria.shinyPID.word & 0x1) ^
                             ((criteria.shinyPID.word >> 31) & 0x1)),
      m_eggPID((uint64_t(criteria.shinyPID.word) * 0xFFFFFFFFULL) >> 32)
  {}
  
  bool operator()(const Gen5TrainerIDFrame &frame) const
  {
    if (m_criteria.hasTID && (frame.tid != m_criteria.tid))
      return false;
    
    if (m_criteria.hasShinyPID)
    {
      if (m_criteria.giftShiny &&
          !m_criteria.shinyPID.IsShiny(frame.tid, frame.sid))
        return false;
      
      if (m_criteria.wildShiny)
      {
        uint32_t  idLowBitsDiffer = (frame.tid & 0x1) ^ (frame.sid & 0x1);
        uint32_t  pidWord = m_criteria.shinyPID.word;
        
        if (m_pidHighLowBitsDiffer ^ idLowBitsDiffer)
          pidWord = pidWord ^ 0x10000000;
        
        if (!PID(pidWord).IsShiny(frame.tid, frame.sid))
          return false;
      }
      
      if (m_criteria.eggShiny && !m_eggPID.IsShiny(frame.tid, frame.sid))
          return false;
    }
    
    return true;
  }
  
  const TrainerIDSearcher::Criteria  &m_criteria;
  const uint32_t                     m_pidHighLowBitsDiffer;
  const PID                          m_eggPID;
};

struct FrameGeneratorFactory
{
  Gen5TrainerIDFrameGenerator operator()(const HashedSeed &seed) const
  {
    return Gen5TrainerIDFrameGenerator(seed);
  }
};

}

uint64_t TrainerIDSearcher::Criteria::ExpectedNumberOfResults()
{
  uint64_t  seconds = (toTime - fromTime).total_seconds() + 1;
  uint64_t  keyCombos = buttonPresses.size();
  uint64_t  timer0Values = (timer0High - timer0Low) + 1;
  uint64_t  vcountValues = (vcountHigh - vcountLow) + 1;
  uint64_t  vframeValues = (vframeHigh - vframeLow) + 1;
  
  uint64_t  numSeeds =
    seconds * keyCombos * timer0Values * vcountValues * vframeValues;
  
  uint64_t  numFrames = maxFrame - minFrame + 1;
  
  uint64_t  tidDivisor = hasTID ? 65536 : 1;
  uint64_t  shinyDivisor =
    (hasShinyPID && (wildShiny || giftShiny || eggShiny)) ? 8192 : 1;
  
  return numFrames * numSeeds / (tidDivisor * shinyDivisor);
}

void TrainerIDSearcher::Search(const Criteria &criteria,
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
  SearcherType::FrameRange  frameRange(criteria.minFrame, criteria.maxFrame);
  
  SearcherType              searcher;
  
  searcher.Search(seedGenerator, FrameGeneratorFactory(),
                  frameRange, frameChecker,
                  resultHandler, progressHandler);
}

}
