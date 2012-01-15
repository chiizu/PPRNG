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

#ifndef RNG_WRAPPERS_H
#define RNG_WRAPPERS_H

#include "PPRNGTypes.h"
#include "NextPowerOf2.h"
#include <deque>

namespace pprng
{

template <class RNG, uint32_t NumUpperBits>
class UpperBitsRNG
{
public:
  typedef typename RNG::SeedType    SeedType;
  typedef typename RNG::ReturnType  ReturnType;
  
  enum { LowBitOffset = (sizeof(typename RNG::ReturnType) * 8) - NumUpperBits };
  
  UpperBitsRNG(typename RNG::SeedType seed)
    : m_RNG(seed)
  {}
  
  ReturnType Next()
  {
    return (m_RNG.Next() >> LowBitOffset) & ((0x1 << NumUpperBits) - 1);
  }
  
private:
  RNG  m_RNG;
};


template <class RNG, uint32_t NumBufferedResults>
class BufferedRNG
{
public:
  typedef typename RNG::SeedType    SeedType;
  typedef typename RNG::ReturnType  ReturnType;
  
  enum { BufferSize = NextPowerOf2<NumBufferedResults>::result };
  
  BufferedRNG(typename RNG::SeedType seed)
    : m_RNG(seed), m_end(NumBufferedResults - 1), m_pos(0)
  {
    // fill the buffer
    for (uint32_t i = 0; i < (NumBufferedResults - 1); ++i)
    {
      m_buffer[i] = m_RNG.Next();
    }
  }
  
  ReturnType Next()
  {
    ReturnType  result = m_buffer[m_pos];
    m_pos = (m_pos + 1) & (BufferSize - 1);
    return result;
  }
  
  ReturnType PeekNext()
  {
    return m_buffer[m_pos];
  }
  
  void ResetPos()
  {
    m_pos = (m_end - NumBufferedResults) & (BufferSize - 1);
  }
  
  void AdvanceBuffer()
  {
    m_buffer[m_end] = m_RNG.Next();
    m_end = (m_end + 1) & (BufferSize - 1);
    ResetPos();
  }
  
private:
  RNG             m_RNG;
  ReturnType      m_buffer[BufferSize];
  uint32_t        m_end;
  uint32_t        m_pos;
};


template <class RNG>
class QueuedRNG
{
public:
  typedef typename RNG::SeedType    SeedType;
  typedef typename RNG::ReturnType  ReturnType;
  
  QueuedRNG(typename RNG::SeedType seed)
    : m_RNG(seed), m_Queue(), m_Position(m_Queue.end())
  {}
  
  ReturnType Next()
  {
    MaybeAddNext();
    
    return *m_Position++;
  }
  
  ReturnType PeekNext()
  {
    MaybeAddNext();
    
    return *m_Position;
  }
  
  void AdvanceBuffer()
  {
    if (m_Queue.size() > 0)
    {
      m_Queue.pop_front();
    }
    m_Position = m_Queue.begin();
  }
  
private:
  void MaybeAddNext()
  {
    if (m_Position == m_Queue.end())
    {
      m_Queue.push_back(m_RNG.Next());
      m_Position = m_Queue.end() - 1;
    }
  }
  
  typedef std::deque<ReturnType>              QueueType;
  typedef typename QueueType::const_iterator  QueueIterator;
  
  RNG            m_RNG;
  QueueType      m_Queue;
  QueueIterator  m_Position;
};

}

#endif
