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

#ifndef CGEAR_SEED_H
#define CGEAR_SEED_H

#include "PPRNGTypes.h"
#include "TimeSeed.h"

namespace pprng
{

struct CGearSeed : public TimeSeed
{
  using TimeSeed::TimeElement;
  
  CGearSeed(uint32_t rawSeed, uint32_t macAddressLow)
    : TimeSeed(rawSeed - macAddressLow), m_rawSeed(rawSeed),
      m_macAddressLow(macAddressLow)
  {}
  
  CGearSeed(uint32_t year, uint32_t month, uint32_t day,
            uint32_t hour, uint32_t minute, uint32_t second,
            uint32_t delay, uint32_t macAddressLow)
    : TimeSeed(year, month, day, hour, minute, second, delay),
      m_rawSeed(m_seed + macAddressLow),
      m_macAddressLow(macAddressLow)
  {}
  
  CGearSeed(const TimeSeed &timeSeed, uint32_t macAddressLow)
    : TimeSeed(timeSeed), m_rawSeed(m_seed + macAddressLow),
      m_macAddressLow(macAddressLow)
  {}
  
  uint32_t  m_rawSeed;
  uint32_t  m_macAddressLow;
};

}

#endif
