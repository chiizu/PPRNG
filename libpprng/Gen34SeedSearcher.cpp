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


#include "BasicTypes.h"
#include "Gen34SeedSearcher.h"
#include "SeedGenerator.h"

namespace pprng
{

namespace
{

struct FrameChecker
{
  FrameChecker(const Gen34SeedSearcher::Criteria &criteria)
    : m_criteria(criteria)
  {}
  
  bool operator()(const Gen34Frame &frame) const
  {
    return CheckNature(frame.pid) && CheckIVs(frame.ivs) &&
           CheckHiddenPower(frame.ivs);
  }
  
  bool CheckNature(const PID &pid) const
  {
    return (m_criteria.nature == Nature::ANY) ||
           (m_criteria.nature == Nature::UNKNOWN) ||
           (m_criteria.nature == pid.Gen34Nature());
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
  
  const Gen34SeedSearcher::Criteria  &m_criteria;
};

struct FrameGeneratorFactory
{
  Method1FrameGenerator operator()(uint32_t seed) const
  {
    return Method1FrameGenerator(seed);
  }
};

}

uint64_t Gen34SeedSearcher::Criteria::ExpectedNumberOfResults()
{
  uint64_t  delays = maxDelay - minDelay + 1;
  
  uint64_t  numSeeds = delays * 256 * 24;
  
  uint64_t  numFrames = maxFrame - minFrame + 1;
  
  uint64_t  hpDivisor = 1;
  if (hiddenType != Element::UNKNOWN)
  {
    hpDivisor = 40; // number of power levels
    
    if (hiddenType != Element::ANY)
    {
      hpDivisor *= 16;
    }
  }
  
  IVs  maxIVs = shouldCheckMaxIVs ? this->maxIVs : IVs(0x7FFF7FFF);
  
  uint32_t  numIVs = (maxIVs.hp() - minIVs.hp() + 1) *
                     (maxIVs.at() - minIVs.at() + 1) *
                     (maxIVs.df() - minIVs.df() + 1) *
                     (maxIVs.sa() - minIVs.sa() + 1) *
                     (maxIVs.sd() - minIVs.sd() + 1) *
                     (maxIVs.sp() - minIVs.sp() + 1);
  
  uint64_t  natureDivisor = (nature != Nature::ANY) ? 25 : 1;
  
  return numFrames * numSeeds * numIVs /
         (32 * 32 * 32 * 32 * 32 * 32 * natureDivisor * hpDivisor);
}

void Gen34SeedSearcher::Search(const Criteria &criteria,
                               const ResultCallback &resultHandler,
                               const ProgressCallback &progressHandler)
{
  TimeSeedGenerator         seedGenerator(criteria.minDelay, criteria.maxDelay);
  
  FrameChecker              frameChecker(criteria);
  SearcherType::FrameRange  frameRange(criteria.minFrame, criteria.maxFrame);
  
  SearcherType              searcher;
  
  searcher.Search(seedGenerator, FrameGeneratorFactory(),
                  frameRange, frameChecker,
                  resultHandler, progressHandler);
}

}
