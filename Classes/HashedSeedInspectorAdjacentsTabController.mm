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
  uint32_t  timer0Low = [gen5ConfigController timer0Low];
  uint32_t  timer0High = [gen5ConfigController timer0High];
  
  if ((targetSeed.m_timer0 < timer0Low) || (targetSeed.m_timer0 > timer0High))
  {
    timer0Low = targetSeed.m_timer0 - 1;
    timer0High = targetSeed.m_timer0 + 1;
  }
  
  uint32_t  secondVariance = [adjacentsTimeVarianceField intValue];
  
  ptime     dt(date(targetSeed.m_year, targetSeed.m_month, targetSeed.m_day),
               hours(targetSeed.m_hour) + minutes(targetSeed.m_minute) +
               seconds(targetSeed.m_second));
  ptime     endTime = dt + seconds(secondVariance);
  dt = dt - seconds(secondVariance);
  
  uint32_t  ivFrameNum = [adjacentsIVFrameField intValue];
  bool      isRoamer = [adjacentsRoamerButton state];
  
  uint32_t  pidFrameNum = [adjacentsPIDFrameField intValue];
  uint32_t  pidFrameVariance = [adjacentsPIDFrameVarianceField intValue];
  uint32_t  pidStartFrameNum;
  if (pidFrameNum < pidFrameVariance)
  {
    pidStartFrameNum = 1;
  }
  else
  {
    pidStartFrameNum = pidFrameNum - pidFrameVariance;
  }
  uint32_t  pidEndFrameNum = pidFrameNum + pidFrameVariance;
  
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
                       targetSeed.m_nazo, 0, 0, 0,
                       targetSeed.m_vcount, timer0, HashedSeed::GxStat,
                       targetSeed.m_vframe, targetSeed.m_keyInput);
      
      HashedIVFrameGenerator  ivGenerator(seed,
                                          (isRoamer ?
                                           HashedIVFrameGenerator::Roamer :
                                           HashedIVFrameGenerator::Normal));
      
      for (uint32_t j = 0; j < ivFrameNum; ++j)
        ivGenerator.AdvanceFrame();
      
      IVs  ivs = ivGenerator.CurrentFrame().ivs;
      
      Gen5PIDFrameGenerator  pidGenerator(seed, frameType, false, tid, sid);
      
      for (uint32_t j = 0; j < pidStartFrameNum; ++j)
        pidGenerator.AdvanceFrame();
      
      for (pidFrameNum = pidStartFrameNum;
           pidFrameNum <= pidEndFrameNum;
           ++pidFrameNum)
      {
        Gen5PIDFrame  frame = pidGenerator.CurrentFrame();
        
        [rowArray addObject:
        [NSMutableDictionary dictionaryWithObjectsAndKeys:
          dateStr, @"date",
          timeStr, @"time",
          [NSNumber numberWithUnsignedInt: timer0], @"timer0",
          [NSNumber numberWithUnsignedInt: pidFrameNum], @"pidFrame",
          [NSString stringWithFormat: @"%s",
            Nature::ToString(frame.nature).c_str()], @"nature",
          [NSNumber numberWithUnsignedInt: frame.pid.Gen5Ability()], @"ability",
          GenderString(frame.pid), @"gender",
          [NSNumber numberWithUnsignedInt: ivs.hp()], @"hp",
          [NSNumber numberWithUnsignedInt: ivs.at()], @"atk",
          [NSNumber numberWithUnsignedInt: ivs.df()], @"def",
          [NSNumber numberWithUnsignedInt: ivs.sa()], @"spa",
          [NSNumber numberWithUnsignedInt: ivs.sd()], @"spd",
          [NSNumber numberWithUnsignedInt: ivs.sp()], @"spe",
          [NSString stringWithFormat: @"%s",
              Characteristic::ToString
                (Characteristic::Get(frame.pid, ivs)).c_str()],
            @"characteristic",
          nil]];
        
        pidGenerator.AdvanceFrame();
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
