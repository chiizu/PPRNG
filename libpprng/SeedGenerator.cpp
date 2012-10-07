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
#include "LinearCongruentialRNG.h"
#include <iostream>

using namespace boost::posix_time;
using namespace boost::gregorian;

namespace pprng
{

namespace
{

static bool ShouldIterateHpAtkDef(IVs minIVs, IVs maxIVs)
{
  IVs tempMin(minIVs.word & 0x7fff0000);
  IVs tempMax(maxIVs.word & 0x7fff0000);
  uint32_t  numCombosHi = IVs::CalculateNumberOfCombinations(tempMin, tempMax);
  
  tempMin = minIVs.word & 0x00007fff;
  tempMax = maxIVs.word & 0x00007fff;
  
  uint32_t  numCombosLo = IVs::CalculateNumberOfCombinations(tempMin, tempMax);
  
  return numCombosLo < numCombosHi;
}

}

Gen34IVSeedGenerator::Gen34IVSeedGenerator(IVs minIVs, IVs maxIVs,
                                           uint32_t method)
  : m_minIVs(minIVs), m_maxIVs(maxIVs), m_method(method),
    m_iteratingHpAtDef(ShouldIterateHpAtkDef(minIVs, maxIVs)),
    m_numRollbacks(m_iteratingHpAtDef ?
                    (((m_method == 2) || (m_method == 3)) ? 1 : 0) :
                    ((m_method == 4) ? 2 : 1)),
    m_iv0(0), m_iv1(0), m_iv2(0),
    m_iv0Low(0), m_iv0High(0), m_iv1Low(0), m_iv1High(0),
    m_iv2Low(0), m_iv2High(0),
    m_otherHalfCounter(0x1ffff)
{
  if (m_iteratingHpAtDef)
  {
    m_iv0Low = minIVs.hp(); m_iv0High = maxIVs.hp();
    m_iv1Low = minIVs.at(); m_iv1High = maxIVs.at();
    m_iv2Low = minIVs.df(); m_iv2High = maxIVs.df();
  }
  else
  {
    m_iv0Low = minIVs.sp(); m_iv0High = maxIVs.sp();
    m_iv1Low = minIVs.sa(); m_iv1High = maxIVs.sa();
    m_iv2Low = minIVs.sd(); m_iv2High = maxIVs.sd();
  }
  
  m_iv0 = m_iv0High;
  m_iv1 = m_iv1High;
  m_iv2 = m_iv2High;
}

Gen34IVSeedGenerator::SeedCountType Gen34IVSeedGenerator::NumberOfSeeds() const
{
  uint32_t  mask = m_iteratingHpAtDef ? 0x00007fff : 0x7fff0000;
  
  return IVs::CalculateNumberOfCombinations(IVs(m_minIVs.word & mask),
                                            IVs(m_maxIVs.word & mask)) *
         (0x1ffff + 1);
}

Gen34IVSeedGenerator::SeedType Gen34IVSeedGenerator::Next()
{
  if (++m_otherHalfCounter > 0x1ffff)
  {
    m_otherHalfCounter = 0;
    if (++m_iv0 > m_iv0High)
    {
      m_iv0 = m_iv0Low;
      if (++m_iv1 > m_iv1High)
      {
        m_iv1 = m_iv1Low;
        if (++m_iv2 > m_iv2High)
        {
          m_iv2 = m_iv2Low;
        }
      }
    }
  }
  
  uint32_t  seed = ((m_otherHalfCounter & 0x10000) << 15) | // unused bit
    (m_iv2 << 26) | (m_iv1 << 21) | (m_iv0 << 16) | // ivs from upper 16 bits
    (m_otherHalfCounter & 0xffff); // lower 16 bits unknown
  
  // back up a number of frames to account for which IVs are being generated
  // and the generation method
  uint32_t  i = m_numRollbacks;
  while (i > 0)
  {
    seed = LCRNG34_R::NextForSeed(seed);
    --i;
  }
  
  // back up 2 frames for PID generation
  seed = LCRNG34_R::NextForSeed(seed);
  seed = LCRNG34_R::NextForSeed(seed);
  
  // backup 1 frame to the 'seed'
  seed = LCRNG34_R::NextForSeed(seed);
  
  return seed;
}


HashedSeedGenerator::HashedSeedGenerator
  (const HashedSeedGenerator::Parameters &parameters)
: m_parameters(parameters),
  m_seedMessage(parameters.ToInitialSeedParameters()),
  m_timer0(parameters.timer0High), m_vcount(parameters.vcountHigh),
  m_vframe(parameters.vframeHigh),
  m_heldButtonsIter(m_parameters.heldButtons.end() - 1)
{}

HashedSeedGenerator::HashedSeedGenerator(const HashedSeedGenerator &other)
: m_parameters(other.m_parameters), m_seedMessage(other.m_seedMessage),
  m_timer0(other.m_timer0), m_vcount(other.m_vcount), m_vframe(other.m_vframe),
  m_heldButtonsIter(m_parameters.heldButtons.begin() +
                    (other.m_heldButtonsIter -
                     other.m_parameters.heldButtons.begin()))
{}

HashedSeed::Parameters
  HashedSeedGenerator::Parameters::ToInitialSeedParameters() const
{
  HashedSeed::Parameters  parameters;
  
  parameters.version = version;
  parameters.dsType = dsType;
  parameters.macAddress = macAddress;
  parameters.gxStat = HashedSeed::HardResetGxStat;
  parameters.timer0 = timer0High;
  parameters.vcount = vcountHigh;
  parameters.vframe = vframeHigh;
  
  ptime          dt = fromTime - seconds(1);
  time_duration  t = dt.time_of_day();
  
  parameters.date = dt.date();
  parameters.hour = t.hours();
  parameters.minute = t.minutes();
  parameters.second = t.seconds();
  
  return parameters;
}

HashedSeedGenerator::SeedCountType
  HashedSeedGenerator::Parameters::NumberOfSeeds() const
{
  SeedCountType  seconds = (toTime - fromTime).total_seconds() + 1;
  SeedCountType  keyCombos = heldButtons.size();
  SeedCountType  timer0Values = (timer0High - timer0Low) + 1;
  SeedCountType  vcountValues = (vcountHigh - vcountLow) + 1;
  SeedCountType  vframeValues = (vframeHigh - vframeLow) + 1;
  
  return seconds * keyCombos * timer0Values * vcountValues * vframeValues;
}

HashedSeedGenerator::SeedCountType HashedSeedGenerator::NumberOfSeeds() const
{
  return m_parameters.NumberOfSeeds();
}

HashedSeedGenerator::SeedType HashedSeedGenerator::Next()
{
  if (++m_heldButtonsIter == m_parameters.heldButtons.end())
  {
    m_heldButtonsIter = m_parameters.heldButtons.begin();
    
    if (++m_timer0 > m_parameters.timer0High)
    {
      m_timer0 = m_parameters.timer0Low;
      
      if (++m_vcount > m_parameters.vcountHigh)
      {
        m_vcount = m_parameters.vcountLow;
        
        if (++m_vframe > m_parameters.vframeHigh)
        {
          m_vframe = m_parameters.vframeLow;
          
          m_seedMessage.NextSecond();
        }
        
        m_seedMessage.SetVFrame(m_vframe);
      }
      
      m_seedMessage.SetVCount(m_vcount);
    }
    
    m_seedMessage.SetTimer0(m_timer0);
  }
  
  m_seedMessage.SetHeldButtons(*m_heldButtonsIter);
  
  HashedSeed  seed = m_seedMessage.AsHashedSeed();
  
  return seed;
}


std::list<HashedSeedGenerator> HashedSeedGenerator::Split(uint32_t parts)
{
  std::list<HashedSeedGenerator>  result;
  
  HashedSeedGenerator::Parameters  p = m_parameters;
  
  uint32_t  totalSeconds =
    (m_parameters.toTime - m_parameters.fromTime).total_seconds() + 1;
  
  if (parts > totalSeconds)
    parts = totalSeconds;
  
  uint32_t  partSeconds = (totalSeconds + parts - 1) / parts;
  seconds   delta(partSeconds);
  ptime     fromTime = m_parameters.fromTime;
  ptime     toTime = fromTime + seconds(partSeconds - 1);
  
  for (uint32_t i = 0; i < parts; ++i)
  {
    p.fromTime = fromTime;
    p.toTime = toTime;
    
    HashedSeedGenerator  part(p);
    
    result.push_back(part);
    
    fromTime = fromTime + delta;
    toTime = (i == (parts - 1)) ? m_parameters.toTime : toTime + delta;
  }
  
  return result;
}

}
