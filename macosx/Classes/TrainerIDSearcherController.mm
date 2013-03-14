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


#import "TrainerIDSearcherController.h"

#import "SearchResultProtocols.h"

#include "TrainerIDSearcher.h"
#include "Utilities.h"


using namespace pprng;

@interface DesiredShinyPIDFrameResult : NSObject <PIDResult>
{
  uint64_t  rawSeed;
  uint32_t  frame;
  
  DECLARE_PID_RESULT_VARIABLES();
  
  NSString  *grassCaveSurfSpotFrame;
  NSString  *swarmFrame, *dustFrame, *shadowFrame;
  NSString  *stationaryFrame, *fishFrame;
  
  ESV::Value  landESV, surfESV, fishESV;
}

@property uint64_t  rawSeed;
@property uint32_t  frame;

@property (copy) NSString  *grassCaveSurfSpotFrame;
@property (copy) NSString  *swarmFrame, *dustFrame, *shadowFrame;
@property (copy) NSString  *stationaryFrame, *fishFrame;

@property ESV::Value  landESV, surfESV, fishESV;

@end

@implementation DesiredShinyPIDFrameResult

@synthesize rawSeed, frame;

SYNTHESIZE_PID_RESULT_PROPERTIES();

@synthesize grassCaveSurfSpotFrame;
@synthesize swarmFrame, dustFrame, shadowFrame;
@synthesize stationaryFrame, fishFrame;
@synthesize landESV, surfESV, fishESV;

@end


@interface TrainerIDSeedSearchResult : NSObject <HashedSeedResultParameters>
{
  DECLARE_HASHED_SEED_RESULT_PARAMETERS_VARIABLES();
  
  uint32_t  frame;
  uint32_t  tid, sid;
  BOOL      wildShiny, giftShiny, eggShiny;
}

@property uint32_t  frame, tid, sid;
@property BOOL      wildShiny, giftShiny, eggShiny;

@end

@implementation TrainerIDSeedSearchResult

SYNTHESIZE_HASHED_SEED_RESULT_PARAMETERS_PROPERTIES();

@synthesize frame, tid, sid;
@synthesize wildShiny, giftShiny, eggShiny;

@end



namespace
{

struct TrainerIDSearchResultHandler
{
  TrainerIDSearchResultHandler(SearcherController *c)
    : controller(c)
  {}
  
  void operator()(const Gen5TrainerIDFrame &frame)
  {
    TrainerIDSeedSearchResult  *result =
      [[TrainerIDSeedSearchResult alloc] init];
    
    SetHashedSeedResultParameters(result, frame.seed);
    
    result.frame = frame.number;
    result.tid = frame.tid;
    result.sid = frame.sid;
    result.wildShiny = frame.wildShiny;
    result.giftShiny = frame.giftShiny;
    result.eggShiny = frame.eggShiny;
    
    [controller performSelectorOnMainThread: @selector(addResult:)
                withObject: result
                waitUntilDone: NO];
  }
  
  SearcherController  *controller;
};

struct TrainerIDSearchProgressHandler
{
  TrainerIDSearchProgressHandler(SearcherController *c)
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

struct IDFrameSearchResultHandler
{
  IDFrameSearchResultHandler(SearcherController *c)
    : controller(c)
  {}
  
  void operator()(const Gen5TrainerIDFrame &frame)
  {
    TrainerIDSeedSearchResult  *result =
      [[TrainerIDSeedSearchResult alloc] init];
    
    SetHashedSeedResultParameters(result, frame.seed);
    
    result.frame = frame.number;
    result.tid = frame.tid;
    result.sid = frame.sid;
    
    [controller performSelectorOnMainThread: @selector(addResult:)
                withObject: result
                waitUntilDone: NO];
  }
  
  SearcherController  *controller;
};

struct IDFrameSearchProgressHandler
{
  IDFrameSearchProgressHandler(SearcherController *c)
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


@implementation TrainerIDSearcherController

@synthesize  ivSeed;
  
@synthesize  startFromInitialPIDFrame;
@synthesize  minPIDFrame, maxPIDFrame;

@synthesize  fromDate, toDate;
@synthesize  noButtonHeld, oneButtonHeld, twoButtonsHeld, threeButtonsHeld;

@synthesize  minTIDFrame, maxTIDFrame;
@synthesize  wildShiny, giftShiny, eggShiny;
@synthesize  desiredTID, desiredSID;

@synthesize  foundTID;
@synthesize  startDate;
  
@synthesize  considerHour;
@synthesize  startHour;
  
@synthesize  considerMinute;
@synthesize  startMinute;
  
@synthesize  considerSecond;
@synthesize  startSecond;
  
@synthesize  button1, button2, button3;
  
@synthesize  minFoundTIDFrame, maxFoundTIDFrame;

- (NSString *)windowNibName
{
	return @"TrainerIDSearcher";
}

- (void)awakeFromNib
{
  [super awakeFromNib];
  
  self.ivSeed = nil;
  self.startFromInitialPIDFrame = YES;
  self.minPIDFrame = 50;
  self.maxPIDFrame = 100;
  
  [tidSidSearcherController setGetValidatedSearchCriteriaSelector:
                            @selector(tidSidSearchGetValidatedSearchCriteria)];
  [tidSidSearcherController setDoSearchWithCriteriaSelector:
                            @selector(tidSidSearchDoSearchWithCriteria:)];
  
  NSDate  *now = [NSDate date];
  self.fromDate = now;
  self.toDate = now;
  
  self.noButtonHeld = YES;
  self.oneButtonHeld = NO;
  self.twoButtonsHeld = NO;
  self.threeButtonsHeld = NO;
  
  self.minTIDFrame = 30;
  self.maxTIDFrame = 50;
  
  self.wildShiny = YES;
  self.giftShiny = YES;
  self.eggShiny = YES;
  
  self.desiredTID = nil;
  self.desiredSID = nil;
  
  [idFrameSearcherController setGetValidatedSearchCriteriaSelector:
                             @selector(idFrameSearchGetValidatedSearchCriteria)];
  [idFrameSearcherController setDoSearchWithCriteriaSelector:
                             @selector(idFrameSearchDoSearchWithCriteria:)];
  
  self.foundTID = nil;
  self.startDate = now;
  self.considerHour = YES;
  self.startHour = 0;
  self.considerMinute = YES;
  self.startMinute = 0;
  self.considerSecond = YES;
  self.startSecond = 0;
  
  self.button1 = 0;
  self.button2 = 0;
  self.button3 = 0;
  
  self.minFoundTIDFrame = 10;
  self.maxFoundTIDFrame = 50;
}

- (void)windowWillClose:(NSNotification *)notification
{
  if ([tidSidSearcherController isSearching])
    [tidSidSearcherController startStop: self];
  if ([idFrameSearcherController isSearching])
    [idFrameSearcherController startStop: self];
}

- (IBAction) generatePIDFrames:(id)sender
{
  if (!EndEditing([self window]))
    return;
  
  if (!ivSeed)
    return;
  
  [pidFrameContentArray setContent: [NSMutableArray array]];
  
  HashedSeed  seed([gen5ConfigController version],
                   [ivSeed unsignedLongLongValue]);
  
  uint32_t  skippedFrames = startFromInitialPIDFrame ?
                              seed.GetSkippedPIDFrames(false) : minPIDFrame - 1;
  
  Gen5PIDFrameGenerator::Parameters  frameParameters;
  
  frameParameters.leadAbility = EncounterLead::SYNCHRONIZE;
  frameParameters.targetGender = Gender::ANY;
  frameParameters.targetRatio = Gender::ANY_RATIO;
  frameParameters.tid = 0;
  frameParameters.sid = 0;
  
  frameParameters.isBlack2White2 =
    Game::IsBlack2White2([gen5ConfigController version]);
  frameParameters.hasShinyCharm = false;
  frameParameters.memoryLinkUsed = false;
  
  frameParameters.startFromLowestFrame = false;  // need to handle manually
  
  frameParameters.frameType = Gen5PIDFrameGenerator::GrassCaveFrame;
  Gen5PIDFrameGenerator  gcGenerator(seed, frameParameters);
  
  frameParameters.frameType = Gen5PIDFrameGenerator::SurfingFrame;
  Gen5PIDFrameGenerator  sfGenerator(seed, frameParameters);
  
  frameParameters.frameType = Gen5PIDFrameGenerator::FishingFrame;
  Gen5PIDFrameGenerator  fsGenerator(seed, frameParameters);
  
  frameParameters.frameType = Gen5PIDFrameGenerator::SwarmFrame;
  Gen5PIDFrameGenerator  swGenerator(seed, frameParameters);
  
  frameParameters.frameType = Gen5PIDFrameGenerator::SwirlingDustFrame;
  Gen5PIDFrameGenerator  sdGenerator(seed, frameParameters);
  
  frameParameters.frameType = Gen5PIDFrameGenerator::BridgeShadowFrame;
  Gen5PIDFrameGenerator  bsGenerator(seed, frameParameters);
  
  frameParameters.frameType = Gen5PIDFrameGenerator::StationaryFrame;
  Gen5PIDFrameGenerator  stGenerator(seed, frameParameters);
  
  frameParameters.frameType = Gen5PIDFrameGenerator::StarterFossilGiftFrame;
  Gen5PIDFrameGenerator  pidGenerator(seed, frameParameters);
  
  // get the PIDs in sync
  gcGenerator.AdvanceFrame();
  sfGenerator.AdvanceFrame();
  stGenerator.AdvanceFrame();
  stGenerator.AdvanceFrame();
  stGenerator.AdvanceFrame();
  pidGenerator.AdvanceFrame();
  pidGenerator.AdvanceFrame();
  pidGenerator.AdvanceFrame();
  pidGenerator.AdvanceFrame();
  
  while (pidGenerator.CurrentFrame().number <= skippedFrames)
  {
    gcGenerator.AdvanceFrame();
    sfGenerator.AdvanceFrame();
    fsGenerator.AdvanceFrame();
    swGenerator.AdvanceFrame();
    sdGenerator.AdvanceFrame();
    bsGenerator.AdvanceFrame();
    stGenerator.AdvanceFrame();
    pidGenerator.AdvanceFrame();
  }
  
  NSMutableArray  *rowArray =
    [NSMutableArray arrayWithCapacity: maxPIDFrame - minPIDFrame + 1];
  
  while (pidGenerator.CurrentFrame().number < maxPIDFrame)
  {
    gcGenerator.AdvanceFrame();
    sfGenerator.AdvanceFrame();
    fsGenerator.AdvanceFrame();
    swGenerator.AdvanceFrame();
    sdGenerator.AdvanceFrame();
    bsGenerator.AdvanceFrame();
    stGenerator.AdvanceFrame();
    pidGenerator.AdvanceFrame();
    
    Gen5PIDFrame  gcFrame = gcGenerator.CurrentFrame();
    Gen5PIDFrame  sfFrame = sfGenerator.CurrentFrame();
    Gen5PIDFrame  fsFrame = fsGenerator.CurrentFrame();
    Gen5PIDFrame  swFrame = swGenerator.CurrentFrame();
    Gen5PIDFrame  sdFrame = sdGenerator.CurrentFrame();
    Gen5PIDFrame  bsFrame = bsGenerator.CurrentFrame();
    Gen5PIDFrame  stFrame = stGenerator.CurrentFrame();
    Gen5PIDFrame  pidFrame = pidGenerator.CurrentFrame();
    
    DesiredShinyPIDFrameResult  *row =
      [[DesiredShinyPIDFrameResult alloc] init];
    
    row.frame = pidFrame.number;
    SetPIDResult(row, pidFrame.pid, 0, 0,
                 pidFrame.nature, pidFrame.pid.Gen5Ability(),
                 Gender::ANY, Gender::ANY_RATIO);
    
    if (gcFrame.number > skippedFrames)
    {
      row.grassCaveSurfSpotFrame = gcFrame.abilityActivated ? @"S" : @"O";
      row.landESV = gcFrame.esv;
      row.surfESV = sfFrame.esv;
      row.fishESV = fsFrame.esv; // waterspot
    }
    if ((fsFrame.number > skippedFrames) && fsFrame.isEncounter)
    {
      row.fishFrame = fsFrame.abilityActivated ? @"S" : @"O";
      row.fishESV = fsFrame.esv;
    }
    if ((swFrame.number > skippedFrames) && (swFrame.esv == ESV::SWARM))
    {
      row.swarmFrame = swFrame.abilityActivated ? @"S" : @"O";
    }
    if ((sdFrame.number > skippedFrames) && sdFrame.isEncounter)
    {
      row.dustFrame = sdFrame.abilityActivated ? @"S" : @"O";
    }
    if ((bsFrame.number > skippedFrames) && bsFrame.isEncounter)
    {
      row.shadowFrame = bsFrame.abilityActivated ? @"S" : @"O";
    }
    if (stFrame.number > skippedFrames)
    {
      row.stationaryFrame = stFrame.abilityActivated ? @"S" : @"O";
    }
    
    [rowArray addObject: row];
  }
  
  [pidFrameContentArray addObjects: rowArray];
}

// dummy method for error panel
- (void)alertDidEnd:(NSAlert *)alert returnCode:(NSInteger)returnCode
        contextInfo:(void *)contextInfo
{}


- (NSValue*)tidSidSearchGetValidatedSearchCriteria
{
  using namespace boost::gregorian;
  using namespace boost::posix_time;
  
  if (!EndEditing([self window]))
    return nil;
  
  NSInteger  rowNum = [pidFrameTableView selectedRow];
  if ((rowNum < 0) && (desiredTID == nil))
  {
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Please Set Additional Criteria"];
    [alert setInformativeText:@"You need to choose a row from the upper table to indicate the PID that you wish to be shiny and/or set a desired TID (with optional SID) in order to run a search."];
    [alert setAlertStyle:NSWarningAlertStyle];
    
    [alert beginSheetModalForWindow:[self window] modalDelegate:self
           didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
           contextInfo:nil];
    
    return nil;
  }
  
  TrainerIDSearcher::Criteria  criteria;
  
  criteria.seedParameters.macAddress = [gen5ConfigController macAddress];
  
  criteria.seedParameters.version = [gen5ConfigController version];
  criteria.seedParameters.dsType = [gen5ConfigController dsType];
  
  criteria.seedParameters.timer0Low = [gen5ConfigController timer0Low];
  criteria.seedParameters.timer0High = [gen5ConfigController timer0High];
  
  criteria.seedParameters.vcountLow = [gen5ConfigController vcountLow];
  criteria.seedParameters.vcountHigh = [gen5ConfigController vcountHigh];
  
  criteria.seedParameters.vframeLow = [gen5ConfigController vframeLow];
  criteria.seedParameters.vframeHigh = [gen5ConfigController vframeHigh];
  
  if (noButtonHeld)
  {
    criteria.seedParameters.heldButtons.push_back(0);  // no buttons
  }
  if (oneButtonHeld)
  {
    criteria.seedParameters.heldButtons.insert
      (criteria.seedParameters.heldButtons.end(),
       Button::SingleButtons().begin(),
       Button::SingleButtons().end());
  }
  if (twoButtonsHeld)
  {
    criteria.seedParameters.heldButtons.insert
      (criteria.seedParameters.heldButtons.end(),
       Button::TwoButtonCombos().begin(),
       Button::TwoButtonCombos().end());
  }
  if (threeButtonsHeld)
  {
    criteria.seedParameters.heldButtons.insert
      (criteria.seedParameters.heldButtons.end(),
       Button::ThreeButtonCombos().begin(),
       Button::ThreeButtonCombos().end());
  }
  
  criteria.seedParameters.fromTime =
    ptime(NSDateToBoostDate(fromDate), seconds(0));
  
  criteria.seedParameters.toTime =
    ptime(NSDateToBoostDate(toDate), hours(23) + minutes(59) + seconds(59));
  
  criteria.frame.min = minTIDFrame;
  criteria.frame.max = maxTIDFrame;
  
  criteria.hasTID = (desiredTID != nil);
  if (criteria.hasTID)
    criteria.tid = [desiredTID unsignedIntValue];
  
  criteria.hasSID = (desiredSID != nil);
  if (criteria.hasSID)
    criteria.sid = [desiredSID unsignedIntValue];
  
  if (rowNum >= 0)
  {
    DesiredShinyPIDFrameResult  *row =
      [[pidFrameContentArray arrangedObjects] objectAtIndex: rowNum];
    criteria.shinyPID = row.pid;
    criteria.hasShinyPID = true;
  }
  else
  {
    criteria.hasShinyPID = false;
  }
  
  criteria.wildShiny = wildShiny;
  criteria.giftShiny = giftShiny;
  criteria.eggShiny = eggShiny;
  
  if (criteria.ExpectedNumberOfResults() > 10000)
  {
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Please Limit Search Parameters"];
    [alert setInformativeText:@"The current search parameters are expected to return more than 10,000 results. Please specify a TID, limit the date range, use fewer held keys, or other similar settings to reduce the number of expected results."];
    [alert setAlertStyle:NSWarningAlertStyle];
    
    [alert beginSheetModalForWindow:[self window] modalDelegate:self
           didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
           contextInfo:nil];
    
    return nil;
  }
  else
  {
    return [NSValue
             valueWithPointer: new TrainerIDSearcher::Criteria(criteria)];
  }
}

- (void)tidSidSearchDoSearchWithCriteria:(NSValue*)criteriaPtr
{
  std::auto_ptr<TrainerIDSearcher::Criteria> 
    criteria(static_cast<TrainerIDSearcher::Criteria*>
      ([criteriaPtr pointerValue]));
  
  TrainerIDSearcher  searcher;
  
  searcher.Search(*criteria,
                  TrainerIDSearchResultHandler(tidSidSearcherController),
                  TrainerIDSearchProgressHandler(tidSidSearcherController));
}

- (NSValue*)idFrameSearchGetValidatedSearchCriteria
{
  using namespace boost::gregorian;
  using namespace boost::posix_time;

  if (!EndEditing([self window]))
    return nil;
  
  if (!foundTID)
    return nil;
  
  TrainerIDSearcher::Criteria  criteria;
  
  criteria.seedParameters.macAddress = [gen5ConfigController macAddress];
  
  criteria.seedParameters.version = [gen5ConfigController version];
  criteria.seedParameters.dsType = [gen5ConfigController dsType];
  
  criteria.seedParameters.timer0Low = [gen5ConfigController timer0Low];
  criteria.seedParameters.timer0High = [gen5ConfigController timer0High];
  
  criteria.seedParameters.vcountLow = [gen5ConfigController vcountLow];
  criteria.seedParameters.vcountHigh = [gen5ConfigController vcountHigh];
  
  criteria.seedParameters.vframeLow = [gen5ConfigController vframeLow];
  criteria.seedParameters.vframeHigh = [gen5ConfigController vframeHigh];
  
  date  d = NSDateToBoostDate(startDate);
  
  time_duration  startTime, endTime;
  
  if (considerHour)
  {
    startTime = hours(startHour);
    if (considerMinute)
    {
      startTime = startTime + minutes(startMinute);
      if (considerSecond)
      {
        startTime = startTime + seconds(startSecond);
        
        endTime = startTime + seconds(10);
        startTime = startTime - seconds(5);
      }
      else
      {
        endTime = startTime + seconds(59);
      }
    }
    else
    {
      endTime = startTime + minutes(59) + seconds(59);
    }
  }
  else
  {
    startTime = seconds(0);
    endTime = hours(23) + minutes(59) + seconds(59);
  }
  
  criteria.seedParameters.fromTime = ptime(d, startTime);
  criteria.seedParameters.toTime = ptime(d, endTime);
  
  criteria.seedParameters.heldButtons.push_back(button1 | button2 | button3);
  
  criteria.frame.min = minFoundTIDFrame;
  criteria.frame.max = maxFoundTIDFrame;
  
  criteria.hasTID = true;
  criteria.tid = [foundTID unsignedIntValue];
  criteria.hasSID = false;
  criteria.hasShinyPID = false;
  
  if (criteria.ExpectedNumberOfResults() > 100)
  {
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Please Limit Search Parameters"];
    [alert setInformativeText:@"The current search parameters are expected to return more than 100 results. Please limit the frame range of the search in order to reduce the number of expected results."];
    [alert setAlertStyle:NSWarningAlertStyle];
    
    [alert beginSheetModalForWindow:[self window] modalDelegate:self
           didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
           contextInfo:nil];
    
    return nil;
  }
  else
  {
    return [NSValue valueWithPointer: new TrainerIDSearcher::Criteria(criteria)];
  }
}

- (void)idFrameSearchDoSearchWithCriteria:(NSValue*)criteriaPtr
{
  std::auto_ptr<TrainerIDSearcher::Criteria> 
    criteria(static_cast<TrainerIDSearcher::Criteria*>
      ([criteriaPtr pointerValue]));
  
  TrainerIDSearcher  searcher;
  
  searcher.Search(*criteria,
                  IDFrameSearchResultHandler(idFrameSearcherController),
                  IDFrameSearchProgressHandler(idFrameSearcherController));
}


@end
