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

#ifndef PID_RNG_H
#define PID_RNG_H

#include "PPRNGTypes.h"

namespace pprng
{

template <int Method, class RNG>
class Gen34PIDRNG
{
public:
  Gen34PIDRNG(RNG &rng)
    : m_RNG(rng)
  {}
  
  uint32_t NextPIDWord()
  {
    return (m_RNG.Next() >> 16) | (m_RNG.Next() & 0xffff0000);
  }
  
private:
  RNG  &m_RNG;
};

// specialize for method 3 due to split between the two PID calls
template <class RNG>
class Gen34PIDRNG<3, RNG>
{
public:
  Gen34PIDRNG(RNG &rng)
    : m_RNG(rng)
  {}
  
  uint32_t NextPIDWord()
  {
    uint32_t  temp = m_RNG.Next() >> 16;
    
    // skip one call
    m_RNG.Next();
    
    return temp | (m_RNG.Next() & 0xffff0000);
  }
  
private:
  RNG  &m_RNG;
};


struct Gen5PIDRNG
{
  template <class RNG>
  static uint32_t NextWildPIDWord(RNG &rng, uint32_t tid, uint32_t sid)
  {
    return TIDBitTwiddle(FlipAbility(NextRawPIDWord(rng)), tid, sid);
  }
  
  template <class RNG>
  static uint32_t NextGiftPIDWord(RNG &rng)
  {
    return FlipAbility(NextRawPIDWord(rng));
  }
  
  template <class RNG>
  static uint32_t NextNonShinyPIDWord(RNG &rng, uint32_t tid, uint32_t sid)
  {
    return ForceNonShiny(FlipAbility(NextRawPIDWord(rng)), tid, sid);
  }
  
  template <class RNG>
  static uint32_t NextRoamerPIDWord(RNG &rng)
  {
    return NextRawPIDWord(rng);
  }
  
  template <class RNG>
  static uint32_t NextEggPIDWord(RNG &rng)
  {
    return (NextRawPIDWord(rng) * 0xFFFFFFFFULL) >> 32;
  }
  
  template <class RNG>
  static uint32_t NextEntraLinkPIDWord(RNG &rng,
                                       Gender::Type gender, Gender::Ratio ratio,
                                       uint32_t tid, uint32_t sid)
  {
    return ForceNonShiny(ClearAbility(ForceGender(NextRawPIDWord(rng),
                                                  gender, ratio, rng)),
                         tid, sid);
  }
  
  template <class RNG>
  static uint32_t NextDreamRadarPIDWord(RNG &rng,
                                       Gender::Type gender, Gender::Ratio ratio,
                                       uint32_t tid, uint32_t sid)
  {
    return ForceNonShiny(FlipAbility(ForceGender(NextRawPIDWord(rng),
                                                 gender, ratio, rng)),
                         tid, sid);
  }
  
  template <class RNG>
  static uint32_t NextCuteCharmPIDWord(RNG &rng,
                                       Gender::Type gender, Gender::Ratio ratio,
                                       uint32_t tid, uint32_t sid)
  {
    return TIDBitTwiddle(FlipAbility(ForceGender(NextRawPIDWord(rng),
                                                 gender, ratio, rng)),
                         tid, sid);
  }
  
  template <class RNG>
  static uint32_t NextRawPIDWord(RNG &rng)
  {
    return rng.Next() >> 32;
  }
  
  static uint32_t FlipAbility(uint32_t pid)
  {
    return pid ^ 0x00010000;
  }
  
  static uint32_t ClearAbility(uint32_t pid)
  {
    return pid & ~0x00010000U;
  }
  
  static uint32_t TIDBitTwiddle(uint32_t pid, uint32_t tid, uint32_t sid)
  {
    if ((tid ^ sid ^ pid) & 0x1)
    {
      return pid | 0x80000000;
    }
    else
    {
      return pid & 0x7fffffff;
    }
  }
  
  static uint32_t ForceAbility(uint32_t pid, Ability::Type ability)
  {
    switch (ability)
    {
    case Ability::NONE:
    case Ability::ANY:
    default:
      return pid;
      break;
      
    case Ability::ZERO:
      return pid & ~0x00010000U;
      break;
      
    case Ability::ONE:
      return pid | 0x00010000;
      break;
      
    case Ability::HIDDEN:
      return pid & ~0x00010000U;
      break;
    }
  }
  
  static uint32_t ForceShiny(uint32_t pid, uint32_t tid, uint32_t sid)
  {
    uint32_t  lowByte = pid & 0x000000ff;
    
    return ((lowByte ^ tid ^ sid) << 16) | lowByte;
  }
  
  static uint32_t ForceNonShiny(uint32_t pid, uint32_t tid, uint32_t sid)
  {
    if (((pid >> 16) ^ (pid & 0xffff) ^ sid ^ tid) < 8)
    {
      pid = pid ^ 0x10000000;
    }
    
    return pid;
  }
  
  template <class RNG>
  static uint32_t ForceGender(uint32_t pid,
                              Gender::Type gender, Gender::Ratio ratio,
                              RNG &rng)
  {
    if ((gender == Gender::GENDERLESS) || (gender == Gender::ANY))
      return pid;
    
    return (pid & 0xFFFFFF00) |
           Gender::MakeGenderValue(gender, ratio, (rng.Next() >> 32));
  }
};

}

#endif
