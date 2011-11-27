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

#ifndef TRAINER_ID_SEARCHER_H
#define TRAINER_ID_SEARCHER_H

#include "BasicTypes.h"
#include "SeedGenerator.h"
#include "SeedSearcher.h"
#include "FrameGenerator.h"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace pprng
{

class TrainerIDSearcher
{
public:
  typedef SeedSearcher<Gen5TrainerIDFrameGenerator>  SeedSearcherType;
  typedef SeedSearcherType::Frame                    Frame;
  typedef SeedSearcherType::ResultCallback           ResultCallback;
  typedef SeedSearcherType::ProgressCallback         ProgressCallback;
  
  struct Criteria : public SeedSearchCriteria
  {
    HashedSeedGenerator::Parameters       seedParameters;
    SeedSearcherType::FrameRange          frame;
    
    bool                      hasTID;
    uint32_t                  tid;
    bool                      hasShinyPID;
    PID                       shinyPID;
    bool                      wildShiny;
    bool                      giftShiny;
    bool                      eggShiny;
    
    uint64_t ExpectedNumberOfResults() const;
  };
  
  TrainerIDSearcher() {}
  
  void Search(const Criteria &criteria, const ResultCallback &resultHandler,
              const ProgressCallback &progressHandler);
};

}

#endif
