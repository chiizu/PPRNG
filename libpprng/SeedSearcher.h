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

#ifndef SEED_SEARCHER_H
#define SEED_SEARCHER_H

#include "BasicTypes.h"
#include "FrameSearcher.h"
#include <boost/function.hpp>
#include <functional>

namespace pprng
{

template <class FrameGenerator>
class SeedSearcher
{
public:
  typedef typename FrameGenerator::Frame          Frame;
  typedef FrameSearcher<FrameGenerator>           SeedFrameSearcher;
  typedef typename SeedFrameSearcher::FrameRange  FrameRange;
  
  typedef boost::function<void (const Frame&)> ResultCallback;
  
  typedef boost::function<bool (double percent)>  ProgressCallback;
  
  template <class SeedGenerator, class FrameGeneratorFactory, class FrameChecker>
  void Search(SeedGenerator &seedGenerator,
              const FrameGeneratorFactory &frameGeneratorFactory,
              const FrameRange &frameRange,
              FrameChecker &frameChecker,
              const ResultCallback &resultHandler,
              const ProgressCallback &progressHandler)
  {
    typename SeedGenerator::SeedCountType  numSeeds = seedGenerator.NumSeeds();
    
    double  seedPercent = double(SeedGenerator::SeedsPerChunk) / numSeeds;
    
    if (seedPercent > 0.002)
    {
      seedPercent = 0.002;
    }
    
    typename SeedGenerator::SeedCountType  stepPercentSeeds =
      (seedPercent * numSeeds) + 1;
    
    const double stepPercent = seedPercent * 100.0;
    
    typename SeedGenerator::SeedCountType  threshold = stepPercentSeeds;
    
    for (typename SeedGenerator::SeedCountType i = 0;
         (i < numSeeds) && progressHandler(stepPercent);
         /* empty */)
    {
      for (/* empty */; i < threshold; ++i)
      {
        typename SeedGenerator::SeedType  seed = seedGenerator.Next();
        
        FrameGenerator     frameGenerator = frameGeneratorFactory(seed);
        SeedFrameSearcher  frameSearcher(frameGenerator);
        
        while(frameSearcher.Search(frameRange, frameChecker, resultHandler))
          /* search all frames, not just first */;
      }
      
      threshold += stepPercentSeeds;
      if (threshold > numSeeds)
      {
        threshold = numSeeds;
      }
    }
  }
  
private:
  template <class SeedGenerator>
  struct NoSeedTransformation
  {
    typename SeedGenerator::SeedType
      operator()(typename SeedGenerator::SeedType seed)
    {
      return seed;
    }
  };
};

}

#endif
