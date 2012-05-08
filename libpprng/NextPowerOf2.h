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

#ifndef NEXT_POWER_OF_2_H
#define NEXT_POWER_OF_2_H

#include "PPRNGTypes.h"

namespace pprng
{

template <uint32_t I, uint32_t R, uint32_t N>
struct NextPowerOf2Helper
{
  enum { result = NextPowerOf2Helper<(I >> 1), R + (I & 0x1), N << 1>::result };
};

template <uint32_t R, uint32_t N>
struct NextPowerOf2Helper<1, R, N>
{
  enum { result = N << 1 };
};

template <uint32_t N>
struct NextPowerOf2Helper<1, 0, N>
{
  enum { result = N };
};

template <uint32_t N>
struct NextPowerOf2Helper<0, 0, N>
{
  enum { result = 0 };
};

template <uint32_t I>
struct NextPowerOf2
{
  enum { result = NextPowerOf2Helper<I, 0, 1>::result };
};

}

#endif
