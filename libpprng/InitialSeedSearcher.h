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

#ifndef INITIAL_SEED_SEARCHER_H
#define INITIAL_SEED_SEARCHER_H

#include "PPRNGTypes.h"
#include "HashedSeed.h"
#include "HashedSeedSearcher.h"
#include "SearchRunner.h"
#include "SeedGenerator.h"

namespace pprng
{

class InitialIVSeedSearcher
{
public:
  struct Criteria : public SearchCriteria
  {
    HashedSeedGenerator::Parameters  seedParameters;
    
    IVs                       minIVs;
    IVs                       maxIVs;
    bool                      isRoamer;
    uint32_t                  maxSkippedFrames;
    
    Criteria()
      : seedParameters(), minIVs(), maxIVs(), maxSkippedFrames(0)
    {}
    
    uint64_t ExpectedNumberOfResults() const;
  };
  
  typedef HashedSeedSearcher::ResultType        ResultType;
  typedef HashedSeedSearcher::ResultCallback    ResultCallback;
  
  InitialIVSeedSearcher() {}
  
  void Search(const Criteria &criteria,
              const ResultCallback &resultHandler,
              const SearchRunner::ProgressCallback &progressHandler);
};

class B2W2InitialSeedSearcher
{
public:
  struct Criteria : public SearchCriteria
  {
    HashedSeedGenerator::Parameters  seedParameters;
    bool              memoryLinkUsed;
    SpinnerPositions  spins;
    
    Criteria()
      : seedParameters(), memoryLinkUsed(false), spins()
    {}
    
    uint64_t ExpectedNumberOfResults() const;
  };
  
  typedef HashedSeed                                 ResultType;
  typedef boost::function<void (const ResultType&)>  ResultCallback;
  
  B2W2InitialSeedSearcher() {}
  
  void Search(const Criteria &criteria,
              const ResultCallback &resultHandler,
              const SearchRunner::ProgressCallback &progressHandler);
};

}

#endif
