/*
  Copyright (C) 2011-2012 chiizu
  chiizu.pprng@gmail.com
  
  This file is part of PPRNG.
  
  PPRNG is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  PPRNG is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with PPRNG.  If not, see <http://www.gnu.org/licenses/>.
*/



#import <Cocoa/Cocoa.h>
#include "PPRNGTypes.h"

@interface IVParameterController : NSObject
{
  pprng::IVPattern::Type  ivPattern;
  BOOL                    isSettingPattern;
  uint32_t                minHP, minAT, minDF, minSA, minSD, minSP;
  uint32_t                maxHP, maxAT, maxDF, maxSA, maxSD, maxSP;
  BOOL                    considerHiddenPower;
  pprng::Element::Type    hiddenType;
  uint32_t                minHiddenPower;
  BOOL                    isRoamer;
}

@property pprng::IVPattern::Type  ivPattern;
@property uint32_t                minHP, minAT, minDF, minSA, minSD, minSP;
@property uint32_t                maxHP, maxAT, maxDF, maxSA, maxSD, maxSP;
@property BOOL                    considerHiddenPower;
@property pprng::Element::Type    hiddenType;
@property uint32_t                minHiddenPower;
@property BOOL                    isRoamer;

@property pprng::IVs  minIVs, maxIVs;

- (uint32_t)numberOfIVCombinations;

@end
