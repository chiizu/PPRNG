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

#import "VertResizeOnlyWindowController.h"
#import "SearcherController.h"
#import "IVParameterController.h"
#import "Gen5ConfigurationController.h"

#include "PPRNGTypes.h"

@interface WonderCardSeedSearcherController : VertResizeOnlyWindowController
{
  IBOutlet Gen5ConfigurationController  *gen5ConfigController;
  IBOutlet IVParameterController        *ivParameterController;
  IBOutlet SearcherController           *searcherController;
  
  IBOutlet NSPopUpButton  *natureDropDown;
  
  NSDate    *fromDate, *toDate;
  BOOL      noButtonHeld, oneButtonHeld, twoButtonsHeld, threeButtonsHeld;
  
  pprng::Nature::Type               cardNature;
  pprng::Ability::Type              cardAbility;
  pprng::Gender::Type               cardGender;
  pprng::Gender::Ratio              cardGenderRatio;
  pprng::WonderCardShininess::Type  cardShininess;
  NSNumber                          *cardTID, *cardSID;
  
  BOOL  natureSearchable, abilitySearchable;
  BOOL  genderSearchable, shininessSearchable;
  
  BOOL                  showShinyOnly;
  pprng::Ability::Type  ability;
  pprng::Gender::Type   gender;
  
  BOOL      startFromInitialFrame;
  uint32_t  minFrame, maxFrame;
}

@property (copy) NSDate  *fromDate, *toDate;

@property BOOL  noButtonHeld, oneButtonHeld, twoButtonsHeld, threeButtonsHeld;

@property pprng::Nature::Type               cardNature;
@property pprng::Ability::Type              cardAbility;
@property pprng::Gender::Type               cardGender;
@property pprng::Gender::Ratio              cardGenderRatio;
@property pprng::WonderCardShininess::Type  cardShininess;
@property (copy) NSNumber                   *cardTID, *cardSID;

@property BOOL  natureSearchable, abilitySearchable;
@property BOOL  genderSearchable, shininessSearchable;

@property BOOL                  showShinyOnly;
@property pprng::Ability::Type  ability;
@property pprng::Gender::Type   gender;

@property BOOL      startFromInitialFrame;
@property uint32_t  minFrame, maxFrame;

- (IBAction)toggleDropDownChoice:(id)sender;

@end
