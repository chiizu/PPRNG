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


// Adapted from:

/* 
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)  
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/


#include "MersenneTwisterRNG.h"

#include <cstring>

/* constant vector a */
#define MATRIX_A 0x9908b0dfUL

/* most significant w-r bits */
#define UPPER_MASK 0x80000000UL

/* least significant r bits */
#define LOWER_MASK 0x7fffffffUL

namespace pprng
{

/// <summary>
/// Creates a new pseudo-random number generator with a given seed.
/// </summary>
/// <param name="seed">A value to use as a seed.</param>
MersenneTwisterRNG::MersenneTwisterRNG(uint32_t seed)
  : m_mti(N)
{
  InitGenRand(seed);
}

MersenneTwisterRNG::~MersenneTwisterRNG()
{}

MersenneTwisterRNG::MersenneTwisterRNG(const MersenneTwisterRNG &c)
  : m_mti(c.m_mti)
{
  ::memcpy(m_mt, c.m_mt, sizeof(uint32_t[N]));
}

MersenneTwisterRNG& MersenneTwisterRNG::operator=(const MersenneTwisterRNG &c)
{
  m_mti = c.m_mti;
  ::memcpy(m_mt, c.m_mt, sizeof(uint32_t[N]));
  return *this;
}

/* generates a random number on [0,0xffffffff]-interval */
uint32_t MersenneTwisterRNG::NextUInt32()
{
  uint32_t  y;
  
  if (m_mti >= N) /* generate N words at one time */
  {
    uint32_t  kk = 0;
    uint32_t  mtkk = m_mt[kk], mtkkP1;

    for (; kk < N - M; ++kk)
    {
      mtkkP1 = m_mt[kk + 1];
      
      y = (mtkk & UPPER_MASK) | (mtkkP1 & LOWER_MASK);
      m_mt[kk] = m_mt[kk + M] ^ (y >> 1) ^ ((y & 0x1) * MATRIX_A);
      
      mtkk = mtkkP1;
    }

    for (; kk < N - 1; ++kk)
    {
      mtkkP1 = m_mt[kk + 1];
      
      y = (mtkk & UPPER_MASK) | (mtkkP1 & LOWER_MASK);
      m_mt[kk] = m_mt[kk + (M - N)] ^ (y >> 1) ^ ((y & 0x1) * MATRIX_A);
      
      mtkk = mtkkP1;
    }

    y = (mtkk & UPPER_MASK) | (m_mt[0] & LOWER_MASK);
    m_mt[N - 1] = m_mt[M - 1] ^ (y >> 1) ^ ((y & 0x1) * MATRIX_A);

    m_mti = 0;
  }
  
  y = m_mt[m_mti++];
  
  /* Tempering */
  y ^= y >> 11;
  y ^= (y << 7) & 0x9d2c5680UL;
  y ^= (y << 15) & 0xefc60000UL;
  y ^= y >> 10;

  return y;
}

/* initializes mt[N] with a seed */
void MersenneTwisterRNG::InitGenRand(uint32_t seed)
{
  uint32_t  mti;
  uint32_t  prevMt = seed;

  m_mt[0] = seed;
  for (mti = 1; mti < N; ++mti)
  {
    m_mt[mti] = prevMt = (1812433253UL * (prevMt ^ (prevMt >> 30))) + mti;
    /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
    /* In the previous versions, MSBs of the seed affect   */
    /* only MSBs of the array mt[].                        */
    /* 2002/01/09 modified by Makoto Matsumoto             */           
    
    // m_mt[mti] &= 0xffffffffUL;
    /* for >32 bit machines */
  }
  m_mti = N;
}


/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
void MersenneTwisterRNG::InitByArray(uint32_t initKey[], uint32_t keyLength)
{
  uint32_t i, j, k;
  InitGenRand(19650218UL);

  i = 1; j = 0;
  k = (N > keyLength) ? N : keyLength;

  for (; k > 0; --k)
  {
    m_mt[i] = (m_mt[i] ^ ((m_mt[i - 1] ^ (m_mt[i - 1] >> 30)) * 1664525UL))
        + initKey[j] + j; /* non linear */
    m_mt[i] &= 0xffffffffUL; // for WORDSIZE > 32 machines
    i++; j++;
    if (i >= N) { m_mt[0] = m_mt[N - 1]; i = 1; }
    if (j >= keyLength) j = 0;
  }

  for (k = N - 1; k > 0; k--)
  {
    m_mt[i] = (m_mt[i] ^ ((m_mt[i - 1] ^ (m_mt[i - 1] >> 30)) * 1566083941UL))
        - i; /* non linear */
    m_mt[i] &= 0xffffffffU; // for WORDSIZE > 32 machines
    i++;

    if (i < N)
    {
        continue;
    }

    m_mt[0] = m_mt[N - 1]; i = 1;
  }

  m_mt[0] = 0x80000000U; // MSB is 1; assuring non-zero initial array
}

}
