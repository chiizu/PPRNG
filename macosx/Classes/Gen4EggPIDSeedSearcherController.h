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
#import "Gen4ConfigurationController.h"

#include "PPRNGTypes.h"

@interface Gen4EggPIDSeedSearcherController : VertResizeOnlyWindowController
{
  IBOutlet Gen4ConfigurationController  *gen4ConfigController;
  IBOutlet SearcherController           *searcherController;
  
  IBOutlet NSPopUpButton  *natureDropDown;
  
  int                   mode;
  
  BOOL                  internationalParents;
  uint32_t              minDelay, maxDelay;
  
  BOOL                  allowTaps;
  uint32_t              maxTaps;
  uint32_t              maxFrame;
  
  BOOL                  shinyOnly;
  pprng::Ability::Type  ability;
  pprng::Gender::Type   gender;
  pprng::Gender::Ratio  genderRatio;
}

@property int                   mode;
@property BOOL                  internationalParents;
@property uint32_t              minDelay, maxDelay;
@property BOOL                  allowTaps;
@property uint32_t              maxTaps;
@property uint32_t              maxFrame;
@property BOOL                  shinyOnly;
@property pprng::Ability::Type  ability;
@property pprng::Gender::Type   gender;
@property pprng::Gender::Ratio  genderRatio;

- (IBAction)toggleDropDownChoice:(id)sender;

@end
