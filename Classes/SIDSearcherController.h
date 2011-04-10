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

@interface SIDSearcherController : VertResizeOnlyWindowController
{
  IBOutlet Gen5ConfigurationController  *gen5ConfigController;
  IBOutlet SearcherController           *searcherController;
  
  IBOutlet NSDatePicker         *dateField;
  
  IBOutlet NSButton             *enableHourButton;
  IBOutlet NSTextField          *startHour;
  IBOutlet NSStepper            *hourStepper;
  
  IBOutlet NSButton             *enableMinuteButton;
  IBOutlet NSTextField          *startMinute;
  IBOutlet NSStepper            *minuteStepper;
  
  IBOutlet NSButton             *enableSecondButton;
  IBOutlet NSTextField          *startSecond;
  IBOutlet NSStepper            *secondStepper;
  
  IBOutlet NSTextField          *minFrameField;
  IBOutlet NSTextField          *maxFrameField;
}

- (IBAction) toggleTime:(id)sender;

@end
