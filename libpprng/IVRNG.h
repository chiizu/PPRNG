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
// uses the shifting functionality of the IVs class to do its own buffering
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
    : m_RNG(rng), m_IVs(),
      m_IVWordGenerator((frameType == Normal) ?
                        &Gen5BufferingIVRNG::NextNormalIVWord :
                        &Gen5BufferingIVRNG::NextRoamerIVWord)
  {
    if (frameType == Normal)
    {
      m_IVs.word = ((m_RNG.Next() >> LowBitOffset) << IVs::AT_SHIFT) |
                   ((m_RNG.Next() >> LowBitOffset) << IVs::DF_SHIFT) |
                   ((m_RNG.Next() >> LowBitOffset) << IVs::SA_SHIFT) |
                   ((m_RNG.Next() >> LowBitOffset) << IVs::SD_SHIFT) |
                   ((m_RNG.Next() >> LowBitOffset) << IVs::SP_SHIFT);
    }
    else
    {
      // unknown call
      m_RNG.Next();
      
      m_IVs.word = ((m_RNG.Next() >> LowBitOffset) << IVs::AT_SHIFT) |
                   ((m_RNG.Next() >> LowBitOffset) << IVs::DF_SHIFT) |
                   ((m_RNG.Next() >> LowBitOffset) << IVs::SD_SHIFT) |
                   ((m_RNG.Next() >> LowBitOffset) << IVs::SP_SHIFT) |
                   ((m_RNG.Next() >> LowBitOffset) << IVs::SA_SHIFT);
    }
  }
  
  uint32_t NextIVWord()
  {
    return (this->*m_IVWordGenerator)();
  }
  
private:
  typedef uint32_t (Gen5BufferingIVRNG::*IVWordGenerator)();
  
  const IVWordGenerator  m_IVWordGenerator;
  
  uint32_t NextNormalIVWord()
  {
    m_IVs.ShiftDownNormal(m_RNG.Next() >> LowBitOffset);
    
    return m_IVs.word;
  }
  
  uint32_t NextRoamerIVWord()
  {
    m_IVs.ShiftDownRoamer(m_RNG.Next() >> LowBitOffset);
    
    return m_IVs.word;
  }
  
  RNG  &m_RNG;
  IVs  m_IVs;
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
