/*
  Copyright (C) 2011 chiizu
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
  IBOutlet NSPopUpButton  *ivPatternMenu;
  IBOutlet NSPopUpButton  *hiddenTypeMenu;
  
  uint32_t  minHP, minAT, minDF, minSA, minSD, minSP;
  uint32_t  maxHP, maxAT, maxDF, maxSA, maxSD, maxSP;
  BOOL      considerHiddenPower;
  uint32_t  hiddenPowerType;
  uint32_t  minHiddenPower;
  BOOL      isRoamer;
}

@property uint32_t  minHP, minAT, minDF, minSA, minSD, minSP;
@property uint32_t  maxHP, maxAT, maxDF, maxSA, maxSD, maxSP;
@property BOOL      considerHiddenPower;
@property uint32_t  hiddenPowerType;
@property uint32_t  minHiddenPower;
@property BOOL      isRoamer;

- (IBAction)switchIVPattern:(id)sender;

- (pprng::IVs)minIVs;
- (void)setMinIVs:(pprng::IVs)ivs;

- (pprng::IVs)maxIVs;
- (void)setMaxIVs:(pprng::IVs)ivs;

- (pprng::Element::Type)hiddenType;
- (void)setHiddenType:(pprng::Element::Type)type;

- (pprng::IVPattern::Type)ivPattern;
- (void)setIVPattern:(pprng::IVPattern::Type)ivPattern;

- (uint32_t)numberOfIVCombinations;

@end
