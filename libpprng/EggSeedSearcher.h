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

#ifndef EGG_SEED_SEARCHER_H
#define EGG_SEED_SEARCHER_H


#include "PPRNGTypes.h"
#include "SearchCriteria.h"
#include "SearchRunner.h"
#include "SeedGenerator.h"
#include "FrameGenerator.h"

#include <boost/function.hpp>

namespace pprng
{

class EggSeedSearcher
{
public:
  struct Criteria : public SearchCriteria
  {
    HashedSeedGenerator::Parameters         seedParameters;
    Gen5BreedingFrameGenerator::Parameters  frameParameters;
    
    SearchCriteria::IVCriteria   ivs;
    SearchCriteria::FrameRange   ivFrame;
    OptionalIVs                  femaleIVs, maleIVs;
    
    SearchCriteria::PIDCriteria  pid;
    SearchCriteria::FrameRange   pidFrame;
    bool                         inheritsHiddenAbility;
    bool                         shinyOnly;
    EggSpecies::Type             eggSpecies;
    
    Criteria()
      : seedParameters(), frameParameters(),
        ivs(), ivFrame(), femaleIVs(), maleIVs(),
        pid(), pidFrame(),
        inheritsHiddenAbility(false), shinyOnly(false),
        eggSpecies(EggSpecies::ANY)
    {}
    
    uint64_t ExpectedNumberOfResults() const;
  };
  
  typedef Gen5EggFrame                               ResultType;
  typedef boost::function<void (const ResultType&)>  ResultCallback;
  
  EggSeedSearcher() {}
  
  void Search(const Criteria &criteria, const ResultCallback &resultHandler,
              const SearchRunner::ProgressCallback &progressHandler);
  
  // if not running from the command line, working directory may not be set
  static void SetCacheDirectory(const std::string &dir);
  
  static bool HasCacheFile();
  
  enum CacheLoadResult
  {
    LOADED = 0,
    NO_CACHE_FILE,
    BAD_CACHE_FILE,
    NOT_ENOUGH_MEMORY,
    UNKNOWN_ERROR
  };
  static CacheLoadResult LoadSeedCache();
  static void ReleaseSeedCache();
  static void EnsureSeedCacheReleased();
};

}

#endif
