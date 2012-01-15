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

#ifndef LAZY_MERSENNE_TWISTER_RNG_H
#define LAZY_MERSENNE_TWISTER_RNG_H

#include "PPRNGTypes.h"

namespace pprng
{

// the original Mersenne Twister in C++ class form
class MersenneTwisterRNG
{
public:
  // some templates expect these typedefs
  typedef uint32_t  SeedType;
  typedef uint32_t  ReturnType;
  
  MersenneTwisterRNG(uint32_t seed);
  ~MersenneTwisterRNG();
  
  MersenneTwisterRNG(const MersenneTwisterRNG&);
  MersenneTwisterRNG& operator=(const MersenneTwisterRNG&);
  
  ReturnType Next() { return NextUInt32(); }

  /* generates a random number on [0,0xffffffff]-interval */
  uint32_t NextUInt32();

private:
  MersenneTwisterRNG();
  
  enum PeriodParameters
  {
    N = 624,
    M = 397
  };
  
  uint32_t m_mt[N]; /* the array for the state vector  */
  uint32_t m_mti; /* mti==N+1 means mt[N] is not initialized */

  /* initializes mt[N] with a seed */
  void InitGenRand(uint32_t seed);

  /* initialize by an array with array-length */
  /* init_key is the array for initializing keys */
  /* key_length is its length */
  /* slight change for C++, 2004/2/26 */
  void InitByArray(uint32_t initKey[], uint32_t keyLength);
};


// This is a lazy version of the Mersenne Twister, doing only the minimum work
// required to generate each seed, since most searches won't be looking beyond
// the first 50 IV frames.  It assumes it will be called at least once, however,
// so if there is a situation where it would be instantiated, but not actually
// called, it could be made even more lazy.
class LazyMersenneTwisterRNG
{
public:
  // some templates expect these typedefs
  typedef uint32_t  SeedType;
  typedef uint32_t  ReturnType;
  
  LazyMersenneTwisterRNG(uint32_t seed);
  ~LazyMersenneTwisterRNG();
  
  LazyMersenneTwisterRNG(const LazyMersenneTwisterRNG&);
  LazyMersenneTwisterRNG& operator=(const LazyMersenneTwisterRNG&);
  
  ReturnType Next() { return NextUInt32(); }

  /* generates a random number on [0,0xffffffff]-interval */
  uint32_t NextUInt32()
  { return (this->*m_nextUInt32Generator)(); }

private:
  LazyMersenneTwisterRNG();
  
  enum PeriodParameters
  {
    N = 624,
    M = 397,
    L = N - M
  };
  
  uint32_t m_mt[N]; /* the array for the state vector  */
  uint32_t m_mti; /* mti==N+1 means mt[N] is not initialized */
  
  typedef uint32_t (LazyMersenneTwisterRNG::*NextUInt32Generator)();
  
  NextUInt32Generator  m_nextUInt32Generator;
  
  uint32_t FirstSectionLNextUInt32();
  uint32_t SectionLNextUInt32();
  uint32_t SectionMNextUInt32();

  /* initializes mt[N] with a seed */
  void InitGenRand(uint32_t seed);
};

typedef LazyMersenneTwisterRNG MTRNG;

}

#endif
