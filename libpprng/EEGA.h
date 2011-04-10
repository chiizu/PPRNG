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


#ifndef EEGA_H
#define EEGA_H

//  http://www.smogon.com/forums/group.php?do=discuss&group=&discussionid=198

namespace pprng
{

template <uint64_t A, uint64_t B>
struct EEGA
{
  static const uint64_t  f = ((A % B) == 0) ? 0 :
    EEGA<B, (A % B)>::g;
  
  static const uint64_t  g = ((A % B) == 0) ? 1 :
    EEGA<B, (A % B)>::f - (EEGA<B, (A % B)>::g * (A / B));
};


template <uint64_t A>
struct EEGA<A, 1>
{
  static const uint64_t  f = 0;
  
  static const uint64_t  g = 1;
};


template <typename IntType, IntType A, IntType C>
{
  static const IntType  r_A;
  static const IntType  r_C;
}


}

#endif
