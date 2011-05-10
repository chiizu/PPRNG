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

#ifndef PID_RNG_H
#define PID_RNG_H

#include "BasicTypes.h"

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

template <class RNG>
class Gen5PIDRNG
{
public:
  enum Type
  {
    WildPID = 0,
    ZekReshVicPID,
    GiftPID,
    GiftNoShinyPID,
    RoamerPID,
    EntraLinkPID,
    EggPID
  };
  
  Gen5PIDRNG(RNG &rng, Type type, uint32_t tid, uint32_t sid)
    : m_RNG(rng),
      m_PIDWordGenerator(GeneratorForType(type)),
      m_idLowBitsDiffer((tid ^ sid) & 0x1),
      m_fullId((sid << 16) | tid)
  {}
  
  uint32_t NextPIDWord()
  {
    return (this->*m_PIDWordGenerator)();
  }
  
private:
  typedef uint32_t (Gen5PIDRNG::*PIDWordGenerator)();
  
  const PIDWordGenerator  m_PIDWordGenerator;
  
  uint32_t NextWildPIDWord()
  {
    uint32_t  result = (m_RNG.Next() >> 32) ^ 0x00010000;
    
    if ((m_idLowBitsDiffer ^ result) & 0x1)
    {
      result = result | 0x80000000;
    }
    else
    {
      result = result & 0x7fffffff;
    }
    
    return result;
  }
  
  uint32_t NextNonShinyPIDWord()
  {
    uint32_t  result = (m_RNG.Next() >> 32) ^ 0x00010000;
    uint32_t  temp = result ^ m_fullId;
    
    // force non-shiny
    if (((temp >> 16) ^ (temp & 0xffff)) < 8)
    {
      result = result ^ 0x10000000;
    }
    
    return result;
  }
  
  uint32_t NextGiftPIDWord()
  {
    return (m_RNG.Next() >> 32) ^ 0x00010000;
  }
  
  uint32_t NextRoamerPIDWord()
  {
    return m_RNG.Next() >> 32;
  }
  
  uint32_t NextEggPIDWord()
  {
    return ((m_RNG.Next() >> 32) * 0xFFFFFFFFULL) >> 32;
  }
  
  RNG             &m_RNG;
  const uint32_t  m_idLowBitsDiffer;
  const uint32_t  m_fullId;
  
  static PIDWordGenerator GeneratorForType(Type type)
  {
    switch (type)
    {
    case WildPID:
      return &Gen5PIDRNG::NextWildPIDWord;
      break;
    case ZekReshVicPID:
      return &Gen5PIDRNG::NextNonShinyPIDWord;
      break;
    case GiftPID:
      return &Gen5PIDRNG::NextGiftPIDWord;
      break;
    case GiftNoShinyPID:
      return &Gen5PIDRNG::NextNonShinyPIDWord;
      break;
    case RoamerPID:
      return &Gen5PIDRNG::NextRoamerPIDWord;
      break;
    case EntraLinkPID:
      return &Gen5PIDRNG::NextWildPIDWord;
      break;
    case EggPID:
      return &Gen5PIDRNG::NextEggPIDWord;
      break;
    default:
      return &Gen5PIDRNG::NextWildPIDWord;
      break;
    }
  }
};

}

#endif
