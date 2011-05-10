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

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>

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
  
  const char *dstr = [[[startDate objectValue] description] UTF8String];
  date  d(boost::lexical_cast<uint32_t>(std::string(dstr, 4)),
          boost::lexical_cast<uint32_t>(std::string(dstr + 5, 2)),
          boost::lexical_cast<uint32_t>(std::string(dstr + 8, 2)));
  
  time_duration t
    (hours([startHour intValue]) +
     minutes([startMinute intValue]) +
     seconds([startSecond intValue]));
  
  uint32_t  macAddressLow = [gen5ConfigController macAddressLow];
  uint32_t  macAddressHigh = [gen5ConfigController macAddressHigh];
  
  Game::Version  version = [gen5ConfigController version];
  
  uint32_t  timer0 = [timer0Field intValue];
  uint32_t  vcount = [vcountField intValue];
  uint32_t  vframe = [vframeField intValue];
  
  uint32_t  pressedKeys = [[key1Menu selectedItem] tag] |
                          [[key2Menu selectedItem] tag] |
                          [[key3Menu selectedItem] tag];
  
  HashedSeed  seed(d.year(), d.month(), d.day(), d.day_of_week(),
                   t.hours(), t.minutes(), t.seconds(),
                   macAddressLow, macAddressHigh,
                   HashedSeed::NazoForVersion(version), 0, 0, 0,
                   vcount, timer0, HashedSeed::GxStat, vframe, pressedKeys);
  
  currentSeed = [NSData dataWithBytes: &seed length: sizeof(HashedSeed)];
  [adjacentsTabController setSeed: currentSeed];
  
  [seedField setObjectValue:
    [NSNumber numberWithUnsignedLongLong: seed.m_rawSeed]];
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
        seed.m_year, seed.m_month, seed.m_day,
        seed.m_hour, seed.m_minute, seed.m_second,
        [[now description] substringWithRange: timeZoneOffsetRange]];
    
    [startDate setObjectValue: [NSDate dateWithString: dateTime]];
    [startHour setIntValue: seed.m_hour];
    [startMinute setIntValue: seed.m_minute];
    [startSecond setIntValue: seed.m_second];
    
    [timer0Field setIntValue: seed.m_timer0];
    [vcountField setIntValue: seed.m_vcount];
    [vframeField setIntValue: seed.m_vframe];
    
    NSPopUpButton  *keyMenu[3] = { key1Menu, key2Menu, key3Menu };
    uint32_t  i = 0;
    uint32_t  dpadPress = seed.m_keyInput & Button::DPAD_MASK;
    uint32_t  buttonPress = seed.m_keyInput & Button::SINGLE_BUTTON_MASK;
    
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
      [NSNumber numberWithUnsignedLongLong: seed.m_rawSeed]];
  }
}

@end
