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

@interface WonderCardSeedInspectorController : VertResizeOnlyWindowController
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
  IBOutlet NSTextField            *initialPIDFrameField;
  
  IBOutlet NSPopUpButton          *typePopUp;
  IBOutlet NSTextField            *ivSkipField;
  IBOutlet NSTextField            *pidSkipField;
  IBOutlet NSTextField            *natureSkipField;
  IBOutlet NSButton               *fixedNatureCheckBox;
  IBOutlet NSButton               *fixedAbilityCheckBox;
  IBOutlet NSButton               *fixedGenderCheckBox;
  
  
  IBOutlet NSButton               *useInitialPIDButton;
  IBOutlet NSTextField            *minFrameField;
  IBOutlet NSTextField            *maxFrameField;
  
  IBOutlet NSTableView            *frameTableView;
  IBOutlet NSArrayController      *frameContentArray;
  
  IBOutlet IVParameterController  *ivParameterController;
  IBOutlet NSPopUpButton          *naturePopUp;
  IBOutlet NSPopUpButton          *abilityPopUp;
  IBOutlet NSPopUpButton          *genderPopUp;
  IBOutlet NSPopUpButton          *genderRatioPopUp;
  IBOutlet NSPopUpButton          *characteristicPopUp;
  
  
  IBOutlet NSTextField            *adjacentsTimeVarianceField;
  IBOutlet NSTextField            *adjacentsFrameVarianceField;
  IBOutlet NSTextField            *adjacentsFrameField;
  IBOutlet NSButton               *adjacentsUseInitialPIDOffsetButton;
  
  IBOutlet NSTableView            *adjacentsTableView;
  IBOutlet NSArrayController      *adjacentsContentArray;
  
  NSData  *currentSeed;
}

- (IBAction)calculateSeed:(id)sender;
- (IBAction)onTypeChange:(id)sender;
- (IBAction)toggleFixedNature:(id)sender;
- (IBAction)toggleFixedAbility:(id)sender;
- (IBAction)toggleFixedGender:(id)sender;
- (IBAction)toggleUseInitialPID:(id)sender;
- (IBAction)generateFrames:(id)sender;
- (IBAction)generateAdjacents:(id)sender;

// take HashedSeed from NSData pointer
- (void)setSeed:(NSData*)seedData;

// for updating initial PID frame when user types a new seed value
- (void)controlTextDidEndEditing:(NSNotification*)notification;

@end
