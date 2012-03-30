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



#import "WonderCardSeedInspectorController.h"

#include "HashedSeed.h"
#include "FrameGenerator.h"
#include "Utilities.h"

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace pprng;

@interface WonderCardSeedInspectorFrame : NSObject <PIDResult, IVResult>
{
  uint32_t              frame;
  uint32_t              chatotPitch;
  DECLARE_PID_RESULT_VARIABLES();
  DECLARE_IV_RESULT_VARIABLES();
  Characteristic::Type  characteristic;
}

@property uint32_t              frame;
@property uint32_t              chatotPitch;
@property Characteristic::Type  characteristic;

@end

@implementation WonderCardSeedInspectorFrame

@synthesize frame;
@synthesize chatotPitch;
SYNTHESIZE_PID_RESULT_PROPERTIES();
SYNTHESIZE_IV_RESULT_PROPERTIES();
@synthesize characteristic;

@end



@interface WonderCardSeedInspectorAdjacentFrame :
  NSObject <HashedSeedResultParameters, PIDResult, IVResult>
{
  DECLARE_HASHED_SEED_RESULT_PARAMETERS_VARIABLES();
  uint32_t              startFrame, frame;
  DECLARE_PID_RESULT_VARIABLES();
  DECLARE_IV_RESULT_VARIABLES();
  Characteristic::Type  characteristic;
}

@property uint32_t              startFrame, frame;
@property Characteristic::Type  characteristic;

@end

@implementation WonderCardSeedInspectorAdjacentFrame

SYNTHESIZE_HASHED_SEED_RESULT_PARAMETERS_PROPERTIES();
@synthesize startFrame, frame;
SYNTHESIZE_PID_RESULT_PROPERTIES();
SYNTHESIZE_IV_RESULT_PROPERTIES();
@synthesize characteristic;

@end



@implementation WonderCardSeedInspectorController

@synthesize startDate, startHour, startMinute, startSecond;
@synthesize timer0, vcount, vframe;

@synthesize button1, button2, button3;

@synthesize rawSeed;
@synthesize initialFrame;

@synthesize cardNature, cardAbility;
@synthesize cardGender, cardGenderRatio;
@synthesize cardShininess, cardTID, cardSID;

@synthesize natureSearchable, abilitySearchable;
@synthesize shininessSearchable, genderSearchable;

@synthesize startFromInitialFrame;
@synthesize minFrame, maxFrame;

@synthesize secondsVariance, timer0Variance;

@synthesize matchOffsetFromInitialFrame;
@synthesize targetFrame, targetFrameVariance;

- (NSString *)windowNibName
{
	return @"WonderCardSeedInspector";
}

- (void)awakeFromNib
{
  [super awakeFromNib];
  
  self.startDate = [NSDate date];
  self.button1 = 0;
  self.button2 = 0;
  self.button3 = 0;
  
  self.cardNature = Nature::ANY;
  self.cardAbility = Ability::ANY;
  self.cardGender = Gender::ANY;
  self.cardGenderRatio = Gender::ANY_RATIO;
  self.cardShininess = WonderCardShininess::NEVER_SHINY;
  self.cardTID = nil;
  self.cardSID = [NSNumber numberWithUnsignedInt: 0];
  
  self.startFromInitialFrame = YES;
  self.minFrame = 50;
  self.maxFrame = 200;
  
  self.secondsVariance = 1;
  self.timer0Variance = 1;
  self.matchOffsetFromInitialFrame = YES;
  self.targetFrame = 50;
  self.targetFrameVariance = 10;
}

- (void)setCardNature:(Nature::Type)newValue
{
  if (cardNature != newValue)
  {
    cardNature = newValue;
    
    self.natureSearchable = (newValue == Nature::ANY);
  }
}

- (void)setCardAbility:(Ability::Type)newValue
{
  if (cardAbility != newValue)
  {
    cardAbility = newValue;
    
    self.abilitySearchable = (newValue == Ability::ANY);
  }
}

- (void)setCardShininess:(WonderCardShininess::Type)newValue
{
  if (cardShininess != newValue)
  {
    cardShininess = newValue;
    
    self.shininessSearchable = (newValue == 1);
  }
}

- (void)setCardGender:(Gender::Type)newValue
{
  if (cardGender != newValue)
  {
    cardGender = newValue;
    
    self.genderSearchable = (newValue == Gender::ANY);
  }
}

- (NSNumber*)calcRawSeed
{
  if (!startDate || !startHour || !startMinute || !startSecond ||
      !timer0 || !vcount || !vframe)
  {
    return nil;
  }
  
  HashedSeed::Parameters  p;
  
  p.version = [gen5ConfigController version];
  p.dsType = [gen5ConfigController dsType];
  p.macAddress = [gen5ConfigController macAddress];
  p.gxStat = HashedSeed::HardResetGxStat;
  p.vcount = [vcount unsignedIntValue];
  p.vframe = [vframe unsignedIntValue];
  p.timer0 = [timer0 unsignedIntValue];
  p.date = NSDateToBoostDate(startDate);
  p.hour = [startHour unsignedIntValue];
  p.minute = [startMinute unsignedIntValue];
  p.second = [startSecond unsignedIntValue];
  p.heldButtons = button1 | button2 | button3;
  
  HashedSeed  s(p);
  
  return [NSNumber numberWithUnsignedLongLong: s.rawSeed];
}

- (IBAction)seedParameterChanged:(id)sender
{
  self.rawSeed = [self calcRawSeed];
}

- (IBAction) seedValueChanged:(id)sender
{
  NSNumber  *paramSeed = [self calcRawSeed];
  
  if ((paramSeed == nil) || ![paramSeed isEqualToNumber: rawSeed])
  {
    self.startDate = [NSDate date];
    self.startHour = nil;
    self.startMinute = nil;
    self.startSecond = nil;
    self.timer0 = nil;
    self.vcount = nil;
    self.vframe = nil;
    self.button1 = 0;
    self.button2 = 0;
    self.button3 = 0;
  }
}

- (void)setButton1:(uint32_t)newButton
{
  if (newButton != button1)
  {
    button1 = newButton;
    [self seedParameterChanged:self];
  }
}

- (void)setButton2:(uint32_t)newButton
{
  if (newButton != button2)
  {
    button2 = newButton;
    [self seedParameterChanged:self];
  }
}

- (void)setButton3:(uint32_t)newButton
{
  if (newButton != button3)
  {
    button3 = newButton;
    [self seedParameterChanged:self];
  }
}

- (void)setSeedFromResult:(id <HashedSeedResultParameters>)result
{
  HashedSeed::Parameters  p;
  
  p.timer0 = result.timer0;
  p.vcount = result.vcount;
  p.vframe = result.vframe;
  p.date = UInt32DateToBoostDate(result.date);
  p.hour = GetUInt32TimeHour(result.time);
  p.minute = GetUInt32TimeMinute(result.time);
  p.second = GetUInt32TimeSecond(result.time);
  p.heldButtons = result.heldButtons;
  
  [self setSeed: HashedSeed(p, result.rawSeed)];
}

- (void)setSeed:(const pprng::HashedSeed&)seed
{
  self.startDate = MakeNSDate(seed.year(), seed.month(), seed.day());
  self.startHour = [NSNumber numberWithUnsignedInt: seed.hour];
  self.startMinute = [NSNumber numberWithUnsignedInt: seed.minute];
  self.startSecond = [NSNumber numberWithUnsignedInt: seed.second];
  
  self.timer0 = [NSNumber numberWithUnsignedInt: seed.timer0];
  self.vcount = [NSNumber numberWithUnsignedInt: seed.vcount];
  self.vframe = [NSNumber numberWithUnsignedInt: seed.vframe];
  
  uint32_t  button[3] = { 0, 0, 0 };
  uint32_t  i = 0;
  uint32_t  dpadPress = seed.heldButtons & Button::DPAD_MASK;
  uint32_t  buttonPress = seed.heldButtons & Button::SINGLE_BUTTON_MASK;
  
  if (dpadPress != 0)
  {
    button[0] = dpadPress;
    ++i;
  }
  
  uint32_t  j = 1;
  while ((buttonPress != 0) && (i < 3))
  {
    if (buttonPress & 0x1)
    {
      button[i++] = j;
    }
    
    buttonPress >>= 1;
    j <<= 1;
  }
  
  self.button1 = button[0];
  self.button2 = button[1];
  self.button3 = button[2];
  
  [self seedParameterChanged:self];
}

- (void)setRawSeed:(NSNumber*)newSeed
{
  if (newSeed != rawSeed)
  {
    if (newSeed == nil)
    {
      self.initialFrame = nil;
    }
    else
    {
      HashedSeed  seed([newSeed unsignedLongLongValue]);
      self.initialFrame =
        [NSNumber numberWithUnsignedInt: seed.GetSkippedPIDFrames() + 1];
    }
    
    rawSeed = newSeed;
  }
}


- (IBAction)generateFrames:(id)sender
{
  if (!EndEditing([self window]))
    return;
  
  if (!rawSeed)
    return;
  
  [frameContentArray setContent: [NSMutableArray array]];
  
  BOOL  hidePIDInfo = !cardTID || !cardSID;
  [[frameTableView tableColumnWithIdentifier:@"pid"] setHidden: hidePIDInfo];
  [[frameTableView tableColumnWithIdentifier:@"characteristic"]
   setHidden: hidePIDInfo];
  
  HashedSeed  seed([rawSeed unsignedLongLongValue]);
  
  uint32_t  frameNum = 0;
  
  WonderCardFrameGenerator::Parameters  p;
  
  p.startFromLowestFrame = startFromInitialFrame;
  p.cardNature = cardNature;
  p.cardAbility = cardAbility;
  p.cardGender = cardGender;
  p.cardGenderRatio = cardGenderRatio;
  p.cardShininess = cardShininess;
  p.cardTID = [cardTID unsignedIntValue];
  p.cardSID = [cardSID unsignedIntValue];
  
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
  
  while (frameNum < maxFrame)
  {
    generator.AdvanceFrame();
    ++frameNum;
    
    WonderCardFrame  frame = generator.CurrentFrame();
    
    WonderCardSeedInspectorFrame  *row =
      [[WonderCardSeedInspectorFrame alloc] init];
    
    row.frame = frame.number;
    row.chatotPitch = frame.chatotPitch;
    
    SetGen5PIDResult(row, frame.nature, frame.pid, p.cardTID, p.cardSID,
                     p.cardGender, p.cardGenderRatio);
    if (frame.hasHiddenAbility)
      row.ability = Ability::HIDDEN;
    
    SetIVResult(row, frame.ivs, NO);
    
    row.characteristic = Characteristic::Get(frame.pid, frame.ivs);
    
    [rowArray addObject: row];
  }
  
  [frameContentArray addObjects: rowArray];
}

- (void)selectAndShowFrame:(uint32_t)frame
{
  NSArray  *rows = [frameContentArray arrangedObjects];
  if (rows && ([rows count] > 0))
  {
    WonderCardSeedInspectorFrame  *row = [rows objectAtIndex: 0];
    
    if (row.frame <= frame)
    {
      NSInteger  rowNum = frame - row.frame;
      
      [frameTableView
        selectRowIndexes: [NSIndexSet indexSetWithIndex: rowNum]
        byExtendingSelection: NO];
      [frameTableView scrollRowToVisible: rowNum];
    }
  }
}


- (IBAction)generateAdjacents:(id)sender
{
  using namespace boost::gregorian;
  using namespace boost::posix_time;
  
  if (!EndEditing([self window]))
    return;
  
  if (!startDate || !startHour || !startMinute || !startSecond ||
      !timer0 || !vcount || !vframe)
    return;
  
  [adjacentsContentArray setContent: [NSMutableArray array]];
  
  BOOL  hidePIDInfo = !cardTID || !cardSID;
  [[adjacentsTableView tableColumnWithIdentifier:@"characteristic"]
   setHidden: hidePIDInfo];
  
  HashedSeed::Parameters  targetSeedParams;
  
  targetSeedParams.version = [gen5ConfigController version];
  targetSeedParams.dsType = [gen5ConfigController dsType];
  targetSeedParams.macAddress = [gen5ConfigController macAddress];
  targetSeedParams.gxStat = HashedSeed::HardResetGxStat;
  targetSeedParams.vcount = [vcount unsignedIntValue];
  targetSeedParams.vframe = [vframe unsignedIntValue];
  targetSeedParams.timer0 = [timer0 unsignedIntValue];
  targetSeedParams.date = NSDateToBoostDate(startDate);
  targetSeedParams.hour = [startHour unsignedIntValue];
  targetSeedParams.minute = [startMinute unsignedIntValue];
  targetSeedParams.second = [startSecond unsignedIntValue];
  targetSeedParams.heldButtons = button1 | button2 | button3;
  HashedSeed  targetSeed(targetSeedParams);
  
  uint32_t  timer0Low = targetSeed.timer0 - timer0Variance;
  uint32_t  timer0High = targetSeed.timer0 + timer0Variance;
  
  if (targetSeed.timer0 == 0)
  {
    timer0Low = 0;
  }
  if (targetSeed.timer0 == 0xffffffff)
  {
    timer0High = 0xffffffff;
  }
  
  ptime  seedTime(date(targetSeed.year(), targetSeed.month(), targetSeed.day()),
                  hours(targetSeed.hour) + minutes(targetSeed.minute) +
                  seconds(targetSeed.second));
  ptime  dt = seedTime;
  ptime  endTime = dt + seconds(secondsVariance);
  dt = dt - seconds(secondsVariance);
  
  uint32_t  frameOffset = matchOffsetFromInitialFrame ?
       (targetFrame - targetSeed.GetSkippedPIDFrames() - 1) :
       targetFrame - 1;
  
  HashedSeed::Parameters  seedParams;
  seedParams.macAddress = targetSeed.macAddress;
  seedParams.version = targetSeed.version;
  seedParams.dsType = targetSeed.dsType;
  seedParams.gxStat = targetSeed.gxStat;
  seedParams.vcount = targetSeed.vcount;
  seedParams.vframe = targetSeed.vframe;
  seedParams.heldButtons = targetSeed.heldButtons;
  
  WonderCardFrameGenerator::Parameters  frameParams;
  frameParams.startFromLowestFrame = matchOffsetFromInitialFrame;
  frameParams.cardNature = cardNature;
  frameParams.cardAbility = cardAbility;
  frameParams.cardGender = cardGender;
  frameParams.cardGenderRatio = cardGenderRatio;
  frameParams.cardShininess = cardShininess;
  frameParams.cardTID = [cardTID unsignedIntValue];
  frameParams.cardSID = [cardSID unsignedIntValue];
  
  NSMutableArray  *rowArray =
    [NSMutableArray arrayWithCapacity:
      (timer0High - timer0Low + 1) * ((2 * secondsVariance) + 1)];
  
  for (; dt <= endTime; dt = dt + seconds(1))
  {
    seedParams.date = dt.date();
    
    time_duration  t = dt.time_of_day();
    
    seedParams.hour = t.hours();
    seedParams.minute = t.minutes();
    seedParams.second = t.seconds();
    
    for (uint32_t t0 = timer0Low; t0 <= timer0High; ++t0)
    {
      seedParams.timer0 = t0;
      
      HashedSeed  seed(seedParams);
      
      uint32_t  adjacentFrameNum = matchOffsetFromInitialFrame ?
        (seed.GetSkippedPIDFrames() + 1 + frameOffset) :
        targetFrame;
      
      uint32_t  skippedFrames;
      if (matchOffsetFromInitialFrame)
      {
        if (frameOffset < (targetFrameVariance + 1))
          skippedFrames = 0;
        else
          skippedFrames = frameOffset - targetFrameVariance - 1;
      }
      else
      {
        if (adjacentFrameNum < (targetFrameVariance + 1))
          skippedFrames = 0;
        else
          skippedFrames = adjacentFrameNum - targetFrameVariance - 1;
      }
      
      uint32_t  endFrameNum = adjacentFrameNum + targetFrameVariance;
      
      WonderCardFrameGenerator  generator(seed, frameParams);
      
      for (uint32_t j = 0; j < skippedFrames; ++j)
        generator.AdvanceFrame();
      
      for (uint32_t f = generator.CurrentFrame().number;
           f < endFrameNum;
           ++f)
      {
        generator.AdvanceFrame();
        WonderCardFrame  frame = generator.CurrentFrame();
        
        WonderCardSeedInspectorAdjacentFrame  *row =
          [[WonderCardSeedInspectorAdjacentFrame alloc] init];
        
        SetHashedSeedResultParameters(row, seed);
        
        row.startFrame = seed.GetSkippedPIDFrames() + 1;
        row.frame = frame.number;
        
        SetGen5PIDResult(row, frame.nature, frame.pid,
                         frameParams.cardTID, frameParams.cardSID,
                         frameParams.cardGender, frameParams.cardGenderRatio);
        if (frame.hasHiddenAbility)
          row.ability = Ability::HIDDEN;
        
        SetIVResult(row, frame.ivs, NO);
        
        row.characteristic = Characteristic::Get(frame.pid, frame.ivs);
        
        [rowArray addObject: row];
      }
    }
  }
  
  [adjacentsContentArray addObjects: rowArray];
}

@end
