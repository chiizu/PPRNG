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
  
  IBOutlet NSTextField            *eggsMinPIDFrameField;
  IBOutlet NSTextField            *eggsMaxPIDFrameField;
  
  IBOutlet NSButton               *eggsEnableParentIVsCheckBox;
  
  IBOutlet NSTextField            *eggsFemaleHPField;
  IBOutlet NSStepper              *eggsFemaleHPStepper;
  IBOutlet NSTextField            *eggsFemaleAtkField;
  IBOutlet NSStepper              *eggsFemaleAtkStepper;
  IBOutlet NSTextField            *eggsFemaleDefField;
  IBOutlet NSStepper              *eggsFemaleDefStepper;
  IBOutlet NSTextField            *eggsFemaleSpAField;
  IBOutlet NSStepper              *eggsFemaleSpAStepper;
  IBOutlet NSTextField            *eggsFemaleSpDField;
  IBOutlet NSStepper              *eggsFemaleSpDStepper;
  IBOutlet NSTextField            *eggsFemaleSpeField;
  IBOutlet NSStepper              *eggsFemaleSpeStepper;
  
  IBOutlet NSTextField            *eggsMaleHPField;
  IBOutlet NSStepper              *eggsMaleHPStepper;
  IBOutlet NSTextField            *eggsMaleAtkField;
  IBOutlet NSStepper              *eggsMaleAtkStepper;
  IBOutlet NSTextField            *eggsMaleDefField;
  IBOutlet NSStepper              *eggsMaleDefStepper;
  IBOutlet NSTextField            *eggsMaleSpAField;
  IBOutlet NSStepper              *eggsMaleSpAStepper;
  IBOutlet NSTextField            *eggsMaleSpDField;
  IBOutlet NSStepper              *eggsMaleSpDStepper;
  IBOutlet NSTextField            *eggsMaleSpeField;
  IBOutlet NSStepper              *eggsMaleSpeStepper;
  
  IBOutlet NSTableView            *eggsTableView;
  IBOutlet NSArrayController      *eggsContentArray;
}

- (IBAction)toggleEggIVs:(id)sender;
- (IBAction)toggleParentIVs:(id)sender;
- (IBAction)generateEggs:(id)sender;

@end
