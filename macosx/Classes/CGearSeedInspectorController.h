/*
  Copyright (C) 2011-2012 chiizu
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
#import "SearcherController.h"
#import "IVParameterController.h"

@interface CGearSeedInspectorController : VertResizeOnlyWindowController
{
  IBOutlet Gen5ConfigurationController  *gen5ConfigController;
  
  NSNumber  *seed, *baseDelay;
  
  uint32_t  minIVFrame, maxIVFrame;
  
  IBOutlet IVParameterController  *ivFrameParameterController;
  
  IBOutlet NSTableView            *ivFrameTableView;
  IBOutlet NSArrayController      *ivFrameContentArray;
  
  
  uint32_t  year;
  NSNumber  *actualDelay;
  
  BOOL      considerSeconds;
  uint32_t  second;
  
  IBOutlet NSTableView            *timeFinderTableView;
  IBOutlet NSArrayController      *timeFinderContentArray;
  
  
  uint32_t  adjacentsDelayVariance, adjacentsTimeVariance;
  uint32_t  adjacentsMinIVFrame, adjacentsMaxIVFrame;
  
  IBOutlet IVParameterController  *adjacentsIVParameterController;
  
  IBOutlet NSTableView            *adjacentsTableView;
  IBOutlet NSArrayController      *adjacentsContentArray;
  
  
  BOOL      noButtonHeld, oneButtonHeld, twoButtonsHeld, threeButtonsHeld;
  
  IBOutlet NSPopUpButton  *natureDropDown;
  BOOL      isGenderless;
  uint32_t  minClusterSize;
  int32_t   secondsAdjustment;
  
  BOOL      startFromInitialPIDFrame;
  uint32_t  minPIDFrame, maxPIDFrame;
  
  IBOutlet NSTableView         *naturesTableView;
  IBOutlet NSArrayController   *naturesContentArray;
  
  IBOutlet SearcherController  *searcherController;
}

@property (copy) NSNumber  *seed, *baseDelay;

@property uint32_t  minIVFrame, maxIVFrame;
@property IVParameterController  *ivFrameParameterController;

- (IBAction)generateIVFrames:(id)sender;

- (void)selectAndShowFrame:(uint32_t)frame;

@property uint32_t         year;
@property (copy) NSNumber  *actualDelay;

@property BOOL      considerSeconds;
@property uint32_t  second;

- (IBAction)calculateTimes:(id)sender;

@property uint32_t  adjacentsDelayVariance, adjacentsTimeVariance;
@property uint32_t  adjacentsMinIVFrame, adjacentsMaxIVFrame;
@property IVParameterController  *adjacentsIVParameterController;

- (IBAction)generateAdjacents:(id)sender;
- (IBAction)findAdjacent:(id)sender;

@property BOOL  noButtonHeld, oneButtonHeld, twoButtonsHeld, threeButtonsHeld;

@property BOOL      isGenderless;
@property uint32_t  minClusterSize;
@property int32_t   secondsAdjustment;

@property BOOL      startFromInitialPIDFrame;
@property uint32_t  minPIDFrame, maxPIDFrame;

- (IBAction)toggleDropDownChoice:(id)sender;

@end
