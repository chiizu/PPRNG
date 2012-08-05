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


#include "IVSeedCache.h"

namespace pprng
{

namespace
{

typedef uint32_t IVSeedRow[3];

// macro for parsing data file
#define DEFINE_IV_SEED(SEED, FRAME, IVWORD)    { SEED, FRAME, IVWORD },

// normal sets
IVSeedRow  PerfectIVsData[] = {
#include "data/ivcache/perfect.txt"
};


IVSeedRow  PhysIVsData[] = {
#include "data/ivcache/phys.txt"
};


IVSeedRow  SpecIVsData[] = {
#include "data/ivcache/spec.txt"
};

// trick room sets
IVSeedRow  PerfectTrickIVsData[] = {
#include "data/ivcache/perfect_trick.txt"
};


IVSeedRow  PhysTrickIVsData[] = {
#include "data/ivcache/phys_trick.txt"
};


IVSeedRow  SpecTrickIVsData[] = {
#include "data/ivcache/spec_trick.txt"
};

// roamer normal sets
IVSeedRow  RoamerPerfectIVsData[] = {
#include "data/roamer_enc/perfect.txt"
};

IVSeedRow  RoamerPhysIVsData[] = {
#include "data/roamer_enc/phys.txt"
};

IVSeedRow  RoamerSpecIVsData[] = {
#include "data/roamer_enc/spec.txt"
};

// roamer trick room sets
IVSeedRow  RoamerPerfectTrickIVsData[] = {
#include "data/roamer_enc/perfect_trick.txt"
};

IVSeedRow  RoamerPhysTrickIVsData[] = {
#include "data/roamer_enc/phys_trick.txt"
};

IVSeedRow  RoamerSpecTrickIVsData[] = {
#include "data/roamer_enc/spec_trick.txt"
};


// redefine macro for parsing hidden power files
#undef DEFINE_IV_SEED
#define DEFINE_IV_SEED(SEED, FRAME, IVWORD, HPTYPE)  { SEED, FRAME, IVWORD },

IVSeedRow  HpIVsData[] = {
#include "data/ivcache/hp.txt"
};

IVSeedRow  HpTrickIVsData[] = {
#include "data/ivcache/hp_trick.txt"
};

IVSeedRow  RoamerHpIVsData[] = {
#include "data/roamer_enc/hp.txt"
};

IVSeedRow  RoamerHpTrickIVsData[] = {
#include "data/roamer_enc/hp_trick.txt"
};




struct InputIterator
{
  typedef std::random_access_iterator_tag  iterator_category;
  typedef IVSeedMap::value_type            value_type;
  typedef std::ptrdiff_t                   difference_type;
  typedef value_type*                      pointer;
  typedef value_type&                      reference;
  typedef const value_type&                const_reference;
  
  InputIterator(const IVSeedRow *data)
    : m_data(data), m_item((*m_data)[0], SeedData((*m_data)[1], (*m_data)[2]))
  {}
  
  bool operator!=(const InputIterator &other)
  {
    return m_data != other.m_data;
  }
  
  InputIterator& operator++()
  {
    ++m_data;
    m_item.first = (*m_data)[0];
    m_item.second = SeedData((*m_data)[1], (*m_data)[2]);
    return *this;
  }
  
  InputIterator& operator+=(difference_type i)
  {
    m_data += i;
    m_item.first = (*m_data)[0];
    m_item.second = SeedData((*m_data)[1], (*m_data)[2]);
    return *this;
  }
  
  const_reference operator*() const
  {
    return m_item;
  }
  
  difference_type operator-(const InputIterator &other) const
  {
    return m_data - other.m_data;
  }
  
  const IVSeedRow        *m_data;
  IVSeedMap::value_type  m_item;
};

// macro for defining the seed map
#define DEFINE_IV_SEED_MAP(NAME, DATASOURCE) \
  IVSeedMap  NAME(InputIterator(DATASOURCE), \
                  InputIterator(DATASOURCE + (sizeof(DATASOURCE) / \
                                              sizeof(IVSeedRow))))

DEFINE_IV_SEED_MAP(PerfectIVsMap, PerfectIVsData);
DEFINE_IV_SEED_MAP(PhysIVsMap, PhysIVsData);
DEFINE_IV_SEED_MAP(SpecIVsMap, SpecIVsData);
DEFINE_IV_SEED_MAP(HpIVsMap, HpIVsData);
DEFINE_IV_SEED_MAP(PerfectTrickIVsMap, PerfectTrickIVsData);
DEFINE_IV_SEED_MAP(PhysTrickIVsMap, PhysTrickIVsData);
DEFINE_IV_SEED_MAP(SpecTrickIVsMap, SpecTrickIVsData);
DEFINE_IV_SEED_MAP(HpTrickIVsMap, HpTrickIVsData);

}


const IVSeedMap& GetIVSeedMap(IVPattern::Type pattern)
{
  switch (pattern)
  {
  case IVPattern::HEX_FLAWLESS:
    return PerfectIVsMap;
    break;
    
  case IVPattern::PHYSICAL_FLAWLESS:
    return PhysIVsMap;
    break;
    
  case IVPattern::SPECIAL_FLAWLESS:
    return SpecIVsMap;
    break;
    
  case IVPattern::SPECIAL_HIDDEN_POWER_FLAWLESS:
    return HpIVsMap;
    break;
    
  case IVPattern::HEX_FLAWLESS_TRICK:
    return PerfectTrickIVsMap;
    break;
    
  case IVPattern::PHYSICAL_FLAWLESS_TRICK:
    return PhysTrickIVsMap;
    break;
    
  case IVPattern::SPECIAL_FLAWLESS_TRICK:
    return SpecTrickIVsMap;
    break;
    
  case IVPattern::SPECIAL_HIDDEN_POWER_FLAWLESS_TRICK:
    return HpTrickIVsMap;
    break;
  
  case IVPattern::CUSTOM:
  default:
    throw;
    break;
  }
}




IVSeedHash MakeHash(IVSeedRow *data, uint32_t rowCount)
{
  IVSeedHash  result;
  
  for (uint32_t i = 0; i < rowCount; ++i)
  {
    if (data[i][1] <= 8)
      result[data[i][0]].push_back(SeedData(data[i][1], data[i][2]));
  }
  
  return result;
}

// macro for parsing data file
#undef DEFINE_IV_SEED
#define DEFINE_IV_SEED(SEED, FRAME, IVWORD)   { SEED, FRAME, IVWORD },

// macro for defining the seed map
#define DEFINE_IV_SEED_HASH(NAME, DATASOURCE) \
IVSeedHash  NAME = MakeHash(DATASOURCE, sizeof(DATASOURCE) / sizeof(IVSeedRow))


DEFINE_IV_SEED_HASH(PerfectIVsHash, PerfectIVsData);
DEFINE_IV_SEED_HASH(PhysIVsHash, PhysIVsData);
DEFINE_IV_SEED_HASH(SpecIVsHash, SpecIVsData);
DEFINE_IV_SEED_HASH(HpIVsHash, HpIVsData);
DEFINE_IV_SEED_HASH(PerfectTrickIVsHash, PerfectTrickIVsData);
DEFINE_IV_SEED_HASH(PhysTrickIVsHash, PhysTrickIVsData);
DEFINE_IV_SEED_HASH(SpecTrickIVsHash, SpecTrickIVsData);
DEFINE_IV_SEED_HASH(HpTrickIVsHash, HpTrickIVsData);

DEFINE_IV_SEED_HASH(RoamerPerfectIVsHash, RoamerPerfectIVsData);
DEFINE_IV_SEED_HASH(RoamerPhysIVsHash, RoamerPhysIVsData);
DEFINE_IV_SEED_HASH(RoamerSpecIVsHash, RoamerSpecIVsData);
DEFINE_IV_SEED_HASH(RoamerHpIVsHash, RoamerHpIVsData);
DEFINE_IV_SEED_HASH(RoamerPerfectTrickIVsHash, RoamerPerfectTrickIVsData);
DEFINE_IV_SEED_HASH(RoamerPhysTrickIVsHash, RoamerPhysTrickIVsData);
DEFINE_IV_SEED_HASH(RoamerSpecTrickIVsHash, RoamerSpecTrickIVsData);
DEFINE_IV_SEED_HASH(RoamerHpTrickIVsHash, RoamerHpTrickIVsData);


const IVSeedHash& GetIVSeedHash(IVPattern::Type pattern, bool isRoamer)
{
  switch (pattern)
  {
  case IVPattern::HEX_FLAWLESS:
    return isRoamer ? RoamerPerfectIVsHash : PerfectIVsHash;
    break;
    
  case IVPattern::PHYSICAL_FLAWLESS:
    return isRoamer ? RoamerPhysIVsHash : PhysIVsHash;
    break;
    
  case IVPattern::SPECIAL_FLAWLESS:
    return isRoamer ? RoamerSpecIVsHash : SpecIVsHash;
    break;
    
  case IVPattern::SPECIAL_HIDDEN_POWER_FLAWLESS:
    return isRoamer ? RoamerHpIVsHash : HpIVsHash;
    break;
    
  case IVPattern::HEX_FLAWLESS_TRICK:
    return isRoamer ? RoamerPerfectTrickIVsHash : PerfectTrickIVsHash;
    break;
    
  case IVPattern::PHYSICAL_FLAWLESS_TRICK:
    return isRoamer ? RoamerPhysTrickIVsHash : PhysTrickIVsHash;
    break;
    
  case IVPattern::SPECIAL_FLAWLESS_TRICK:
    return isRoamer ? RoamerSpecTrickIVsHash : SpecTrickIVsHash;
    break;
    
  case IVPattern::SPECIAL_HIDDEN_POWER_FLAWLESS_TRICK:
    return isRoamer ? RoamerHpTrickIVsHash : HpTrickIVsHash;
    break;
  
  case IVPattern::CUSTOM:
  default:
    throw;
    break;
  }
}

}
