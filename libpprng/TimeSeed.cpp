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


#include "TimeSeed.h"
#include <boost/date_time/gregorian/gregorian_types.hpp>

using namespace boost::gregorian;

namespace pprng
{

TimeSeed::TimeElements
TimeSeed::GetTimeElements(uint32_t year, uint32_t wantedSecond) const
{
  std::vector<TimeElement>  result;
  
  uint32_t  seedMoDayMinSec = (m_seed >> 24) & 0xff;
  uint32_t  hour = Hour();
  uint32_t  delay = BaseDelay() - (year - 2000);
  uint32_t  startSecond, endSecond;
  
  if ((wantedSecond >= 0) && (wantedSecond < 60))
  {
    startSecond = endSecond = wantedSecond;
  }
  else
  {
    startSecond = 0;
    endSecond = 59;
  }
  
  for (uint32_t month = 1; month <= 12; ++month)
  {
    date  d(year, month, 1);
    
    while (d.month() == month)
    {
      for (uint32_t minute = 0; minute < 60; ++minute)
      {
        for (uint32_t second = startSecond; second <= endSecond; ++second)
        {
          uint32_t  moDayMinSec = ((month * d.day()) + minute + second) & 0xff;
          
          if (seedMoDayMinSec == moDayMinSec)
          {
            TimeElement  element;
            
            element.year = year;
            element.month = month;
            element.day = d.day();
            element.hour = hour;
            element.minute = minute;
            element.second = second;
            element.delay = delay;
            
            result.push_back(element);
          }
        }
      }
      
      d = d + date_duration(1);
    }
  }
  
  return result;
}

}
