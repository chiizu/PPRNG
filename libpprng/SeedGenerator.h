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

#ifndef SEED_GENERATOR_H
#define SEED_GENERATOR_H

#include "BasicTypes.h"
#include "HashedSeed.h"
#include <list>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace pprng
{

class TimeSeedGenerator
{
public:
  typedef uint32_t  SeedType;
  typedef uint32_t  SeedCountType;
  
  enum { SeedsPerChunk = 20000 };
  
  TimeSeedGenerator(uint32_t minDelay, uint32_t maxDelay)
    : m_minDelay(minDelay), m_maxDelay(maxDelay & 0xffff),
      m_dayMonthMinuteSecond(0xff000000), m_hour(0x00170000),
      m_delay(maxDelay)
  {}
  
  SeedCountType NumSeeds() const
  {
    return 256 * 24 * (m_maxDelay - m_minDelay + 1);
  }
  
  SeedType Next()
  {
    if (++m_delay > m_maxDelay)
    {
      m_delay = m_minDelay;
      m_hour += 0x00010000;
      
      if (m_hour > 0x00170000)
      {
        m_hour = 0x00000000;
        m_dayMonthMinuteSecond += 0x01000000;
      }
    }
    
    return m_dayMonthMinuteSecond | m_hour | m_delay;
  }
  
  std::list<TimeSeedGenerator>  Split(uint32_t parts)
  {
    return std::list<TimeSeedGenerator>();
  }
  
private:
  const uint32_t  m_minDelay;
  const uint32_t  m_maxDelay;
  
  uint32_t  m_dayMonthMinuteSecond;
  uint32_t  m_hour;
  uint32_t  m_delay;
};


class CGearSeedGenerator
{
public:
  typedef TimeSeedGenerator::SeedType       SeedType;
  typedef TimeSeedGenerator::SeedCountType  SeedCountType;
  
  enum { SeedsPerChunk = 10000 };
  
  CGearSeedGenerator(uint32_t minDelay, uint32_t maxDelay,
                     uint32_t macAddressLow)
    : m_macAddressLow(macAddressLow), m_timeSeedGenerator(minDelay, maxDelay)
  {}
  
  SeedCountType NumSeeds() const
  {
    return m_timeSeedGenerator.NumSeeds();
  }
  
  SeedType Next()
  {
    SeedType  result = m_timeSeedGenerator.Next();
    
    return result + m_macAddressLow;
  }
  
  std::list<CGearSeedGenerator>  Split(uint32_t parts)
  {
    return std::list<CGearSeedGenerator>();
  }
  
private:
  const uint32_t  m_macAddressLow;
  
  TimeSeedGenerator  m_timeSeedGenerator;
};


class HashedSeedGenerator
{
public:
  typedef HashedSeed  SeedType;
  typedef uint64_t    SeedCountType;
  
  enum { SeedsPerChunk = 50000 };
  
  HashedSeedGenerator(Game::Version version,
                      uint32_t macAddressLow, uint32_t macAddressHigh,
                      uint32_t timer0Low, uint32_t timer0High,
                      uint32_t vcountLow, uint32_t vcountHigh,
                      uint32_t vframeLow, uint32_t vframeHigh,
                      boost::posix_time::ptime fromTime,
                      boost::posix_time::ptime toTime,
                      const std::vector<uint32_t> &keyCombos);
  
  HashedSeedGenerator(const HashedSeedGenerator &other);
  
  SeedCountType NumSeeds() const;
  
  SeedType Next();
  
  std::list<HashedSeedGenerator>  Split(uint32_t parts);
  
private:
  const uint32_t                  m_nazo;
  const uint32_t                  m_macAddressLow, m_macAddressHigh;
  const uint32_t                  m_timer0Low, m_timer0High;
  const uint32_t                  m_vcountLow, m_vcountHigh;
  const uint32_t                  m_vframeLow, m_vframeHigh;
  const std::vector<uint32_t>     m_keyCombos;
  boost::posix_time::ptime        m_fromTime, m_toTime;
  
  boost::posix_time::ptime               m_time;
  uint32_t                               m_timer0;
  uint32_t                               m_vcount;
  uint32_t                               m_vframe;
  std::vector<uint32_t>::const_iterator  m_keyComboIter;
};

}

#endif
