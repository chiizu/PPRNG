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



#import "CGearSeedInspectorController.h"

#include "HashedSeed.h"
#include "FrameGenerator.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>

using namespace pprng;

@implementation CGearSeedInspectorController

- (NSString *)windowNibName
{
	return @"CGearSeedInspector";
}

- (void)awakeFromNib
{
  [super awakeFromNib];
  
  [[seedField formatter] setFormatWidth: 8];
  
  const char *dstr = [[[NSDate date] description] UTF8String];
  [timeFinderYearField
   setIntValue: boost::lexical_cast<uint32_t>(std::string(dstr, 4))];
  
  [timeFinderSecondField setIntValue: 0];
}


- (IBAction)generateIVFrames:(id)sender
{
  if ([[seedField stringValue] length] == 0)
  {
    return;
  }
  
  [ivFrameContentArray setContent: [NSMutableArray array]];
  
  uint32_t  seed = [[seedField objectValue] unsignedIntValue];
  uint32_t  minIVFrame = [minIVFrameField intValue];
  uint32_t  maxIVFrame = [maxIVFrameField intValue];
  uint32_t  frameNum = 0, limitFrame = minIVFrame - 1;
  
  CGearIVFrameGenerator  generator(seed, [ivParameterController isRoamer] ?
                                         CGearIVFrameGenerator::Roamer :
                                         CGearIVFrameGenerator::Normal);
  
  while (frameNum < limitFrame)
  {
    generator.AdvanceFrame();
    ++frameNum;
  }
  
  NSMutableArray  *rowArray =
    [NSMutableArray arrayWithCapacity: maxIVFrame - minIVFrame + 1];
  
  while (frameNum < maxIVFrame)
  {
    generator.AdvanceFrame();
    ++frameNum;
    
    CGearIVFrame  frame = generator.CurrentFrame();
    
    NSMutableDictionary  *result =
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
				[NSNumber numberWithUnsignedInt: frame.number], @"frame",
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
        nil];
    
    [rowArray addObject: result];
  }
  
  [ivFrameContentArray addObjects: rowArray];
}


- (IBAction)toggleTimeFinderSeconds:(id)sender
{
  BOOL enabled = [useSecondButton state];
  
  [timeFinderSecondField setEnabled: enabled];
}


- (IBAction)calculateTimes:(id)sender
{
  if ([[seedField stringValue] length] == 0)
  {
    return;
  }
  
  [timeFinderContentArray setContent: [NSMutableArray array]];
  
  CGearSeed  seed([[seedField objectValue] unsignedIntValue],
                  [gen5ConfigController macAddressLow]);
  
  uint32_t  wantedSecond;
  if ([useSecondButton state])
  {
    wantedSecond = [timeFinderSecondField intValue];
  }
  else
  {
    wantedSecond = -1;
  }
  
  TimeSeed::TimeElements  elements =
    seed.GetTimeElements([timeFinderYearField intValue], wantedSecond);
  
  NSMutableArray  *rows = [NSMutableArray arrayWithCapacity: elements.size()];
  TimeSeed::TimeElements::iterator  i;
  for (i = elements.begin(); i < elements.end(); ++i)
  {
    [rows addObject:
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
        [NSString stringWithFormat:@"%.4d/%.2d/%.2d",
            i->year, i->month, i->day],
        @"date",
        [NSString stringWithFormat:@"%.2d:%.2d:%.2d",
            i->hour, i->minute, i->second],
        @"time",
        [NSNumber numberWithInt: i->delay], @"delay",
        [NSData dataWithBytes: &(*i) length: sizeof(TimeSeed::TimeElement)],
        @"fullTime",
        nil]];
  }
  [timeFinderContentArray addObjects: rows];
  [timeFinderContentArray setSelectionIndex: 0];
}


- (IBAction)generateAdjacents:(id)sender
{
  using namespace boost::gregorian;
  using namespace boost::posix_time;
  
  if ([[seedField stringValue] length] == 0)
  {
    return;
  }
  
  NSInteger  rowNum = [timeFinderTableView selectedRow];
  if (rowNum < 0)
  {
    return;
  }
  NSDictionary  *row =
    [[timeFinderContentArray arrangedObjects] objectAtIndex: rowNum];
  NSData  *timeElementData = [row objectForKey: @"fullTime"];
  
  TimeSeed::TimeElement  timeElement;
  [timeElementData getBytes:&timeElement length:sizeof(TimeSeed::TimeElement)];
  
  [adjacentsContentArray setContent: [NSMutableArray array]];
  
  uint32_t   macAddressLow = [gen5ConfigController macAddressLow];
  CGearSeed  seed([[seedField objectValue] unsignedIntValue], macAddressLow);
  
  uint32_t  delayVariance = [adjacentsDelayVarianceField intValue];
  uint32_t  secondVariance = [adjacentsTimeVarianceField intValue];
  uint32_t  ivFrameNum = [adjacentsIVFrameField intValue];
  bool      isRoamer = [adjacentsRoamerButton state];
  
  uint32_t  targetDelay = timeElement.delay;
  uint32_t  endDelay = targetDelay + delayVariance;
  
  ptime  targetTime(date(timeElement.year, timeElement.month, timeElement.day),
                    hours(timeElement.hour) + minutes(timeElement.minute) +
                    seconds(timeElement.second));
  
  ptime  endTime = targetTime + seconds(secondVariance);
  targetTime = targetTime - seconds(secondVariance);
  
  NSMutableArray  *rowArray =
    [NSMutableArray arrayWithCapacity:
      ((2 * delayVariance) + 1) * ((2 * secondVariance) + 1)];
  
  for (; targetTime <= endTime; targetTime = targetTime + seconds(1))
  {
    date           d = targetTime.date();
    time_duration  t = targetTime.time_of_day();
    
    NSString  *dateStr =
      [NSString stringWithFormat: @"%.4d/%.2d/%.2d",
                uint32_t(d.year()), uint32_t(d.month()), uint32_t(d.day())];
    NSString  *timeStr = [NSString stringWithFormat:@"%.2d:%.2d:%.2d",
                           t.hours(), t.minutes(), t.seconds()];
    
    for (uint32_t delay = targetDelay - delayVariance;
         delay <= endDelay;
         ++delay)
    {
      CGearSeed  s(d.year(), d.month(), d.day(),
                   t.hours(), t.minutes(), t.seconds(),
                   delay, macAddressLow);
      
      CGearIVFrameGenerator  ivGenerator(s.m_rawSeed,
                                         (isRoamer ?
                                          CGearIVFrameGenerator::Roamer :
                                          CGearIVFrameGenerator::Normal));
      
      for (uint32_t f = 0; f < ivFrameNum; ++f)
        ivGenerator.AdvanceFrame();
      
      IVs  ivs = ivGenerator.CurrentFrame().ivs;
      
      [rowArray addObject:
        [NSMutableDictionary dictionaryWithObjectsAndKeys:
          [NSNumber numberWithUnsignedInt: s.m_rawSeed], @"seed",
          dateStr, @"date",
          timeStr, @"time",
          [NSNumber numberWithUnsignedInt: delay], @"delay",
          [NSNumber numberWithUnsignedInt: ivs.hp()], @"hp",
          [NSNumber numberWithUnsignedInt: ivs.at()], @"atk",
          [NSNumber numberWithUnsignedInt: ivs.df()], @"def",
          [NSNumber numberWithUnsignedInt: ivs.sa()], @"spa",
          [NSNumber numberWithUnsignedInt: ivs.sd()], @"spd",
          [NSNumber numberWithUnsignedInt: ivs.sp()], @"spe",
          nil]];
    }
  }
  
  [adjacentsContentArray addObjects: rowArray];
}


- (void)setSeed:(uint32_t)seed
{
  [seedField setObjectValue: [NSNumber numberWithUnsignedInt: seed]];
}

@end
