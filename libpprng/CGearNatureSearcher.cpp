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


#include "CGearNatureSearcher.h"
#include "SeedSearcher.h"
#include "FrameGenerator.h"

#include <list>
#include <memory>

namespace pprng
{

namespace
{

struct CGearNatureSeed
{
  CGearNatureSeed(const CGearSeed::TimeElement &te, const HashedSeed &hs)
    : cgearTime(te), natureSeed(hs)
  {}
  
  CGearSeed::TimeElement  cgearTime;
  HashedSeed              natureSeed;
};


class NatureSeedGenerator
{
public:
  typedef CGearNatureSeed  SeedType;
  typedef uint32_t         SeedCountType;
  
  enum { SeedsPerChunk = 1000 };
  
  NatureSeedGenerator(const CGearNatureSearcher::Criteria &criteria)
    : m_timesList(ChooseTimes(criteria)), m_iter(m_timesList.begin()),
      m_parameters(SetTimes(criteria.hashedSeedParameters, m_timesList)),
      m_generator(new HashedSeedGenerator(m_parameters)),
      m_numGeneratorSeeds(m_parameters.NumberOfSeeds()),
      m_seedNum(0)
  {}
  
  SeedCountType NumberOfSeeds() const
  {
    return m_timesList.size() * m_numGeneratorSeeds;
  }
  
  SeedType Next()
  {
    if (m_seedNum++ >= m_numGeneratorSeeds)
    {
      ++m_iter;
      m_parameters.fromTime = m_parameters.toTime = m_iter->natureTime;
      m_generator.reset(new HashedSeedGenerator(m_parameters));
      
      m_seedNum = 1;
    }
    
    return CGearNatureSeed(m_iter->cgearTime, m_generator->Next());
  }
  
private:
  struct Times
  {
    CGearSeed::TimeElement    cgearTime;
    boost::posix_time::ptime  natureTime;
  };
  
  typedef std::list<Times>  TimesList;
  
  static TimesList ChooseTimes(const CGearNatureSearcher::Criteria &criteria)
  {
    using namespace boost::posix_time;
    using namespace boost::gregorian;
    
    TimesList  result;
    
    CGearSeed  cgearSeed(criteria.cgearSeed,
                         criteria.hashedSeedParameters.macAddress.low);
    
    TimeSeed::TimeElements  cgearTimes =
      cgearSeed.GetTimeElements(criteria.year);
    
    TimeSeed::TimeElements::iterator  i = cgearTimes.begin();
    while (i != cgearTimes.end())
    {
      ptime  cgearTime(date(i->year, i->month, i->day),
                       hours(i->hour) + minutes(i->minute) +
                         seconds(i->second));
      
      ptime  natureTime = cgearTime - milliseconds((i->delay * 1000) / 60);
      if (criteria.secondsAdjustment < 0)
        natureTime = natureTime - seconds(-1 * criteria.secondsAdjustment);
      else
        natureTime = natureTime + seconds(criteria.secondsAdjustment);
      
      if (natureTime.date() == cgearTime.date())
      {
        Times  times;
        times.cgearTime = *i;
        times.natureTime = natureTime;
        
        result.push_back(times);
      }
      
      ++i;
    }
    
    return result;
  }
  
  static HashedSeedGenerator::Parameters
    SetTimes(const HashedSeedGenerator::Parameters &parameters,
             const TimesList &timesList)
  {
    HashedSeedGenerator::Parameters  p(parameters);
    
    if (timesList.size() > 0)
    {
      p.fromTime = p.toTime = timesList.begin()->natureTime;
    }
    
    return p;
  }
  
  const TimesList                           m_timesList;
  TimesList::const_iterator                 m_iter;
  HashedSeedGenerator::Parameters           m_parameters;
  std::auto_ptr<HashedSeedGenerator>        m_generator;
  const HashedSeedGenerator::SeedCountType  m_numGeneratorSeeds;
  HashedSeedGenerator::SeedCountType        m_seedNum;
};


class NatureFrameGenerator
{
public:
  typedef CGearNatureSeed                        Seed;
  typedef CGearNatureSearcher::CGearNatureFrame  Frame;
  
  NatureFrameGenerator(const CGearNatureSeed &cgearNatureSeed,
                       const Gen5PIDFrameGenerator::Parameters &pidParameters,
                       const SearchCriteria::PIDCriteria &pidCriteria,
                       uint32_t maxFrame)
    : m_pidFrameGenerator(cgearNatureSeed.natureSeed, pidParameters),
      m_pidCriteria(pidCriteria), m_maxFrame(maxFrame),
      m_frame(cgearNatureSeed.natureSeed, cgearNatureSeed.cgearTime)
  {
    m_frame.number = 0;
    m_frame.nature = Nature::NONE;
    m_frame.clusterSize = 0;
  }
  
  void SkipFrames(uint32_t numFrames)
  {
    m_pidFrameGenerator.SkipFrames(numFrames);
    m_frame.number += numFrames;
  }
  
  void AdvanceFrame()
  {
    m_frame.nature = Nature::NONE;
    m_frame.clusterSize = 0;
    
    uint32_t  lastFrame = 0;
    
    do
    {
      m_pidFrameGenerator.AdvanceFrame();
      
      Gen5PIDFrame  frame = m_pidFrameGenerator.CurrentFrame();
      
      if (m_pidCriteria.CheckNature(frame.nature))
      {
        if (m_frame.nature == Nature::NONE)
        {
          m_frame.nature = frame.nature;
          m_frame.number = frame.number;
        }
        else if (m_frame.nature != frame.nature)
        {
          m_frame.nature = Nature::MIXED;
        }
        
        ++m_frame.clusterSize;
        lastFrame = frame.number;
      }
      else if ((lastFrame > 0) && ((frame.number - lastFrame) > 1))
      {
        break;
      }
    }
    while (m_pidFrameGenerator.CurrentFrame().number < m_maxFrame);
  }
  
  const Frame& CurrentFrame() { return m_frame; }
  
private:
  HashedSeed                         m_seed;
  Gen5PIDFrameGenerator              m_pidFrameGenerator;
  const SearchCriteria::PIDCriteria  &m_pidCriteria;
  const uint32_t                     m_maxFrame;
  Frame                              m_frame;
};


struct FrameChecker
{
  FrameChecker(const CGearNatureSearcher::Criteria &criteria)
    : m_criteria(criteria)
  {}
  
  bool operator()(const CGearNatureSearcher::CGearNatureFrame &frame) const
  {
    return frame.clusterSize >= m_criteria.minClusterSize;
  }
  
  const CGearNatureSearcher::Criteria  &m_criteria;
};

struct FrameGeneratorFactory
{
  typedef NatureFrameGenerator  FrameGenerator;
  
  FrameGeneratorFactory(const CGearNatureSearcher::Criteria &criteria)
    : m_pidParameters(MakePIDParameters(criteria)), m_pidCriteria(criteria.pid),
      m_maxFrame(criteria.frameRange.max)
  {}
  
  NatureFrameGenerator operator()(const CGearNatureSeed &seed) const
  {
    return NatureFrameGenerator(seed, m_pidParameters, m_pidCriteria,
                                m_maxFrame);
  }
  
  static Gen5PIDFrameGenerator::Parameters
    MakePIDParameters(const CGearNatureSearcher::Criteria &criteria)
  {
    Gen5PIDFrameGenerator::Parameters  p;
    
    p.frameType = Gen5PIDFrameGenerator::EntraLinkFrame;
    p.leadAbility = EncounterLead::OTHER;
    p.targetGender = criteria.pid.gender;
    p.targetRatio = criteria.pid.genderRatio;
    p.tid = 0;
    p.sid = 0;
    p.startFromLowestFrame = criteria.pid.startFromLowestFrame;
    
    return p;
  }
  
  Gen5PIDFrameGenerator::Parameters  m_pidParameters;
  const SearchCriteria::PIDCriteria  &m_pidCriteria;
  const uint32_t                     m_maxFrame;
};

}

uint64_t CGearNatureSearcher::Criteria::ExpectedNumberOfResults() const
{
  return 1;
}

void CGearNatureSearcher::Search
  (const Criteria &criteria, const ResultCallback &resultHandler,
   const SearchRunner::ProgressCallback &progressHandler)
{
  NatureSeedGenerator       seedGenerator(criteria);
  
  FrameGeneratorFactory     frameGenFactory(criteria);
  
  // slightly hacky...
  SearchCriteria::FrameRange  frameRange
    (criteria.pid.startFromLowestFrame ? 1 : criteria.frameRange.min,
     criteria.frameRange.max);
  
  SeedFrameSearcher<FrameGeneratorFactory>  seedSearcher(frameGenFactory,
                                                         frameRange);
  FrameChecker              frameChecker(criteria);
  
  SearchRunner              searcher;
  
  searcher.Search(seedGenerator, seedSearcher, frameChecker,
                  resultHandler, progressHandler);
}

}
