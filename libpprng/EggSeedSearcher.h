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
#include "SeedSearcher.h"
#include "FrameGenerator.h"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace pprng
{

class EggSeedSearcher
{
public:
  struct Criteria
  {
    Game::Version             version;
    uint32_t                  macAddressLow, macAddressHigh;
    uint32_t                  timer0Low, timer0High;
    uint32_t                  vcountLow, vcountHigh;
    uint32_t                  vframeLow, vframeHigh;
    boost::posix_time::ptime  fromTime, toTime;
    Button::List              buttonPresses;
    
    uint32_t                  tid, sid;
    
    IVs                       femaleIVs, maleIVs;
    bool                      usingEverstone;
    bool                      usingDitto;
    bool                      internationalParents;
    
    uint32_t                  minPIDFrame, maxPIDFrame;
    Nature::Type              nature;
    uint32_t                  ability;
    bool                      inheritsDreamworldAbility;
    bool                      shinyOnly;
    
    uint32_t                  minIVFrame, maxIVFrame;
    bool                      shouldCheckMaxIVs;
    IVs                       minIVs, maxIVs;
    Element::Type             hiddenType;
    uint32_t                  minHiddenPower;
    
    uint64_t ExpectedNumberOfResults();
  };
  
  EggSeedSearcher() {}
  
  typedef Gen5EggFrame                            Frame;
  typedef boost::function<void (const Frame&)>    ResultCallback;
  typedef boost::function<bool (double percent)>  ProgressCallback;
  
  void Search(const Criteria &criteria, const ResultCallback &resultHandler,
              const ProgressCallback &progressHandler);
};

}

#endif
