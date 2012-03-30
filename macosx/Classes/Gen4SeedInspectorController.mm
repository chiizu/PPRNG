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

#import "Gen4SeedInspectorController.h"

#include "LinearCongruentialRNG.h"
#include "TimeSeed.h"
#include "FrameGenerator.h"
#include "Utilities.h"

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace pprng;

namespace
{

NSMutableDictionary* MakeMethod1Row(const Gen34Frame &frame)
{
  uint32_t  genderValue = frame.pid.GenderValue();
  
  return
    [NSMutableDictionary dictionaryWithObjectsAndKeys:
      [NSNumber numberWithUnsignedInt: frame.number], @"frame",
      [NSNumber numberWithUnsignedLongLong:
                ProfElmResponses(frame.frameSeed, 1).word],
        @"profElmResponse",
      @"", @"sync",
      @"", @"esv",
      [NSNumber numberWithUnsignedInt: frame.pid.word], @"pid",
      frame.pid.IsShiny(0, 0) ? @"★" : @"", @"shiny",
      [NSString stringWithFormat: @"%s",
        Nature::ToString(frame.pid.Gen34Nature()).c_str()], @"nature",
      [NSNumber numberWithUnsignedInt: frame.pid.Gen34Ability()],
        @"ability",
      ((genderValue < 31) ? @"♀" : @"♂"), @"gender18",
      ((genderValue < 63) ? @"♀" : @"♂"), @"gender14",
      ((genderValue < 127) ? @"♀" : @"♂"), @"gender12",
      ((genderValue < 191) ? @"♀" : @"♂"), @"gender34",
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
}

NSMutableDictionary* MakeEncounterRow(const Gen4EncounterFrame &frame)
{
  ProfElmResponses  responses;  responses.AddResponse(frame.profElmResponse);
  uint32_t          genderValue = frame.pid.GenderValue();
  
  return
    [NSMutableDictionary dictionaryWithObjectsAndKeys:
      [NSNumber numberWithUnsignedInt: frame.number], @"frame",
      [NSNumber numberWithUnsignedLongLong: responses.word],
        @"profElmResponse",
      (frame.synched ? @"Y" : @""), @"sync",
      [NSString stringWithFormat: @"%d", ESV::Slot(frame.esv)],
        @"esv",
      [NSNumber numberWithUnsignedInt: frame.pid.word], @"pid",
      frame.pid.IsShiny(0, 0) ? @"★" : @"", @"shiny",
      [NSString stringWithFormat: @"%s",
        Nature::ToString(frame.pid.Gen34Nature()).c_str()], @"nature",
      [NSNumber numberWithUnsignedInt: frame.pid.Gen34Ability()],
        @"ability",
      ((genderValue < 31) ? @"♀" : @"♂"), @"gender18",
      ((genderValue < 63) ? @"♀" : @"♂"), @"gender14",
      ((genderValue < 127) ? @"♀" : @"♂"), @"gender12",
      ((genderValue < 191) ? @"♀" : @"♂"), @"gender34",
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
}

BOOL CheckRoamerLocation(uint32_t location, id rowLocation)
{
  return (location < 1) ||
    (rowLocation && [rowLocation isKindOfClass: [NSNumber class]] &&
     (location == [rowLocation unsignedIntValue]));
}


NSString* GetFrameTypeResult(const Gen4Frame::EncounterData::Frames &frames,
                             Gen4Frame::EncounterData::FrameType frameType)
{
  uint32_t  frameNumber = frames.number[frameType];
  
  return (frameNumber > 0) ?
    [NSString stringWithFormat: @"%d", frameNumber] :
    @"None";
}

void AddESVRows(NSMutableArray *dest,
                const std::map<ESV::Value, Gen4Frame::EncounterData::Frames> &esvs)
{
  std::map<ESV::Value, Gen4Frame::EncounterData::Frames>::const_iterator  i;
  
  for (i = esvs.begin(); i != esvs.end(); ++i)
  {
    [dest addObject:
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
        [NSString stringWithFormat: @"%s", ESV::ToString(i->first).c_str()],
          @"slotName",
        GetFrameTypeResult(i->second, Gen4Frame::EncounterData::NoSync),
          @"noSyncFrame",
        GetFrameTypeResult(i->second, Gen4Frame::EncounterData::Sync),
          @"syncFrame",
        GetFrameTypeResult(i->second, Gen4Frame::EncounterData::FailedSync),
          @"failedSyncFrame",
        nil]];
  }
}

}


@implementation Gen4SeedInspectorController

@synthesize mode;
@synthesize raikouLocation;
@synthesize enteiLocation;
@synthesize latiLocation;
@synthesize nextRaikouLocation;
@synthesize nextEnteiLocation;
@synthesize nextLatiLocation;
@synthesize skippedFrames;
@synthesize seedCoinFlips;
@synthesize seedProfElmResponses;
@synthesize encounterType;
@synthesize syncNature;
@synthesize showRealFrame;
@synthesize useSpecifiedSecond;
@synthesize matchSeedDelayParity;
@synthesize coinFlipsSearchValue;
@synthesize profElmResponsesSearchValue;
@synthesize raikouLocationSearchValue;
@synthesize enteiLocationSearchValue;
@synthesize latiLocationSearchValue;

- (NSString *)windowNibName
{
	return @"Gen4SeedInspector";
}

- (void)awakeFromNib
{
  [super awakeFromNib];
  
  [[seedField formatter] setFormatWidth: 8];
  
  dpptFrames = [NSMutableArray array];
  hgssFrames = [NSMutableArray array];
  
  [timeFinderYearField
   setIntValue: NSDateToBoostDate([NSDate date]).year()];
  
  [timeFinderSecondField setIntValue: 0];
  
  [[[[adjacentsTableView tableColumnWithIdentifier:@"seed"] dataCell] formatter]
   setFormatWidth: 8];
  
  // set initial columns displayed
  [[frameTableView tableColumnWithIdentifier:@"profElmResponse"]
      setHidden: YES];
  [[adjacentsTableView tableColumnWithIdentifier:@"coinFlips"]
    setHidden: NO];
  [[adjacentsTableView tableColumnWithIdentifier:@"profElmResponses"]
    setHidden: YES];
  [[adjacentsTableView tableColumnWithIdentifier:@"raikou"]
    setHidden: YES];
  [[adjacentsTableView tableColumnWithIdentifier:@"entei"]
    setHidden: YES];
  [[adjacentsTableView tableColumnWithIdentifier:@"lati@s"]
    setHidden: YES];
  
  self.mode = 0;
  self.raikouLocation = 0;
  self.enteiLocation = 0;
  self.latiLocation = 0;
  self.seedCoinFlips = 0;
  self.seedProfElmResponses = 0;
  
  self.encounterType = DPPtEncounterFrameGenerator::GrassCaveEncounter;
  self.syncNature = Nature::ANY;
  
  self.useSpecifiedSecond = YES;
  self.matchSeedDelayParity = YES;
  self.coinFlipsSearchValue = 0;
  self.profElmResponsesSearchValue = 0ULL;
  self.raikouLocationSearchValue = 0;
  self.enteiLocationSearchValue = 0;
  self.latiLocationSearchValue = 0;
}

- (void)setSeed:(uint32_t)seed
{
  [seedField setObjectValue: [NSNumber numberWithUnsignedInt: seed]];
  [self seedUpdated: self];
}

- (void)setFrame:(uint32_t)frame
{
  [esvMethod1FrameField setObjectValue:[NSNumber numberWithUnsignedInt: frame]];
  [self esvMethod1FrameUpdated: self];
}

- (void)setMode:(int)newMode
{
  if (newMode != mode)
  {
    mode = newMode;
    
    // change source of frames and esvs
    [frameContentArray setContent: (mode == 0) ? dpptFrames : hgssFrames];
    [esvContentArray setContent: (mode == 0) ? dpptESVs : hgssESVs];
    
    // reset search params on mode switch
    [self resetSearch: self];
    
    // change which table columns are displayed
    [[frameTableView tableColumnWithIdentifier:@"profElmResponse"]
      setHidden: !newMode];
    [[adjacentsTableView tableColumnWithIdentifier:@"coinFlips"]
      setHidden: newMode];
    [[adjacentsTableView tableColumnWithIdentifier:@"profElmResponses"]
      setHidden: !newMode];
    [[adjacentsTableView tableColumnWithIdentifier:@"raikou"]
      setHidden: !newMode];
    [[adjacentsTableView tableColumnWithIdentifier:@"entei"]
      setHidden: !newMode];
    [[adjacentsTableView tableColumnWithIdentifier:@"lati@s"]
      setHidden: !newMode];
  }
}


- (void)updateActualDelay
{
  if (([[timeFinderYearField stringValue] length] == 0) ||
      ([[seedField stringValue] length] == 0))
  {
    [timeFinderActualDelayField setObjectValue: nil];
  }
  else
  {
    uint32_t  baseDelay = [baseDelayField intValue];
    uint32_t  offset = ([timeFinderYearField intValue] - 2000);
    
    if (offset > baseDelay)
      baseDelay += 65536;
    
    [timeFinderActualDelayField setIntValue: baseDelay - offset];
  }
}

- (void)updateHGSSData:(uint32_t)seed
{
  HGSSRoamers  roamers(seed, raikouLocation, enteiLocation, latiLocation);
  
  self.nextRaikouLocation = roamers.Location(HGSSRoamers::RAIKOU);
  self.nextEnteiLocation = roamers.Location(HGSSRoamers::ENTEI);
  self.nextLatiLocation = roamers.Location(HGSSRoamers::LATI);
  self.skippedFrames = roamers.ConsumedFrames();
  
  LCRNG34  rng(seed);
  for (uint32_t i = 0; i < skippedFrames; ++i)
    rng.Next();
  
  self.seedProfElmResponses = ProfElmResponses(rng.Seed(), 10).word;
}

- (IBAction)seedUpdated:(id)sender
{
  if ([[seedField stringValue] length] == 0)
  {
    [baseDelayField setObjectValue: nil];
    self.seedCoinFlips = 0;
    self.seedProfElmResponses = 0;
  }
  else
  {
    TimeSeed  seed([[seedField objectValue] unsignedIntValue]);
    [baseDelayField setObjectValue:
                    [NSNumber numberWithUnsignedInt: seed.BaseDelay()]];
    self.seedCoinFlips = CoinFlips(seed.m_seed, 10).word;
    
    [self updateHGSSData: seed.m_seed];
  }
  
  [self updateActualDelay];
}


- (IBAction)roamerLocationChanged:(id)sender
{
  if ([[seedField stringValue] length] > 0)
  {
    [self updateHGSSData: [[seedField objectValue] unsignedIntValue]];
  }
}

- (IBAction)generateFrames:(id)sender
{
  if (!EndEditing([self window]))
    return;
  
  if ([[seedField stringValue] length] == 0)
    return;
  
  [frameContentArray setContent: [NSMutableArray array]];
  
  uint32_t  seed = [[seedField objectValue] unsignedIntValue];
  uint32_t  minFrame = [minFrameField intValue];
  uint32_t  maxFrame = [maxFrameField intValue];
  uint32_t  frameNum = 0, limitFrame = minFrame - 1;
  
  if (encounterType >= 0)
  {
    // DPPt frames
    if (!showRealFrame)
    {
      uint32_t  offsets[] = { 1, 2, 3, 3, 3, 0 };
      uint32_t  offset = offsets[encounterType];
      
      LCRNG34_R  rRNG(seed);
      while (offset-- > 0)
        rRNG.Next();
      
      seed = rRNG.Seed();
    }
    
    dpptFrames = [NSMutableArray arrayWithCapacity: maxFrame - minFrame + 1];
    
    DPPtEncounterFrameGenerator::Parameters  dpptParameters;
    dpptParameters.encounterType
      = DPPtEncounterFrameGenerator::EncounterType(encounterType);
    dpptParameters.syncNature = Nature::Type(syncNature);
    
    DPPtEncounterFrameGenerator  dpptGenerator(seed, dpptParameters);
    while (frameNum < limitFrame)
    {
      dpptGenerator.AdvanceFrame();
      ++frameNum;
    }
    
    while (frameNum < maxFrame)
    {
      dpptGenerator.AdvanceFrame();
      ++frameNum;
      
      [dpptFrames addObject: MakeEncounterRow(dpptGenerator.CurrentFrame())];
    }
    
    // HGSS frames
    if (showRealFrame)
    {
      uint32_t  skipped = skippedFrames;
      LCRNG34   rng(seed);
      
      while (skipped-- > 0)
        rng.Next();
      
      seed = rng.Seed();
    }
    
    frameNum = 0;
    hgssFrames = [NSMutableArray arrayWithCapacity: maxFrame - minFrame + 1];
    
    HGSSEncounterFrameGenerator::Parameters  hgssParameters;
    hgssParameters.encounterType
      = HGSSEncounterFrameGenerator::EncounterType(encounterType);
    hgssParameters.syncNature = Nature::Type(syncNature);
    
    HGSSEncounterFrameGenerator  hgssGenerator(seed, hgssParameters);
    while (frameNum < limitFrame)
    {
      hgssGenerator.AdvanceFrame();
      ++frameNum;
    }
    
    while (frameNum < maxFrame)
    {
      hgssGenerator.AdvanceFrame();
      ++frameNum;
      
      [hgssFrames addObject: MakeEncounterRow(hgssGenerator.CurrentFrame())];
    }
  }
  else
  {
    dpptFrames = hgssFrames =
      [NSMutableArray arrayWithCapacity: maxFrame - minFrame + 1];
    
    Method1FrameGenerator  generator(seed);
    while (frameNum < limitFrame)
    {
      generator.AdvanceFrame();
      ++frameNum;
    }
    
    while (frameNum < maxFrame)
    {
      generator.AdvanceFrame();
      ++frameNum;
      
      [dpptFrames addObject: MakeMethod1Row(generator.CurrentFrame())];
    }
  }
  
  [frameContentArray setContent: (mode == 0) ? dpptFrames : hgssFrames];
}

- (IBAction)esvMethod1FrameUpdated:(id)sender
{
  if (([[seedField stringValue] length] == 0) ||
      ([[esvMethod1FrameField stringValue] length] == 0))
  {
    return;
  }
  
  [esvContentArray setContent: [NSMutableArray array]];
  
  uint32_t  seed = [[seedField objectValue] unsignedIntValue];
  uint32_t  targetM1FrameNum = [esvMethod1FrameField intValue];
  
  Gen34Frame  targetFrame;
  {
    Method1FrameGenerator  m1fg(seed);
    do
    {
      m1fg.AdvanceFrame();
    }
    while (m1fg.CurrentFrame().number != targetM1FrameNum);
    
    targetFrame = m1fg.CurrentFrame();
  }
  
  [esvFrameDescriptionField setObjectValue:
    [NSString stringWithFormat: @"%s  %d/%d/%d/%d/%d/%d  %s/%d",
      Nature::ToString(targetFrame.pid.Gen34Nature()).c_str(),
      targetFrame.ivs.hp(), targetFrame.ivs.at(), targetFrame.ivs.df(),
      targetFrame.ivs.sa(), targetFrame.ivs.sd(), targetFrame.ivs.sp(),
      Element::ToString(targetFrame.ivs.HiddenType()).c_str(),
      targetFrame.ivs.HiddenPower()]];
  
  Gen4Frame  gen4Frame(targetFrame);

  dpptESVs =
    [NSMutableArray arrayWithCapacity: gen4Frame.methodJ.esvFrames.size()];
  AddESVRows(dpptESVs, gen4Frame.methodJ.esvFrames);
  
  hgssESVs =
    [NSMutableArray arrayWithCapacity: gen4Frame.methodK.esvFrames.size()];
  AddESVRows(hgssESVs, gen4Frame.methodK.esvFrames);
  
  [esvContentArray setContent: (mode == 0) ? dpptESVs : hgssESVs];
}

- (IBAction)yearUpdated:(id)sender
{
  [self updateActualDelay];
}

- (IBAction)calculateTimes:(id)sender
{
  if (!EndEditing([self window]))
    return;
  
  if (([[seedField stringValue] length] == 0) ||
      ([[timeFinderYearField stringValue] length] == 0))
    return;
  
  [timeFinderContentArray setContent: [NSMutableArray array]];
  
  TimeSeed  seed([[seedField objectValue] unsignedIntValue]);
  
  uint32_t  wantedSecond;
  if (useSpecifiedSecond)
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
  
  if (!EndEditing([self window]))
    return;
  
  if ([[seedField stringValue] length] == 0)
    return;
  
  NSInteger  rowNum = [timeFinderTableView selectedRow];
  if (rowNum < 0)
    return;
  
  NSDictionary  *row =
    [[timeFinderContentArray arrangedObjects] objectAtIndex: rowNum];
  NSData  *timeElementData = [row objectForKey: @"fullTime"];
  
  TimeSeed::TimeElement  timeElement;
  [timeElementData getBytes:&timeElement length:sizeof(TimeSeed::TimeElement)];
  
  [adjacentsContentArray setContent: [NSMutableArray array]];
  
  TimeSeed  seed([[seedField objectValue] unsignedIntValue]);
  
  uint32_t  delayVariance = [adjacentsDelayVarianceField intValue];
  uint32_t  secondVariance = [adjacentsTimeVarianceField intValue];
  
  uint32_t  targetDelay = timeElement.delay;
  uint32_t  delayStep = matchSeedDelayParity ? 2 : 1;
  uint32_t  endDelay = targetDelay + delayVariance;
  uint32_t  startDelay = (targetDelay < delayVariance) ? 0 :
                           targetDelay - delayVariance;
  if (matchSeedDelayParity && ((startDelay & 0x1) != (targetDelay & 0x1)))
    ++startDelay;
  
  ptime  targetTime(date(timeElement.year, timeElement.month, timeElement.day),
                    hours(timeElement.hour) + minutes(timeElement.minute) +
                    seconds(timeElement.second));
  
  ptime  endTime = targetTime + seconds(secondVariance);
  targetTime = targetTime - seconds(secondVariance);
  
  int32_t  rLocation = raikouLocation;
  int32_t  eLocation = enteiLocation;
  int32_t  lLocation = latiLocation;
  
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
    
    for (uint32_t delay = startDelay; delay <= endDelay; delay += delayStep)
    {
      TimeSeed  s(d.year(), d.month(), d.day(),
                  t.hours(), t.minutes(), t.seconds(),
                  delay);
      
      uint32_t     coinFlips = CoinFlips(s.m_seed, 10).word;
      
      HGSSRoamers  roamers(s.m_seed, rLocation, eLocation, lLocation);
      LCRNG34      rng(s.m_seed);
      uint32_t     skipped = roamers.ConsumedFrames();
      
      while (skipped-- > 0)
        rng.Next();
      
      uint64_t  profElmResponses = ProfElmResponses(rng.Seed(), 10).word;
      
      [rowArray addObject:
        [NSMutableDictionary dictionaryWithObjectsAndKeys:
          [NSNumber numberWithUnsignedInt: s.m_seed], @"seed",
          dateStr, @"date",
          timeStr, @"time",
          [NSNumber numberWithUnsignedInt: delay], @"delay",
          [NSNumber numberWithUnsignedLong: coinFlips], @"coinFlips",
          [NSNumber numberWithUnsignedLongLong: profElmResponses],
            @"profElmResponses",
          ((rLocation > 0) ?
             id([NSNumber numberWithUnsignedInt:
                          roamers.Location(HGSSRoamers::RAIKOU)]) :
             id(@"")),
            @"raikou",
          ((eLocation > 0) ?
             id([NSNumber numberWithUnsignedInt:
                          roamers.Location(HGSSRoamers::ENTEI)]) :
             id(@"")),
            @"entei",
          ((lLocation > 0) ?
             id([NSNumber numberWithUnsignedInt:
                          roamers.Location(HGSSRoamers::LATI)]) :
             id(@"")),
            @"lati@s",
          nil]];
    }
  }
  
  [adjacentsContentArray addObjects: rowArray];
}

- (BOOL)findCoinFlips:(CoinFlips)flips
{
  NSArray       *rows = [adjacentsContentArray arrangedObjects];
  NSInteger     numRows = [rows count];
  NSInteger     rowNum = 0;
  NSDictionary  *foundRow = nil;
  
  while (rowNum < numRows)
  {
    NSDictionary  *row = [rows objectAtIndex: rowNum];
    CoinFlips     rowFlips([[row objectForKey: @"coinFlips"] unsignedIntValue]);
    
    if (rowFlips.Contains(flips))
    {
      [adjacentsTableView
        selectRowIndexes: [NSIndexSet indexSetWithIndex: rowNum]
        byExtendingSelection: NO];
      [adjacentsTableView scrollRowToVisible: rowNum];
      foundRow = row;
      break;
    }
    
    ++rowNum;
  }
  
  if (foundRow == nil)
  {
    NSBeep();
    return NO;
  }
  
  return YES;
}

- (void)addCoinFlipResult:(CoinFlips::Result)r
{
  CoinFlips  searchFlips(coinFlipsSearchValue);
  
  if (searchFlips.NumFlips() < 10)
  {
    searchFlips.AddFlipResult(r);
    
    if ([self findCoinFlips: searchFlips])
      self.coinFlipsSearchValue = searchFlips.word;
  }
}

- (IBAction)addHeads:(id)sender
{
  [self addCoinFlipResult: CoinFlips::HEADS];
}

- (IBAction)addTails:(id)sender
{
  [self addCoinFlipResult: CoinFlips::TAILS];
}


- (BOOL)findProfElmResponses:(ProfElmResponses)responses
        withRaikouAt:(uint32_t)rLocation
        withEnteiAt:(uint32_t)eLocation
        withLatiAt:(uint32_t)lLocation
{
  NSArray       *rows = [adjacentsContentArray arrangedObjects];
  NSInteger     numRows = [rows count];
  NSInteger     rowNum = 0;
  NSDictionary  *foundRow = nil;
  
  while (rowNum < numRows)
  {
    NSDictionary      *row = [rows objectAtIndex: rowNum];
    ProfElmResponses  rowResponses
      ([[row objectForKey: @"profElmResponses"] unsignedLongLongValue]);
    id                rowRLocation = [row objectForKey: @"raikou"];
    id                rowELocation = [row objectForKey: @"entei"];
    id                rowLLocation = [row objectForKey: @"lati@s"];
    
    if (rowResponses.Contains(responses) &&
        CheckRoamerLocation(rLocation, rowRLocation) &&
        CheckRoamerLocation(eLocation, rowELocation) &&
        CheckRoamerLocation(lLocation, rowLLocation))
    {
      [adjacentsTableView
        selectRowIndexes: [NSIndexSet indexSetWithIndex: rowNum]
        byExtendingSelection: NO];
      [adjacentsTableView scrollRowToVisible: rowNum];
      foundRow = row;
      break;
    }
    
    ++rowNum;
  }
  
  if (foundRow == nil)
  {
    NSBeep();
    return NO;
  }
  
  return YES;
}

- (void)addProfElmResponse:(ProfElmResponses::Response)response
{
  ProfElmResponses  responses(profElmResponsesSearchValue);
  
  if (responses.NumResponses() < 10)
  {
    responses.AddResponse(response);
    
    if ([self findProfElmResponses: responses
              withRaikouAt: raikouLocationSearchValue
              withEnteiAt: enteiLocationSearchValue
              withLatiAt: latiLocationSearchValue])
      self.profElmResponsesSearchValue = responses.word;
    else
      NSBeep();
  }
}

- (IBAction)addEResponse:(id)sender
{
  [self addProfElmResponse: ProfElmResponses::EVOLUTION];
}

- (IBAction)addKResponse:(id)sender
{
  [self addProfElmResponse: ProfElmResponses::KANTO];
}

- (IBAction)addPResponse:(id)sender
{
  [self addProfElmResponse: ProfElmResponses::POKERUS];
}

- (IBAction)searchRoamerLocation:(id)sender
{
  if (![self findProfElmResponses: ProfElmResponses(profElmResponsesSearchValue)
             withRaikouAt: raikouLocationSearchValue
             withEnteiAt: enteiLocationSearchValue
             withLatiAt: latiLocationSearchValue])
    NSBeep();
}

- (IBAction)removeLastSearchItem:(id)sender
{
  if (mode == 0)
  {
    uint32_t  numFlips = CoinFlips(coinFlipsSearchValue).NumFlips();
    if (numFlips > 0)
    {
      CoinFlips  flips(coinFlipsSearchValue);
      flips.RemoveFlipResult();
      
      self.coinFlipsSearchValue = flips.word;
    }
  }
  else
  {
    uint32_t  numResponses =
      ProfElmResponses(profElmResponsesSearchValue).NumResponses();
    if (numResponses > 0)
    {
      ProfElmResponses  responses(profElmResponsesSearchValue);
      responses.RemoveResponse();
      
      self.profElmResponsesSearchValue = responses.word;
    }
  }
}

- (IBAction)resetSearch:(id)sender
{
  self.coinFlipsSearchValue = 0;
  self.profElmResponsesSearchValue = 0ULL;
  self.raikouLocationSearchValue = 0;
  self.enteiLocationSearchValue = 0;
  self.latiLocationSearchValue = 0;
}


@end
