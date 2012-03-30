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
#import "SearcherController.h"
#import "Gen5ConfigurationController.h"

@interface TrainerIDSearcherController : VertResizeOnlyWindowController
{
  IBOutlet Gen5ConfigurationController  *gen5ConfigController;
  
  NSNumber  *ivSeed;
  
  BOOL      startFromInitialPIDFrame;
  uint32_t  minPIDFrame, maxPIDFrame;
  
  IBOutlet NSTableView        *pidFrameTableView;
  IBOutlet NSArrayController  *pidFrameContentArray;
  
  
  NSDate    *fromDate, *toDate;
  BOOL      noButtonHeld, oneButtonHeld, twoButtonsHeld, threeButtonsHeld;
  
  uint32_t  minTIDFrame, maxTIDFrame;
  BOOL      wildShiny, giftShiny, eggShiny;
  
  NSNumber  *desiredTID;
  
  IBOutlet SearcherController  *tidSidSearcherController;
  
  NSNumber  *foundTID;
  NSDate    *startDate;
  
  BOOL      considerHour;
  uint32_t  startHour;
  
  BOOL      considerMinute;
  uint32_t  startMinute;
  
  BOOL      considerSecond;
  uint32_t  startSecond;
  
  uint32_t  button1, button2, button3;
  
  uint32_t  minFoundTIDFrame, maxFoundTIDFrame;
  
  IBOutlet SearcherController  *idFrameSearcherController;
}

@property (copy) NSNumber  *ivSeed;
  
@property BOOL      startFromInitialPIDFrame;
@property uint32_t  minPIDFrame, maxPIDFrame;

@property (copy) NSDate  *fromDate, *toDate;
@property BOOL  noButtonHeld, oneButtonHeld, twoButtonsHeld, threeButtonsHeld;

@property uint32_t  minTIDFrame, maxTIDFrame;
@property BOOL      wildShiny, giftShiny, eggShiny;
@property (copy) NSNumber  *desiredTID;

@property (copy) NSNumber  *foundTID;
@property (copy) NSDate    *startDate;
  
@property BOOL      considerHour;
@property uint32_t  startHour;
  
@property BOOL      considerMinute;
@property uint32_t  startMinute;
  
@property BOOL      considerSecond;
@property uint32_t  startSecond;
  
@property uint32_t  button1, button2, button3;
  
@property uint32_t  minFoundTIDFrame, maxFoundTIDFrame;

- (IBAction) generatePIDFrames:(id)sender;

@end
