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

#ifndef LINEAR_CONGRUENTIAL_RNG_H
#define LINEAR_CONGRUENTIAL_RNG_H

#include "PPRNGTypes.h"

namespace pprng
{

// templated operands for speed
template <typename IntegerType, IntegerType Mux, IntegerType Inc>
class LinearCongruentialRNG
{
public:
  typedef IntegerType  SeedType;
  typedef IntegerType  ReturnType;
  
  LinearCongruentialRNG(IntegerType seed)
    : m_seed(seed)
  {}
  
  IntegerType Seed() const { return m_seed; }
  void Seed(IntegerType seed) { m_seed = seed; }
  IntegerType Multiplier() const { return Mux; }
  IntegerType Increment() const { return Inc; }
  
  ReturnType Next()
  {
    IntegerType  result = NextForSeed(m_seed);
    m_seed = result;
    return result;
  }
  
  static ReturnType NextForSeed(IntegerType seed)
  {
    return (seed * Mux) + Inc;
  }

private:
  IntegerType  m_seed;
};

// 3rd & 4th Generation RNGs
typedef LinearCongruentialRNG<uint32_t, 0x41c64e6dUL, 0x6073UL>     LCRNG34;
typedef LinearCongruentialRNG<uint32_t, 0xeeb9eb65UL, 0xa3561a1UL>  LCRNG34_R;
typedef LinearCongruentialRNG<uint32_t, 0x6c078965UL, 1UL>          ARNG;

// 5th Generation RNGs
typedef LinearCongruentialRNG
  <uint64_t, 0x5D588B656C078965ULL, 0x269EC3ULL>            LCRNG5;
typedef LinearCongruentialRNG
  <uint64_t, 0xDEDCEDAE9638806DULL, 0x9B1AE6E9A384E6F9ULL>  LCRNG5_R;


// argument operands for flexibility
template <typename IntegerType>
class ParameterizedLCRNG
{
public:
  typedef IntegerType  ReturnType;
  
  ParameterizedLCRNG(IntegerType seed, IntegerType multiplier,
                     IntegerType increment)
    : m_seed(seed), m_multiplier(multiplier), m_increment(increment)
  {}
  
  IntegerType Seed() const { return m_seed; }
  void Seed(IntegerType seed) { m_seed = seed; }
  IntegerType Multiplier() const { return m_multiplier; }
  IntegerType Increment() const { return m_increment; }
  
  ReturnType Next()
  {
    IntegerType  result = (m_seed * m_multiplier) + m_increment;
    m_seed = result;
    return result;
  }

private:
  IntegerType        m_seed;
  const IntegerType  m_multiplier;
  const IntegerType  m_increment;
};

}

#endif
