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

#import "Gen5ConfigurationController.h"

@interface HashedSeedInspectorEggsTabController : NSObject
{
  IBOutlet Gen5ConfigurationController  *gen5ConfigController;
  
  IBOutlet NSTextField            *seedField;
  
  IBOutlet NSButton               *eggsInternationalButton;
  IBOutlet NSButton               *eggsUseEverstoneButton;
  IBOutlet NSButton               *eggsUseDittoButton;
  
  IBOutlet NSButton               *eggsEnableIVsButton;
  IBOutlet NSTextField            *eggsIVFrameField;
  
  IBOutlet NSButton               *eggsUseInitialPIDButton;
  IBOutlet NSTextField            *eggsMinPIDFrameField;
  IBOutlet NSTextField            *eggsMaxPIDFrameField;
  
  IBOutlet NSPopUpButton          *eggsFemaleSpeciesPopup;
  
  IBOutlet NSTableView            *eggsTableView;
  IBOutlet NSArrayController      *eggsContentArray;
  
  BOOL      enableParentIVs;
  NSNumber  *femaleHP, *femaleAT, *femaleDF, *femaleSA, *femaleSD, *femaleSP;
  NSNumber  *maleHP, *maleAT, *maleDF, *maleSA, *maleSD, *maleSP;
}

@property BOOL  enableParentIVs;

@property (copy)
NSNumber  *femaleHP, *femaleAT, *femaleDF, *femaleSA, *femaleSD, *femaleSP;

@property (copy)
NSNumber  *maleHP, *maleAT, *maleDF, *maleSA, *maleSD, *maleSP;

- (IBAction)toggleUseInitialPID:(id)sender;
- (IBAction)toggleEggIVs:(id)sender;
- (IBAction)generateEggs:(id)sender;

@end
