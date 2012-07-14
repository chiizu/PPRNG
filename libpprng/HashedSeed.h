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

#ifndef NON_C_GEAR_SEED_H
#define NON_C_GEAR_SEED_H

#include "PPRNGTypes.h"
#include "LinearCongruentialRNG.h"

#include <boost/date_time/gregorian/gregorian.hpp>

namespace pprng
{

class HashedSeed
{
public:
  enum GxStat
  {
    HardResetGxStat = 0x06000000
  };
  
  struct Parameters
  {
    Game::Version           version;
    DS::Type                dsType;
    MACAddress              macAddress;
    GxStat                  gxStat;
    uint32_t                vcount, vframe, timer0;
    boost::gregorian::date  date;
    uint32_t                hour, minute, second;
    uint32_t                heldButtons;
    
    Parameters()
      : version(Game::Version(0)), dsType(DS::Type(0)), macAddress(),
        gxStat(HardResetGxStat), vcount(0), vframe(0), timer0(0),
        date(), hour(0), minute(0), second(0), heldButtons(0)
    {}
  };
  
  HashedSeed(const Parameters &parameters);
  
  // to be used when raw seed is already calculated (see UnhashedSeed)
  HashedSeed(const Parameters &parameters, uint64_t rawSeed);
  
  // sometimes you just want to work with the raw seed value,
  //  but still need the game version because of differences between bw and b2w2
  HashedSeed(uint64_t rawSeed, Game::Version v)
    : version(v), dsType(DS::Type(0)), macAddress(),
      gxStat(HardResetGxStat), vcount(0), vframe(0), timer0(0),
      date(), hour(0), minute(0), second(0), heldButtons(0),
      rawSeed(rawSeed),
      m_skippedPIDFramesCalculated(false), m_skippedPIDFrames(0),
      m_skippedPIDFramesSeed(0)
  {}
  
  // this is needed to decode a HashedSeed from a byte array
  HashedSeed()
    : version(Game::Version(0)), dsType(DS::Type(0)), macAddress(),
      gxStat(HardResetGxStat), vcount(0), vframe(0), timer0(0),
      date(), hour(0), minute(0), second(0), heldButtons(0),
      rawSeed(rawSeed),
      m_skippedPIDFramesCalculated(false), m_skippedPIDFrames(0),
      m_skippedPIDFramesSeed(0)
  {}
  
  const Game::Version           version;
  const DS::Type                dsType;
  const MACAddress              macAddress;
  const GxStat                  gxStat;
  const uint32_t                vcount, vframe, timer0;
  const boost::gregorian::date  date;
  const uint32_t                hour, minute, second;
  const uint32_t                heldButtons;
  
  uint32_t year() const { return date.year(); }
  uint32_t month() const { return date.month(); }
  uint32_t day() const { return date.day(); }
  
  // calculated raw seed
  const uint64_t    rawSeed;
  
  uint32_t SeedAndSkipPIDFrames(LCRNG5 &rng) const;
  
  uint32_t GetSkippedPIDFrames() const;
  
private:
  // skipped frames calculated lazily and cached
  mutable bool      m_skippedPIDFramesCalculated;
  mutable uint32_t  m_skippedPIDFrames;
  mutable uint64_t  m_skippedPIDFramesSeed;
};

}

#endif
