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
#import "IVParameterController.h"

@interface HashedSeedInspectorFramesTabController : NSObject
{
  IBOutlet NSWindow                     *owner;
  
  IBOutlet Gen5ConfigurationController  *gen5ConfigController;
  
  IBOutlet NSTextField                  *seedField;
  
  IBOutlet NSPopUpButton                *pidFrameTypeMenu;
  IBOutlet NSButton                     *useCompoundEyesCheckBox;
  IBOutlet NSButton                     *useInitialPIDButton;
  IBOutlet NSTextField                  *minPIDFrameField;
  IBOutlet NSTextField                  *maxPIDFrameField;
  
  IBOutlet NSTableView                  *pidFrameTableView;
  IBOutlet NSArrayController            *pidFrameContentArray;
  
  IBOutlet NSButton                     *shinyCheckBox;
  IBOutlet NSPopUpButton                *natureMenu;
  
  
  IBOutlet NSTextField                  *minIVFrameField;
  IBOutlet NSTextField                  *maxIVFrameField;
  
  IBOutlet NSTableView                  *ivFrameTableView;
  IBOutlet NSArrayController            *ivFrameContentArray;
  
  IBOutlet IVParameterController        *ivParameterController;
}

- (IBAction)toggleUseInitialPID:(id)sender;
- (IBAction)generatePIDFrames:(id)sender;
- (IBAction)generateIVFrames:(id)sender;

@end
