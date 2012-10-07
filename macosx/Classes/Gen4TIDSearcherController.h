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
#import "SearcherController.h"

@interface Gen4TIDSearcherController : VertResizeOnlyWindowController
{
  NSNumber  *shinyPID;
  NSNumber  *desiredTID;
  NSNumber  *desiredSID;
  
  uint32_t  minSearchDelay, maxSearchDelay;
  
  IBOutlet SearcherController  *tidSidSearcherController;
  
  NSNumber  *foundTID;
  
  NSDate    *startDate;
  
  uint32_t  startHour;
  uint32_t  startMinute;
  uint32_t  startSecond;
  
  uint32_t  minFoundDelay, maxFoundDelay;
  
  IBOutlet SearcherController  *foundTIDSearcherController;
}

@property (copy) NSNumber  *shinyPID, *desiredTID, *desiredSID;
  
@property uint32_t  minSearchDelay, maxSearchDelay;

@property (copy) NSNumber  *foundTID;

@property (copy) NSDate    *startDate;

@property uint32_t  startHour, startMinute, startSecond;
  
@property uint32_t  minFoundDelay, maxFoundDelay;

@end
