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

@interface CGearSeedInspectorController : VertResizeOnlyWindowController
{
  IBOutlet Gen5ConfigurationController  *gen5ConfigController;
  
  IBOutlet NSTextField            *seedField;
  
  
  IBOutlet NSTextField            *minIVFrameField;
  IBOutlet NSTextField            *maxIVFrameField;
  
  IBOutlet NSTableView            *ivFrameTableView;
  IBOutlet NSArrayController      *ivFrameContentArray;
  
  IBOutlet IVParameterController  *ivFrameParameterController;
  
  
  IBOutlet NSTextField            *timeFinderYearField;
  IBOutlet NSButton               *useSecondButton;
  IBOutlet NSTextField            *timeFinderSecondField;
  
  IBOutlet NSTableView            *timeFinderTableView;
  IBOutlet NSArrayController      *timeFinderContentArray;
  
  
  IBOutlet NSTextField            *adjacentsMinIVFrameField;
  IBOutlet NSTextField            *adjacentsMaxIVFrameField;
  IBOutlet NSButton               *adjacentsRoamerButton;
  
  IBOutlet NSTextField            *adjacentsDelayVarianceField;
  IBOutlet NSTextField            *adjacentsTimeVarianceField;
  
  IBOutlet IVParameterController  *adjacentsIVParameterController;
  
  IBOutlet NSTableView            *adjacentsTableView;
  IBOutlet NSArrayController      *adjacentsContentArray;
}

- (IBAction)toggleTimeFinderSeconds:(id)sender;

- (IBAction)generateIVFrames:(id)sender;
- (IBAction)calculateTimes:(id)sender;
- (IBAction)generateAdjacents:(id)sender;
- (IBAction)findAdjacent:(id)sender;

- (void)setSeed:(uint32_t)seed;

@end
