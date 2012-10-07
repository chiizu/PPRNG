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

#ifndef GEN_4_TID_SEARCHER_H
#define GEN_4_TID_SEARCHER_H

#include "PPRNGTypes.h"
#include "SearchCriteria.h"
#include "SearchRunner.h"
#include "SeedGenerator.h"
#include "FrameGenerator.h"

#include <boost/function.hpp>

namespace pprng
{

class Gen4TIDSearcher
{
public:
  struct Criteria : public SearchCriteria
  {
    uint32_t  minDelay, maxDelay;
    
    bool      considerTID, considerSID;
    uint32_t  tid, sid;
    
    bool      considerPID;
    PID       pid;
    
    uint64_t ExpectedNumberOfResults() const;
  };
  
  typedef Gen4TrainerIDFrame                         ResultType;
  typedef boost::function<void (const ResultType&)>  ResultCallback;
  
  Gen4TIDSearcher() {}
  
  void Search(const Criteria &criteria, const ResultCallback &resultHandler,
              const SearchRunner::ProgressCallback &progressHandler);
};

}

#endif
