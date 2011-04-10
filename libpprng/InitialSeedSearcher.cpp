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


#include "InitialSeedSearcher.h"
#include "LinearCongruentialRNG.h"
#include "FrameGenerator.h"
#include <iostream>
#include <iomanip>
#include <set>

namespace pprng
{

namespace
{

// the difference between each seed which matches TID2 is one of these
// memo: 0x307
static const uint64_t  TID2SeedStep[3] = { 0x195c, 0x21793, 0x230ef };

}

// 名前入力の時点での次に使われる乱数をr1[n]とすると、
// r1[n]×0xFFFFFFFFの上位32bitがトレーナーIDとなる。
void TIDSeedSearcher::Search(const Criteria &criteria,
                             const ResultCallback &resultHandler)
{
  uint64_t  tid1, tid2, tid3 /*, tid4*/;
  
  // multiplying by 0xFFFFFFFF has the effect of subtracting 1
  // from the value that will become the TID
  // (but shifting it up 32 bits)
  tid1 = criteria.tid1 + 1;
  tid1 = tid1 << 32;
  tid2 = criteria.tid2 + 1;
  tid2 = tid2 << 32;
  tid3 = criteria.tid3 + 1;
  tid3 = tid3 << 32;
  //tid4 = criteria.tid4 + 1;
  //tid4 = tid4 << 32;
  
  LCRNG5    rng(0);
  uint64_t  i = 0;
  while (i <= 0xffffffffULL)
  {
    rng.Seed(tid1 + i);
    
    if ((rng.Next() & 0xffff00000000ULL) == tid2)
      break;
    
    ++i;
  }
  
  while (i <= 0xffffffffULL)
  {
    if ((rng.Next() & 0xffff00000000ULL) == tid3)
    {
      Result  result;
      
      result.tidSeed = tid1 + i;
      
      resultHandler(result);
    }
    
    uint32_t  j;
    for (j = 0; j < 3; ++j)
    {
      rng.Seed(tid1 + i + TID2SeedStep[j]);
      if ((rng.Next() & 0xffff00000000ULL) == tid2)
      {
        i += TID2SeedStep[j];
        break;
      }
    }
    
    if (j == 3)
    {
      // some strange error...
      std::cerr << "OMG!" << std::endl;
      return;
    }
  }
}

uint64_t InitialIVSeedSearcher::Criteria::ExpectedNumberOfResults()
{
  uint64_t  seconds = 11;
  uint64_t  keyCombos = 1;
  uint64_t  timer0Values = (timer0High - timer0Low) + 1;
  uint64_t  vcountValues = (vcountHigh - vcountLow) + 1;
  uint64_t  vframeValues = (vframeHigh - vframeLow) + 1;
  
  uint64_t  numSeeds =
    seconds * keyCombos * timer0Values * vcountValues * vframeValues;
  
  uint32_t  numIVs = (maxIVs.hp() - minIVs.hp() + 1) *
                     (maxIVs.at() - minIVs.at() + 1) *
                     (maxIVs.df() - minIVs.df() + 1) *
                     (maxIVs.sa() - minIVs.sa() + 1) *
                     (maxIVs.sd() - minIVs.sd() + 1) *
                     (maxIVs.sp() - minIVs.sp() + 1);
  return numSeeds * numIVs / (32 * 32 * 32 * 32 * 32 * 32);
}

void InitialIVSeedSearcher::Search(const Criteria &criteria,
                                   const ResultCallback &resultHandler,
                                   const ProgressCallback &progressHandler)
{
  using namespace boost::posix_time;
  
  HashedSeedSearcher::Criteria  c;
  
  c.version = criteria.version;
  c.macAddressLow = criteria.macAddressLow;
  c.macAddressHigh = criteria.macAddressHigh;
  c.timer0Low = criteria.timer0Low;
  c.timer0High = criteria.timer0High;
  c.vcountLow = criteria.vcountLow;
  c.vcountHigh = criteria.vcountHigh;
  c.vframeLow = criteria.vframeLow;
  c.vframeHigh = criteria.vframeHigh;
  c.buttonPresses.push_back(0);
  c.fromTime = criteria.startTime - seconds(5);
  c.toTime = criteria.startTime + seconds(5);
  c.minFrame = 1;
  c.maxFrame = 1;
  c.maxResults = 1;
  c.minIVs = criteria.minIVs;
  c.shouldCheckMaxIVs = true;
  c.maxIVs = criteria.maxIVs;
  c.hiddenType = Element::UNKNOWN;
  c.minHiddenPower = 30;
  c.isRoamer = false;
  
  HashedSeedSearcher  searcher;
  
  searcher.Search(c, resultHandler, progressHandler);
}


// 名前入力の時点での次に使われる乱数をr1[n]とすると、
// r1[n]×0xFFFFFFFFの上位32bitがトレーナーIDとなる。
uint64_t InitialSeedSearcher::Search(const Criteria &criteria)
{
  uint64_t  tid1, tid2, tid3, tid4, result;
  
  // multiplying by 0xFFFFFFFF has the effect of subtracting 1
  // from the value that will become the TID
  // (but shifting it up 32 bits)
  tid1 = criteria.tid1 + 1;
  tid1 = tid1 << 32;
  tid2 = criteria.tid2 + 1;
  tid2 = tid2 << 32;
  tid3 = criteria.tid3 + 1;
  tid3 = tid3 << 32;
  tid4 = criteria.tid4 + 1;
  tid4 = tid4 << 32;
  
  result = tid1;
  
  std::cout << std::hex << std::setfill('0');
  std::cout << "Step 1: " << std::setw(16) << result << std::endl;
  
  uint64_t  i;
  for (i = 0; i <= 0xffffffffULL; ++i)
  {
    uint64_t  seed1 = tid1 + i;
    LCRNG5    rng(seed1);
    
    if (((rng.Next() & 0xffff00000000ULL) == tid2) &&
        ((rng.Next() & 0xffff00000000ULL) == tid3) &&
        ((rng.Next() & 0xffff00000000ULL) == tid4))
    {
      result = seed1;
      break;
    }
  }
  
  if (i > 0xffffffffULL)
  {
    return 0;
  }
  
  std::cout << "Step 2: seed1 = " << std::setw(16) << result << std::endl;
  
  uint32_t  skippedFrames;
  
  for (i = 0; i <= 0xffffULL; ++i)
  {
    uint64_t  fullSeed = (i << 48) | result;
    LCRNG5_R  rRNG(fullSeed);
    
    for (skippedFrames = 0; skippedFrames < 100; ++skippedFrames)
    {
      uint32_t               mtSeed = (rRNG.Next() >> 32) & 0xffffffff;
      CGearIVFrameGenerator  ivFrameGenerator(mtSeed,
                                              CGearIVFrameGenerator::Normal);
      
      ivFrameGenerator.AdvanceFrame();
      
      IVs  ivs = ivFrameGenerator.CurrentFrame().ivs;
      
      if (ivs.betterThanOrEqual(criteria.minIVs) &&
          ivs.worseThanOrEqual(criteria.maxIVs))
      {
        std::cout << "\nStep 3: seed2 = "
          << std::setw(16) << fullSeed << std::endl;
        result = rRNG.Seed();
  
  std::cout << "\nInitial Seed = " << std::setw(16) << result << std::endl;
  std::cout << "Skipped Frames = " << std::dec << skippedFrames << std::endl;
      }
    }
    
    if ((i & 0xff) == 0)
    {
      std::cout << '.';
      std::cout.flush();
    }
  }
  
  if (skippedFrames >= 100)
  {
    return 0;
  }
  
  return result;
}

}
