/*
  Copyright (C) 2012 chiizu
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
#include "FrameGenerator.h"

@interface DreamRadarSeedSearcherController : VertResizeOnlyWindowController
{
  IBOutlet Gen5ConfigurationController  *gen5ConfigController;
  IBOutlet IVParameterController        *ivParameterController;
  IBOutlet SearcherController           *searcherController;
  
  IBOutlet NSPopUpButton  *natureDropDown;
  
  NSDate    *fromDate, *toDate;
  BOOL      noButtonHeld, oneButtonHeld, twoButtonsHeld, threeButtonsHeld;
  uint32_t  minFrame, maxFrame;
  
  pprng::DreamRadarFrameGenerator::FrameType  type;
  
  BOOL                  genderRequired;
  pprng::Gender::Type   gender;
  pprng::Gender::Ratio  genderRatio;
  
  BOOL                  numPrecedingGenderlessRequired;
  uint32_t              slot, numPrecedingGenderless, maxNumPrecedingGenderless;
}

@property (copy) NSDate  *fromDate, *toDate;

@property BOOL  noButtonHeld, oneButtonHeld, twoButtonsHeld, threeButtonsHeld;

@property uint32_t  minFrame, maxFrame;

@property pprng::DreamRadarFrameGenerator::FrameType  type;

@property BOOL                  genderRequired;
@property pprng::Gender::Type   gender;
@property pprng::Gender::Ratio  genderRatio;

@property BOOL      numPrecedingGenderlessRequired;
@property uint32_t  slot, numPrecedingGenderless, maxNumPrecedingGenderless;

- (IBAction)toggleDropDownChoice:(id)sender;

@end
