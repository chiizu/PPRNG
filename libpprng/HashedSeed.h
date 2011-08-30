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

#ifndef NON_C_GEAR_SEED_H
#define NON_C_GEAR_SEED_H

#include "BasicTypes.h"

namespace pprng
{

class HashedSeed
{
public:
  enum Nazo
  {
    JPBlackNazo = 0x02215f10,
    JPWhiteNazo = 0x02215f30,
    JPDSiWhiteNazo = 0x02737ed0,
    //JPDSiWhiteNazo = 0x02214bf0,
    ENBlackNazo = 0x022160B0,
    ENWhiteNazo = 0x022160D0,
    ENDSiWhiteNazo = 0x02738050,
    SPBlackNazo = 0x02216050,
    SPWhiteNazo = 0x02216070,
    FRBlackNazo = 0x02216030,
    FRWhiteNazo = 0x02216050,
    DEBlackNazo = 0x02215FF0,
    DEWhiteNazo = 0x02216010,
    ITBlackNazo = 0x02215FB0,
    ITWhiteNazo = 0x02215FD0,
    KRBlackNazo = 0x02216790,
    KRWhiteNazo = 0x022167B0
  };
  
  static Nazo NazoForVersion(Game::Version version);
  
  enum
  {
    GxStat = 0x06000000,
    SRGxStat = 0x86000000
  };
  
  HashedSeed(uint32_t year, uint32_t month, uint32_t day, uint32_t dayOfWeek,
             uint32_t hour, uint32_t minute, uint32_t second,
             uint32_t macAddressLow, uint32_t macAddressHigh,
             uint32_t nazo,
             uint32_t vcount, uint32_t timer0,
             uint32_t gxStat, uint32_t vframe,
             uint32_t keyInput,
             uint32_t n21510F8 = 0, uint32_t n21510FC = 0,
             uint32_t n2FFFF90 = 0, uint32_t n2FFFF94 = 0,
             uint32_t n2FFFFAA = 0, uint32_t n2FFFFAC = 0,
             uint32_t n2FFFF98 = 0,
             uint32_t pmFlag = 0x40);
  
  // sometimes you just want to work with the raw seed value
  HashedSeed(uint64_t rawSeed)
    : m_year(0), m_month(0), m_day(0), m_dayOfWeek(0),
      m_hour(0), m_minute(0), m_second(0),
      m_macAddressLow(0), m_macAddressHigh(0), m_nazo(0), m_vcount(0),
      m_timer0(0), m_GxStat(0), m_vframe(0), m_keyInput(0),
      m_n21510F8(0), m_n21510FC(0), m_n2FFFF90(0), m_n2FFFF94(0),
      m_n2FFFFAA(0), m_n2FFFFAC(0), m_n2FFFF98(0), m_pmFlag(0x40),
      m_rawSeed(rawSeed), m_skippedPIDFramesCalculated(false),
      m_skippedPIDFrames(0)
  {}
  
  // this is needed to decode a HashedSeed from a byte array
  HashedSeed()
    : m_year(0), m_month(0), m_day(0), m_dayOfWeek(0),
      m_hour(0), m_minute(0), m_second(0),
      m_macAddressLow(0), m_macAddressHigh(0), m_nazo(0), m_vcount(0),
      m_timer0(0), m_GxStat(0), m_vframe(0), m_keyInput(0),
      m_n21510F8(0), m_n21510FC(0), m_n2FFFF90(0), m_n2FFFF94(0),
      m_n2FFFFAA(0), m_n2FFFFAC(0), m_n2FFFF98(0), m_pmFlag(0x40),
      m_rawSeed(0), m_skippedPIDFramesCalculated(false),
      m_skippedPIDFrames(0)
  {}
  
  const uint32_t m_year;
  const uint32_t m_month;
  const uint32_t m_day;
  const uint32_t m_dayOfWeek;
  const uint32_t m_hour;
  const uint32_t m_minute;
  const uint32_t m_second;
  const uint32_t m_macAddressLow;
  const uint32_t m_macAddressHigh;
  const uint32_t m_nazo;
  const uint32_t m_vcount;
  const uint32_t m_timer0;
  const uint32_t m_GxStat;
  const uint32_t m_vframe;
  const uint32_t m_keyInput;
  const uint32_t m_n21510F8;
  const uint32_t m_n21510FC;
  const uint32_t m_n2FFFF90;
  const uint32_t m_n2FFFF94;
  const uint32_t m_n2FFFFAA;
  const uint32_t m_n2FFFFAC;
  const uint32_t m_n2FFFF98;
  const uint32_t m_pmFlag;
  
  const uint64_t m_rawSeed;
  
  uint32_t GetSkippedPIDFrames() const;
  
private:
  // skipped frames calculated lazily and cached
  mutable bool      m_skippedPIDFramesCalculated;
  mutable uint32_t  m_skippedPIDFrames;
};

}

#endif
