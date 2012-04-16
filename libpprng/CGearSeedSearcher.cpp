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


#include "CGearSeedSearcher.h"
#include "SeedSearcher.h"

#include <boost/interprocess/containers/flat_map.hpp>

namespace pprng
{

namespace
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

typedef boost::container::flat_multimap<uint32_t, SeedData> IVSeedMap;

typedef uint32_t IVSeedRow[3];

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

// macro for parsing data file
#define DEFINE_IV_SEED(SEED, FRAME, IVWORD)    { SEED, FRAME, IVWORD },

// macro for defining the seed map
#define DEFINE_IV_SEED_MAP(NAME, DATASOURCE) \
  IVSeedMap  NAME(InputIterator(DATASOURCE), \
                  InputIterator(DATASOURCE + (sizeof(DATASOURCE) / \
                                              sizeof(IVSeedRow))))

// normal sets
IVSeedRow  PerfectIVsData[] = {
#include "data/entralink/perfect.txt"
};

DEFINE_IV_SEED_MAP(PerfectIVs, PerfectIVsData);

IVSeedRow  PhysIVsData[] = {
#include "data/entralink/phys.txt"
};

DEFINE_IV_SEED_MAP(PhysIVs, PhysIVsData);

IVSeedRow  SpecIVsData[] = {
#include "data/entralink/spec.txt"
};

DEFINE_IV_SEED_MAP(SpecIVs, SpecIVsData);


// trick room sets
IVSeedRow  PerfectTrickIVsData[] = {
#include "data/entralink/perfect_trick.txt"
};

DEFINE_IV_SEED_MAP(PerfectTrickIVs, PerfectTrickIVsData);

IVSeedRow  PhysTrickIVsData[] = {
#include "data/entralink/phys_trick.txt"
};

DEFINE_IV_SEED_MAP(PhysTrickIVs, PhysTrickIVsData);

IVSeedRow  SpecTrickIVsData[] = {
#include "data/entralink/spec_trick.txt"
};

DEFINE_IV_SEED_MAP(SpecTrickIVs, SpecTrickIVsData);



// redefine macro for parsing hidden power files
#undef DEFINE_IV_SEED
#define DEFINE_IV_SEED(SEED, FRAME, IVWORD, HPTYPE)  { SEED, FRAME, IVWORD },

IVSeedRow  HpIVsData[] = {
#include "data/entralink/hp.txt"
};

DEFINE_IV_SEED_MAP(HpIVs, HpIVsData);

IVSeedRow  HpTrickIVsData[] = {
#include "data/entralink/hp_trick.txt"
};

DEFINE_IV_SEED_MAP(HpTrickIVs, HpTrickIVsData);


const IVSeedMap& GetIVSeedMap(IVPattern::Type pattern)
{
  switch (pattern)
  {
  case IVPattern::HEX_FLAWLESS:
    return PerfectIVs;
    break;
    
  case IVPattern::PHYSICAL_FLAWLESS:
    return PhysIVs;
    break;
    
  case IVPattern::SPECIAL_FLAWLESS:
    return SpecIVs;
    break;
    
  case IVPattern::SPECIAL_HIDDEN_POWER_FLAWLESS:
    return HpIVs;
    break;
    
  case IVPattern::HEX_FLAWLESS_TRICK:
    return PerfectTrickIVs;
    break;
    
  case IVPattern::PHYSICAL_FLAWLESS_TRICK:
    return PhysTrickIVs;
    break;
    
  case IVPattern::SPECIAL_FLAWLESS_TRICK:
    return SpecTrickIVs;
    break;
    
  case IVPattern::SPECIAL_HIDDEN_POWER_FLAWLESS_TRICK:
    return HpTrickIVs;
    break;
  
  case IVPattern::CUSTOM:
  default:
    throw;
    break;
  }
}


class FastSearchSeedGenerator
{
public:
  typedef uint32_t  SeedType;
  typedef uint32_t  SeedCountType;
  
  enum { SeedsPerChunk = 1000 };
  
  FastSearchSeedGenerator()
    : m_dayMonthMinuteSecond(0xff000000), m_hour(0x00170000)
  {}
  
  SeedCountType NumberOfSeeds() const
  {
    return 256 * 24;
  }
  
  SeedType Next()
  {
    m_hour += 0x00010000;
    
    if (m_hour > 0x00170000)
    {
      m_hour = 0x00000000;
      m_dayMonthMinuteSecond += 0x01000000;
    }
    
    return m_dayMonthMinuteSecond | m_hour;
  }
  
private:
  uint32_t  m_dayMonthMinuteSecond;
  uint32_t  m_hour;
};

struct FrameChecker
{
  FrameChecker(const CGearSeedSearcher::Criteria &criteria)
    : m_criteria(criteria)
  {}
  
  bool operator()(const CGearIVFrame &frame) const
  {
    return CheckIVs(frame.ivs) && CheckHiddenPower(frame.ivs);
  }
  
  bool CheckIVs(const IVs &ivs) const
  {
    return ivs.betterThanOrEqual(m_criteria.ivs.min) &&
           (!m_criteria.ivs.shouldCheckMax ||
            ivs.worseThanOrEqual(m_criteria.ivs.max));
  }

  bool CheckHiddenPower(const IVs &ivs) const
  {
    if (m_criteria.ivs.hiddenType == Element::NONE)
    {
      return true;
    }
    
    if ((m_criteria.ivs.hiddenType == Element::ANY) ||
        (m_criteria.ivs.hiddenType == ivs.HiddenType()))
    {
      return ivs.HiddenPower() >= m_criteria.ivs.minHiddenPower;
    }
    
    return false;
  }
  
  const CGearSeedSearcher::Criteria  &m_criteria;
};

class FastSeedSearcher
{
public:
  typedef CGearIVFrame  ResultType;
  
  FastSeedSearcher(const IVSeedMap &seedMap,
                   const CGearSeedSearcher::Criteria &criteria)
    : m_seedMap(seedMap), m_frameRange(criteria.frameRange),
      m_minDelay(criteria.minDelay), m_maxDelay(criteria.maxDelay),
      m_macAddressLow(criteria.macAddressLow)
  {}
  
  void Search(uint32_t baseSeed, const FrameChecker &frameChecker,
              const CGearSeedSearcher::ResultCallback &resultHandler)
  {
    uint32_t  lowSeed = baseSeed + (m_minDelay & 0xFFFF) + m_macAddressLow;
    uint32_t  highSeed = baseSeed + (m_maxDelay & 0xFFFF) + m_macAddressLow;
    
    if (lowSeed <= highSeed)
    {
      SearchUntil(m_seedMap.lower_bound(lowSeed),
                  m_seedMap.upper_bound(highSeed), frameChecker, resultHandler);
    }
    else
    {
      SearchUntil(m_seedMap.lower_bound(lowSeed), m_seedMap.end(),
                  frameChecker, resultHandler);
      SearchUntil(m_seedMap.begin(), m_seedMap.upper_bound(highSeed),
                  frameChecker, resultHandler);
    }
  }
  
private:
  void SearchUntil(IVSeedMap::const_iterator i, IVSeedMap::const_iterator end,
                   const FrameChecker &frameChecker,
                   const CGearSeedSearcher::ResultCallback &resultHandler)
  {
    while (i != end)
    {
      if ((i->second.frame >= m_frameRange.min) &&
          (i->second.frame <= m_frameRange.max))
      {
        CGearIVFrame  frame;
        
        frame.seed = i->first;
        frame.number = i->second.frame;
        frame.ivs = i->second.ivWord;
        
        if (frameChecker(frame))
          resultHandler(frame);
      }
      
      ++i;
    }
  }
  
  const IVSeedMap                   &m_seedMap;
  const SearchCriteria::FrameRange  &m_frameRange;
  const uint32_t                    m_minDelay, m_maxDelay, m_macAddressLow;
};


struct FrameGeneratorFactory
{
  typedef CGearIVFrameGenerator  FrameGenerator;
  
  FrameGeneratorFactory(CGearIVFrameGenerator::FrameType frameType)
    : m_frameType(frameType)
  {}
  
  CGearIVFrameGenerator operator()(uint32_t seed) const
  {
    return CGearIVFrameGenerator(seed, m_frameType);
  }
  
  const CGearIVFrameGenerator::FrameType m_frameType;
};

}

uint64_t CGearSeedSearcher::Criteria::ExpectedNumberOfResults() const
{
  uint64_t  delays = maxDelay - minDelay + 1;
  
  uint64_t  numSeeds = delays * 256 * 24;
  
  uint64_t  numFrames = frameRange.max - frameRange.min + 1;
  
  IVs  maxIVs = ivs.shouldCheckMax ? ivs.max : IVs(0x7FFF7FFF);
  
  uint64_t  numIVs = IVs::CalculateNumberOfCombinations(ivs.min, ivs.max);
  
  uint64_t  numResults = numFrames * numSeeds * numIVs /
                           (32 * 32 * 32 * 32 * 32 * 32);
  
  if (ivs.hiddenType != Element::NONE)
  {
    numResults = IVs::AdjustExpectedResultsForHiddenPower
      (numResults, ivs.min, ivs.max, ivs.hiddenType, ivs.minHiddenPower);
  }
  
  return numResults;
}

void CGearSeedSearcher::Search
  (const Criteria &criteria, const ResultCallback &resultHandler,
   const SearchRunner::ProgressCallback &progressHandler)
{
  FrameChecker  frameChecker(criteria);
  SearchRunner  searcher;
  
  if ((criteria.ivs.pattern == IVPattern::CUSTOM) ||
      (criteria.frameRange.min < 21) || (criteria.frameRange.min > 26) ||
      (criteria.frameRange.max < 21) || (criteria.frameRange.max > 26) ||
      criteria.ivs.isRoamer)
  {
    CGearSeedGenerator     seedGenerator(criteria.minDelay,
                                         criteria.maxDelay,
                                         criteria.macAddressLow);
    
    FrameGeneratorFactory  frameGenFactory(criteria.ivs.isRoamer ?
                                           CGearIVFrameGenerator::Roamer :
                                           CGearIVFrameGenerator::Normal);
  
    SeedFrameSearcher<FrameGeneratorFactory>  seedSearcher(frameGenFactory,
                                                           criteria.frameRange);
    
    searcher.Search(seedGenerator, seedSearcher, frameChecker,
                    resultHandler, progressHandler);
  }
  else
  {
    FastSearchSeedGenerator  seedGenerator;
    FastSeedSearcher         seedSearcher(GetIVSeedMap(criteria.ivs.pattern),
                                          criteria);
    
    searcher.Search(seedGenerator, seedSearcher, frameChecker,
                    resultHandler, progressHandler);
  }
}

}
