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

#ifndef IV_RNG_H
#define IV_RNG_H

#include "BasicTypes.h"

namespace pprng
{

template <int Method, class RNG>
class Gen34IVRNG
{
public:
  Gen34IVRNG(RNG &rng)
    : m_RNG(rng)
  {}
  
  uint32_t NextIVWord()
  {
    return (m_RNG.Next() >> 16) | (m_RNG.Next() & 0xffff0000);
  }
  
private:
  RNG  &m_RNG;
};

// specialize for method 4 due to extra RNG call between IV calls
template <class RNG>
class Gen34IVRNG<4, RNG>
{
public:
  Gen34IVRNG(RNG &rng)
    : m_RNG(rng)
  {}
  
  uint32_t NextIVWord()
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
class Gen5IVRNG
{
public:
  enum { LowBitOffset = (sizeof(typename RNG::ReturnType) * 8) - 5 };
  
  enum FrameType
  {
    Normal = 0,
    Roamer
  };
  
  Gen5IVRNG(RNG &rng, FrameType frameType)
    : m_RNG(rng),
    m_IVWordGenerator((frameType == Normal) ? &Gen5IVRNG::NextNormalIVWord : &Gen5IVRNG::NextRoamerIVWord)
  {}
  
  uint32_t NextIVWord()
  {
    return (this->*m_IVWordGenerator)();
  }
  
private:
  typedef uint32_t (Gen5IVRNG::*IVWordGenerator)();
  
  const IVWordGenerator  m_IVWordGenerator;
  
  uint32_t NextNormalIVWord()
  {
    IVs  result;
    
    result.hp(m_RNG.Next() >> LowBitOffset);
    result.at(m_RNG.Next() >> LowBitOffset);
    result.df(m_RNG.Next() >> LowBitOffset);
    result.sa(m_RNG.Next() >> LowBitOffset);
    result.sd(m_RNG.Next() >> LowBitOffset);
    result.sp(m_RNG.Next() >> LowBitOffset);
    
    return result.word;
  }
  
  uint32_t NextRoamerIVWord()
  {
    IVs  result;
    
    // unknown call
    m_RNG.Next();
    
    result.hp(m_RNG.Next() >> LowBitOffset);
    result.at(m_RNG.Next() >> LowBitOffset);
    result.df(m_RNG.Next() >> LowBitOffset);
    result.sd(m_RNG.Next() >> LowBitOffset);
    result.sp(m_RNG.Next() >> LowBitOffset);
    result.sa(m_RNG.Next() >> LowBitOffset);
    
    return result.word;
  }
  
  RNG                  &m_RNG;
};

}

#endif
