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

- (IBAction)onTypeChange:(id)sender
{
  NSInteger  selection = [[sender selectedItem] tag];
  
  if (selection == -1)
  {
    [ivSkipField setEnabled: YES];
    [pidSkipField setEnabled: YES];
    [natureSkipField setEnabled: YES];
  }
  else
  {
    [ivSkipField setEnabled: NO];
    [pidSkipField setEnabled: NO];
    [natureSkipField setEnabled: NO];
    
    if (selection == 0)
    {
      [ivSkipField setIntValue: 22];
      [pidSkipField setIntValue: 2];
      [natureSkipField setIntValue: 1];
    }
    else
    {
      [ivSkipField setIntValue: 24];
      [pidSkipField setIntValue: 2];
      [natureSkipField setIntValue: 3];
    }
  }
}

- (IBAction)toggleFixedNature:(id)sender
{
  /*
  BOOL  checked = [fixedNatureCheckBox state];
  [naturePopUp setEnabled: !checked];
  if (checked)
  {
    [naturePopUp selectItemWithTag: -1];
  }
  */
}

- (IBAction)toggleFixedAbility:(id)sender
{
  /*
  BOOL  checked = [fixedAbilityCheckBox state];
  [abilityPopUp setEnabled: !checked];
  if (checked)
  {
    [abilityPopUp selectItemWithTag: -1];
  }
  */
}

- (IBAction)toggleFixedGender:(id)sender
{
  /*
  BOOL  checked = [fixedGenderCheckBox state];
  [genderPopUp setEnabled: !checked];
  [genderRatioPopUp setEnabled: !checked];
  if (checked)
  {
    [genderPopUp selectItemWithTag: -1];
    [genderRatioPopUp selectItemWithTag: -1];
  }
  */
}

- (IBAction)toggleUseInitialPID:(id)sender
{
  BOOL enabled = [useInitialPIDButton state];
  [minFrameField setEnabled: !enabled];
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
  
  [seedField setObjectValue:
    [NSNumber numberWithUnsignedLongLong: seed.rawSeed]];
  [initialPIDFrameField setObjectValue:
    [NSNumber numberWithUnsignedInt: seed.GetSkippedPIDFrames() + 1]];
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
  uint32_t  frameNum = 0;
  
  WonderCardFrameGenerator::Parameters  p;
  
  p.startFromLowestFrame = [useInitialPIDButton state];
  p.ivSkip = [ivSkipField intValue];
  p.pidSkip = [pidSkipField intValue];
  p.natureSkip = [natureSkipField intValue];
  p.canBeShiny = false;
  p.tid = [gen5ConfigController tid];
  p.sid = [gen5ConfigController sid];
  
  WonderCardFrameGenerator  generator(seed, p);
  
  if (!p.startFromLowestFrame)
  {
    uint32_t  limitFrame = minFrame - 1;
    
    while (frameNum < limitFrame)
    {
      generator.AdvanceFrame();
      ++frameNum;
    }
  }
  else
  {
    frameNum = generator.CurrentFrame().number;
  }
  
  NSMutableArray  *rowArray =
    [NSMutableArray arrayWithCapacity: maxFrame - minFrame + 1];
  
  BOOL  showNature = ![fixedNatureCheckBox state];
  BOOL  showAbility = ![fixedAbilityCheckBox state];
  BOOL  showGender = ![fixedGenderCheckBox state];
  
  while (frameNum < maxFrame)
  {
    generator.AdvanceFrame();
    ++frameNum;
    
    WonderCardFrame  frame = generator.CurrentFrame();
    uint32_t         genderValue = frame.pid.GenderValue();
    
    [rowArray addObject:
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
				[NSNumber numberWithUnsignedInt: frame.number], @"frame",
        (showNature ? [NSString stringWithFormat: @"%s",
                       Nature::ToString(frame.nature).c_str()] : @""),
          @"nature",
        (showAbility ?
         [NSString stringWithFormat: @"%d", frame.pid.Gen5Ability()] : @""),
          @"ability",
        (showGender ? ((genderValue < 31) ? @"♀" : @"♂") : @""), @"gender18",
        (showGender ? ((genderValue < 63) ? @"♀" : @"♂") : @""), @"gender14",
        (showGender ? ((genderValue < 127) ? @"♀" : @"♂") : @""), @"gender12",
        (showGender ? ((genderValue < 191) ? @"♀" : @"♂") : @""), @"gender34",
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
        [NSString stringWithFormat: @"%s",
            Characteristic::ToString
              (Characteristic::Get(frame.pid, frame.ivs)).c_str()],
          @"characteristic",
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
  
  if (targetSeed.rawSeed != [[seedField objectValue] unsignedLongLongValue])
  {
    return;
  }
  
  [adjacentsContentArray setContent: [NSMutableArray array]];
  
  uint32_t  timer0Low = targetSeed.timer0 - 1;
  uint32_t  timer0High = targetSeed.timer0 + 1;
  
  if (targetSeed.timer0 == 0)
  {
    timer0Low = 0;
  }
  if (targetSeed.timer0 == 0xffffffff)
  {
    timer0High = 0xffffffff;
  }
  
  uint32_t  secondVariance = [adjacentsTimeVarianceField intValue];
  
  ptime  seedTime(date(targetSeed.year(), targetSeed.month(), targetSeed.day()),
                  hours(targetSeed.hour) + minutes(targetSeed.minute) +
                  seconds(targetSeed.second));
  ptime  dt = seedTime;
  ptime  endTime = dt + seconds(secondVariance);
  dt = dt - seconds(secondVariance);
  
  uint32_t  targetFrameNum = [adjacentsFrameField intValue];
  BOOL      useInitialPIDOffset = [adjacentsUseInitialPIDOffsetButton state];
  uint32_t  frameOffset = useInitialPIDOffset ?
       (targetFrameNum - targetSeed.GetSkippedPIDFrames() - 1) :
       targetFrameNum - 1;
  uint32_t  frameVariance = [adjacentsFrameVarianceField intValue];
  
  HashedSeed::Parameters  seedParams;
  seedParams.macAddress = targetSeed.macAddress;
  seedParams.version = targetSeed.version;
  seedParams.dsType = targetSeed.dsType;
  seedParams.gxStat = targetSeed.gxStat;
  seedParams.vcount = targetSeed.vcount;
  seedParams.vframe = targetSeed.vframe;
  seedParams.heldButtons = targetSeed.heldButtons;
  
  WonderCardFrameGenerator::Parameters  frameParams;
  frameParams.startFromLowestFrame = useInitialPIDOffset;
  frameParams.ivSkip = [ivSkipField intValue];
  frameParams.pidSkip = [pidSkipField intValue];
  frameParams.natureSkip = [natureSkipField intValue];
  frameParams.canBeShiny = false;
  frameParams.tid = [gen5ConfigController tid];
  frameParams.sid = [gen5ConfigController sid];
  
  NSMutableArray  *rowArray =
    [NSMutableArray arrayWithCapacity:
      (timer0High - timer0Low + 1) * ((2 * secondVariance) + 1)];
  
  BOOL  showNature = ![fixedNatureCheckBox state];
  BOOL  showAbility = ![fixedAbilityCheckBox state];
  BOOL  showGender = ![fixedGenderCheckBox state];
  
  for (; dt <= endTime; dt = dt + seconds(1))
  {
    seedParams.date = dt.date();
    
    time_duration  t = dt.time_of_day();
    
    NSString  *timeStr = (dt == seedTime) ?
      [NSString stringWithFormat:@"%.2d:%.2d:%.2d",
                                 t.hours(), t.minutes(), t.seconds()] :
      [NSString stringWithFormat:@"%+dsec", (dt - seedTime).total_seconds()];
    
    seedParams.hour = t.hours();
    seedParams.minute = t.minutes();
    seedParams.second = t.seconds();
    
    for (uint32_t timer0 = timer0Low; timer0 <= timer0High; ++timer0)
    {
      seedParams.timer0 = timer0;
      
      HashedSeed  seed(seedParams);
      
      uint32_t  adjacentFrameNum = useInitialPIDOffset ?
        (seed.GetSkippedPIDFrames() + 1 + frameOffset) :
        targetFrameNum;
      uint32_t  startFrameNum =
        (adjacentFrameNum < (frameVariance + 1)) ?
          1 : (adjacentFrameNum - frameVariance);
      uint32_t  endFrameNum = adjacentFrameNum + frameVariance;
      uint32_t  initialFrame = seed.GetSkippedPIDFrames() + 1;
      
      if (startFrameNum < initialFrame)
        startFrameNum = initialFrame;
      
      WonderCardFrameGenerator  generator(seed, frameParams);
      
      uint32_t  limit = (startFrameNum - 1);
      while (generator.CurrentFrame().number < limit)
        generator.AdvanceFrame();
      
      for (uint32_t frameNum = startFrameNum;
           frameNum <= endFrameNum;
           ++frameNum)
      {
        generator.AdvanceFrame();
        WonderCardFrame  frame = generator.CurrentFrame();
        uint32_t         genderValue = frame.pid.GenderValue();
        
        [rowArray addObject:
        [NSMutableDictionary dictionaryWithObjectsAndKeys:
          timeStr, @"time",
          [NSNumber numberWithUnsignedInt: timer0], @"timer0",
          [NSNumber numberWithUnsignedInt: seed.GetSkippedPIDFrames() + 1],
            @"startFrame",
          [NSNumber numberWithUnsignedInt: frame.number], @"frame",
          (showNature ? [NSString stringWithFormat: @"%s",
                         Nature::ToString(frame.nature).c_str()] : @""),
            @"nature",
          (showAbility ?
           [NSString stringWithFormat: @"%d", frame.pid.Gen5Ability()] : @""),
            @"ability",
          (showGender ? ((genderValue < 31) ? @"♀" : @"♂") : @""), @"gender18",
          (showGender ? ((genderValue < 63) ? @"♀" : @"♂") : @""), @"gender14",
          (showGender ? ((genderValue < 127) ? @"♀" : @"♂") : @""), @"gender12",
          (showGender ? ((genderValue < 191) ? @"♀" : @"♂") : @""), @"gender34",
          [NSNumber numberWithUnsignedInt: frame.ivs.hp()], @"hp",
          [NSNumber numberWithUnsignedInt: frame.ivs.at()], @"atk",
          [NSNumber numberWithUnsignedInt: frame.ivs.df()], @"def",
          [NSNumber numberWithUnsignedInt: frame.ivs.sa()], @"spa",
          [NSNumber numberWithUnsignedInt: frame.ivs.sd()], @"spd",
          [NSNumber numberWithUnsignedInt: frame.ivs.sp()], @"spe",
          [NSString stringWithFormat: @"%s",
            Element::ToString(frame.ivs.HiddenType()).c_str()], @"hiddenType",
          [NSString stringWithFormat: @"%s",
              Characteristic::ToString
                (Characteristic::Get(frame.pid, frame.ivs)).c_str()],
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
