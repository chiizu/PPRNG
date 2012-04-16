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



#import "CGearSeedInspectorController.h"

#import "SearchResultProtocols.h"
#import "StandardSeedInspectorController.h"

#include "CGearNatureSearcher.h"
#include "FrameGenerator.h"
#include "Utilities.h"

using namespace pprng;

@interface CGearFrameResult : NSObject <IVResult>
{
  uint32_t  frame;
  DECLARE_IV_RESULT_VARIABLES();
}

@property uint32_t  frame;

@end

@implementation CGearFrameResult

@synthesize frame;
SYNTHESIZE_IV_RESULT_PROPERTIES();

@end


@interface CGearSeedTimeResult : NSObject
{
  uint32_t  date, time, delay;
}

@property uint32_t  date, time, delay;

@end

@implementation CGearSeedTimeResult

@synthesize date, time, delay;

@end


@interface CGearAdjacentSeedResult : NSObject <IVResult>
{
  uint32_t  seed, date, time, delay, frame;
  DECLARE_IV_RESULT_VARIABLES();
  uint32_t  ivWord;
}

@property uint32_t  seed, date, time, delay, frame, ivWord;

@end

@implementation CGearAdjacentSeedResult

@synthesize seed, date, time, delay, frame, ivWord;
SYNTHESIZE_IV_RESULT_PROPERTIES();

@end


@interface NatureSeedSearchResult : NSObject <HashedSeedResultParameters>
{
  DECLARE_HASHED_SEED_RESULT_PARAMETERS_VARIABLES();
  
  uint32_t      cgearTime;
  Nature::Type  nature;
  uint32_t      frame;
  uint32_t      clusterSize;
  BOOL          isGenderless;
}

@property uint32_t      cgearTime;
@property Nature::Type  nature;
@property uint32_t      frame;
@property uint32_t      clusterSize;
@property BOOL          isGenderless;

@end

@implementation NatureSeedSearchResult

SYNTHESIZE_HASHED_SEED_RESULT_PARAMETERS_PROPERTIES();

@synthesize cgearTime, nature, frame, clusterSize, isGenderless;

@end


namespace
{

struct ResultHandler
{
  ResultHandler(SearcherController *c, bool isGenderless)
    : m_controller(c), m_isGenderless(isGenderless)
  {}
  
  void operator()(const CGearNatureSearcher::CGearNatureFrame &frame)
  {
    NatureSeedSearchResult  *result = [[NatureSeedSearchResult alloc] init];
    
    SetHashedSeedResultParameters(result, frame.seed);
    
    result.cgearTime = MakeUInt32Time(frame.cgearTime.hour,
                                      frame.cgearTime.minute,
                                      frame.cgearTime.second);
    result.nature = frame.nature;
    result.frame = frame.number;
    result.clusterSize = frame.clusterSize;
    result.isGenderless = m_isGenderless;
    
    [m_controller performSelectorOnMainThread: @selector(addResult:)
                  withObject: result
                  waitUntilDone: NO];
  }
  
  SearcherController  *m_controller;
  const bool          m_isGenderless;
};

struct ProgressHandler
{
  ProgressHandler(SearcherController *c)
    : controller(c)
  {}
  
  bool operator()(double progressDelta)
  {
    [controller performSelectorOnMainThread: @selector(adjustProgress:)
                withObject: [NSNumber numberWithDouble: progressDelta]
                waitUntilDone: NO];
    
    return ![controller searchIsCanceled];
  }
  
  SearcherController  *controller;
};

}



@implementation CGearSeedInspectorController

@synthesize seed, baseDelay;

@synthesize ivFrameParameterController;
@synthesize minIVFrame, maxIVFrame;

@synthesize year, actualDelay;
@synthesize considerSeconds, second;
@synthesize adjacentsDelayVariance, adjacentsTimeVariance;
@synthesize adjacentsMinIVFrame, adjacentsMaxIVFrame;
@synthesize adjacentsIVParameterController;

@synthesize noButtonHeld, oneButtonHeld, twoButtonsHeld, threeButtonsHeld;
@synthesize isGenderless, minClusterSize, secondsAdjustment;
@synthesize startFromInitialPIDFrame;
@synthesize minPIDFrame, maxPIDFrame;

- (NSString *)windowNibName
{
	return @"CGearSeedInspector";
}

- (void)awakeFromNib
{
  [super awakeFromNib];
  
  self.seed = nil;
  
  self.minIVFrame = 1;
  self.maxIVFrame = 100;
  
  self.year = uint32_t(NSDateToBoostDate([NSDate date]).year());
  self.considerSeconds = YES;
  self.second = 25;
  
  self.adjacentsDelayVariance = 10;
  self.adjacentsTimeVariance = 1;
  self.adjacentsMinIVFrame = 21;
  self.adjacentsMaxIVFrame = 26;
  
  [searcherController setGetValidatedSearchCriteriaSelector:
                      @selector(getValidatedSearchCriteria)];
  [searcherController setDoSearchWithCriteriaSelector:
                      @selector(doSearchWithCriteria:)];
  
  [[searcherController tableView] setTarget: self];
  [[searcherController tableView] setDoubleAction: @selector(inspectSeed:)];
  
  self.noButtonHeld = YES;
  self.oneButtonHeld = NO;
  self.twoButtonsHeld = NO;
  self.threeButtonsHeld = NO;
  
  self.isGenderless = NO;
  self.minClusterSize = 3;
  self.secondsAdjustment = 0;
  
  self.startFromInitialPIDFrame = YES;
  self.minPIDFrame = 50;
  self.maxPIDFrame = 100;
}

- (void)windowWillClose:(NSNotification *)notification
{
  if ([searcherController isSearching])
    [searcherController startStop: self];
}

- (void)updateActualDelay
{
  if (!seed)
  {
    self.actualDelay = nil;
  }
  else
  {
    uint32_t  base = [baseDelay unsignedIntValue];
    uint32_t  offset = year - 2000;
    
    if (offset > base)
      base += 65536;
    
    self.actualDelay = [NSNumber numberWithUnsignedInt: base - offset];
  }
}

- (void)setSeed:(NSNumber *)newValue
{
  if (seed != newValue)
  {
    seed = newValue;
    
    if (newValue == nil)
    {
      self.baseDelay = nil;
    }
    else
    {
      CGearSeed  cgSeed([seed unsignedIntValue],
                        [gen5ConfigController macAddressLow]);
      self.baseDelay = [NSNumber numberWithUnsignedInt: cgSeed.BaseDelay()];
      [self updateActualDelay];
    }
  }
}

- (void)setYear:(uint32_t)newValue
{
  if (year != newValue)
  {
    year = newValue;
    [self updateActualDelay];
  }
}

- (IBAction)toggleDropDownChoice:(id)sender
{
  HandleComboMenuItemChoice(sender);
}


- (IBAction)generateIVFrames:(id)sender
{
  if (!EndEditing([self window]))
    return;
  
  if (!seed)
    return;
  
  [ivFrameContentArray setContent: [NSMutableArray array]];
  
  uint32_t  frameNum = 0, limitFrame = minIVFrame - 1;
  
  CGearIVFrameGenerator  generator([seed unsignedIntValue],
                                   ivFrameParameterController.isRoamer ?
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
    
    CGearFrameResult  *row = [[CGearFrameResult alloc] init];
    
    row.frame = frame.number;
    SetIVResult(row, frame.ivs, ivFrameParameterController.isRoamer);
    
    [rowArray addObject: row];
  }
  
  [ivFrameContentArray addObjects: rowArray];
}


- (void)selectAndShowFrame:(uint32_t)frame
{
  NSArray  *rows = [ivFrameContentArray arrangedObjects];
  if (rows && ([rows count] > 0))
  {
    CGearFrameResult  *row = [rows objectAtIndex: 0];
    
    if (row.frame <= frame)
    {
      NSInteger  rowNum = frame - row.frame;
      
      [ivFrameTableView
        selectRowIndexes: [NSIndexSet indexSetWithIndex: rowNum]
        byExtendingSelection: NO];
      [ivFrameTableView scrollRowToVisible: rowNum];
    }
  }
}


- (IBAction)calculateTimes:(id)sender
{
  if (!EndEditing([self window]))
    return;
  
  if (!seed)
    return;
  
  [timeFinderContentArray setContent: [NSMutableArray array]];
  
  CGearSeed  cgSeed([seed unsignedIntValue],
                    [gen5ConfigController macAddressLow]);
  
  uint32_t  wantedSecond = considerSeconds ? second : -1;
  
  TimeSeed::TimeElements  elements = cgSeed.GetTimeElements(year, wantedSecond);
  
  NSMutableArray  *rows = [NSMutableArray arrayWithCapacity: elements.size()];
  
  TimeSeed::TimeElements::iterator  i;
  for (i = elements.begin(); i != elements.end(); ++i)
  {
    CGearSeedTimeResult  *row = [[CGearSeedTimeResult alloc] init];
    
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
  
  if (!seed)
    return;
  
  NSInteger  rowNum = [timeFinderTableView selectedRow];
  if (rowNum < 0)
    return;
  
  CGearSeedTimeResult  *seedTime =
    [[timeFinderContentArray arrangedObjects] objectAtIndex: rowNum];
  
  [adjacentsContentArray setContent: [NSMutableArray array]];
  
  uint32_t   macAddressLow = [gen5ConfigController macAddressLow];
  
  uint32_t  targetDelay = seedTime.delay;
  uint32_t  endDelay = targetDelay + adjacentsDelayVariance;
  uint32_t  startDelay = (targetDelay < adjacentsDelayVariance) ? 0 :
                           targetDelay - adjacentsDelayVariance;
  
  ptime  targetTime(UInt32DateAndTimeToBoostTime(seedTime.date, seedTime.time));
  
  ptime  endTime = targetTime + seconds(adjacentsTimeVariance);
  targetTime = targetTime - seconds(adjacentsTimeVariance);
  
  NSMutableArray  *rowArray =
    [NSMutableArray arrayWithCapacity:
      ((2 * adjacentsDelayVariance) + 1) * ((2 * adjacentsTimeVariance) + 1)];
  
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
    
    for (uint32_t delay = startDelay; delay <= endDelay; ++delay)
    {
      CGearSeed  s(yr, mo, dy, hr, mi, sc, delay, macAddressLow);
      
      CGearIVFrameGenerator  ivGenerator
        (s.m_rawSeed, (adjacentsIVParameterController.isRoamer ?
                        CGearIVFrameGenerator::Roamer :
                        CGearIVFrameGenerator::Normal));
      uint32_t   f;
      for (f = 0; f < (adjacentsMinIVFrame - 1); ++f)
        ivGenerator.AdvanceFrame();
      
      while (f++ < adjacentsMaxIVFrame)
      {
        ivGenerator.AdvanceFrame();
        
        CGearAdjacentSeedResult  *row = [[CGearAdjacentSeedResult alloc] init];
        
        row.seed = s.m_rawSeed;
        row.date = dt;
        row.time = tm;
        row.delay = delay;
        row.frame = f;
        
        IVs  ivs = ivGenerator.CurrentFrame().ivs;
        
        SetIVResult(row, ivs, adjacentsIVParameterController.isRoamer);
        row.ivWord = ivs.word;
        
        [rowArray addObject: row];
      }
    }
  }
  
  [adjacentsContentArray addObjects: rowArray];
}


// dummy method for error panel
- (void)alertDidEnd:(NSAlert *)alert returnCode:(NSInteger)returnCode
        contextInfo:(void *)contextInfo
{}


- (IBAction)findAdjacent:(id)sender
{
  IVs  minIVs = adjacentsIVParameterController.minIVs;
  IVs  maxIVs = adjacentsIVParameterController.maxIVs;
  
  NSArray                  *rows = [adjacentsContentArray arrangedObjects];
  NSInteger                numRows = [rows count];
  NSInteger                rowNum = [adjacentsTableView selectedRow];
  CGearAdjacentSeedResult  *row;
  
  while (++rowNum < numRows)
  {
    row = [rows objectAtIndex: rowNum];
    IVs  rowIVs(row.ivWord);
    
    if (rowIVs.betterThanOrEqual(minIVs) && rowIVs.worseThanOrEqual(maxIVs))
    {
      [adjacentsTableView
        selectRowIndexes: [NSIndexSet indexSetWithIndex: rowNum]
        byExtendingSelection: NO];
      [adjacentsTableView scrollRowToVisible: rowNum];
      break;
    }
  }
  
  if (row == nil)
  {
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"No matching row found"];
    [alert setInformativeText:@"No adjacents row has IVs matching those specified."];
    [alert setAlertStyle:NSWarningAlertStyle];
    
    [alert beginSheetModalForWindow: [self window] modalDelegate: self
           didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
           contextInfo:nil];
  }
}


- (NSValue*)getValidatedSearchCriteria
{
  using namespace boost::gregorian;
  using namespace boost::posix_time;
  
  if (!EndEditing([self window]))
    return nil;
  
  if (!seed)
    return nil;
  
  CGearNatureSearcher::Criteria  criteria;
  
  criteria.cgearSeed = [seed unsignedIntValue];
  
  criteria.hashedSeedParameters.macAddress = [gen5ConfigController macAddress];
  
  criteria.hashedSeedParameters.version = [gen5ConfigController version];
  criteria.hashedSeedParameters.dsType = [gen5ConfigController dsType];
  
  criteria.hashedSeedParameters.timer0Low = [gen5ConfigController timer0Low];
  criteria.hashedSeedParameters.timer0High = [gen5ConfigController timer0High];
  
  criteria.hashedSeedParameters.vcountLow = [gen5ConfigController vcountLow];
  criteria.hashedSeedParameters.vcountHigh = [gen5ConfigController vcountHigh];
  
  criteria.hashedSeedParameters.vframeLow = [gen5ConfigController vframeLow];
  criteria.hashedSeedParameters.vframeHigh = [gen5ConfigController vframeHigh];
  
  if (noButtonHeld)
  {
    criteria.hashedSeedParameters.heldButtons.push_back(0);  // no keys
  }
  if (oneButtonHeld)
  {
    criteria.hashedSeedParameters.heldButtons.insert
      (criteria.hashedSeedParameters.heldButtons.end(),
       Button::SingleButtons().begin(),
       Button::SingleButtons().end());
  }
  if (twoButtonsHeld)
  {
    criteria.hashedSeedParameters.heldButtons.insert
      (criteria.hashedSeedParameters.heldButtons.end(),
       Button::TwoButtonCombos().begin(),
       Button::TwoButtonCombos().end());
  }
  if (threeButtonsHeld)
  {
    criteria.hashedSeedParameters.heldButtons.insert
      (criteria.hashedSeedParameters.heldButtons.end(),
       Button::ThreeButtonCombos().begin(),
       Button::ThreeButtonCombos().end());
  }
  
  criteria.year = year;
  
  criteria.pid.natureMask = GetComboMenuBitMask(natureDropDown);
  if (isGenderless)
  {
    criteria.pid.gender = Gender::GENDERLESS;
    criteria.pid.genderRatio = Gender::NO_RATIO;
  }
  else
  {
    // just use any gender and ratio
    criteria.pid.gender = Gender::FEMALE;
    criteria.pid.genderRatio = Gender::ONE_HALF_FEMALE;
  }
  
  criteria.pid.startFromLowestFrame = startFromInitialPIDFrame;
  criteria.frameRange.min = minPIDFrame;
  criteria.frameRange.max = maxPIDFrame;
  
  criteria.minClusterSize = minClusterSize;
  criteria.secondsAdjustment = secondsAdjustment;
  
  return [NSValue valueWithPointer:new CGearNatureSearcher::Criteria(criteria)];
}

- (void)doSearchWithCriteria:(NSValue*)criteriaPtr
{
  std::auto_ptr<CGearNatureSearcher::Criteria> 
    criteria(static_cast<CGearNatureSearcher::Criteria*>
      ([criteriaPtr pointerValue]));
  
  CGearNatureSearcher  searcher;
  
  searcher.Search(*criteria,
                  ResultHandler(searcherController,
                                (criteria->pid.gender == Gender::GENDERLESS)),
                  ProgressHandler(searcherController));
}


- (void)inspectSeed:(id)sender
{
  NSInteger  rowNum = [sender clickedRow];
  
  if (rowNum >= 0)
  {
    NatureSeedSearchResult  *row =
      [[[searcherController arrayController] arrangedObjects]
        objectAtIndex: rowNum];
    
    if (row != nil)
    {
      StandardSeedInspectorController  *inspector =
        [[StandardSeedInspectorController alloc] init];
      [inspector window];
      
      [inspector setSeedFromResult: row];
      
      HashedSeedInspectorFramesTabController *framesTab =
        inspector.framesTabController;
      HashedSeedInspectorAdjacentsTabController *adjacentsTab =
        inspector.adjacentsTabController;
      
      framesTab.encounterLeadAbility = EncounterLead::OTHER;
      adjacentsTab.encounterLeadAbility = EncounterLead::OTHER;
      
      if (row.isGenderless)
      {
        framesTab.targetGender = Gender::GENDERLESS;
        framesTab.targetGenderRatio = Gender::NO_RATIO;
      }
      else
      {
        framesTab.targetGender = Gender::FEMALE;
        framesTab.targetGenderRatio = Gender::ONE_HALF_FEMALE;
      }
      
      adjacentsTab.targetGender = framesTab.targetGender;
      adjacentsTab.targetGenderRatio = framesTab.targetGenderRatio;
      
      framesTab.encounterFrameType = Gen5PIDFrameGenerator::EntraLinkFrame;
      adjacentsTab.encounterFrameType = Gen5PIDFrameGenerator::EntraLinkFrame;
      
      if (row.frame < [inspector.initialPIDFrame unsignedIntValue])
      {
        framesTab.startFromInitialPIDFrame = NO;
        framesTab.minPIDFrame = 1;
        adjacentsTab.matchOffsetFromInitialPIDFrame = NO;
      }
      
      adjacentsTab.pidFrame = row.frame;
      framesTab.maxPIDFrame = row.frame + 20;
      
      [framesTab generatePIDFrames: self];
      [framesTab selectAndShowPIDFrame: row.frame];
      
      [inspector showWindow: self];
    }
  }
}


@end
