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


#include "SeedGenerator.h"
#include "HashedSeed.h"
#include <iostream>

using namespace boost::posix_time;
using namespace boost::gregorian;

namespace pprng
{

HashedSeedGenerator::HashedSeedGenerator
  (Game::Version version,
   uint32_t macAddressLow, uint32_t macAddressHigh,
   uint32_t timer0Low, uint32_t timer0High,
   uint32_t vcountLow, uint32_t vcountHigh,
   uint32_t vframeLow, uint32_t vframeHigh,
   ptime fromTime, ptime toTime,
   const std::vector<uint32_t> &keyCombos)
: m_nazo(HashedSeed::NazoForVersion(version)),
  m_macAddressLow(macAddressLow), m_macAddressHigh(macAddressHigh),
  m_timer0Low(timer0Low), m_timer0High(timer0High),
  m_vcountLow(vcountLow), m_vcountHigh(vcountHigh),
  m_vframeLow(vframeLow), m_vframeHigh(vframeHigh),
  m_fromTime(fromTime), m_toTime(toTime),
  m_keyCombos(keyCombos),
  m_time(fromTime - seconds(1)),
  m_timer0(timer0High), m_vcount(vcountHigh), m_vframe(vframeHigh),
  m_keyComboIter(m_keyCombos.end() - 1)
{}

HashedSeedGenerator::SeedCountType HashedSeedGenerator::NumSeeds() const
{
  SeedCountType  seconds = (m_toTime - m_fromTime).total_seconds() + 1;
  SeedCountType  keyCombos = m_keyCombos.size();
  SeedCountType  timer0Values = (m_timer0High - m_timer0Low) + 1;
  SeedCountType  vcountValues = (m_vcountHigh - m_vcountLow) + 1;
  SeedCountType  vframeValues = (m_vframeHigh - m_vframeLow) + 1;
  
  return seconds * keyCombos * timer0Values * vcountValues * vframeValues;
}

HashedSeedGenerator::SeedType HashedSeedGenerator::Next()
{
  if (++m_keyComboIter == m_keyCombos.end())
  {
    m_keyComboIter = m_keyCombos.begin();
    
    if (++m_timer0 > m_timer0High)
    {
      m_timer0 = m_timer0Low;
      
      if (++m_vcount > m_vcountHigh)
      {
        m_vcount = m_vcountLow;
        
        if (++m_vframe > m_vframeHigh)
        {
          m_vframe = m_vframeLow;
          
          m_time = m_time + seconds(1);
        }
      }
    }
  }
  
  date           d = m_time.date();
  time_duration  t = m_time.time_of_day();
  
  HashedSeed  seed(d.year(), d.month(), d.day(), d.day_of_week(),
                   t.hours(), t.minutes(), t.seconds(),
                   m_macAddressLow, m_macAddressHigh, m_nazo, 0, 0, 0,
                   m_vcount, m_timer0, HashedSeed::GxStat,
                   m_vframe, *m_keyComboIter);
  
  return seed;
}

}
