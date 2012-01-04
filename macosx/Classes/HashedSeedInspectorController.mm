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



#import "HashedSeedInspectorController.h"

#include "HashedSeed.h"
#include "FrameGenerator.h"
#include "Utilities.h"

using namespace pprng;


@implementation HashedSeedInspectorController

- (NSString *)windowNibName
{
	return @"HashedSeedInspector";
}

- (void)awakeFromNib
{
  [super awakeFromNib];
  
  [[seedField formatter] setFormatWidth: 16];
  [startDate setObjectValue: [NSDate date]];
}

- (IBAction)calculateSeed:(id)sender
{
  using namespace boost::gregorian;
  using namespace boost::posix_time;
  
  HashedSeed::Parameters  p;
  
  p.version = [gen5ConfigController version];
  p.dsType = [gen5ConfigController dsType];
  p.macAddress = [gen5ConfigController macAddress];
  p.gxStat = HashedSeed::HardResetGxStat;
  p.vcount = [vcountField intValue];
  p.vframe = [vframeField intValue];
  p.timer0 = [timer0Field intValue];
  p.date = NSDateToBoostDate([startDate objectValue]);
  p.hour = [startHour intValue];
  p.minute = [startMinute intValue];
  p.second = [startSecond intValue];
  p.heldButtons = [[key1Menu selectedItem] tag] |
                  [[key2Menu selectedItem] tag] |
                  [[key3Menu selectedItem] tag];
  
  HashedSeed  seed(p);
  
  currentSeed = [NSData dataWithBytes: &seed length: sizeof(HashedSeed)];
  [adjacentsTabController setSeed: currentSeed];
  
  [seedField setObjectValue:
    [NSNumber numberWithUnsignedLongLong: seed.rawSeed]];
  [initialPIDFrameField setObjectValue:
    [NSNumber numberWithUnsignedInt: seed.GetSkippedPIDFrames() + 1]];
}


- (void)setSeed:(NSData*)seedData
{
  if (seedData != currentSeed)
  {
    currentSeed = seedData;
    [adjacentsTabController setSeed: seedData];
    
    HashedSeed  seed;
    [currentSeed getBytes: &seed length: sizeof(HashedSeed)];
    
    NSDate  *now = [NSDate date];
    NSRange  timeZoneOffsetRange;
    timeZoneOffsetRange.location = 20;
    timeZoneOffsetRange.length = 5;
    NSString  *dateTime =
      [NSString stringWithFormat: @"%.4d-%.2d-%.2d %.2d:%.2d:%.2d %@",
        seed.year(), seed.month(), seed.day(),
        seed.hour, seed.minute, seed.second,
        [[now description] substringWithRange: timeZoneOffsetRange]];
    
    [startDate setObjectValue: [NSDate dateWithString: dateTime]];
    [startHour setIntValue: seed.hour];
    [startMinute setIntValue: seed.minute];
    [startSecond setIntValue: seed.second];
    
    [timer0Field setIntValue: seed.timer0];
    [vcountField setIntValue: seed.vcount];
    [vframeField setIntValue: seed.vframe];
    
    NSPopUpButton  *keyMenu[3] = { key1Menu, key2Menu, key3Menu };
    uint32_t  i = 0;
    uint32_t  dpadPress = seed.heldButtons & Button::DPAD_MASK;
    uint32_t  buttonPress = seed.heldButtons & Button::SINGLE_BUTTON_MASK;
    
    if (dpadPress != 0)
    {
      [key1Menu selectItemWithTag: dpadPress];
      ++i;
    }
    
    uint32_t  j = 1;
    while ((buttonPress != 0) && (i < 3))
    {
      if (buttonPress & 0x1)
      {
        [keyMenu[i++] selectItemWithTag: j];
      }
      
      buttonPress >>= 1;
      j <<= 1;
    }
    
    [seedField setObjectValue:
      [NSNumber numberWithUnsignedLongLong: seed.rawSeed]];
    [initialPIDFrameField setObjectValue:
      [NSNumber numberWithUnsignedInt: seed.GetSkippedPIDFrames() + 1]];
  }
}

- (void)controlTextDidEndEditing:(NSNotification*)notification
{
  if ([[seedField stringValue] length] == 0)
  {
    [initialPIDFrameField setObjectValue: nil];
  }
  else
  {
    HashedSeed  seed([[seedField objectValue] unsignedLongLongValue]);
    [initialPIDFrameField setObjectValue:
        [NSNumber numberWithUnsignedInt: seed.GetSkippedPIDFrames() + 1]];
  }
}

@end
