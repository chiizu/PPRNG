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
#include "UnhashedSeed.h"
#include <list>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace pprng
{

class Gen34IVSeedGenerator
{
public:
  typedef uint32_t  SeedType;
  typedef uint32_t  SeedCountType;
  
  enum { SeedsPerChunk = 20000 };
  
  Gen34IVSeedGenerator(IVs minIVs, IVs maxIVs, uint32_t method = 1);
  
  SeedCountType NumberOfSeeds() const;
  
  SeedType Next();
  
private:
  const IVs       m_minIVs, m_maxIVs;
  const uint32_t  m_method;
  const bool      m_iteratingHpAtDef;
  const uint32_t  m_numRollbacks;
  uint32_t        m_iv0, m_iv1, m_iv2;
  uint32_t        m_iv0Low, m_iv0High, m_iv1Low, m_iv1High, m_iv2Low, m_iv2High;
  uint32_t        m_otherHalfCounter;
};

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
  
  SeedCountType NumberOfSeeds() const
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
  
  SeedCountType NumberOfSeeds() const
  {
    return m_timeSeedGenerator.NumberOfSeeds();
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
  
  struct Parameters
  {
    Game::Version             version;
    DS::Type                  dsType;
    MACAddress                macAddress;
    uint32_t                  timer0Low, timer0High;
    uint32_t                  vcountLow, vcountHigh;
    uint32_t                  vframeLow, vframeHigh;
    boost::posix_time::ptime  fromTime, toTime;
    Button::List              heldButtons;
    
    Parameters()
      : version(Game::Version(0)), macAddress(), timer0Low(0), timer0High(0),
        vcountLow(0), vcountHigh(0), vframeLow(0), vframeHigh(0),
        fromTime(), toTime(), heldButtons()
    {}
    
    HashedSeed::Parameters ToInitialSeedParameters() const;
    
    SeedCountType NumberOfSeeds() const;
  };
  
  enum { SeedsPerChunk = 50000 };
  
  HashedSeedGenerator(const Parameters &parameters);
  
  HashedSeedGenerator(const HashedSeedGenerator &other);
  
  SeedCountType NumberOfSeeds() const;
  
  SeedType Next();
  
  std::list<HashedSeedGenerator>  Split(uint32_t parts);
  
private:
  const Parameters              m_parameters;
  
  HashedSeed::Parameters        m_hashedSeedParameters;
  Button::List::const_iterator  m_heldButtonsIter;
};


class UnhashedSeedGenerator
{
public:
  typedef HashedSeed  SeedType;
  typedef uint64_t    SeedCountType;
  
  enum { SeedsPerChunk = 50000 };
  
  UnhashedSeedGenerator(const HashedSeedGenerator::Parameters &parameters);
  
  UnhashedSeedGenerator(const UnhashedSeedGenerator &other);
  
  SeedCountType NumberOfSeeds() const;
  
  SeedType Next();
  
  std::list<UnhashedSeedGenerator>  Split(uint32_t parts);
  
private:
  const HashedSeedGenerator::Parameters  m_parameters;
  
  UnhashedSeed                           m_unhashedSeed;
  
  uint32_t                               m_timer0, m_vcount, m_vframe;
  Button::List::const_iterator           m_heldButtonsIter;
};

}

#endif
