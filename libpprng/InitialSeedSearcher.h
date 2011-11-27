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

#ifndef INITIAL_SEED_SEARCHER_H
#define INITIAL_SEED_SEARCHER_H

#include "BasicTypes.h"
#include "HashedSeed.h"
#include "HashedSeedSearcher.h"
#include "SeedGenerator.h"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace pprng
{

class TIDSeedSearcher
{
public:
  struct Criteria
  {
    uint32_t  tid1;
    uint32_t  tid2;
    uint32_t  tid3;
  };
  
  struct Result
  {
    uint64_t  tidSeed;
  };
  typedef boost::function<void (const Result&)>  ResultCallback;
  
  TIDSeedSearcher() {}
  
  void Search(const Criteria &criteria,
              const ResultCallback &resultHandler);
};

class InitialIVSeedSearcher
{
public:
  struct Criteria : public SeedSearchCriteria
  {
    HashedSeedGenerator::Parameters  seedParameters;
    
    IVs                       minIVs;
    IVs                       maxIVs;
    uint32_t                  maxSkippedFrames;
    
    Criteria()
      : seedParameters(), minIVs(), maxIVs(), maxSkippedFrames(0)
    {}
    
    uint64_t ExpectedNumberOfResults() const;
  };
  
  typedef HashedSeedSearcher::Frame             Frame;
  typedef HashedSeedSearcher::ResultCallback    ResultCallback;
  typedef HashedSeedSearcher::ProgressCallback  ProgressCallback;
  
  InitialIVSeedSearcher() {}
  
  void Search(const Criteria &criteria,
              const ResultCallback &resultHandler,
              const ProgressCallback &progressHandler);
};

class InitialSeedSearcher
{
public:
  struct Criteria
  {
    uint32_t  tid1;
    uint32_t  tid2;
    uint32_t  tid3;
    uint32_t  tid4;
    
    IVs       minIVs;
    IVs       maxIVs;
  };
  
  InitialSeedSearcher() {}
  
  uint64_t Search(const Criteria &criteria);
};

}

#endif
