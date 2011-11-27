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


@interface Gen4ConfigurationEditController : NSWindowController
{
  IBOutlet NSTableView         *configurationTableView;
  IBOutlet NSArrayController   *configurationArrayController;
  IBOutlet NSButton            *plusButton;
  IBOutlet NSButton            *minusButton;
  IBOutlet NSButton            *copyButton;
  
  
  IBOutlet NSTextField         *nameField;
  
  IBOutlet NSPopUpButton       *versionPopUp;
  
  IBOutlet NSTextField         *tidField;
  IBOutlet NSTextField         *sidField;
}

- (IBAction)addConfig:(id)sender;
- (IBAction)removeConfig:(id)sender;
- (IBAction)duplicateConfig:(id)sender;
- (IBAction)changeVersion:(id)sender;

- (IBAction)done:(id)sender;
- (IBAction)cancel:(id)sender;

- (BOOL)runModal;
- (BOOL)runModalWithConfigIndex:(NSInteger)configIndex;

@end
