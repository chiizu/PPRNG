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

#ifndef GEN_3_4_SEED_SEARCHER_H
#define GEN_3_4_SEED_SEARCHER_H

#include "BasicTypes.h"
#include "SeedSearcher.h"
#include "FrameGenerator.h"

namespace pprng
{

class Gen34SeedSearcher
{
public:
  struct Criteria
  {
    uint32_t       minDelay, maxDelay;
    uint32_t       minFrame, maxFrame;
    Nature::Type   nature;
    bool           shouldCheckMaxIVs;
    IVs            minIVs, maxIVs;
    Element::Type  hiddenType;
    uint32_t       minHiddenPower, maxHiddenPower;
    uint32_t       tid, sid;
    
    uint64_t ExpectedNumberOfResults();
  };
  
  Gen34SeedSearcher() {}
  
  typedef SeedSearcher<Method1FrameGenerator>  SearcherType;
  typedef SearcherType::Frame                  Frame;
  typedef SearcherType::ResultCallback         ResultCallback;
  typedef SearcherType::ProgressCallback       ProgressCallback;
  
  void Search(const Criteria &criteria, const ResultCallback &resultHandler,
              const ProgressCallback &progressHandler);
};

}

#endif
