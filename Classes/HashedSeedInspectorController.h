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
#import "HashedSeedInspectorFramesTabController.h"
#import "HashedSeedInspectorAdjacentsTabController.h"
#import "HashedSeedInspectorEggsTabController.h"

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
  
  IBOutlet HashedSeedInspectorFramesTabController     *framesTabController;
  
  IBOutlet HashedSeedInspectorAdjacentsTabController  *adjacentsTabController;
  
  IBOutlet HashedSeedInspectorEggsTabController       *eggsTabController;
  
  NSData  *currentSeed;
}

- (IBAction)calculateSeed:(id)sender;

// take HashedSeed from NSData pointer
- (void)setSeed:(NSData*)seedData;

@end
