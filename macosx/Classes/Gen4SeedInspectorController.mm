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

#import "SearchResultProtocols.h"

#include "LinearCongruentialRNG.h"
#include "TimeSeed.h"
#include "FrameGenerator.h"
#include "Utilities.h"

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace pprng;

@interface Gen4EncounterResult : NSObject <IVResult, PIDResult>
{
  uint32_t    frame;
  uint64_t    profElmResponse;
  uint32_t    chatotPitch;
  BOOL        sync;
  ESV::Value  esv;
  DECLARE_PID_RESULT_VARIABLES();
  DECLARE_IV_RESULT_VARIABLES();
}

@property uint32_t    frame;
@property uint64_t    profElmResponse;
@property uint32_t    chatotPitch;
@property BOOL        sync;
@property ESV::Value  esv;

@end

@implementation Gen4EncounterResult

@synthesize frame;
@synthesize profElmResponse;
@synthesize chatotPitch;
@synthesize sync;
@synthesize esv;
SYNTHESIZE_PID_RESULT_PROPERTIES();
SYNTHESIZE_IV_RESULT_PROPERTIES();

@end


@interface Gen4SeedTimeResult : NSObject
{
  uint32_t  date, time, delay;
}

@property uint32_t  date, time, delay;

@end

@implementation Gen4SeedTimeResult

@synthesize date, time, delay;

@end


@interface Gen4AdjacentResult : NSObject
{
  uint32_t  seed, date, time, delay;
  uint32_t  coinFlips;
  uint64_t  profElmResponses;
  uint32_t  raikou, entei, lati;
}

@property uint32_t  seed, date, time, delay;
@property uint32_t  coinFlips;
@property uint64_t  profElmResponses;
@property uint32_t  raikou, entei, lati;

@end

@implementation Gen4AdjacentResult

@synthesize seed, date, time, delay;
@synthesize coinFlips, profElmResponses;
@synthesize raikou, entei, lati;

@end


@interface Gen4EggPIDResult : NSObject <PIDResult>
{
  uint32_t  frame;
  uint32_t  coinFlip;
  DECLARE_PID_RESULT_VARIABLES();
}

@property uint32_t  frame, coinFlip;

@end

@implementation Gen4EggPIDResult

@synthesize frame, coinFlip;
SYNTHESIZE_PID_RESULT_PROPERTIES();

@end


@interface Gen4EggIVResult : NSObject
{
  uint32_t       frame;
  uint64_t       profElmResponse;
  uint32_t       chatotPitch;
  NSString       *hp, *atk, *def, *spa, *spd, *spe;
  Element::Type  hiddenType;
  NSNumber       *hiddenPower;
}

@property uint32_t         frame;
@property uint64_t         profElmResponse;
@property uint32_t         chatotPitch;
@property (copy) NSString  *hp, *atk, *def, *spa, *spd, *spe;
@property Element::Type    hiddenType;
@property (copy) NSNumber  *hiddenPower;

@end

@implementation Gen4EggIVResult

@synthesize frame;
@synthesize profElmResponse, chatotPitch;
@synthesize hp, atk, def, spa, spd, spe;
@synthesize hiddenType, hiddenPower;

@end


namespace
{

static Gen4EncounterResult* MakeMethod1Row(const Gen34Frame &frame)
{
  Gen4EncounterResult  *row = [[Gen4EncounterResult alloc] init];
  
  row.frame = frame.number;
  row.profElmResponse = ProfElmResponses(frame.rngValue, 1).word;
  row.chatotPitch = Chatot::Gen4Pitch(frame.rngValue);
  row.sync = NO;
  row.esv = ESV::NO_SLOT;
  
  SetPIDResult(row, frame.pid, 0, 0, frame.pid.Gen34Nature(),
               frame.pid.Gen34Ability(), Gender::ANY, Gender::ANY_RATIO);
  
  SetIVResult(row, frame.ivs, NO);
  
  return row;
}

static Gen4EncounterResult* MakeEncounterRow(const Gen4EncounterFrame &frame)
{
  Gen4EncounterResult  *row = [[Gen4EncounterResult alloc] init];
  
  row.frame = frame.number;
  row.profElmResponse = ProfElmResponses(frame.rngValue, 1).word;
  row.chatotPitch = Chatot::Gen4Pitch(frame.rngValue);
  row.sync = frame.synched;
  row.esv = frame.esv;
  
  SetPIDResult(row, frame.pid, 0, 0, frame.pid.Gen34Nature(),
               frame.pid.Gen34Ability(), Gender::ANY, Gender::ANY_RATIO);
  
  SetIVResult(row, frame.ivs, NO);
  
  return row;
}

static BOOL CheckRoamerLocation(uint32_t location, uint32_t rowLocation)
{
  return (location < 1) || (location == rowLocation);
}


static
NSString* GetFrameTypeResult(const Gen4Frame::EncounterData::Frames &frames,
                             Gen4Frame::EncounterData::FrameType frameType)
{
  uint32_t  frameNumber = frames.number[frameType];
  
  return (frameNumber > 0) ?
    [NSString stringWithFormat: @"%d", frameNumber] :
    @"None";
}

static
void AddESVRows
  (NSMutableArray *dest,
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

static
NSString* GetEggIV(Gen4BreedingFrame::Inheritance inheritance, uint32_t iv,
                   bool isASet, uint32_t aIV,
                   bool isBSet, uint32_t bIV, bool showParentIVs)
{
  switch (inheritance)
  {
  default:
  case Gen4BreedingFrame::NotInherited:
    return [NSString stringWithFormat:@"%d", iv];
  case Gen4BreedingFrame::ParentA:
    if (showParentIVs && isASet)
      return [NSString stringWithFormat:@"%d", aIV];
    else
      return @"A";
    break;
  case Gen4BreedingFrame::ParentB:
    if (showParentIVs && isBSet)
      return [NSString stringWithFormat:@"%d", bIV];
    else
      return @"B";
    break;
  }
}

static
OptionalIVs GetEggIVs(const Gen4BreedingFrame &frame,
                      const OptionalIVs &aIVs, const OptionalIVs &bIVs)
{
  OptionalIVs  eggIVs;
  uint32_t     i;
  
  for (i = 0; i < 6; ++i)
  {
    switch (frame.inheritance[i])
    {
    case Gen4BreedingFrame::ParentA:
      if (aIVs.isSet(i))
        eggIVs.setIV(i, aIVs.values.iv(i));
      break;
      
    case Gen4BreedingFrame::ParentB:
      if (bIVs.isSet(i))
        eggIVs.setIV(i, bIVs.values.iv(i));
      break;
      
    default:
    case Gen4BreedingFrame::NotInherited:
      eggIVs.setIV(i, frame.baseIVs.iv(i));
      break;
    }
  }
  
  return eggIVs;
}

}


@implementation Gen4SeedInspectorController

@synthesize seed, baseDelay;
@synthesize mode;
@synthesize raikouLocation, enteiLocation, latiLocation;
@synthesize nextRaikouLocation, nextEnteiLocation, nextLatiLocation;
@synthesize skippedFrames;
@synthesize seedCoinFlips, seedProfElmResponses;

@synthesize selectedTabId;

@synthesize minFrame, maxFrame;
@synthesize encounterType, syncNature;
@synthesize showRealFrame;

@synthesize esvMethod1Frame, esvFrameDescription;

@synthesize year, actualDelay;
@synthesize useSpecifiedSecond, second;

@synthesize secondVariance, delayVariance;
@synthesize matchSeedDelayParity;

@synthesize coinFlipsSearchValue, profElmResponsesSearchValue;
@synthesize raikouLocationSearchValue;
@synthesize enteiLocationSearchValue;
@synthesize latiLocationSearchValue;

@synthesize minEggPIDFrame, maxEggPIDFrame, internationalParents;

@synthesize minEggIVFrame, maxEggIVFrame;
@synthesize enableParentIVs;
@synthesize aHP, aAT, aDF, aSA, aSD, aSP;
@synthesize bHP, bAT, bDF, bSA, bSD, bSP;

- (NSString *)windowNibName
{
	return @"Gen4SeedInspector";
}

- (void)updateColumnVisibility
{
  [[frameTableView tableColumnWithIdentifier:@"profElmResponse"]
    setHidden: !mode];
  [[adjacentsTableView tableColumnWithIdentifier:@"coinFlips"]
    setHidden: mode];
  [[adjacentsTableView tableColumnWithIdentifier:@"profElmResponses"]
    setHidden: !mode];
  [[adjacentsTableView tableColumnWithIdentifier:@"raikou"]
    setHidden: !mode];
  [[adjacentsTableView tableColumnWithIdentifier:@"entei"]
    setHidden: !mode];
  [[adjacentsTableView tableColumnWithIdentifier:@"lati@s"]
    setHidden: !mode];
}

- (void)awakeFromNib
{
  [super awakeFromNib];
  
  self.seed = nil;
  self.baseDelay = nil;
  
  dpptFrames = [NSMutableArray array];
  hgssFrames = [NSMutableArray array];
  
  self.mode = 0;
  [self updateColumnVisibility];  // force update
  
  self.raikouLocation = 0;
  self.enteiLocation = 0;
  self.latiLocation = 0;
  self.seedCoinFlips = 0;
  self.seedProfElmResponses = 0;
  
  self.encounterType = DPPtEncounterFrameGenerator::GrassCaveEncounter;
  self.syncNature = Nature::ANY;
  self.minFrame = 1;
  self.maxFrame = 100;
  self.showRealFrame = YES;
  
  self.esvMethod1Frame = 1;
  
  self.year = uint32_t(NSDateToBoostDate([NSDate date]).year());
  self.useSpecifiedSecond = YES;
  self.second = 0;
  
  self.secondVariance = 1;
  self.delayVariance = 10;
  self.matchSeedDelayParity = YES;
  
  self.coinFlipsSearchValue = 0;
  self.profElmResponsesSearchValue = 0ULL;
  self.raikouLocationSearchValue = 0;
  self.enteiLocationSearchValue = 0;
  self.latiLocationSearchValue = 0;
  
  self.minEggPIDFrame = 1;
  self.maxEggPIDFrame = 100;
  self.internationalParents = NO;
  
  self.minEggIVFrame = 1;
  self.maxEggIVFrame = 100;
  self.enableParentIVs = NO;
  self.aHP = nil;
  self.aAT = nil;
  self.aDF = nil;
  self.aSA = nil;
  self.aSD = nil;
  self.aSP = nil;
  self.bHP = nil;
  self.bAT = nil;
  self.bDF = nil;
  self.bSA = nil;
  self.bSD = nil;
  self.bSP = nil;
}

- (void)updateActualDelay
{
  if (seed == nil)
  {
    self.actualDelay = nil;
  }
  else
  {
    uint32_t  delay = [baseDelay intValue];
    uint32_t  offset = year - 2000;
    
    if (offset > delay)
      delay += 65536;
    
    self.actualDelay = [NSNumber numberWithUnsignedInt: delay - offset];
  }
}

- (void)updateHGSSData
{
  if (seed == nil)
  {
    self.nextRaikouLocation = 0;
    self.nextEnteiLocation = 0;
    self.nextLatiLocation = 0;
    self.skippedFrames = 0;
    self.seedProfElmResponses = 0;
  }
  else
  {
    uint32_t     rawSeed = [seed unsignedIntValue];
    HGSSRoamers  roamers(rawSeed, raikouLocation, enteiLocation, latiLocation);
    
    self.nextRaikouLocation = roamers.Location(HGSSRoamers::RAIKOU);
    self.nextEnteiLocation = roamers.Location(HGSSRoamers::ENTEI);
    self.nextLatiLocation = roamers.Location(HGSSRoamers::LATI);
    self.skippedFrames = roamers.ConsumedFrames();
    
    LCRNG34  rng(rawSeed);
    for (uint32_t i = 0; i < skippedFrames; ++i)
      rng.Next();
    
    self.seedProfElmResponses = ProfElmResponses(rng.Seed(), 10).word;
  }
}

- (void)setSeed:(NSNumber*)newValue
{
  if (newValue != seed)
  {
    seed = newValue;
    
    if (newValue == nil)
    {
      self.baseDelay = nil;
      self.seedCoinFlips = 0;
    }
    else
    {
      TimeSeed  timeSeed([newValue unsignedIntValue]);
      self.baseDelay = [NSNumber numberWithUnsignedInt: timeSeed.BaseDelay()];
      self.seedCoinFlips = CoinFlips(timeSeed.m_seed, 10).word;
    }
    
    [self updateHGSSData];
    [self updateActualDelay];
  }
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
    [self updateColumnVisibility];
  }
}

- (void)setRaikouLocation:(uint32_t)newValue
{
  if (newValue != raikouLocation)
  {
    raikouLocation = newValue;
    
    [self updateHGSSData];
  }
}

- (void)setEnteiLocation:(uint32_t)newValue
{
  if (newValue != enteiLocation)
  {
    enteiLocation = newValue;
    
    [self updateHGSSData];
  }
}

- (void)setLatiLocation:(uint32_t)newValue
{
  if (newValue != latiLocation)
  {
    latiLocation = newValue;
    
    [self updateHGSSData];
  }
}

- (IBAction)generateFrames:(id)sender
{
  if (!EndEditing([self window]))
    return;
  
  if (seed == nil)
    return;
  
  uint32_t  rawSeed = [seed unsignedIntValue];
  
  [frameContentArray setContent: [NSMutableArray array]];
  
  uint32_t  frameNum = 0, limitFrame = minFrame - 1;
  
  if (encounterType >= 0)
  {
    // DPPt frames
    if (!showRealFrame)
    {
      uint32_t  offsets[] = { 1, 2, 3, 3, 3, 0 };
      uint32_t  offset = offsets[encounterType];
      
      LCRNG34_R  rRNG(rawSeed);
      while (offset-- > 0)
        rRNG.Next();
      
      rawSeed = rRNG.Seed();
    }
    
    dpptFrames = [NSMutableArray arrayWithCapacity: maxFrame - minFrame + 1];
    
    DPPtEncounterFrameGenerator::Parameters  dpptParameters;
    dpptParameters.encounterType
      = DPPtEncounterFrameGenerator::EncounterType(encounterType);
    dpptParameters.syncNature = syncNature;
    
    DPPtEncounterFrameGenerator  dpptGenerator(rawSeed, dpptParameters);
    dpptGenerator.SkipFrames(limitFrame);
    frameNum = limitFrame;
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
      LCRNG34   rng(rawSeed);
      
      while (skipped-- > 0)
        rng.Next();
      
      rawSeed = rng.Seed();
    }
    
    frameNum = 0;
    hgssFrames = [NSMutableArray arrayWithCapacity: maxFrame - minFrame + 1];
    
    HGSSEncounterFrameGenerator::Parameters  hgssParameters;
    hgssParameters.encounterType
      = HGSSEncounterFrameGenerator::EncounterType(encounterType);
    hgssParameters.syncNature = syncNature;
    
    HGSSEncounterFrameGenerator  hgssGenerator(rawSeed, hgssParameters);
    hgssGenerator.SkipFrames(limitFrame);
    frameNum = limitFrame;
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
    
    Method1FrameGenerator  generator(rawSeed);
    generator.SkipFrames(limitFrame);
    frameNum = limitFrame;
    while (frameNum < maxFrame)
    {
      generator.AdvanceFrame();
      ++frameNum;
      
      [dpptFrames addObject: MakeMethod1Row(generator.CurrentFrame())];
    }
  }
  
  [frameContentArray setContent: (mode == 0) ? dpptFrames : hgssFrames];
}

- (void)selectAndShowFrame:(uint32_t)frame
{
  NSArray  *rows = [frameContentArray arrangedObjects];
  if (rows && ([rows count] > 0))
  {
    Gen4EncounterResult  *row = [rows objectAtIndex: 0];
    
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

- (void)setEsvMethod1Frame:(uint32_t)newValue
{
  if (newValue == esvMethod1Frame)
    return;
  
  esvMethod1Frame = newValue;
  
  if (seed == nil)
    return;
  
  [esvContentArray setContent: [NSMutableArray array]];
  
  uint32_t  rawSeed = [seed unsignedIntValue];
  
  Gen34Frame  targetFrame;
  {
    Method1FrameGenerator  m1fg(rawSeed);
    m1fg.SkipFrames(esvMethod1Frame - 1);
    m1fg.AdvanceFrame();
    
    targetFrame = m1fg.CurrentFrame();
  }
  
  self.esvFrameDescription =
    [NSString stringWithFormat: @"%s  %d/%d/%d/%d/%d/%d  %s/%d",
      Nature::ToString(targetFrame.pid.Gen34Nature()).c_str(),
      targetFrame.ivs.hp(), targetFrame.ivs.at(), targetFrame.ivs.df(),
      targetFrame.ivs.sa(), targetFrame.ivs.sd(), targetFrame.ivs.sp(),
      Element::ToString(targetFrame.ivs.HiddenType()).c_str(),
      targetFrame.ivs.HiddenPower()];
  
  Gen4Frame  gen4Frame(targetFrame);

  dpptESVs =
    [NSMutableArray arrayWithCapacity: gen4Frame.methodJ.esvFrames.size()];
  AddESVRows(dpptESVs, gen4Frame.methodJ.esvFrames);
  
  hgssESVs =
    [NSMutableArray arrayWithCapacity: gen4Frame.methodK.esvFrames.size()];
  AddESVRows(hgssESVs, gen4Frame.methodK.esvFrames);
  
  [esvContentArray setContent: (mode == 0) ? dpptESVs : hgssESVs];
}

- (void)setYear:(uint32_t)newValue
{
  if (newValue != year)
  {
    year = newValue;
    [self updateActualDelay];
  }
}

- (IBAction)calculateTimes:(id)sender
{
  if (!EndEditing([self window]))
    return;
  
  if (seed == nil)
    return;
  
  [timeFinderContentArray setContent: [NSMutableArray array]];
  
  TimeSeed  timeSeed([seed unsignedIntValue]);
  
  uint32_t  wantedSecond = useSpecifiedSecond ? second : -1;
  
  TimeSeed::TimeElements  elements =
    timeSeed.GetTimeElements(year, wantedSecond);
  
  NSMutableArray  *rows = [NSMutableArray arrayWithCapacity: elements.size()];
  TimeSeed::TimeElements::iterator  i;
  for (i = elements.begin(); i != elements.end(); ++i)
  {
    Gen4SeedTimeResult  *row = [[Gen4SeedTimeResult alloc] init];
    
    row.date = MakeUInt32Date(i->year, i->month, i->day);
    row.time = MakeUInt32Time(i->hour, i->minute, i->second);
    row.delay = i->delay;
    
    [rows addObject: row];
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
  
  if (seed == nil)
    return;
  
  NSInteger  rowNum = [timeFinderTableView selectedRow];
  if (rowNum < 0)
    return;
  
  Gen4SeedTimeResult  *seedTime =
    [[timeFinderContentArray arrangedObjects] objectAtIndex: rowNum];
  
  [adjacentsContentArray setContent: [NSMutableArray array]];
  
  uint16_t  targetDelay = seedTime.delay;
  uint16_t  delayStep = matchSeedDelayParity ? 2 : 1;
  uint16_t  endDelay = targetDelay + delayVariance;
  uint16_t  startDelay = (targetDelay < delayVariance) ? 0 :
                           targetDelay - delayVariance;
  if (matchSeedDelayParity && ((startDelay & 0x1) != (targetDelay & 0x1)))
    ++startDelay;
  
  ptime  targetTime(UInt32DateAndTimeToBoostTime(seedTime.date, seedTime.time));
  
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
    uint32_t  dt = MakeUInt32Date(targetTime.date());
    uint32_t  yr = GetUInt32DateYear(dt);
    uint32_t  mo = GetUInt32DateMonth(dt);
    uint32_t  dy = GetUInt32DateDay(dt);
    
    uint32_t  tm = MakeUInt32Time(targetTime.time_of_day());
    uint32_t  hr = GetUInt32TimeHour(tm);
    uint32_t  mi = GetUInt32TimeMinute(tm);
    uint32_t  sc = GetUInt32TimeSecond(tm);
    
    for (uint32_t delay = startDelay; delay <= endDelay; delay += delayStep)
    {
      TimeSeed     s(yr, mo, dy, hr, mi, sc, delay);
      
      HGSSRoamers  roamers(s.m_seed, rLocation, eLocation, lLocation);
      LCRNG34      rng(s.m_seed);
      uint32_t     skipped = roamers.ConsumedFrames();
      
      while (skipped-- > 0)
        rng.Next();
      
      Gen4AdjacentResult  *result = [[Gen4AdjacentResult alloc] init];
      
      result.seed = s.m_seed;
      result.date = dt;
      result.time = tm;
      result.delay = delay;
      result.coinFlips = CoinFlips(s.m_seed, 10).word;
      result.profElmResponses = ProfElmResponses(rng.Seed(), 10).word;
      result.raikou = roamers.Location(HGSSRoamers::RAIKOU);
      result.entei = roamers.Location(HGSSRoamers::ENTEI);
      result.lati = roamers.Location(HGSSRoamers::LATI);
      
      [rowArray addObject: result];
    }
  }
  
  [adjacentsContentArray addObjects: rowArray];
}

- (BOOL)findCoinFlips:(CoinFlips)flips
{
  NSArray    *rows = [adjacentsContentArray arrangedObjects];
  NSInteger  numRows = [rows count];
  NSInteger  rowNum = 0;
  BOOL       foundRow = NO;
  
  while ((rowNum < numRows) && !foundRow)
  {
    Gen4AdjacentResult  *row = [rows objectAtIndex: rowNum];
    CoinFlips           rowFlips(row.coinFlips);
    
    if (rowFlips.Contains(flips))
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
  NSArray    *rows = [adjacentsContentArray arrangedObjects];
  NSInteger  numRows = [rows count];
  NSInteger  rowNum = 0;
  BOOL       foundRow = NO;
  
  while ((rowNum < numRows) && !foundRow)
  {
    Gen4AdjacentResult  *row = [rows objectAtIndex: rowNum];
    ProfElmResponses    rowResponses(row.profElmResponses);
    
    if (rowResponses.Contains(responses) &&
        CheckRoamerLocation(rLocation, row.raikou) &&
        CheckRoamerLocation(eLocation, row.entei) &&
        CheckRoamerLocation(lLocation, row.lati))
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

- (void)setRaikouLocationSearchValue:(uint32_t)newValue
{
  if (newValue != raikouLocationSearchValue)
  {
    if ([self findProfElmResponses:ProfElmResponses(profElmResponsesSearchValue)
              withRaikouAt: newValue
              withEnteiAt: enteiLocationSearchValue
              withLatiAt: latiLocationSearchValue])
      raikouLocationSearchValue = newValue;
    else
      NSBeep();
  }
}

- (void)setEnteiLocationSearchValue:(uint32_t)newValue
{
  if (newValue != enteiLocationSearchValue)
  {
    if ([self findProfElmResponses:ProfElmResponses(profElmResponsesSearchValue)
              withRaikouAt: raikouLocationSearchValue
              withEnteiAt: newValue
              withLatiAt: latiLocationSearchValue])
      enteiLocationSearchValue = newValue;
    else
      NSBeep();
  }
}

- (void)setLatiLocationSearchValue:(uint32_t)newValue
{
  if (newValue != latiLocationSearchValue)
  {
    if ([self findProfElmResponses:ProfElmResponses(profElmResponsesSearchValue)
              withRaikouAt: raikouLocationSearchValue
              withEnteiAt: enteiLocationSearchValue
              withLatiAt: newValue])
      latiLocationSearchValue = newValue;
    else
      NSBeep();
  }
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


- (IBAction)generateEggPIDFrames:(id)sender
{
  if (!EndEditing([self window]))
    return;
  
  if (seed == nil)
    return;
  
  [eggPIDsContentArray setContent: [NSMutableArray array]];
  
  uint32_t  rawSeed = [seed unsignedIntValue];
  uint32_t  frameNum = minEggPIDFrame - 1;
  
  NSMutableArray  *rows =
    [NSMutableArray arrayWithCapacity: maxEggPIDFrame - minEggPIDFrame + 1];
  
  Gen4EggPIDFrameGenerator::Parameters  p;
  p.internationalParents = internationalParents;
  p.tid = [gen4ConfigController tid];
  p.sid = [gen4ConfigController sid];
  
  Gen4EggPIDFrameGenerator  generator(rawSeed, p);
  generator.SkipFrames(frameNum);
  
  while (frameNum < maxEggPIDFrame)
  {
    generator.AdvanceFrame();
    ++frameNum;
    
    Gen4EggPIDFrame  frame = generator.CurrentFrame();
    
    Gen4EggPIDResult  *result = [[Gen4EggPIDResult alloc] init];
    result.frame = frame.number;
    
    CoinFlips  flips;
    flips.AddFlipResult(CoinFlips::CalcResult(frame.rngValue));
    result.coinFlip = flips.word;
    
    SetPIDResult(result, frame.pid, p.tid, p.sid, frame.pid.Gen34Nature(),
                 frame.pid.Gen34Ability(), Gender::ANY, Gender::ANY_RATIO);
    
    [rows addObject: result];
  }
  
  [eggPIDsContentArray addObjects: rows];
}

- (void)selectAndShowEggPIDFrame:(uint32_t)frame
{
  NSArray  *rows = [eggPIDsContentArray arrangedObjects];
  if (rows && ([rows count] > 0))
  {
    Gen4EggPIDResult  *row = [rows objectAtIndex: 0];
    
    if (row.frame <= frame)
    {
      NSInteger  rowNum = frame - row.frame;
      
      [eggPIDsTableView
        selectRowIndexes: [NSIndexSet indexSetWithIndex: rowNum]
        byExtendingSelection: NO];
      [eggPIDsTableView scrollRowToVisible: rowNum];
    }
  }
}

- (void)setAIVs:(const pprng::OptionalIVs&)ivs
{
  self.aHP = ivs.isSet(IVs::HP) ?
    [NSNumber numberWithUnsignedInt: ivs.hp()] : nil;
  self.aAT = ivs.isSet(IVs::AT) ?
    [NSNumber numberWithUnsignedInt: ivs.at()] : nil;
  self.aDF = ivs.isSet(IVs::DF) ?
    [NSNumber numberWithUnsignedInt: ivs.df()] : nil;
  self.aSA = ivs.isSet(IVs::SA) ?
    [NSNumber numberWithUnsignedInt: ivs.sa()] : nil;
  self.aSD = ivs.isSet(IVs::SD) ?
    [NSNumber numberWithUnsignedInt: ivs.sd()] : nil;
  self.aSP = ivs.isSet(IVs::SP) ?
    [NSNumber numberWithUnsignedInt: ivs.sp()] : nil;
}

- (void)setBIVs:(const pprng::OptionalIVs&)ivs
{
  self.bHP = ivs.isSet(IVs::HP) ?
    [NSNumber numberWithUnsignedInt: ivs.hp()] : nil;
  self.bAT = ivs.isSet(IVs::AT) ?
    [NSNumber numberWithUnsignedInt: ivs.at()] : nil;
  self.bDF = ivs.isSet(IVs::DF) ?
    [NSNumber numberWithUnsignedInt: ivs.df()] : nil;
  self.bSA = ivs.isSet(IVs::SA) ?
    [NSNumber numberWithUnsignedInt: ivs.sa()] : nil;
  self.bSD = ivs.isSet(IVs::SD) ?
    [NSNumber numberWithUnsignedInt: ivs.sd()] : nil;
  self.bSP = ivs.isSet(IVs::SP) ?
    [NSNumber numberWithUnsignedInt: ivs.sp()] : nil;
}

- (IBAction)generateEggIVFrames:(id)sender
{
  if (!EndEditing([self window]))
    return;
  
  if (seed == nil)
    return;
  
  [[eggIVsTableView tableColumnWithIdentifier:@"profElmResponse"]
    setHidden: !mode];
  
  [eggIVsContentArray setContent: [NSMutableArray array]];
  
  uint32_t  rawSeed = [seed unsignedIntValue];
  uint32_t  frameNum = minEggIVFrame - 1;
  
  NSMutableArray  *rows =
    [NSMutableArray arrayWithCapacity: maxEggIVFrame - minEggIVFrame + 1];
  
  Gen4BreedingFrameGenerator  generator(rawSeed,
                                        mode ? Game::HeartGold : Game::Diamond);
  generator.SkipFrames(frameNum);
  
  OptionalIVs  aIVs, bIVs;
  
  if (enableParentIVs)
  {
    if (aHP)
      aIVs.hp([aHP unsignedIntValue]);
    if (aAT)
      aIVs.at([aAT unsignedIntValue]);
    if (aDF)
      aIVs.df([aDF unsignedIntValue]);
    if (aSA)
      aIVs.sa([aSA unsignedIntValue]);
    if (aSD)
      aIVs.sd([aSD unsignedIntValue]);
    if (aSP)
      aIVs.sp([aSP unsignedIntValue]);
    
    if (bHP)
      bIVs.hp([bHP unsignedIntValue]);
    if (bAT)
      bIVs.at([bAT unsignedIntValue]);
    if (bDF)
      bIVs.df([bDF unsignedIntValue]);
    if (bSA)
      bIVs.sa([bSA unsignedIntValue]);
    if (bSD)
      bIVs.sd([bSD unsignedIntValue]);
    if (bSP)
      bIVs.sp([bSP unsignedIntValue]);
  }
  
  while (frameNum < maxEggIVFrame)
  {
    generator.AdvanceFrame();
    ++frameNum;
    
    Gen4BreedingFrame  frame = generator.CurrentFrame();
    
    Gen4EggIVResult  *result = [[Gen4EggIVResult alloc] init];
    
    result.frame = frame.number;
    
    ProfElmResponses  profElmResponse;
    profElmResponse.AddResponse(ProfElmResponses::CalcResponse(frame.rngValue));
    result.profElmResponse = profElmResponse.word;
    
    result.chatotPitch = Chatot::Gen4Pitch(frame.rngValue);
    
    result.hp = GetEggIV(frame.inheritance[0], frame.baseIVs.hp(),
                          aIVs.isSet(IVs::HP), aIVs.hp(),
                          bIVs.isSet(IVs::HP), bIVs.hp(), enableParentIVs);
    result.atk = GetEggIV(frame.inheritance[1], frame.baseIVs.at(),
                          aIVs.isSet(IVs::AT), aIVs.at(),
                          bIVs.isSet(IVs::AT), bIVs.at(), enableParentIVs);
    result.def = GetEggIV(frame.inheritance[2], frame.baseIVs.df(),
                          aIVs.isSet(IVs::DF), aIVs.df(),
                          bIVs.isSet(IVs::DF), bIVs.df(), enableParentIVs);
    result.spa = GetEggIV(frame.inheritance[3], frame.baseIVs.sa(),
                          aIVs.isSet(IVs::SA), aIVs.sa(),
                          bIVs.isSet(IVs::SA), bIVs.sa(), enableParentIVs);
    result.spd = GetEggIV(frame.inheritance[4], frame.baseIVs.sd(),
                          aIVs.isSet(IVs::SD), aIVs.sd(),
                          bIVs.isSet(IVs::SD), bIVs.sd(), enableParentIVs);
    result.spe = GetEggIV(frame.inheritance[5], frame.baseIVs.sp(),
                          aIVs.isSet(IVs::SP), aIVs.sp(),
                          bIVs.isSet(IVs::SP), bIVs.sp(), enableParentIVs);
    
    result.hiddenType = Element::NONE;
    result.hiddenPower = nil;
    
    if (enableParentIVs)
    {
      OptionalIVs  eggIVs = GetEggIVs(frame, aIVs, bIVs);
      
      if (eggIVs.allSet())
      {
        result.hiddenType = eggIVs.values.HiddenType();
        result.hiddenPower =
          [NSNumber numberWithUnsignedInt: eggIVs.values.HiddenPower()];
      }
    }
    
    [rows addObject: result];
  }
  
  [eggIVsContentArray addObjects: rows];
}

- (void)selectAndShowEggIVFrame:(uint32_t)frame
{
  NSArray  *rows = [eggIVsContentArray arrangedObjects];
  if (rows && ([rows count] > 0))
  {
    Gen4EggIVResult  *row = [rows objectAtIndex: 0];
    
    if (row.frame <= frame)
    {
      NSInteger  rowNum = frame - row.frame;
      
      [eggIVsTableView
        selectRowIndexes: [NSIndexSet indexSetWithIndex: rowNum]
        byExtendingSelection: NO];
      [eggIVsTableView scrollRowToVisible: rowNum];
    }
  }
}

@end
