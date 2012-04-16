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

#ifndef TIME_SEED_H
#define TIME_SEED_H

#include "PPRNGTypes.h"
#include <list>

namespace pprng
{

struct TimeSeed
{
  struct TimeElement
  {
    uint32_t  year;
    uint32_t  month;
    uint32_t  day;
    uint32_t  hour;
    uint32_t  minute;
    uint32_t  second;
    uint32_t  delay;
  };
  typedef std::list<TimeElement>  TimeElements;
  
  TimeSeed(uint32_t seed)
    : m_seed(seed)
  {}
  
  TimeSeed(uint32_t year, uint32_t month, uint32_t day,
           uint32_t hour, uint32_t minute, uint32_t second,
           uint32_t delay)
    : m_seed(((((month * day) + minute + second) & 0xff) << 24) |
             (hour << 16) | (((year - 2000) + delay) & 0xffff))
  {}
  
  bool IsValid() const { return Hour() < 24; }
  
  uint32_t BaseDelay() const
  {
    return m_seed & 0x0000ffff;
  }
  
  uint32_t Hour() const
  {
    return (m_seed >> 16) & 0xff;
  }
  
  TimeElements GetTimeElements(uint32_t year, uint32_t second = -1) const;
  
  uint32_t  m_seed;
};


}

#endif
