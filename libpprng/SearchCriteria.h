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

#ifndef SEARCH_CRITERIA_H
#define SEARCH_CRITERIA_H

#include "PPRNGTypes.h"
#include <boost/lexical_cast.hpp>
#include <sstream>

namespace pprng
{

struct SearchCriteria
{
  struct PIDCriteria
  {
    uint32_t       natureMask;
    Ability::Type  ability;
    Gender::Type   gender;
    Gender::Ratio  genderRatio;
    bool           startFromLowestFrame;
    
    PIDCriteria()
      : natureMask(0), ability(Ability::ANY),
        gender(Gender::ANY), genderRatio(Gender::ANY_RATIO),
        startFromLowestFrame(false)
    {}
    
    bool CheckNature(Nature::Type nature) const
    {
      return (natureMask == 0) || (((0x1 << nature) & natureMask) != 0);
    }
    
    uint32_t NumNatures() const
    {
      uint32_t  c = 0, n = natureMask;
      while (n != 0)
      {
        n &= n - 1;
        ++c;
      }
      
      return (c == 0) ? 25 : c;
    }
  };
  
  struct IVCriteria
  {
    bool             shouldCheckMax;
    IVs              min, max;
    Element::Type    hiddenType;
    uint32_t         minHiddenPower;
    bool             isRoamer;
    
    IVCriteria()
      : shouldCheckMax(true), min(), max(),
        hiddenType(Element::ANY), minHiddenPower(30), isRoamer(false)
    {}
    
    IVPattern::Type GetPattern() const
    {
      return IVPattern::Get(min, max,
                            (hiddenType != Element::NONE), minHiddenPower);
    }
  };
  
  struct NumberRange
  {
    NumberRange() : min(0), max(0) {}
    NumberRange(uint32_t mi, uint32_t ma) : min(mi), max(ma) {}
    
    uint32_t  min;
    uint32_t  max;
  };
  
  typedef NumberRange  FrameRange;
  typedef NumberRange  DelayRange;
  
  class ImpossibleMinMaxFrameRangeException : public Exception
  {
  public:
    ImpossibleMinMaxFrameRangeException
      (uint32_t minFrame, uint32_t maxFrame, const std::string &frameType)
      : Exception
        ("Minimum " + frameType + " frame " +
         boost::lexical_cast<std::string>(minFrame) +
         " is not less than or equal to maximum " + frameType + " frame " +
         boost::lexical_cast<std::string>(maxFrame))
    {}
  };
  
  virtual uint64_t ExpectedNumberOfResults() const = 0;
};

}

#endif
