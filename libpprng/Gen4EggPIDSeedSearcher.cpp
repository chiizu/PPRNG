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


#include "Gen4EggPIDSeedSearcher.h"
#include "SeedSearcher.h"
#include "SearchCriteria.h"

namespace pprng
{

namespace
{

struct FrameChecker
{
  FrameChecker(const Gen4EggPIDSeedSearcher::Criteria &criteria)
    : m_criteria(criteria)
  {}
  
  bool operator()(const Gen4EggPIDFrame &frame) const
  {
    return CheckShiny(frame.pid) && CheckNature(frame.pid) &&
           CheckAbility(frame.pid) && CheckGender(frame.pid);
  }
  
  bool CheckShiny(const PID &pid) const
  {
    return !m_criteria.shinyOnly ||
           pid.IsShiny(m_criteria.frameParameters.tid,
                       m_criteria.frameParameters.sid);
  }
  
  bool CheckNature(const PID &pid) const
  {
    return m_criteria.pid.CheckNature(pid.Gen34Nature());
  }
  
  bool CheckAbility(const PID &pid) const
  {
    return (m_criteria.pid.ability == Ability::ANY) ||
           (m_criteria.pid.ability == pid.Gen34Ability());
  }
  
  bool CheckGender(const PID &pid) const
  {
    return Gender::GenderValueMatches(pid.GenderValue(),
                                      m_criteria.pid.gender,
                                      m_criteria.pid.genderRatio);
  }
  
  const Gen4EggPIDSeedSearcher::Criteria  &m_criteria;
};

struct FrameGeneratorFactory
{
  typedef Gen4EggPIDFrameGenerator  FrameGenerator;
  
  FrameGeneratorFactory(const Gen4EggPIDFrameGenerator::Parameters &params)
    : m_parameters(params)
  {}
  
  Gen4EggPIDFrameGenerator operator()(uint32_t seed) const
  {
    return Gen4EggPIDFrameGenerator(seed, m_parameters);
  }
  
  const Gen4EggPIDFrameGenerator::Parameters  &m_parameters;
};

}

uint64_t Gen4EggPIDSeedSearcher::Criteria::ExpectedNumberOfResults()
{
  uint64_t  delays = delay.max - delay.min + 1;
  
  uint64_t  numSeeds = delays * 256 * 24;
  
  uint64_t  numFrames = frame.max - frame.min + 1;
  
  uint64_t  shinyMultiplier, shinyDivisor;
  if (shinyOnly)
  {
    shinyMultiplier = frameParameters.internationalParents ? 4 : 1;
    shinyDivisor = 8192;
  }
  else
  {
    shinyMultiplier = shinyDivisor = 1;
  }
  
  uint64_t  natureMultiplier = pid.NumNatures();
  uint64_t  natureDivisor = 25;
  
  uint64_t  numResults =
    numFrames * numSeeds * natureMultiplier * shinyMultiplier /
    (shinyDivisor * natureDivisor);
  
  return numResults;
}

void Gen4EggPIDSeedSearcher::Search
  (const Criteria &criteria, const ResultCallback &resultHandler,
   const SearchRunner::ProgressCallback &progressHandler)
{
  TimeSeedGenerator      seedGenerator(criteria.delay.min, criteria.delay.max);
  FrameGeneratorFactory  frameGeneratorFactory(criteria.frameParameters);
  
  SeedFrameSearcher<FrameGeneratorFactory>  seedSearcher(frameGeneratorFactory,
                                                         criteria.frame);
  
  FrameChecker           frameChecker(criteria);
  
  SearchRunner           searcher;
  
  searcher.Search(seedGenerator, seedSearcher, frameChecker,
                  resultHandler, progressHandler);
}

}
