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

#ifndef WONDER_CARD_SEED_SEARCHER_H
#define WONDER_CARD_SEED_SEARCHER_H

#include "BasicTypes.h"
#include "SeedSearcher.h"
#include "FrameGenerator.h"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace pprng
{

class WonderCardSeedSearcher
{
public:
  struct Criteria : public SeedSearchCriteria
  {
    Game::Version             version;
    uint32_t                  macAddressLow, macAddressHigh;
    uint32_t                  timer0Low, timer0High;
    uint32_t                  vcountLow, vcountHigh;
    uint32_t                  vframeLow, vframeHigh;
    boost::posix_time::ptime  fromTime, toTime;
    Button::List              buttonPresses;
    bool                      startFromLowestFrame;
    uint32_t                  minFrame, maxFrame;
    uint32_t                  ivSkip, pidSkip, natureSkip;
    uint32_t                  maxResults;
    Nature::Type              nature;
    uint32_t                  ability;
    Gender::Type              gender;
    Gender::Ratio             genderRatio;
    bool                      canBeShiny;
    uint32_t                  tid, sid;
    bool                      shouldCheckMaxIVs;
    IVs                       minIVs, maxIVs;
    Element::Type             hiddenType;
    uint32_t                  minHiddenPower;
    
    uint64_t ExpectedNumberOfResults() const;
  };
  
  WonderCardSeedSearcher() {}
  
  typedef SeedSearcher<WonderCardFrameGenerator>  SearcherType;
  typedef SearcherType::Frame                     Frame;
  typedef SearcherType::ResultCallback            ResultCallback;
  typedef SearcherType::ProgressCallback          ProgressCallback;
  
  void Search(const Criteria &criteria, const ResultCallback &resultHandler,
              const ProgressCallback &progressHandler);
};

}

#endif
