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



#import "WonderCardSeedInspectorController.h"

#include "HashedSeed.h"
#include "FrameGenerator.h"
#include "Utilities.h"

using namespace pprng;

@implementation WonderCardSeedInspectorController

- (NSString *)windowNibName
{
	return @"WonderCardSeedInspector";
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
  
  date  d = NSDateToBoostDate([startDate objectValue]);
  
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
                   HashedSeed::NazoForVersion(version),
                   vcount, timer0, HashedSeed::GxStat, vframe, pressedKeys,
                   0, 0, 0, 0, 0, 0, 0, 0x40);
  
  currentSeed = [NSData dataWithBytes: &seed length: sizeof(HashedSeed)];
  
  [seedField setObjectValue:
    [NSNumber numberWithUnsignedLongLong: seed.m_rawSeed]];
}


- (IBAction)generateFrames:(id)sender
{
  if ([[seedField stringValue] length] == 0)
  {
    return;
  }
  
  [frameContentArray setContent: [NSMutableArray array]];
  
  HashedSeed  seed([[seedField objectValue] unsignedLongLongValue]);
  
  uint32_t  minFrame = [minFrameField intValue];
  uint32_t  maxFrame = [maxFrameField intValue];
  uint32_t  frameNum = 0, limitFrame = minFrame - 1;
  uint32_t  tid = [gen5ConfigController tid];
  uint32_t  sid = [gen5ConfigController sid];
  
  WonderCardFrameGenerator  generator(seed, false, tid, sid);
  
  while (frameNum < limitFrame)
  {
    generator.AdvanceFrame();
    ++frameNum;
  }
  
  NSMutableArray  *rowArray =
    [NSMutableArray arrayWithCapacity: maxFrame - minFrame + 1];
  
  while (frameNum < maxFrame)
  {
    generator.AdvanceFrame();
    ++frameNum;
    
    WonderCardFrame  frame = generator.CurrentFrame();
    
    [rowArray addObject:
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
				[NSNumber numberWithUnsignedInt: frame.number], @"frame",
        [NSNumber numberWithUnsignedInt: frame.pid.word], @"pid",
        [NSString stringWithFormat: @"%s",
          Nature::ToString(frame.nature).c_str()], @"nature",
        [NSNumber numberWithUnsignedInt: frame.pid.Gen5Ability()], @"ability",
        [NSNumber numberWithUnsignedInt: frame.ivs.hp()], @"hp",
        [NSNumber numberWithUnsignedInt: frame.ivs.at()], @"atk",
        [NSNumber numberWithUnsignedInt: frame.ivs.df()], @"def",
        [NSNumber numberWithUnsignedInt: frame.ivs.sa()], @"spa",
        [NSNumber numberWithUnsignedInt: frame.ivs.sd()], @"spd",
        [NSNumber numberWithUnsignedInt: frame.ivs.sp()], @"spe",
        [NSString stringWithFormat: @"%s",
          Element::ToString(frame.ivs.HiddenType()).c_str()], @"hiddenType",
        [NSNumber numberWithUnsignedInt: frame.ivs.HiddenPower()],
          @"hiddenPower",
        nil]];
  }
  
  [frameContentArray addObjects: rowArray];
}


- (IBAction)generateAdjacents:(id)sender
{
  using namespace boost::gregorian;
  using namespace boost::posix_time;
  
  if (currentSeed == nil)
  {
    return;
  }
  
  HashedSeed  targetSeed;
  [currentSeed getBytes: &targetSeed length: sizeof(HashedSeed)];
  
  if (targetSeed.m_rawSeed != [[seedField objectValue] unsignedLongLongValue])
  {
    return;
  }
  
  [adjacentsContentArray setContent: [NSMutableArray array]];
  
  uint32_t  tid = [gen5ConfigController tid];
  uint32_t  sid = [gen5ConfigController sid];
  uint32_t  timer0Low = [gen5ConfigController timer0Low];
  uint32_t  timer0High = [gen5ConfigController timer0High];
  
  if ((targetSeed.m_timer0 < timer0Low) || (targetSeed.m_timer0 > timer0High))
  {
    timer0Low = targetSeed.m_timer0 - 1;
    timer0High = targetSeed.m_timer0 + 1;
  }
  
  uint32_t  secondVariance = [adjacentsTimeVarianceField intValue];
  uint32_t  frameNum = [adjacentsFrameField intValue];
  uint32_t  frameVariance = [adjacentsFrameVarianceField intValue];
  uint32_t  startFrameNum;
  if (frameNum < frameVariance)
  {
    startFrameNum = 1;
  }
  else
  {
    startFrameNum = frameNum - frameVariance;
  }
  uint32_t  endFrameNum = frameNum + frameVariance;
  
  ptime  dt(date(targetSeed.m_year, targetSeed.m_month, targetSeed.m_day),
            hours(targetSeed.m_hour) + minutes(targetSeed.m_minute) +
            seconds(targetSeed.m_second));
  ptime  endTime = dt + seconds(secondVariance);
  dt = dt - seconds(secondVariance);
  
  NSMutableArray  *rowArray =
    [NSMutableArray arrayWithCapacity:
      (timer0High - timer0Low + 1) * ((2 * secondVariance) + 1)];
  
  for (; dt <= endTime; dt = dt + seconds(1))
  {
    date           d = dt.date();
    time_duration  t = dt.time_of_day();
    
    NSString  *dateStr =
      [NSString stringWithFormat: @"%.4d/%.2d/%.2d",
                uint32_t(d.year()), uint32_t(d.month()), uint32_t(d.day())];
    NSString  *timeStr = [NSString stringWithFormat:@"%.2d:%.2d:%.2d",
                           t.hours(), t.minutes(), t.seconds()];
    
    for (uint32_t timer0 = timer0Low; timer0 <= timer0High; ++timer0)
    {
      HashedSeed  seed(d.year(), d.month(), d.day(), d.day_of_week(),
                       t.hours(), t.minutes(), t.seconds(),
                       targetSeed.m_macAddressLow, targetSeed.m_macAddressHigh,
                       targetSeed.m_nazo,
                       targetSeed.m_vcount, timer0, HashedSeed::GxStat,
                       targetSeed.m_vframe, targetSeed.m_keyInput,
                       0, 0, 0, 0, 0, 0, 0, 0x40);
      
      WonderCardFrameGenerator  generator(seed, false, tid, sid);
      
      for (uint32_t j = 0; j < startFrameNum; ++j)
        generator.AdvanceFrame();
      
      for (frameNum = startFrameNum;
           frameNum <= endFrameNum;
           ++frameNum)
      {
        WonderCardFrame  frame = generator.CurrentFrame();
        
        [rowArray addObject:
        [NSMutableDictionary dictionaryWithObjectsAndKeys:
          dateStr, @"date",
          timeStr, @"time",
          [NSNumber numberWithUnsignedInt: timer0], @"timer0",
          [NSNumber numberWithUnsignedInt: frameNum], @"frame",
          [NSNumber numberWithUnsignedInt: frame.pid.word], @"pid",
          [NSString stringWithFormat: @"%s",
            Nature::ToString(frame.nature).c_str()], @"nature",
          [NSNumber numberWithUnsignedInt: frame.pid.Gen5Ability()], @"ability",
          [NSNumber numberWithUnsignedInt: frame.ivs.hp()], @"hp",
          [NSNumber numberWithUnsignedInt: frame.ivs.at()], @"atk",
          [NSNumber numberWithUnsignedInt: frame.ivs.df()], @"def",
          [NSNumber numberWithUnsignedInt: frame.ivs.sa()], @"spa",
          [NSNumber numberWithUnsignedInt: frame.ivs.sd()], @"spd",
          [NSNumber numberWithUnsignedInt: frame.ivs.sp()], @"spe",
          nil]];
        
        generator.AdvanceFrame();
      }
    }
  }
  
  [adjacentsContentArray addObjects: rowArray];
}


- (void)setSeed:(NSData*)seedData
{
  if (seedData != currentSeed)
  {
    currentSeed = seedData;
    
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
