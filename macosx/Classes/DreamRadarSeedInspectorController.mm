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



#import "DreamRadarSeedInspectorController.h"

#include "HashedSeed.h"
#include "FrameGenerator.h"
#include "Utilities.h"

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace pprng;

@interface DreamRadarSeedInspectorFrame : NSObject <PIDResult, IVResult>
{
  uint32_t              frame;
  uint64_t              spinnerPosition;
  DECLARE_PID_RESULT_VARIABLES();
  DECLARE_IV_RESULT_VARIABLES();
  Characteristic::Type  characteristic;
}

@property uint32_t              frame;
@property uint64_t              spinnerPosition;
@property Characteristic::Type  characteristic;

@end

@implementation DreamRadarSeedInspectorFrame

@synthesize frame;
@synthesize spinnerPosition;
SYNTHESIZE_PID_RESULT_PROPERTIES();
SYNTHESIZE_IV_RESULT_PROPERTIES();
@synthesize characteristic;

@end



@interface DreamRadarSeedInspectorAdjacentFrame :
  NSObject <HashedSeedResultParameters>
{
  DECLARE_HASHED_SEED_RESULT_PARAMETERS_VARIABLES();
  uint64_t  spinnerSequence;
}

@property uint64_t  spinnerSequence;

@end

@implementation DreamRadarSeedInspectorAdjacentFrame

SYNTHESIZE_HASHED_SEED_RESULT_PARAMETERS_PROPERTIES();
@synthesize spinnerSequence;

@end



@implementation DreamRadarSeedInspectorController

@synthesize spinnerSequence;

@synthesize type;

@synthesize genderRequired;
@synthesize gender, genderRatio;

@synthesize numPrecedingGenderlessRequired;
@synthesize slot, numPrecedingGenderless, maxNumPrecedingGenderless;

@synthesize minFrame, maxFrame;

@synthesize secondsVariance, timer0Variance;
@synthesize spinnerSequenceSearchValue;

- (NSString *)windowNibName
{
	return @"DreamRadarSeedInspector";
}

- (void)awakeFromNib
{
  [super awakeFromNib];
  
  self.spinnerSequence = 0ULL;
  
  self.type = DreamRadarFrameGenerator::NonLegendaryFrame;
  self.genderRequired = YES;
  self.gender = Gender::FEMALE;
  self.genderRatio = Gender::ONE_HALF_FEMALE;
  
  self.slot = 1;
  self.numPrecedingGenderless = 0;
  
  self.minFrame = 1;
  self.maxFrame = 10;
  
  self.secondsVariance = 1;
  self.timer0Variance = 3;
}

- (void)calcInitialSpinnerSequence
{
  if (rawSeed == nil)
  {
    self.spinnerSequence = 0ULL;
  }
  else
  {
    HashedSeed  seed([rawSeed unsignedLongLongValue], version);
    
    self.spinnerSequence = SpinnerPositions(seed, memoryLinkUsed, 10).word;
  }
}

- (IBAction) memoryLinkUsedChanged:(id)sender
{
  [self calcInitialSpinnerSequence];
}

- (void)setRawSeed:(NSNumber*)newSeed
{
  if (newSeed != rawSeed)
  {
    rawSeed = newSeed;
    [self calcInitialSpinnerSequence];
  }
}

- (void)setType:(DreamRadarFrameGenerator::FrameType)newValue
{
  if (type != newValue)
  {
    type = newValue;
    
    self.genderRequired = (type == DreamRadarFrameGenerator::NonLegendaryFrame);
  }
}

- (void)setGender:(Gender::Type)newValue
{
  if (gender != newValue)
  {
    gender = newValue;
    
    if (gender == Gender::GENDERLESS)
      self.genderRatio = Gender::NO_RATIO;
  }
}

- (void)setSlot:(uint32_t)newValue
{
  if (slot != newValue)
  {
    slot = newValue;
    self.maxNumPrecedingGenderless = slot - 1;
    
    self.numPrecedingGenderlessRequired = (slot > 1);
    if (numPrecedingGenderless >= slot)
      self.numPrecedingGenderless = slot - 1;
  }
}

- (void)setNumPrecedingGenderlessRequired:(BOOL)newValue
{
  if (numPrecedingGenderlessRequired != newValue)
  {
    numPrecedingGenderlessRequired = newValue;
    if (newValue)
    {
      self.numPrecedingGenderless = 0;
    }
  }
}


- (IBAction)generateFrames:(id)sender
{
  if (!EndEditing([self window]))
    return;
  
  if (!rawSeed)
    return;
  
  [frameContentArray setContent: [NSMutableArray array]];
  
  HashedSeed  seed([rawSeed unsignedLongLongValue], version);
  
  uint32_t  frameNum = 0;
  
  DreamRadarFrameGenerator::Parameters  p;
  
  p.frameType = type;
  if (type == DreamRadarFrameGenerator::NonLegendaryFrame)
  {
    p.targetGender = gender;
    p.targetRatio = genderRatio;
  }
  else
  {
    p.targetGender = Gender::MALE;
    p.targetRatio = Gender::MALE_ONLY;
  }
  p.slot = slot;
  p.numPrecedingGenderless = numPrecedingGenderless;
  p.tid = [tid unsignedIntValue];
  p.sid = [sid unsignedIntValue];
  p.memoryLinkUsed = memoryLinkUsed;
  
  DreamRadarFrameGenerator  generator(seed, p);
  
  uint32_t  limitFrame = minFrame - 1;
  
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
    
    DreamRadarFrame  frame = generator.CurrentFrame();
    
    DreamRadarSeedInspectorFrame  *row =
      [[DreamRadarSeedInspectorFrame alloc] init];
    
    row.frame = frame.number;
    
    SpinnerPositions  spins;
    spins.AddSpin(SpinnerPositions::CalcPosition(frame.rngValue));
    
    row.spinnerPosition = spins.word;
    
    SetPIDResult(row, frame.pid, p.tid, p.sid,
                 frame.nature, frame.pid.Gen5Ability(),
                 p.targetGender, p.targetRatio);
    
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
    DreamRadarSeedInspectorFrame  *row = [rows objectAtIndex: 0];
    
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
  
  self.spinnerSequenceSearchValue = 0ULL;
  
  [adjacentsContentArray setContent: [NSMutableArray array]];
  
  HashedSeed::Parameters  targetSeedParams;
  
  targetSeedParams.version = version;
  targetSeedParams.dsType = dsType;
  targetSeedParams.macAddress = [self macAddress];
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
  
  if (targetSeed.timer0 < timer0Variance)
  {
    timer0Low = 0;
  }
  if (targetSeed.timer0 > (0xffffffff - timer0Variance))
  {
    timer0High = 0xffffffff;
  }
  
  ptime  seedTime(date(targetSeed.year(), targetSeed.month(), targetSeed.day()),
                  hours(targetSeed.hour) + minutes(targetSeed.minute) +
                  seconds(targetSeed.second));
  ptime  dt = seedTime;
  ptime  endTime = dt + seconds(secondsVariance);
  dt = dt - seconds(secondsVariance);
  
  HashedSeed::Parameters  seedParams;
  seedParams.macAddress = targetSeed.macAddress;
  seedParams.version = targetSeed.version;
  seedParams.dsType = targetSeed.dsType;
  seedParams.gxStat = targetSeed.gxStat;
  seedParams.vcount = targetSeed.vcount;
  seedParams.vframe = targetSeed.vframe;
  seedParams.heldButtons = targetSeed.heldButtons;
  
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
      
      DreamRadarSeedInspectorAdjacentFrame  *row =
        [[DreamRadarSeedInspectorAdjacentFrame alloc] init];
      
      SetHashedSeedResultParameters(row, seed);
      
      row.spinnerSequence = SpinnerPositions(seed, memoryLinkUsed,
                                             SpinnerPositions::MAX_SPINS).word;
      
      [rowArray addObject: row];
    }
  }
  
  [adjacentsContentArray addObjects: rowArray];
}



- (BOOL)findSpinnerPositions:(SpinnerPositions)spins
{
  NSArray    *rows = [adjacentsContentArray arrangedObjects];
  NSInteger  numRows = [rows count];
  NSInteger  rowNum = 0;
  BOOL       foundRow = NO;
  
  while ((rowNum < numRows) && !foundRow)
  {
    DreamRadarSeedInspectorAdjacentFrame  *row = [rows objectAtIndex: rowNum];
    SpinnerPositions                      rowSpins(row.spinnerSequence);
    
    if (rowSpins.Contains(spins))
    {
      [adjacentsTableView
        selectRowIndexes: [NSIndexSet indexSetWithIndex: rowNum]
        byExtendingSelection: NO];
      [adjacentsTableView scrollRowToVisible: rowNum];
      foundRow = YES;
    }
    
    ++rowNum;
  }
  
  if (!foundRow)
    NSBeep();
  
  return foundRow;
}

- (void)addSpin:(SpinnerPositions::Position)position
{
  SpinnerPositions  spins(spinnerSequenceSearchValue);
  
  if (spins.NumSpins() < SpinnerPositions::MAX_SPINS)
  {
    spins.AddSpin(position);
    
    if ([self findSpinnerPositions: spins])
      self.spinnerSequenceSearchValue = spins.word;
  }
}

- (IBAction)addUpPosition:(id)sender
{
  [self addSpin: SpinnerPositions::UP];
}

- (IBAction)addUpRightPosition:(id)sender
{
  [self addSpin: SpinnerPositions::UP_RIGHT];
}

- (IBAction)addRightPosition:(id)sender
{
  [self addSpin: SpinnerPositions::RIGHT];
}

- (IBAction)addDownRightPosition:(id)sender
{
  [self addSpin: SpinnerPositions::DOWN_RIGHT];
}

- (IBAction)addDownPosition:(id)sender
{
  [self addSpin: SpinnerPositions::DOWN];
}

- (IBAction)addDownLeftPosition:(id)sender
{
  [self addSpin: SpinnerPositions::DOWN_LEFT];
}

- (IBAction)addLeftPosition:(id)sender
{
  [self addSpin: SpinnerPositions::LEFT];
}

- (IBAction)addUpLeftPosition:(id)sender
{
  [self addSpin: SpinnerPositions::UP_LEFT];
}

- (IBAction)removeLastSearchItem:(id)sender
{
  uint32_t  numSpins = SpinnerPositions(spinnerSequenceSearchValue).NumSpins();
  if (numSpins > 0)
  {
    SpinnerPositions  spins(spinnerSequenceSearchValue);
    spins.RemoveSpin();
    
    self.spinnerSequenceSearchValue = spins.word;
  }
}

- (IBAction)resetSearch:(id)sender
{
  self.spinnerSequenceSearchValue = 0ULL;
}

@end
