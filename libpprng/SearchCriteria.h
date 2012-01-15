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
    Nature::Type   nature;
    Ability::Type  ability;
    Gender::Type   gender;
    Gender::Ratio  genderRatio;
    bool           searchFromInitialFrame;
    
    PIDCriteria()
      : nature(Nature::ANY), ability(Ability::ANY),
        gender(Gender::ANY), genderRatio(Gender::UNSPECIFIED),
        searchFromInitialFrame(false)
    {}
  };
  
  struct IVCriteria
  {
    bool           shouldCheckMax;
    IVs            min, max;
    Element::Type  hiddenType;
    uint32_t       minHiddenPower;
    bool           isRoamer;
    
    IVCriteria()
      : shouldCheckMax(true), min(), max(),
        hiddenType(Element::UNKNOWN), minHiddenPower(30), isRoamer(false)
    {}
  };
  
  struct FrameRange
  {
    FrameRange() : min(0), max(0) {}
    FrameRange(uint32_t mi, uint32_t ma) : min(mi), max(ma) {}
    
    uint32_t  min;
    uint32_t  max;
  };
  
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
