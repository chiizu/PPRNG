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


#include "Gen4EggIVSeedSearcher.h"
#include "SeedSearcher.h"

#include <vector>

namespace pprng
{

namespace
{

struct IVRange
{
  IVs       minIVs, maxIVs;
  uint32_t  inheritancePatterns;
  
  IVRange(IVs min, IVs max, uint32_t i)
    : minIVs(min), maxIVs(max), inheritancePatterns(i)
  {}
};

std::vector<IVRange> GenerateIVRanges(OptionalIVs parent1IVs,
                                      OptionalIVs parent2IVs,
                                      IVs minEggIVs, IVs maxEggIVs)
{
  std::vector<IVRange>  result;
  
  uint32_t  minIV, maxIV, iv;
  uint32_t  i, j, k, ic, jc, kc;
  
  for (i = 0; i < 4; ++i)
  {
    ic = 0;
    
    minIV = minEggIVs.iv(i);  maxIV = maxEggIVs.iv(i);
    
    if ((minIV == 0) && (maxIV == 31))
    {
      ic += 2;
    }
    else
    {
      if (parent1IVs.isSet(i))
      {
        iv = parent1IVs.iv(i);
        ic += ((iv >= minIV) && (iv <= maxIV)) ? 1 : 0;
      }
      if (parent2IVs.isSet(i))
      {
        iv = parent2IVs.iv(i);
        ic += ((iv >= minIV) && (iv <= maxIV)) ? 1 : 0;
      }
    }
    
    if (ic > 0)
    {
      for (j = i + 1; j < 5; ++j)
      {
        jc = 0;
        
        minIV = minEggIVs.iv(j);  maxIV = maxEggIVs.iv(j);
        
        if ((minIV == 0) && (maxIV == 31))
        {
          jc += 2;
        }
        else
        {
          if (parent1IVs.isSet(j))
          {
            iv = parent1IVs.iv(j);
            jc += ((iv >= minIV) && (iv <= maxIV)) ? 1 : 0;
          }
          if (parent2IVs.isSet(j))
          {
            iv = parent2IVs.iv(j);
            jc += ((iv >= minIV) && (iv <= maxIV)) ? 1 : 0;
          }
        }
        
        if (jc > 0)
        {
          for (k = j + 1; k < 6; ++k)
          {
            kc = 0;
            
            minIV = minEggIVs.iv(k);  maxIV = maxEggIVs.iv(k);
            
            if ((minIV == 0) && (maxIV == 31))
            {
              kc += 2;
            }
            else
            {
              if (parent1IVs.isSet(k))
              {
                iv = parent1IVs.iv(k);
                kc += ((iv >= minIV) && (iv <= maxIV)) ? 1 : 0;
              }
              if (parent2IVs.isSet(k))
              {
                iv = parent2IVs.iv(k);
                kc += ((iv >= minIV) && (iv <= maxIV)) ? 1 : 0;
              }
            }
            
            if (kc > 0)
            {
              IVs  min = minEggIVs, max = maxEggIVs;
              
              min.setIV(i, 0);  min.setIV(j, 0);  min.setIV(k, 0);
              max.setIV(i, 31); max.setIV(j, 31); max.setIV(k, 31);
              
              result.push_back(IVRange(min, max, ic * jc * kc));
            }
          }
        }
      }
    }
  }
  
  return result;
}


struct FrameChecker
{
  bool operator()(const Gen4BreedingFrame &frame) const
  {
    return true;
  }
};


struct FrameResultHandler
{
  FrameResultHandler(const Gen4EggIVSeedSearcher::Criteria &criteria,
                     const Gen4EggIVSeedSearcher::ResultCallback &resultHandler)
    : m_criteria(criteria), m_resultHandler(resultHandler)
  {}
  
  void operator()(const Gen4BreedingFrame &frame) const
  {
    Gen4EggIVFrame  eggFrame(frame, m_criteria.aIVs, m_criteria.bIVs);
    
    if (CheckIVs(eggFrame.ivs) && CheckHiddenPower(eggFrame.ivs))
    {
      m_resultHandler(eggFrame);
    }
  }
  
  bool CheckIVs(const OptionalIVs &ivs) const
  {
    return ivs.betterThanOrEqual(m_criteria.ivs.min) &&
           (m_criteria.ivs.max.isMax() ||
            ivs.worseThanOrEqual(m_criteria.ivs.max));
  }

  bool CheckHiddenPower(const OptionalIVs &oivs) const
  {
    return (m_criteria.ivs.hiddenTypeMask == 0) ||
           (oivs.allSet() &&
            m_criteria.ivs.CheckHiddenPower(oivs.values.HiddenType(),
                                            oivs.values.HiddenPower()));
  }
  
  const Gen4EggIVSeedSearcher::Criteria        &m_criteria;
  const Gen4EggIVSeedSearcher::ResultCallback  &m_resultHandler;
};


struct FrameGeneratorFactory
{
  typedef Gen4BreedingFrameGenerator  FrameGenerator;
  
  FrameGeneratorFactory(Game::Version version)
    : m_version(version)
  {}
  
  Gen4BreedingFrameGenerator operator()(uint32_t seed) const
  {
    return Gen4BreedingFrameGenerator(seed, m_version);
  }
  
  const Game::Version  m_version;
};

}

uint64_t Gen4EggIVSeedSearcher::Criteria::ExpectedNumberOfResults() const
{
  std::vector<IVRange>  ivRanges =
    GenerateIVRanges(aIVs, bIVs, ivs.min, ivs.max);
  
  if (ivRanges.size() == 0)
    return 0;
  
  uint64_t  delays = delay.max - delay.min + 1;
  
  uint64_t  numSeeds = delays * 256 * 24;
  
  uint64_t  numFrames = frame.max - frame.min + 1;
  
  
  std::vector<IVRange>::const_iterator  i;
  uint64_t                              ivMatches = 0UL;
  for (i = ivRanges.begin(); i != ivRanges.end(); ++i)
  {
    uint64_t  ivSets = IVs::CalculateNumberOfCombinations(i->minIVs, i->maxIVs);
    
    ivMatches += ivSets * i->inheritancePatterns /
                 /* total inheritance patterns * 6c3 */ 160UL;
  }
  
  uint64_t  result = ivMatches * numSeeds * numFrames /
                     (32ULL * 32ULL * 32ULL * 32ULL * 32ULL * 32ULL);
  
  result = IVs::AdjustExpectedResultsForHiddenPower
    (result, ivs.min, ivs.max, ivs.hiddenTypeMask, ivs.minHiddenPower);
  
  return result + 1;
}

void Gen4EggIVSeedSearcher::Search
  (const Criteria &criteria, const ResultCallback &resultHandler,
   const SearchRunner::ProgressCallback &progressHandler)
{
  TimeSeedGenerator      seedGenerator(criteria.delay.min, criteria.delay.max);
  FrameGeneratorFactory  frameGeneratorFactory(criteria.version);
  
  SeedFrameSearcher<FrameGeneratorFactory>  seedSearcher(frameGeneratorFactory,
                                                         criteria.frame);
  FrameChecker           frameChecker;
  FrameResultHandler     frameResultHandler(criteria, resultHandler);
  SearchRunner           searcher;
  
  searcher.Search(seedGenerator, seedSearcher, frameChecker,
                  frameResultHandler, progressHandler);
}

}
