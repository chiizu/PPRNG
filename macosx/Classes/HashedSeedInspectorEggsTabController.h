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

#import "Gen5ConfigurationController.h"

@class StandardSeedInspectorController;

@interface HashedSeedInspectorEggsTabController : NSObject
{
  IBOutlet StandardSeedInspectorController  *inspectorController;
  
  BOOL      internationalParents;
  BOOL      usingEverstone;
  BOOL      usingDitto;
  
  BOOL      enableIVs;
  uint32_t  ivFrame;
  
  BOOL      startFromInitialPIDFrame;
  uint32_t  minPIDFrame, maxPIDFrame;
  
  pprng::FemaleParent::Type  femaleSpecies;
  
  BOOL      enableParentIVs;
  NSNumber  *femaleHP, *femaleAT, *femaleDF, *femaleSA, *femaleSD, *femaleSP;
  NSNumber  *maleHP, *maleAT, *maleDF, *maleSA, *maleSD, *maleSP;
  
  IBOutlet NSTableView            *eggsTableView;
  IBOutlet NSArrayController      *eggsContentArray;
}

@property BOOL      internationalParents;
@property BOOL      usingEverstone;
@property BOOL      usingDitto;

@property BOOL      enableIVs;
@property uint32_t  ivFrame;

@property BOOL      startFromInitialPIDFrame;
@property uint32_t  minPIDFrame, maxPIDFrame;

@property pprng::FemaleParent::Type  femaleSpecies;

@property BOOL  enableParentIVs;
@property (copy)
NSNumber  *femaleHP, *femaleAT, *femaleDF, *femaleSA, *femaleSD, *femaleSP,
          *maleHP, *maleAT, *maleDF, *maleSA, *maleSD, *maleSP;

- (void)setFemaleIVs:(const pprng::OptionalIVs&)ivs;
- (void)setMaleIVs:(const pprng::OptionalIVs&)ivs;

- (IBAction)generateEggs:(id)sender;

- (void)selectAndShowEggFrame:(uint32_t)frame;

@end
