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
#import "SearcherController.h"
#import "Gen5ConfigurationController.h"

@interface TrainerIDSearcherController : VertResizeOnlyWindowController
{
  IBOutlet Gen5ConfigurationController  *gen5ConfigController;
  
  
  IBOutlet NSTextField                  *pidSeedField;
  IBOutlet NSTextField                  *pidMinFrameField;
  IBOutlet NSTextField                  *pidMaxFrameField;
  
  IBOutlet NSTableView                  *pidFrameTableView;
  IBOutlet NSArrayController            *pidFrameContentArray;
  
  
  IBOutlet NSDatePicker                 *tidSidFromDateField;
  IBOutlet NSDatePicker                 *tidSidToDateField;
  
  IBOutlet NSButton                     *tidSidNoKeyHeldButton;
  IBOutlet NSButton                     *tidSidOneKeyHeldButton;
  IBOutlet NSButton                     *tidSidTwoKeysHeldButton;
  IBOutlet NSButton                     *tidSidThreeKeysHeldButton;
  
  IBOutlet NSTextField                  *tidSidMinFrameField;
  IBOutlet NSTextField                  *tidSidMaxFrameField;
  
  IBOutlet NSButton                     *tidSidWildShinyButton;
  IBOutlet NSButton                     *tidSidGiftShinyButton;
  IBOutlet NSButton                     *tidSidEggShinyButton;
  
  IBOutlet NSButton                     *tidSidEnableDesiredTidButton;
  IBOutlet NSTextField                  *tidSidDesiredTidField;
  
  IBOutlet SearcherController           *tidSidSearcherController;
  
  
  IBOutlet NSTextField                  *idFrameTrainerIDField;
  
  IBOutlet NSDatePicker                 *idFrameDateField;
  
  IBOutlet NSButton                     *idFrameEnableHourButton;
  IBOutlet NSTextField                  *idFrameStartHour;
  IBOutlet NSStepper                    *idFrameHourStepper;
  
  IBOutlet NSButton                     *idFrameEnableMinuteButton;
  IBOutlet NSTextField                  *idFrameStartMinute;
  IBOutlet NSStepper                    *idFrameMinuteStepper;
  
  IBOutlet NSButton                     *idFrameEnableSecondButton;
  IBOutlet NSTextField                  *idFrameStartSecond;
  IBOutlet NSStepper                    *idFrameSecondStepper;
  
  IBOutlet NSPopUpButton                *idFrameKeyOnePopUp;
  IBOutlet NSPopUpButton                *idFrameKeyTwoPopUp;
  IBOutlet NSPopUpButton                *idFrameKeyThreePopUp;
  
  IBOutlet NSTextField                  *idFrameMinFrameField;
  IBOutlet NSTextField                  *idFrameMaxFrameField;
  
  IBOutlet SearcherController           *idFrameSearcherController;
}

- (IBAction) generatePIDFrames:(id)sender;
- (IBAction) toggleTID:(id)sender;
- (IBAction) toggleTime:(id)sender;

@end
