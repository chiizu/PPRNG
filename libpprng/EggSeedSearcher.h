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

#ifndef EGG_SEED_SEARCHER_H
#define EGG_SEED_SEARCHER_H


#include "BasicTypes.h"
#include "SeedGenerator.h"
#include "SeedSearcher.h"
#include "FrameGenerator.h"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace pprng
{

class EggSeedSearcher
{
public:
  typedef SeedSearcher<HashedIVFrameGenerator>    SeedSearcherType;
  typedef Gen5EggFrame                            Frame;
  typedef boost::function<void (const Frame&)>    ResultCallback;
  typedef boost::function<bool (double percent)>  ProgressCallback;
  
  struct Criteria : public SeedSearchCriteria
  {
    HashedSeedGenerator::Parameters         seedParameters;
    Gen5BreedingFrameGenerator::Parameters  frameParameters;
    
    SeedSearchCriteria::IVCriteria   ivs;
    SeedSearcherType::FrameRange     ivFrame;
    IVs                              femaleIVs, maleIVs;
    
    FemaleParent::Type               femaleSpecies;
    
    SeedSearchCriteria::PIDCriteria  pid;
    SeedSearcherType::FrameRange     pidFrame;
    bool                             inheritsHiddenAbility;
    bool                             shinyOnly;
    uint32_t                         childSpecies;
    
    Criteria()
      : seedParameters(), frameParameters(),
        ivs(), ivFrame(), femaleIVs(), maleIVs(),
        femaleSpecies(FemaleParent::OTHER),
        pid(), pidFrame(),
        inheritsHiddenAbility(false), shinyOnly(false),
        childSpecies(0xFFFFFFFF)
    {}
    
    uint64_t ExpectedNumberOfResults() const;
  };
  
  EggSeedSearcher() {}
  
  void Search(const Criteria &criteria, const ResultCallback &resultHandler,
              const ProgressCallback &progressHandler);
};

}

#endif
