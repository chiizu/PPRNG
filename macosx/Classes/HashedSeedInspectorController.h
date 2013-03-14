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
#import "SearchResultProtocols.h"

#include "HashedSeed.h"

@interface HashedSeedInspectorController : VertResizeOnlyWindowController
{
  NSArray   *configurationNames;
  
  pprng::DS::Type  dsType;
  
  NSNumber  *macAddress0, *macAddress1, *macAddress2;
  NSNumber  *macAddress3, *macAddress4, *macAddress5;
  
  pprng::Game::Version  version;
  
  NSNumber  *tid, *sid;
  
  BOOL      memoryLinkUsed, hasShinyCharm;
  
  NSDate    *startDate;
  NSNumber  *startHour, *startMinute, *startSecond;
  NSNumber  *timer0, *vcount, *vframe;
  
  uint32_t  button1, button2, button3;
  
  NSNumber  *rawSeed;
  NSNumber  *initialPIDFrame;
}

@property (copy) NSArray   *configurationNames;

@property        pprng::DS::Type  dsType;
@property (copy) NSNumber  *macAddress0, *macAddress1, *macAddress2;
@property (copy) NSNumber  *macAddress3, *macAddress4, *macAddress5;

@property        pprng::Game::Version  version;
@property (copy) NSNumber  *tid, *sid;
@property        BOOL      memoryLinkUsed, hasShinyCharm;

@property (copy) NSDate    *startDate;
@property (copy) NSNumber  *startHour, *startMinute, *startSecond;
@property (copy) NSNumber  *timer0, *vcount, *vframe;

@property        uint32_t  button1, button2, button3;

@property (copy) NSNumber  *rawSeed;
@property (copy) NSNumber  *initialPIDFrame;

- (IBAction) configChanged:(id)sender;
- (IBAction) seedParameterChanged:(id)sender;
- (IBAction) memoryLinkUsedChanged:(id)sender;
- (IBAction) seedValueChanged:(id)sender;

- (uint64_t)macAddress;
- (void)setMACAddress:(uint64_t)macAddress;

- (void)setSeedFromResult:(id <HashedSeedResultParameters>)result;
- (void)setSeed:(const pprng::HashedSeed&)seed;

@end
