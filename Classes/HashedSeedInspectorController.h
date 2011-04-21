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
#import "Gen5ConfigurationController.h"
#import "IVParameterController.h"

@interface HashedSeedInspectorController : VertResizeOnlyWindowController
{
  IBOutlet Gen5ConfigurationController  *gen5ConfigController;
  
  IBOutlet NSDatePicker           *startDate;
  IBOutlet NSTextField            *startHour;
  IBOutlet NSTextField            *startMinute;
  IBOutlet NSTextField            *startSecond;
  
  IBOutlet NSTextField            *timer0Field;
  IBOutlet NSTextField            *vcountField;
  IBOutlet NSTextField            *vframeField;
  
  IBOutlet NSPopUpButton          *key1Menu;
  IBOutlet NSPopUpButton          *key2Menu;
  IBOutlet NSPopUpButton          *key3Menu;
  
  
  IBOutlet NSTextField            *seedField;
  
  
  IBOutlet NSPopUpButton          *pidFrameTypeMenu;
  IBOutlet NSTextField            *minPIDFrameField;
  IBOutlet NSTextField            *maxPIDFrameField;
  
  IBOutlet NSTableView            *pidFrameTableView;
  IBOutlet NSArrayController      *pidFrameContentArray;
  
  IBOutlet NSPopUpButton          *natureMenu;
  IBOutlet NSButton               *shinyCheckBox;
  
  
  IBOutlet NSTextField            *minIVFrameField;
  IBOutlet NSTextField            *maxIVFrameField;
  
  IBOutlet NSTableView            *ivFrameTableView;
  IBOutlet NSArrayController      *ivFrameContentArray;
  
  IBOutlet IVParameterController  *ivParameterController;
  
  
  IBOutlet NSTextField            *adjacentsTimeVarianceField;
  
  IBOutlet NSTextField            *adjacentsIVFrameField;
  IBOutlet NSButton               *adjacentsRoamerButton;
  
  IBOutlet NSTextField            *adjacentsPIDFrameField;
  IBOutlet NSTextField            *adjacentsPIDFrameVarianceField;
  IBOutlet NSPopUpButton          *adjacentsPIDFrameTypeMenu;
  
  IBOutlet NSTableView            *adjacentsTableView;
  IBOutlet NSArrayController      *adjacentsContentArray;
  
  
  IBOutlet NSButton               *eggsInternationalButton;
  IBOutlet NSButton               *eggsUseEverstoneButton;
  IBOutlet NSButton               *eggsUseDittoButton;
  
  IBOutlet NSButton               *eggsEnableIVsButton;
  IBOutlet NSTextField            *eggsIVFrameField;
  
  IBOutlet NSTextField            *eggsMinPIDFrameField;
  IBOutlet NSTextField            *eggsMaxPIDFrameField;
  
  IBOutlet NSTableView            *eggsTableView;
  IBOutlet NSArrayController      *eggsContentArray;
  
  
  NSData  *currentSeed;
}

- (IBAction)calculateSeed:(id)sender;
- (IBAction)generatePIDFrames:(id)sender;
- (IBAction)generateIVFrames:(id)sender;
- (IBAction)generateAdjacents:(id)sender;
- (IBAction)toggleEggIVs:(id)sender;
- (IBAction)generateEggs:(id)sender;

// take HashedSeed from NSData pointer
- (void)setSeed:(NSData*)seedData;

@end
