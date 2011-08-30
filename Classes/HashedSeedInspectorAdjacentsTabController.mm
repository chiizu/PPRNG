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

#import "HashedSeedInspectorAdjacentsTabController.h"

#include "HashedSeed.h"
#include "FrameGenerator.h"
#include "Utilities.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>

using namespace pprng;

@implementation HashedSeedInspectorAdjacentsTabController

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
  uint32_t  timer0Low = targetSeed.m_timer0 - 1;
  uint32_t  timer0High = targetSeed.m_timer0 + 1;
  
  if (targetSeed.m_timer0 == 0)
  {
    timer0Low = 0;
  }
  if (targetSeed.m_timer0 == 0xffffffff)
  {
    timer0High = 0xffffffff;
  }
  
  uint32_t  secondVariance = [adjacentsTimeVarianceField intValue];
  
  ptime     seedTime(date(targetSeed.m_year, targetSeed.m_month,
                          targetSeed.m_day),
                     hours(targetSeed.m_hour) + minutes(targetSeed.m_minute) +
                     seconds(targetSeed.m_second));
  ptime     dt = seedTime;
  ptime     endTime = dt + seconds(secondVariance);
  dt = dt - seconds(secondVariance);
  
  uint32_t  ivFrameNum = [adjacentsIVFrameField intValue];
  bool      isRoamer = [adjacentsRoamerButton state];
  
  uint32_t  pidFrameNum = [adjacentsPIDFrameField intValue];
  BOOL      useInitialPIDOffset = [adjacentsUseInitialPIDOffsetButton state];
  uint32_t  pidFrameOffset = useInitialPIDOffset ?
       (pidFrameNum - targetSeed.GetSkippedPIDFrames() - 1) : pidFrameNum;
  uint32_t  pidFrameVariance = [adjacentsPIDFrameVarianceField intValue];
  
  Gen5PIDFrameGenerator::FrameType  frameType =
    static_cast<Gen5PIDFrameGenerator::FrameType>
      ([[adjacentsPIDFrameTypeMenu selectedItem] tag]);
  
  NSMutableArray  *rowArray =
    [NSMutableArray arrayWithCapacity:
      (timer0High - timer0Low + 1) * ((2 * secondVariance) + 1)];
  
  for (; dt <= endTime; dt = dt + seconds(1))
  {
    date           d = dt.date();
    time_duration  t = dt.time_of_day();
    
    NSString  *timeStr = (dt == seedTime) ?
      [NSString stringWithFormat:@"%.2d:%.2d:%.2d",
                                 t.hours(), t.minutes(), t.seconds()] :
      [NSString stringWithFormat:@"%+dsec", (dt - seedTime).total_seconds()];
    
    for (uint32_t timer0 = timer0Low; timer0 <= timer0High; ++timer0)
    {
      HashedSeed  seed(d.year(), d.month(), d.day(), d.day_of_week(),
                       t.hours(), t.minutes(), t.seconds(),
                       targetSeed.m_macAddressLow, targetSeed.m_macAddressHigh,
                       targetSeed.m_nazo,
                       targetSeed.m_vcount, timer0, HashedSeed::GxStat,
                       targetSeed.m_vframe, targetSeed.m_keyInput);
      
      HashedIVFrameGenerator  ivGenerator(seed,
                                          (isRoamer ?
                                           HashedIVFrameGenerator::Roamer :
                                           HashedIVFrameGenerator::Normal));
      
      for (uint32_t j = 0; j < ivFrameNum; ++j)
        ivGenerator.AdvanceFrame();
      
      IVs  ivs = ivGenerator.CurrentFrame().ivs;
      
      uint32_t  adjacentPIDFrameNum = useInitialPIDOffset ?
        (seed.GetSkippedPIDFrames() + 1 + pidFrameOffset) :
        pidFrameNum;
      uint32_t  pidStartFrameNum =
        (adjacentPIDFrameNum < (pidFrameVariance + 1)) ?
          1 : (adjacentPIDFrameNum - pidFrameVariance);
      uint32_t  pidEndFrameNum = adjacentPIDFrameNum + pidFrameVariance;
      
      Gen5PIDFrameGenerator  pidGenerator(seed, frameType, false, tid, sid);
      bool  generatesESV = pidGenerator.GeneratesESV();
      bool  generatesIsEncounter = pidGenerator.GeneratesIsEncounter();
      
      for (uint32_t j = 0; j < (pidStartFrameNum - 1); ++j)
        pidGenerator.AdvanceFrame();
      
      for (pidFrameNum = pidStartFrameNum;
           pidFrameNum <= pidEndFrameNum;
           ++pidFrameNum)
      {
        pidGenerator.AdvanceFrame();
        
        Gen5PIDFrame  frame = pidGenerator.CurrentFrame();
        uint32_t      genderValue = frame.pid.GenderValue();
        
        [rowArray addObject:
        [NSMutableDictionary dictionaryWithObjectsAndKeys:
          timeStr, @"time",
          [NSNumber numberWithUnsignedInt: timer0], @"timer0",
          [NSNumber numberWithUnsignedInt: seed.GetSkippedPIDFrames() + 1],
            @"startFrame",
          [NSNumber numberWithUnsignedInt: frame.number], @"pidFrame",
          (frame.pid.IsShiny(tid, sid) ? @"★" : @""), @"shiny",
          [NSString stringWithFormat: @"%s",
            Nature::ToString(frame.nature).c_str()], @"nature",
          [NSNumber numberWithUnsignedInt: frame.pid.Gen5Ability()], @"ability",
          ((genderValue < 31) ? @"♀" : @"♂"), @"gender18",
          ((genderValue < 63) ? @"♀" : @"♂"), @"gender14",
          ((genderValue < 127) ? @"♀" : @"♂"), @"gender12",
          ((genderValue < 191) ? @"♀" : @"♂"), @"gender34",
          (frame.synched ? @"Y" : @""), @"sync",
          (generatesESV ? [NSString stringWithFormat: @"%d", frame.esv] : @""),
            @"esv",
          HeldItemString(frame.heldItem), @"heldItem",
          ((generatesIsEncounter && frame.isEncounter) ? @"Y" : @""),
            @"isEncounter",
          [NSNumber numberWithUnsignedInt: ivs.hp()], @"hp",
          [NSNumber numberWithUnsignedInt: ivs.at()], @"atk",
          [NSNumber numberWithUnsignedInt: ivs.df()], @"def",
          [NSNumber numberWithUnsignedInt: ivs.sa()], @"spa",
          [NSNumber numberWithUnsignedInt: ivs.sd()], @"spd",
          [NSNumber numberWithUnsignedInt: ivs.sp()], @"spe",
          [NSString stringWithFormat: @"%s",
            Element::ToString(ivs.HiddenType()).c_str()], @"hiddenType",
          [NSString stringWithFormat: @"%s",
              Characteristic::ToString
                (Characteristic::Get(frame.pid, ivs)).c_str()],
            @"characteristic",
          nil]];
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
  }
}

@end
