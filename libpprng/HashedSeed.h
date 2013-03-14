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

#ifndef NON_C_GEAR_SEED_H
#define NON_C_GEAR_SEED_H

#include "PPRNGTypes.h"
#include "LinearCongruentialRNG.h"

#include <boost/date_time/gregorian/gregorian.hpp>

namespace pprng
{

class HashedSeed
{
public:
  struct Parameters
  {
    Game::Version           version;
    DS::Type                dsType;
    uint64_t                macAddress;
    uint32_t                timer0, vcount, vframe;
    boost::gregorian::date  date;
    uint32_t                hour, minute, second;
    uint32_t                heldButtons;
    
    Parameters()
      : version(Game::Version(0)), dsType(DS::Type(0)), macAddress(),
        timer0(0), vcount(0), vframe(0), date(),
        hour(0), minute(0), second(0), heldButtons(0)
    {}
    
    explicit Parameters(Game::Version version_)
      : version(version_), dsType(DS::Type(0)), macAddress(),
        timer0(0), vcount(0), vframe(0), date(),
        hour(0), minute(0), second(0), heldButtons(0)
    {}
  };
  
  explicit HashedSeed(const Parameters &parameters);
  
  // to be used when raw seed is already calculated (see UnhashedSeed)
  HashedSeed(const Parameters &parameters_, uint64_t rawSeed)
    : parameters(parameters_), rawSeed(rawSeed),
      m_skippedPIDFramesCalculated(false),
      m_skippedPIDFramesMemoryLinkUsed(false), m_skippedPIDFrames(0),
      m_skippedPIDFramesSeed(0)
  {}
  
  // to be used when only raw seed is known / important
  HashedSeed(Game::Version version, uint64_t rawSeed)
    : parameters(version), rawSeed(rawSeed),
      m_skippedPIDFramesCalculated(false),
      m_skippedPIDFramesMemoryLinkUsed(false), m_skippedPIDFrames(0),
      m_skippedPIDFramesSeed(0)
  {}
  
  // this is needed to decode a HashedSeed from a byte array
  HashedSeed()
    : parameters(), rawSeed(0),
      m_skippedPIDFramesCalculated(false),
      m_skippedPIDFramesMemoryLinkUsed(false), m_skippedPIDFrames(0),
      m_skippedPIDFramesSeed(0)
  {}
  
  const Parameters  parameters;
  
  // calculated raw seed
  const uint64_t    rawSeed;
  
  uint32_t year() const { return parameters.date.year(); }
  uint32_t month() const { return parameters.date.month(); }
  uint32_t day() const { return parameters.date.day(); }
  
  uint32_t SeedAndSkipPIDFrames(LCRNG5 &rng, bool memoryLinkUsed) const;
  
  uint32_t GetSkippedPIDFrames(bool memoryLinkUsed) const;
  
private:
  // skipped frames calculated lazily and cached
  mutable bool      m_skippedPIDFramesCalculated;
  mutable bool      m_skippedPIDFramesMemoryLinkUsed;
  mutable uint32_t  m_skippedPIDFrames;
  mutable uint64_t  m_skippedPIDFramesSeed;
};

}

#endif
