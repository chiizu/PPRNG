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
#include "Gen4QuickSeedSearcher.h"
#include "SeedGenerator.h"

namespace pprng
{

namespace
{

struct SeedChecker
{
  SeedChecker(const Gen4QuickSeedSearcher::Criteria &criteria,
              const Gen4QuickSeedSearcher::ResultCallback &resultHandler)
    : m_criteria(criteria), m_resultHandler(resultHandler)
  {}
  
  void operator()(const Gen34Frame &frame) const
  {
    uint32_t  seed = frame.frameSeed;
    uint32_t  frameNumber = 0, limit = m_criteria.minFrame - 1;
    
    while (frameNumber < limit)
    {
      seed = LCRNG34_R::NextForSeed(seed);
      ++frameNumber;
    }
    
    limit = m_criteria.maxFrame;
    while (frameNumber < limit)
    {
      seed = LCRNG34_R::NextForSeed(seed);
      ++frameNumber;
      
      TimeSeed  ts(seed);
      
      if (ts.IsValid() &&
          (ts.BaseDelay() >= m_criteria.minDelay) &&
          (ts.BaseDelay() <= m_criteria.maxDelay))
      {
        Gen34Frame  actualFrame = frame;
        actualFrame.seed = seed;
        actualFrame.number = frameNumber;
        
        Gen4Frame  g4Frame(actualFrame);
        
        Gen4Frame::EncounterData  *data;
        
        if ((m_criteria.version == Game::HeartGold) ||
            (m_criteria.version == Game::SoulSilver))
        {
          data = &g4Frame.methodK;
        }
        else
        {
          data = &g4Frame.methodJ;
        }
        
        if ((data->landESVs & m_criteria.landESVs) ||
            (data->surfESVs & m_criteria.surfESVs) ||
            (data->oldRodESVs & m_criteria.oldRodESVs) ||
            (data->goodRodESVs & m_criteria.goodRodESVs) ||
            (data->superRodESVs & m_criteria.superRodESVs))
        {
          m_resultHandler(g4Frame);
        }
        
        break;
      }
    }
  }
  
  const Gen4QuickSeedSearcher::Criteria       &m_criteria;
  const Gen4QuickSeedSearcher::ResultCallback &m_resultHandler;
};

struct FrameChecker
{
  FrameChecker(const Gen4QuickSeedSearcher::Criteria &criteria)
    : m_criteria(criteria)
  {}
  
  bool operator()(const Gen34Frame &frame) const
  {
    return CheckShiny(frame.pid) && CheckNature(frame.pid) &&
           CheckAbility(frame.pid) && CheckGender(frame.pid) &&
           CheckIVs(frame.ivs) && CheckHiddenPower(frame.ivs);
  }
  
  bool CheckShiny(const PID &pid) const
  {
    return !m_criteria.shinyOnly || pid.IsShiny(m_criteria.tid, m_criteria.sid);
  }
  
  bool CheckNature(const PID &pid) const
  {
    return (m_criteria.nature == Nature::ANY) ||
           (m_criteria.nature == Nature::UNKNOWN) ||
           (m_criteria.nature == pid.Gen34Nature());
  }
  
  bool CheckAbility(const PID &pid) const
  {
    return (m_criteria.ability > 1) ||
           (m_criteria.ability == pid.Gen34Ability());
  }
  
  bool CheckGender(const PID &pid) const
  {
    return Gender::GenderValueMatches(pid.GenderValue(),
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
  
  const Gen4QuickSeedSearcher::Criteria  &m_criteria;
};

struct FrameGeneratorFactory
{
  Method1FrameGenerator operator()(uint32_t seed) const
  {
    return Method1FrameGenerator(seed);
  }
};

}

uint64_t Gen4QuickSeedSearcher::Criteria::ExpectedNumberOfResults()
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

void Gen4QuickSeedSearcher::Search(const Criteria &criteria,
                                   const ResultCallback &resultHandler,
                                   const ProgressCallback &progressHandler)
{
  Gen34IVSeedGenerator      seedGenerator(criteria.minIVs, criteria.maxIVs);
  
  FrameChecker              frameChecker(criteria);
  SeedChecker               seedChecker(criteria, resultHandler);
  
  typedef SeedSearcher<Method1FrameGenerator>  SearcherType;
  SearcherType::FrameRange  frameRange(1, 1);
  SearcherType              searcher;
  
  searcher.Search(seedGenerator, FrameGeneratorFactory(),
                  frameRange, frameChecker,
                  seedChecker, progressHandler);
}

}
