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
#import "SearcherController.h"

@interface DSParameterSearcherController : VertResizeOnlyWindowController
{
  IBOutlet Gen5ConfigurationController  *gen5ConfigController;
  
  IBOutlet NSTextField            *timer0LowField;
  IBOutlet NSTextField            *timer0HighField;
  IBOutlet NSTextField            *vcountLowField;
  IBOutlet NSTextField            *vcountHighField;
  IBOutlet NSTextField            *frameLowField;
  IBOutlet NSTextField            *frameHighField;
  
  IBOutlet IVParameterController  *ivParameterController;
  
  IBOutlet NSDatePicker           *startDate;
  IBOutlet NSTextField            *startHour;
  IBOutlet NSTextField            *startMinute;
  IBOutlet NSTextField            *startSecond;
  
  IBOutlet NSPopUpButton          *keyOnePopUp;
  IBOutlet NSPopUpButton          *keyTwoPopUp;
  IBOutlet NSPopUpButton          *keyThreePopUp;
  
  IBOutlet SearcherController     *searcherController;
}

@end
