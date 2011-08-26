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
#import "IVParameterController.h"
#import "Gen5ConfigurationController.h"


@interface EggSeedSearcherController : VertResizeOnlyWindowController
{
  IBOutlet Gen5ConfigurationController  *gen5ConfigController;
  
  IBOutlet NSDatePicker           *fromDateField;
  IBOutlet NSDatePicker           *toDateField;
  
  IBOutlet NSButton               *noKeyHeldButton;
  IBOutlet NSButton               *oneKeyHeldButton;
  IBOutlet NSButton               *twoKeysHeldButton;
  IBOutlet NSButton               *threeKeysHeldButton;
  
  IBOutlet NSTextField            *femaleHPField;
  IBOutlet NSTextField            *femaleAtkField;
  IBOutlet NSTextField            *femaleDefField;
  IBOutlet NSTextField            *femaleSpAField;
  IBOutlet NSTextField            *femaleSpDField;
  IBOutlet NSTextField            *femaleSpeField;
  
  IBOutlet NSTextField            *maleHPField;
  IBOutlet NSTextField            *maleAtkField;
  IBOutlet NSTextField            *maleDefField;
  IBOutlet NSTextField            *maleSpAField;
  IBOutlet NSTextField            *maleSpDField;
  IBOutlet NSTextField            *maleSpeField;
  
  IBOutlet NSPopUpButton          *femaleSpeciesPopUp;
  IBOutlet NSButton               *everstoneButton;
  IBOutlet NSButton               *dittoButton;
  IBOutlet NSButton               *internationalButton;
  
  IBOutlet NSButton               *useInitialPIDFrameCheckBox;
  IBOutlet NSTextField            *minPIDFrameField;
  IBOutlet NSTextField            *maxPIDFrameField;
  
  IBOutlet NSTextField            *minIVFrameField;
  IBOutlet NSTextField            *maxIVFrameField;
  
  IBOutlet NSPopUpButton          *naturePopUp;
  IBOutlet NSPopUpButton          *abilityPopUp;
  IBOutlet NSButton               *dreamworldButton;
  IBOutlet NSButton               *shinyButton;
  IBOutlet NSPopUpButton          *speciesPopUp;
  IBOutlet NSPopUpButton          *genderPopUp;
  IBOutlet NSPopUpButton          *genderRatioPopUp;
  
  IBOutlet IVParameterController  *ivParameterController;
  
  IBOutlet SearcherController     *searcherController;
}

- (IBAction)onFemaleSpeciesChange:(id)sender;
- (IBAction)toggleSearchFromStartFrame:(id)sender;

@end
