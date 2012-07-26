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


#include "EggSeedSearcher.h"
#include "SeedSearcher.h"

#include <stdexcept>
#include <vector>
#include <fstream>
#include <string.h>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/interprocess/exceptions.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>

namespace pprng
{

namespace
{

struct IVRange
{
  IVs       minIVs, maxIVs;
  uint32_t  inheritancePatterns;
  
  IVRange(IVs min, IVs max, uint32_t i)
    : minIVs(min), maxIVs(max), inheritancePatterns(i)
  {}
};

std::vector<IVRange> GenerateIVRanges(OptionalIVs parent1IVs,
                                      OptionalIVs parent2IVs,
                                      IVs minEggIVs, IVs maxEggIVs)
{
  std::vector<IVRange>  result;
  
  uint32_t  minIV, maxIV, iv;
  uint32_t  i, j, k, ic, jc, kc;
  
  for (i = 0; i < 4; ++i)
  {
    ic = 0;
    
    minIV = minEggIVs.iv(i);  maxIV = maxEggIVs.iv(i);
    
    if ((minIV == 0) && (maxIV == 31))
    {
      ic += 2;
    }
    else
    {
      if (parent1IVs.isSet(i))
      {
        iv = parent1IVs.iv(i);
        ic += ((iv >= minIV) && (iv <= maxIV)) ? 1 : 0;
      }
      if (parent2IVs.isSet(i))
      {
        iv = parent2IVs.iv(i);
        ic += ((iv >= minIV) && (iv <= maxIV)) ? 1 : 0;
      }
    }
    
    if (ic > 0)
    {
      for (j = i + 1; j < 5; ++j)
      {
        jc = 0;
        
        minIV = minEggIVs.iv(j);  maxIV = maxEggIVs.iv(j);
        
        if ((minIV == 0) && (maxIV == 31))
        {
          jc += 2;
        }
        else
        {
          if (parent1IVs.isSet(j))
          {
            iv = parent1IVs.iv(j);
            jc += ((iv >= minIV) && (iv <= maxIV)) ? 1 : 0;
          }
          if (parent2IVs.isSet(j))
          {
            iv = parent2IVs.iv(j);
            jc += ((iv >= minIV) && (iv <= maxIV)) ? 1 : 0;
          }
        }
        
        if (jc > 0)
        {
          for (k = j + 1; k < 6; ++k)
          {
            kc = 0;
            
            minIV = minEggIVs.iv(k);  maxIV = maxEggIVs.iv(k);
            
            if ((minIV == 0) && (maxIV == 31))
            {
              kc += 2;
            }
            else
            {
              if (parent1IVs.isSet(k))
              {
                iv = parent1IVs.iv(k);
                kc += ((iv >= minIV) && (iv <= maxIV)) ? 1 : 0;
              }
              if (parent2IVs.isSet(k))
              {
                iv = parent2IVs.iv(k);
                kc += ((iv >= minIV) && (iv <= maxIV)) ? 1 : 0;
              }
            }
            
            if (kc > 0)
            {
              IVs  min = minEggIVs, max = maxEggIVs;
              
              min.setIV(i, 0);  min.setIV(j, 0);  min.setIV(k, 0);
              max.setIV(i, 31); max.setIV(j, 31); max.setIV(k, 31);
              
              result.push_back(IVRange(min, max, ic * jc * kc));
            }
          }
        }
      }
    }
  }
  
  return result;
}


struct IVFrameChecker
{
  IVFrameChecker(const EggSeedSearcher::Criteria &criteria)
    : m_possibleIVs(GenerateIVRanges(criteria.femaleIVs, criteria.maleIVs,
                                     criteria.ivs.min,
                                     criteria.ivs.shouldCheckMax ?
                                       criteria.ivs.max : IVs(0x7fff7fff)))
  {}
  
  bool operator()(const HashedIVFrame &frame) const
  {
    std::vector<IVRange>::const_iterator  i;
    for (i = m_possibleIVs.begin(); i != m_possibleIVs.end(); ++i)
    {
      if (frame.ivs.betterThanOrEqual(i->minIVs) &&
          frame.ivs.worseThanOrEqual(i->maxIVs))
      {
        return true;
      }
    }
    
    return false;
  }
  
  const std::vector<IVRange>  m_possibleIVs;
};




struct IVFrameResultHandler
{
  IVFrameResultHandler(const EggSeedSearcher::Criteria &criteria,
                       const EggSeedSearcher::ResultCallback &resultHandler)
    : m_criteria(criteria), m_resultHandler(resultHandler)
  {}
  
  void operator()(const HashedIVFrame &frame) const
  {
    Gen5BreedingFrameGenerator  generator
      (frame.seed, m_criteria.frameParameters);
    
    uint32_t  frameNum = 0;
    uint32_t  limitFrame = m_criteria.pid.startFromLowestFrame ?
      frame.seed.GetSkippedPIDFrames(false) + 1 :
      m_criteria.pidFrame.min - 1;
    while (frameNum < limitFrame)
    {
      generator.AdvanceFrame();
      ++frameNum;
    }
    
    while (frameNum < m_criteria.pidFrame.max)
    {
      generator.AdvanceFrame();
      ++frameNum;
      
      Gen5BreedingFrame  breedingFrame = generator.CurrentFrame();
      
      if (CheckShiny(breedingFrame.pid) && CheckNature(breedingFrame) &&
          CheckAbility(breedingFrame) && CheckGender(breedingFrame) &&
          CheckSpecies(breedingFrame))
      {
        Gen5EggFrame  eggFrame(generator.CurrentFrame(),
                               frame.number, frame.ivs,
                               m_criteria.femaleIVs, m_criteria.maleIVs);
        
        if (CheckIVs(eggFrame.ivs) && CheckHiddenPower(eggFrame.ivs))
        {
          m_resultHandler(eggFrame);
        }
      }
    }
  }
  
  bool CheckShiny(PID pid) const
  {
    return !m_criteria.shinyOnly ||
           pid.IsShiny(m_criteria.frameParameters.tid,
                       m_criteria.frameParameters.sid);
  }
  
  bool CheckNature(const Gen5BreedingFrame &frame) const
  {
    return (m_criteria.frameParameters.usingEverstone &&
            frame.everstoneActivated) ||
           m_criteria.pid.CheckNature(frame.nature);
  }
  
  bool CheckAbility(const Gen5BreedingFrame &frame) const
  {
    return ((m_criteria.pid.ability == Ability::ANY) ||
            (m_criteria.pid.ability == frame.pid.Gen5Ability())) &&
           (!m_criteria.inheritsHiddenAbility ||
            frame.inheritsHiddenAbility);
  }
  
  bool CheckGender(const Gen5BreedingFrame &frame) const
  {
    return Gender::GenderValueMatches(frame.pid.GenderValue(),
                                      m_criteria.pid.gender,
                                      m_criteria.pid.genderRatio);
  }
  
  bool CheckSpecies(const Gen5BreedingFrame &frame) const
  {
    return (m_criteria.frameParameters.femaleSpecies == FemaleParent::OTHER) ||
           (m_criteria.eggSpecies == EggSpecies::ANY) ||
           (frame.species == m_criteria.eggSpecies);
  }
  
  bool CheckIVs(const OptionalIVs &ivs) const
  {
    return ivs.betterThanOrEqual(m_criteria.ivs.min) &&
           (!m_criteria.ivs.shouldCheckMax ||
            ivs.worseThanOrEqual(m_criteria.ivs.max));
  }

  bool CheckHiddenPower(const OptionalIVs &oivs) const
  {
    if (m_criteria.ivs.hiddenType == Element::NONE)
    {
      return true;
    }
    
    if (oivs.allSet() &&
        ((m_criteria.ivs.hiddenType == Element::ANY) ||
         (m_criteria.ivs.hiddenType == oivs.values.HiddenType())))
    {
      return oivs.values.HiddenPower() >= m_criteria.ivs.minHiddenPower;
    }
    
    return false;
  }
  
  const EggSeedSearcher::Criteria        &m_criteria;
  const EggSeedSearcher::ResultCallback  &m_resultHandler;
};


struct IVFrameGeneratorFactory
{
  typedef HashedIVFrameGenerator  FrameGenerator;
  
  HashedIVFrameGenerator operator()(const HashedSeed &seed) const
  {
    return HashedIVFrameGenerator(seed, HashedIVFrameGenerator::Normal);
  }
};

static std::string   s_CacheDirectory;

static uint32_t      s_NumCacheReferences = 0;
static boost::mutex  s_CacheMutex;

struct IVSeedSet
{
  uint64_t  data[0x100000000ULL >> 6];
};

static boost::shared_ptr<IVSeedSet>  s_IVSeedSet;

static bool HasNamedCacheFile(const std::string &seedFile)
{
  std::string  filePath = s_CacheDirectory;
  if (!filePath.empty())
  {
    filePath += '/';
  }
  filePath += seedFile;
  
  return !std::ifstream(filePath.c_str()).fail();
}

static
EggSeedSearcher::CacheLoadResult
LoadSeeds(const std::string &seedFile, boost::shared_ptr<IVSeedSet> &seedSetPtr)
{
  using namespace  boost::interprocess;
  
  std::string  filePath = s_CacheDirectory;
  if (!filePath.empty())
  {
    filePath += '/';
  }
  filePath += seedFile;
  
  static const char  header[] = "SEED_DELTA_FILE";
  const uint32_t     version = 0x0100;
  
  boost::shared_ptr<IVSeedSet>  result;
  
  try
  {
    file_mapping    fm(filePath.c_str(), read_only);
    mapped_region   mr(fm, read_only);
    
    if (mr.get_size() < (sizeof(header) + sizeof(version) + sizeof(uint32_t)))
      return EggSeedSearcher::BAD_CACHE_FILE;
    
    const uint8_t  *buffer = static_cast<const uint8_t*>(mr.get_address());
    const uint8_t  *bufEnd = buffer + mr.get_size() - sizeof(uint32_t);
    
    std::string  fileHeader((char*)buffer, sizeof(header) - 1);
    if (fileHeader != header)
      return EggSeedSearcher::BAD_CACHE_FILE;
    
    buffer += sizeof(header);
    
    uint32_t  fileVersion;
    std::memcpy(&fileVersion, buffer, sizeof(uint32_t));
    
    if (fileVersion != version)
      return EggSeedSearcher::BAD_CACHE_FILE;
    
    buffer += sizeof(uint32_t);
    
    uint32_t  fullseed = 0;
    uint32_t  seedCount = 0;
    
    uint32_t  chunkPos = 0;
    uint64_t  chunk = 0;
    
    result.reset(new IVSeedSet);
    std::memset(result.get(), 0, sizeof(IVSeedSet));
    
    while (buffer < bufEnd)
    {
      uint32_t  delta = 0;
      
      uint8_t   byte = *buffer++;
      uint32_t  pos = 0;
      while ((byte > 0x7f) && (buffer < bufEnd))
      {
        delta |= ((byte & 0x7f) << pos);
        byte = *buffer++;
        pos += 7;
      }
      
      if (byte > 0x7f)
        return EggSeedSearcher::BAD_CACHE_FILE;
      
      delta |= (byte << pos);
      
      // get next seed
      fullseed += delta;
      ++seedCount;
      
      // determine next seed data chunk to use
      uint32_t  nextChunkPos = fullseed >> 6;
      if (nextChunkPos != chunkPos)
      {
        // new chunk, so store previous chunk
        result->data[chunkPos] = chunk;
        chunk = 0;
        chunkPos = nextChunkPos;
      }
      
      // mark bit in chunk
      chunk |= 0x1ULL << (fullseed & 0x3f);
    }
    
    // write final chunk
    result->data[chunkPos] = chunk;
    
    if (buffer != bufEnd)
      return EggSeedSearcher::BAD_CACHE_FILE;
    
    uint32_t  fileSeedCount;
    std::memcpy(&fileSeedCount, buffer, sizeof(uint32_t));
    
    if (fileSeedCount != seedCount)
      return EggSeedSearcher::BAD_CACHE_FILE;
  }
  catch (boost::interprocess::interprocess_exception &e)
  {
    return EggSeedSearcher::NO_CACHE_FILE;
  }
  catch (std::bad_alloc &e)
  {
    return EggSeedSearcher::NOT_ENOUGH_MEMORY;
  }
  catch (...)
  {
    return EggSeedSearcher::UNKNOWN_ERROR;
  }
  
  seedSetPtr = result;
  
  return EggSeedSearcher::LOADED;
}

struct SeedSearcher
{
  typedef HashedIVFrame  ResultType;
  
  SeedSearcher(const IVSeedSet &seedSet) : m_seedSet(seedSet) {}
  
  void Search(const HashedSeed &seed, const IVFrameChecker &frameChecker,
              const boost::function<void (const ResultType&)> &resultHandler)
  {
    uint32_t  ivSeed = seed.rawSeed >> 32;
    
    if (m_seedSet.data[ivSeed >> 6] & (0x1ULL << (ivSeed & 0x3f)))
    {
      HashedIVFrameGenerator  frameGen(seed, HashedIVFrameGenerator::Normal);
      
      frameGen.AdvanceFrame();
      frameGen.AdvanceFrame();
      frameGen.AdvanceFrame();
      frameGen.AdvanceFrame();
      frameGen.AdvanceFrame();
      frameGen.AdvanceFrame();
      frameGen.AdvanceFrame();
      frameGen.AdvanceFrame();
      
      HashedIVFrame  result = frameGen.CurrentFrame();
      
      if (frameChecker(result))
        resultHandler(result);
    }
  }
  
  const IVSeedSet  &m_seedSet;
};

}

uint64_t EggSeedSearcher::Criteria::ExpectedNumberOfResults() const
{
  IVs  maxIVs = ivs.shouldCheckMax ? ivs.max : IVs(0x7FFF7FFF);
  
  std::vector<IVRange>  ivRanges =
    GenerateIVRanges(femaleIVs, maleIVs, ivs.min, maxIVs);
  
  if (ivRanges.size() == 0)
    return 0;
  
  uint64_t  numSeeds = seedParameters.NumberOfSeeds();
  
  uint64_t  numIVFrames = ivFrame.max - ivFrame.min + 1;
  
  uint64_t  natureMultiplier = pid.NumNatures(), natureDivisor = 25;
  if (frameParameters.usingEverstone)
  {
    natureMultiplier += (25 - natureMultiplier) / 2;
  }
  
  uint64_t  abilityDivisor = (pid.ability < 2) ? 2 : 1;
  uint64_t  dwMultiplier, dwDivisor;
  if (inheritsHiddenAbility && !frameParameters.usingDitto)
  {
    dwMultiplier = 3;
    dwDivisor = 5;
  }
  else
  {
    dwMultiplier = dwDivisor = 1;
  }
  uint64_t  shinyMultiplier, shinyDivisor;
  if (shinyOnly)
  {
    shinyMultiplier = frameParameters.internationalParents ? 6 : 1;
    shinyDivisor = 8192;
  }
  else
  {
    shinyMultiplier = shinyDivisor = 1;
  }
  
  uint64_t  numPIDFrames = pidFrame.max - pidFrame.min + 1;
  
  uint64_t  multiplier = numIVFrames * numSeeds * numPIDFrames *
                         natureMultiplier * dwMultiplier * shinyMultiplier;
  
  uint64_t  divisor = natureDivisor * abilityDivisor * dwDivisor *
                      shinyDivisor * 32UL * 32UL * 32UL * 32UL * 32UL * 32UL;
  
  
  std::vector<IVRange>::const_iterator  i;
  uint64_t                              ivMatches = 0UL;
  for (i = ivRanges.begin(); i != ivRanges.end(); ++i)
  {
    uint64_t  ivSets = (i->maxIVs.hp() - i->minIVs.hp() + 1) *
                       (i->maxIVs.at() - i->minIVs.at() + 1) *
                       (i->maxIVs.df() - i->minIVs.df() + 1) *
                       (i->maxIVs.sa() - i->minIVs.sa() + 1) *
                       (i->maxIVs.sd() - i->minIVs.sd() + 1) *
                       (i->maxIVs.sp() - i->minIVs.sp() + 1);
    
    ivMatches += ivSets * i->inheritancePatterns /
                 /* total inheritance patterns * 6c3 */ 160UL;
  }
  
  uint64_t  result = ivMatches * multiplier / divisor;
  
  if (ivs.hiddenType != Element::NONE)
  {
    result = IVs::AdjustExpectedResultsForHiddenPower
      (result, ivs.min, ivs.max, ivs.hiddenType, ivs.minHiddenPower);
  }
  
  return result + 1;
}

void EggSeedSearcher::Search
  (const Criteria &criteria, const ResultCallback &resultHandler,
   const SearchRunner::ProgressCallback &progressHandler)
{
  HashedSeedGenerator   seedGenerator(criteria.seedParameters);
  IVFrameChecker        ivFrameChecker(criteria);
  IVFrameResultHandler  ivFrameResultHandler(criteria, resultHandler);
  SearchRunner          searcher;
  
  if ((criteria.ivs.GetPattern() == IVPattern::CUSTOM) ||
      Game::IsBlack2White2(criteria.seedParameters.version) ||
      (criteria.ivFrame.min != 8) || (criteria.ivFrame.max != 8) ||
      (LoadSeedCache() != LOADED))
  {
    IVFrameGeneratorFactory   ivFrameGenFactory;
    
    SeedFrameSearcher<IVFrameGeneratorFactory>  seedSearcher(ivFrameGenFactory,
                                                             criteria.ivFrame);
    
    searcher.SearchThreaded(seedGenerator, seedSearcher, ivFrameChecker,
                           ivFrameResultHandler, progressHandler);
  }
  else
  {
    SeedSearcher          seedSearcher(*s_IVSeedSet);
    
    searcher.SearchThreaded(seedGenerator, seedSearcher, ivFrameChecker,
                            ivFrameResultHandler, progressHandler);
    ReleaseSeedCache();
  }
}

void EggSeedSearcher::SetCacheDirectory(const std::string &dir)
{
  s_CacheDirectory = dir;
}

bool EggSeedSearcher::HasCacheFile()
{
  return HasNamedCacheFile("eggseeds.dat");
}

EggSeedSearcher::CacheLoadResult EggSeedSearcher::LoadSeedCache()
{
  boost::unique_lock<boost::mutex>  lock(s_CacheMutex);
  
  EggSeedSearcher::CacheLoadResult  result = LOADED;
  
  if (s_NumCacheReferences == 0)
  {
    result = LoadSeeds("eggseeds.dat", s_IVSeedSet);
    if (result != LOADED)
      return result;
  }
  
  ++s_NumCacheReferences;
  
  return result;
}

void EggSeedSearcher::ReleaseSeedCache()
{
  boost::unique_lock<boost::mutex>  lock(s_CacheMutex);
  
  if ((s_NumCacheReferences > 0) && (--s_NumCacheReferences == 0))
  {
    s_IVSeedSet.reset();
  }
}

void EggSeedSearcher::EnsureSeedCacheReleased()
{
  do
  {
    boost::unique_lock<boost::mutex>  lock(s_CacheMutex);
    
    if (s_NumCacheReferences == 0)
      break;
  }
  while (true);
}

}
