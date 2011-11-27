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
#import "VertResizeOnlyWindowController.h"
#import "SearcherController.h"
#import "Gen4ConfigurationController.h"
#import "IVParameterController.h"

@interface Gen4SeedSearcherController : VertResizeOnlyWindowController
{
  IBOutlet Gen4ConfigurationController  *gen4ConfigController;
  IBOutlet IVParameterController        *ivParameterController;
  IBOutlet SearcherController           *searcherController;
  
  IBOutlet NSPopUpButton  *naturePopup;
  IBOutlet NSPopUpButton  *abilityPopUp;
  IBOutlet NSPopUpButton  *genderPopUp;
  IBOutlet NSPopUpButton  *genderRatioPopUp;
  IBOutlet NSPopUpButton  *esvPopUp;
  
  int                     mode;
  
  BOOL                    shinyOnly;
  int                     nature;
  int                     ability;
  int                     gender;
  int                     genderRatio;
  
  uint32_t                minFrame;
  uint32_t                maxFrame;
  uint32_t                minDelay;
  uint32_t                maxDelay;
}

@property int       mode;
@property BOOL      shinyOnly;
@property int       nature;
@property int       ability;
@property int       gender;
@property int       genderRatio;
@property uint32_t  minFrame;
@property uint32_t  maxFrame;
@property uint32_t  minDelay;
@property uint32_t  maxDelay;

- (IBAction)toggleESVChoice:(id)sender;

@end
