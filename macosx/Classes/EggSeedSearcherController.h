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
#import "SearcherController.h"
#import "IVParameterController.h"
#import "Gen5ConfigurationController.h"

#include "PPRNGTypes.h"

@interface EggSeedSearcherController : NSWindowController
{
  IBOutlet Gen5ConfigurationController  *gen5ConfigController;
  IBOutlet IVParameterController        *ivParameterController;
  IBOutlet SearcherController           *searcherController;
  
  IBOutlet NSPopUpButton  *natureDropDown;
  
  NSDate  *fromDate, *toDate;
  BOOL    noButtonHeld, oneButtonHeld, twoButtonsHeld, threeButtonsHeld;
  
  pprng::FemaleParent::Type  femaleSpecies;
  BOOL                       isNidoranFemale;
  
  NSNumber  *femaleHP, *femaleAT, *femaleDF, *femaleSA, *femaleSD, *femaleSP;
  NSNumber  *maleHP, *maleAT, *maleDF, *maleSA, *maleSD, *maleSP;
  
  BOOL      usingEverstone, usingDitto, internationalParents;
  
  BOOL      startFromInitialPIDFrame;
  uint32_t  minPIDFrame, maxPIDFrame;
  uint32_t  minIVFrame, maxIVFrame;
  
  BOOL                     showShinyOnly;
  BOOL                     considerEggSpecies;
  pprng::EggSpecies::Type  eggSpecies;
  pprng::Ability::Type     ability;
  BOOL                     inheritsHiddenAbility;
  pprng::Gender::Type      gender;
  pprng::Gender::Ratio     genderRatio;
  
  BOOL      seedCacheIsLoaded;
  NSString  *seedFileStatus;
}

@property (copy) NSDate  *fromDate, *toDate;

@property BOOL  noButtonHeld, oneButtonHeld, twoButtonsHeld, threeButtonsHeld;

@property pprng::FemaleParent::Type  femaleSpecies;
@property BOOL                       isNidoranFemale;

@property (copy)
NSNumber  *femaleHP, *femaleAT, *femaleDF, *femaleSA, *femaleSD, *femaleSP;

@property (copy)
NSNumber  *maleHP, *maleAT, *maleDF, *maleSA, *maleSD, *maleSP;

@property BOOL      usingEverstone, usingDitto, internationalParents;

@property BOOL      startFromInitialPIDFrame;
@property uint32_t  minPIDFrame, maxPIDFrame;
@property uint32_t  minIVFrame, maxIVFrame;

@property BOOL                     showShinyOnly;
@property BOOL                     considerEggSpecies;
@property pprng::EggSpecies::Type  eggSpecies;
@property pprng::Ability::Type     ability;
@property BOOL                     inheritsHiddenAbility;
@property pprng::Gender::Type      gender;
@property pprng::Gender::Ratio     genderRatio;

@property (copy) NSString  *seedFileStatus;

- (IBAction)toggleDropDownChoice:(id)sender;

@end
