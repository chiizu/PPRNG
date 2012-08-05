/*
  Copyright (C) 2012 chiizu
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

#ifndef IV_SEED_CACHE_H
#define IV_SEED_CACHE_H

#include "PPRNGTypes.h"

#include <boost/interprocess/containers/flat_map.hpp>
#include <boost/unordered_map.hpp>

namespace pprng
{

struct SeedData
{
  SeedData(uint32_t f, uint32_t w) : frame(f), ivWord(w) {}
  
  SeedData& operator=(const SeedData &other)
  {
    frame = other.frame;
    ivWord = other.ivWord;
    return *this;
  }
  
  uint32_t  frame;
  uint32_t  ivWord;
};



// holds a data up to IV frame 103
//  BW   - max IV frame 103
//  B2W2 - max IV frame 101
//       - max Dream Radar frame 40 (for Genie / slot 2 mon)
enum
{
  IVSeedMapMaxFrame = 103
};

typedef boost::container::flat_multimap<uint32_t, SeedData> IVSeedMap;

const IVSeedMap& GetIVSeedMap(IVPattern::Type pattern);




// holds a data up to IV frame 8, supports BW roamers - faster than map above
//  BW   - max IV frame 8
//  B2W2 - max IV frame 6
enum
{
  IVSeedHashMaxFrame = 8
};

typedef boost::unordered_map<uint32_t, std::vector<SeedData> > IVSeedHash;

const IVSeedHash& GetIVSeedHash(IVPattern::Type pattern, bool isRoamer);

}

#endif
