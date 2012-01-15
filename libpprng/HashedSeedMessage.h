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

#ifndef HASHED_SEED_MESSAGE_H
#define HASHED_SEED_MESSAGE_H

#include "PPRNGTypes.h"
#include "HashedSeed.h"
#include <boost/date_time/gregorian/gregorian.hpp>

namespace pprng
{

// this class contains the message that will be run through the SHA1 hash
// and provides an interface for iterating over the individual message parts
class HashedSeedMessage
{
public:
  HashedSeedMessage(const HashedSeed::Parameters &parameters);
  
  HashedSeed AsHashedSeed() const;
  
  // calculated raw seed
  uint64_t GetRawSeed() const;
  
  const MACAddress& GetMACAddress() const { return m_parameters.macAddress; }
  void SetMACAddress(const MACAddress &macAddress);
  
  HashedSeed::GxStat GetGxStat() const { return m_parameters.gxStat; }
  void SetGxStat(HashedSeed::GxStat gxStat);
  
  uint32_t GetVCount() const { return m_parameters.vcount; }
  void SetVCount(uint32_t vcount);
  
  uint32_t GetVFrame() const { return m_parameters.vframe; }
  void SetVFrame(uint32_t vframe);
  
  uint32_t GetTimer0() const { return m_parameters.timer0; }
  void SetTimer0(uint32_t timer0);
  
  const boost::gregorian::date& GetDate() const { return m_parameters.date; }
  void SetDate(boost::gregorian::date d);
  void NextDay();
  
  uint32_t GetHour() const { return m_parameters.hour; }
  void SetHour(uint32_t hour);
  void NextHour();
  
  uint32_t GetMinute() const { return m_parameters.minute; }
  void SetMinute(uint32_t minute);
  void NextMinute();
  
  uint32_t GetSecond() const { return m_parameters.second; }
  void SetSecond(uint32_t second);
  void NextSecond();
  
  uint32_t GetHeldButtons() const { return m_parameters.heldButtons; }
  void SetHeldButtons(uint32_t heldButtons);
  
private:
  HashedSeed::Parameters  m_parameters;
  uint32_t                m_message[16];
  uint32_t                m_monthDays;
  
  // actual seed calculated lazily
  mutable bool      m_rawSeedCalculated;
  mutable uint64_t  m_rawSeed;
};

}

#endif
