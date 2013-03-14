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

#ifndef IV_RNG_H
#define IV_RNG_H

#include "PPRNGTypes.h"
#include "RNGWrappers.h"

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


// standard IVRNG for 5th gen
template <class RNG>
class Gen5BufferingIVRNG
{
public:
  enum { LowBitOffset = (sizeof(typename RNG::ReturnType) * 8) - 5 };
  
  enum FrameType
  {
    Normal = 0,
    Roamer
  };
  
  Gen5BufferingIVRNG(RNG &rng, FrameType frameType)
    : m_RNG(rng), m_word(0),
      m_IVWordGenerator((frameType == Normal) ?
                        &Gen5BufferingIVRNG::NextNormalIVWord :
                        &Gen5BufferingIVRNG::NextRoamerIVWord)
  {
    if (frameType == Roamer)
      m_RNG.Next();  // unknown call
    
    uint32_t  word = 0;
    for (uint32_t i = 0; i < 5; ++i)
      word = NextRawWord(word);
    
    m_word = word;
  }
  
  uint32_t NextIVWord()
  {
    uint32_t  word = NextRawWord(m_word);
    m_word = word;
    
    return (this->*m_IVWordGenerator)(word);
  }
  
private:
  uint32_t NextRawWord(uint32_t currentWord)
  {
    return (currentWord >> 5) | (uint32_t(m_RNG.Next() >> LowBitOffset) << 25);
  }
  
  typedef uint32_t (Gen5BufferingIVRNG::*IVWordGenerator)(uint32_t buffer);
  
  const IVWordGenerator  m_IVWordGenerator;
  
  uint32_t NextNormalIVWord(uint32_t buffer)
  {
    uint32_t  result = (buffer & 0x7fff) |
                       ((buffer & 0x01ff8000) << 6) |
                       ((buffer & 0x3e000000) >> 9);
    return result;
  }
  
  uint32_t NextRoamerIVWord(uint32_t buffer)
  {
    uint32_t  result = (buffer & 0x7fff) |
                       ((buffer & 0x000f8000) << 11) |
                       ((buffer & 0x3ff00000) >> 4);
    return result;
  }
  
  RNG       &m_RNG;
  uint32_t  m_word;
};


// for use in cases when the RNG passed in will handle any buffering needed
// - generally used in cases where IV generation is part of a larger sequence
//   of RNG class (in particular, WonderCards)
template <class RNG>
class Gen5NonBufferingIVRNG
{
public:
  enum { LowBitOffset = (sizeof(typename RNG::ReturnType) * 8) - 5 };
  
  enum FrameType
  {
    Normal = 0,
    Roamer
  };
  
  Gen5NonBufferingIVRNG(RNG &rng, FrameType frameType)
    : m_RNG(rng),
      m_IVWordGenerator((frameType == Normal) ?
                        &Gen5NonBufferingIVRNG::NextNormalIVWord :
                        &Gen5NonBufferingIVRNG::NextRoamerIVWord)
  {}
  
  uint32_t NextIVWord()
  {
    return (this->*m_IVWordGenerator)();
  }
  
private:
  typedef uint32_t (Gen5NonBufferingIVRNG::*IVWordGenerator)();
  
  const IVWordGenerator  m_IVWordGenerator;
  
  uint32_t NextNormalIVWord()
  {
    return ((m_RNG.Next() >> LowBitOffset) << IVs::HP_SHIFT) |
           ((m_RNG.Next() >> LowBitOffset) << IVs::AT_SHIFT) |
           ((m_RNG.Next() >> LowBitOffset) << IVs::DF_SHIFT) |
           ((m_RNG.Next() >> LowBitOffset) << IVs::SA_SHIFT) |
           ((m_RNG.Next() >> LowBitOffset) << IVs::SD_SHIFT) |
           ((m_RNG.Next() >> LowBitOffset) << IVs::SP_SHIFT);
  }
  
  uint32_t NextRoamerIVWord()
  {
    // unknown call
    m_RNG.Next();
    
    return ((m_RNG.Next() >> LowBitOffset) << IVs::HP_SHIFT) |
           ((m_RNG.Next() >> LowBitOffset) << IVs::AT_SHIFT) |
           ((m_RNG.Next() >> LowBitOffset) << IVs::DF_SHIFT) |
           ((m_RNG.Next() >> LowBitOffset) << IVs::SD_SHIFT) |
           ((m_RNG.Next() >> LowBitOffset) << IVs::SP_SHIFT) |
           ((m_RNG.Next() >> LowBitOffset) << IVs::SA_SHIFT);
  }
  
  RNG  &m_RNG;
};

}

#endif
