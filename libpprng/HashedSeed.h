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
#include <boost/date_time/gregorian/gregorian.hpp>

namespace pprng
{

class HashedSeed
{
public:
  enum Nazo
  {
    JPBlackNazo = 0x02215f10,
    JPWhiteNazo = 0x02215f30,
    JPBlackDSiNazo = 0x02761150,
    JPWhiteDSiNazo = 0x02761150,
    
    ENBlackNazo = 0x022160B0,
    ENWhiteNazo = 0x022160D0,
    ENBlackDSiNazo = 0x02760190,
    ENWhiteDSiNazo = 0x027601B0,
    
    SPBlackNazo = 0x02216050,
    SPWhiteNazo = 0x02216070,
    SPBlackDSiNazo = 0x027601f0,
    SPWhiteDSiNazo = 0x027601f0,
    
    FRBlackNazo = 0x02216030,
    FRWhiteNazo = 0x02216050,
    FRBlackDSiNazo = 0x02760230,
    FRWhiteDSiNazo = 0x02760250,
    
    DEBlackNazo = 0x02215FF0,
    DEWhiteNazo = 0x02216010,
    DEBlackDSiNazo = 0x027602f0,
    DEWhiteDSiNazo = 0x027602f0,
    
    ITBlackNazo = 0x02215FB0,
    ITWhiteNazo = 0x02215FD0,
    ITBlackDSiNazo = 0x027601d0,
    ITWhiteDSiNazo = 0x027601d0,
    
    KRBlackNazo = 0x022167B0,
    KRWhiteNazo = 0x022167B0,
    KRBlackDSiNazo = 0x02761150,
    KRWhiteDSiNazo = 0x02761150,
    
    JPBlack2Nazo = 0x0209A8DC,
    JPBlack2Nazo2 = 0x02039AC9,
    JPWhite2Nazo = 0x0209A8FC,
    JPWhite2Nazo2 = 0x02039AF5
  };
  
  static Nazo NazoForVersionAndDS(Game::Version version, DS::Type dsType);
  static Nazo Nazo2ForVersionAndDS(Game::Version version, DS::Type dsType);
  
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
      m_skippedPIDFramesCalculated(false), m_skippedPIDFrames(0)
  {}
  
  // this is needed to decode a HashedSeed from a byte array
  HashedSeed()
    : version(Game::Version(0)), dsType(DS::Type(0)), macAddress(),
      gxStat(HardResetGxStat), vcount(0), vframe(0), timer0(0),
      date(), hour(0), minute(0), second(0), heldButtons(0),
      rawSeed(rawSeed),
      m_skippedPIDFramesCalculated(false), m_skippedPIDFrames(0)
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
  
  uint32_t GetSkippedPIDFrames() const;
  
private:
  // skipped frames calculated lazily and cached
  mutable bool      m_skippedPIDFramesCalculated;
  mutable uint32_t  m_skippedPIDFrames;
};

}

#endif
