/*
  Copyright (C) 2012 chiizu
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

@interface B2W2ParameterSearcherController : VertResizeOnlyWindowController
{
  IBOutlet Gen5ConfigurationController  *gen5ConfigController;
  
  uint32_t  timer0Low, timer0High;
  uint32_t  vcountLow, vcountHigh;
  uint32_t  vframeLow, vframeHigh;
  
  NSDate    *startDate;
  uint32_t  startHour, startMinute, startSecond;
  uint32_t  button1, button2, button3;
  
  uint64_t  spinnerSequenceSearchValue;
  
  IBOutlet SearcherController     *searcherController;
}

@property uint32_t  timer0Low, timer0High;
@property uint32_t  vcountLow, vcountHigh;
@property uint32_t  vframeLow, vframeHigh;

@property (copy) NSDate  *startDate;

@property uint32_t  startHour, startMinute, startSecond;
@property uint32_t  button1, button2, button3;

@property uint64_t  spinnerSequenceSearchValue;

- (IBAction)addUpPosition:(id)sender;
- (IBAction)addUpRightPosition:(id)sender;
- (IBAction)addRightPosition:(id)sender;
- (IBAction)addDownRightPosition:(id)sender;
- (IBAction)addDownPosition:(id)sender;
- (IBAction)addDownLeftPosition:(id)sender;
- (IBAction)addLeftPosition:(id)sender;
- (IBAction)addUpLeftPosition:(id)sender;

- (IBAction)removeLastSearchItem:(id)sender;
- (IBAction)resetSearch:(id)sender;

@end
