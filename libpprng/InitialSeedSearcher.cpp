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

uint64_t InitialIVSeedSearcher::Criteria::ExpectedNumberOfResults() const
{
  uint64_t  numSeeds = seedParameters.NumberOfSeeds();
  
  uint64_t  numIVs = IVs::CalculateNumberOfCombinations(minIVs, maxIVs);
  
  return numSeeds * numIVs / (32 * 32 * 32 * 32 * 32 * 32);
}

void InitialIVSeedSearcher::Search
  (const Criteria &criteria, const ResultCallback &resultHandler,
   const SearchRunner::ProgressCallback &progressHandler)
{
  using namespace boost::posix_time;
  
  HashedSeedSearcher::Criteria  c;
  
  c.seedParameters = criteria.seedParameters;
  c.ivFrame.min = 1;
  c.ivFrame.max = 1;
  c.ivs.shouldCheckMax = true;
  c.ivs.min = criteria.minIVs;
  c.ivs.max = criteria.maxIVs;
  c.ivs.hiddenType = Element::ANY;
  c.ivs.minHiddenPower = 30;
  c.ivs.isRoamer = criteria.isRoamer;
  
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
      std::cout << "Step 2: " << std::setw(16) << tid1 + i << std::endl;
      uint64_t  next = rng.Next();
      std::cout << std::dec << "  Next = " << ((((next >> 32) * 0xFFFFFFFF) >> 32) & 0xffff) << std::hex << std::endl;
      if ((next & 0xffff00000000ULL) == tid4)
      {
        result = tid1 + i;
        break;
      }
      else
      {
        std::cout << "Step 2.5: Failed" << std::endl;
      }
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
      return 0;
    }
  }
  
  if (i > 0xffffffffULL)
  {
    return 0;
  }
  
/*
  for (i = 0; i <= 0xffffffffULL; ++i)
  {
    uint64_t  seed1 = tid1 + i;
    LCRNG5    rng(seed1);
    
    if (((rng.Next() & 0xffff00000000ULL) == tid2) &&
        ((rng.Next() & 0xffff00000000ULL) == tid3))
    {
      uint64_t  next = rng.Next();
      std::cout << "Matched 2, last would be: " << std::setw(16) << next << std::endl;
      if ((next & 0xffff00000000ULL) == tid4)
      {
        result = seed1;
        break;
      }
    }
  }
*/
  
  std::cout << "Step 3: seed1 = " << std::setw(16) << result
            << " Next ID = " << std::dec
            << ((((rng.Next() >> 32) * 0xFFFFFFFF) >> 32) & 0xffff) << std::hex
            << std::endl;
  
  uint32_t  skippedFrames;
  
  for (i = 0; i <= 0xffffULL; ++i)
  {
    uint64_t  fullSeed = (i << 48) | result;
    LCRNG5_R  rRNG(fullSeed);
    
    for (skippedFrames = 0; skippedFrames < 100; ++skippedFrames)
    {
      uint32_t               mtSeed = (rRNG.Next() >> 32) & 0xffffffff;
      CGearIVFrameGenerator  ivFrameGenerator(mtSeed,
                                              CGearIVFrameGenerator::Normal,
                                              false);
      
      for (uint32_t j = 0; j < 50; ++j)
      {
        ivFrameGenerator.AdvanceFrame();
        
        IVs  ivs = ivFrameGenerator.CurrentFrame().ivs;
        
        if (ivs.betterThanOrEqual(criteria.minIVs) &&
            ivs.worseThanOrEqual(criteria.maxIVs))
        {
          std::cout << "\nStep 4: seed2 = "
            << std::setw(16) << fullSeed << std::endl;
          std::cout << std::dec << j << ": "
                    << ivs.hp() << '/' << ivs.at() << '/' << ivs.df() << '/'
                    << ivs.sa() << '/' << ivs.sd() << '/' << ivs.sp()
                    << std::hex << std::endl;
    
    std::cout << "\nInitial Seed = " << std::setw(16) << rRNG.Seed() << std::endl;
    std::cout << "Skipped Frames = " << std::dec << skippedFrames << std::endl;
        }
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
