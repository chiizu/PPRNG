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


#import "TrainerIDSearcherController.h"

#include "TrainerIDSearcher.h"
#include "Utilities.h"


using namespace pprng;

namespace
{

struct TrainerIDSearchResultHandler
{
  TrainerIDSearchResultHandler(SearcherController *c)
    : controller(c)
  {}
  
  void operator()(const Gen5TrainerIDFrame &frame)
  {
    NSMutableDictionary  *result =
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithUnsignedLongLong: frame.seed.rawSeed], @"seed",
        [NSString stringWithFormat: @"%.4d/%.2d/%.2d",
          frame.seed.year(), frame.seed.month(), frame.seed.day()], @"date",
        [NSString stringWithFormat: @"%.2d:%.2d:%.2d",
          frame.seed.hour, frame.seed.minute, frame.seed.second], @"time",
        [NSNumber numberWithUnsignedInt: frame.seed.timer0], @"timer0",
				[NSString stringWithFormat: @"%s",
          Button::ToString(frame.seed.heldButtons).c_str()], @"keys",
				[NSNumber numberWithUnsignedInt: frame.number], @"frame",
        [NSNumber numberWithUnsignedInt: frame.tid], @"tid",
        [NSNumber numberWithUnsignedInt: frame.sid], @"sid",
        nil];
    
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
    NSMutableDictionary  *result =
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithUnsignedLongLong: frame.seed.rawSeed], @"seed",
        [NSString stringWithFormat: @"%.4d/%.2d/%.2d",
          frame.seed.year(), frame.seed.month(), frame.seed.day()], @"date",
        [NSString stringWithFormat: @"%.2d:%.2d:%.2d",
          frame.seed.hour, frame.seed.minute, frame.seed.second], @"time",
        [NSNumber numberWithUnsignedInt: frame.seed.timer0], @"timer0",
				[NSString stringWithFormat: @"%s",
          Button::ToString(frame.seed.heldButtons).c_str()], @"keys",
				[NSNumber numberWithUnsignedInt: frame.number], @"frame",
        [NSNumber numberWithUnsignedInt: frame.tid], @"tid",
        [NSNumber numberWithUnsignedInt: frame.sid], @"sid",
        nil];
    
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

- (NSString *)windowNibName
{
	return @"TrainerIDSearcher";
}

- (void)awakeFromNib
{
  [super awakeFromNib];
  
  [[pidSeedField formatter] setFormatWidth: 16];
  
  [[[[pidFrameTableView tableColumnWithIdentifier: @"pid"] dataCell] formatter]
   setFormatWidth: 8];
  
  [tidSidSearcherController setGetValidatedSearchCriteriaSelector:
                            @selector(tidSidSearchGetValidatedSearchCriteria)];
  [tidSidSearcherController setDoSearchWithCriteriaSelector:
                            @selector(tidSidSearchDoSearchWithCriteria:)];
  
  [[[[[tidSidSearcherController tableView] tableColumnWithIdentifier: @"seed"]
    dataCell] formatter]
   setFormatWidth: 16];
  
  NSDate  *now = [NSDate date];
  [tidSidFromDateField setObjectValue: now];
  [tidSidToDateField setObjectValue: now];
  
  [idFrameSearcherController setGetValidatedSearchCriteriaSelector:
                             @selector(idFrameSearchGetValidatedSearchCriteria)];
  [idFrameSearcherController setDoSearchWithCriteriaSelector:
                             @selector(idFrameSearchDoSearchWithCriteria:)];
  
  [[[[[idFrameSearcherController tableView] tableColumnWithIdentifier: @"seed"]
    dataCell] formatter]
   setFormatWidth: 16];
  
  [idFrameDateField setObjectValue: now];
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
  if ([[pidSeedField stringValue] length] == 0)
  {
    return;
  }
  
  [pidFrameContentArray setContent: [NSMutableArray array]];
  
  HashedSeed  seed([[pidSeedField objectValue] unsignedLongLongValue]);
  
  uint32_t  minPIDFrame = [pidMinFrameField intValue];
  uint32_t  maxPIDFrame = [pidMaxFrameField intValue];
  uint32_t  frameNum = 0, limitFrame = minPIDFrame - 1;
  
  Gen5PIDFrameGenerator::Parameters  frameParameters;
  
  frameParameters.useCompoundEyes = false;
  frameParameters.tid = 0;
  frameParameters.sid = 0;
  
  frameParameters.frameType = Gen5PIDFrameGenerator::GrassCaveFrame;
  Gen5PIDFrameGenerator  gcGenerator(seed, frameParameters);
  
  frameParameters.frameType = Gen5PIDFrameGenerator::FishingFrame;
  Gen5PIDFrameGenerator  fsGenerator(seed, frameParameters);
  
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
  stGenerator.AdvanceFrame();
  stGenerator.AdvanceFrame();
  stGenerator.AdvanceFrame();
  pidGenerator.AdvanceFrame();
  pidGenerator.AdvanceFrame();
  pidGenerator.AdvanceFrame();
  pidGenerator.AdvanceFrame();
  
  while (frameNum < limitFrame)
  {
    gcGenerator.AdvanceFrame();
    fsGenerator.AdvanceFrame();
    sdGenerator.AdvanceFrame();
    bsGenerator.AdvanceFrame();
    stGenerator.AdvanceFrame();
    pidGenerator.AdvanceFrame();
    ++frameNum;
  }
  
  NSMutableArray  *rowArray =
    [NSMutableArray arrayWithCapacity: maxPIDFrame - minPIDFrame + 1];
  
  while (frameNum < maxPIDFrame)
  {
    gcGenerator.AdvanceFrame();
    fsGenerator.AdvanceFrame();
    sdGenerator.AdvanceFrame();
    bsGenerator.AdvanceFrame();
    stGenerator.AdvanceFrame();
    pidGenerator.AdvanceFrame();
    ++frameNum;
    
    Gen5PIDFrame  gcFrame = gcGenerator.CurrentFrame();
    Gen5PIDFrame  fsFrame = fsGenerator.CurrentFrame();
    Gen5PIDFrame  sdFrame = sdGenerator.CurrentFrame();
    Gen5PIDFrame  bsFrame = bsGenerator.CurrentFrame();
    Gen5PIDFrame  stFrame = stGenerator.CurrentFrame();
    Gen5PIDFrame  pidFrame = pidGenerator.CurrentFrame();
    
    uint32_t  genderValue = pidFrame.pid.GenderValue();
    
    NSMutableDictionary  *result =
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
				[NSNumber numberWithUnsignedInt: frameNum], @"frame",
        [NSString stringWithFormat: @"%s",
          Nature::ToString(pidFrame.nature).c_str()], @"nature",
        [NSNumber numberWithUnsignedInt: pidFrame.pid.word], @"pid",
        [NSNumber numberWithUnsignedInt: pidFrame.pid.Gen5Ability()],
          @"ability",
        ((genderValue < 31) ? @"♀" : @"♂"), @"gender18",
        ((genderValue < 63) ? @"♀" : @"♂"), @"gender14",
        ((genderValue < 127) ? @"♀" : @"♂"), @"gender12",
        ((genderValue < 191) ? @"♀" : @"♂"), @"gender34",
        (gcFrame.synched ? @"Y" : @""), @"syncA",
        (fsFrame.synched ? @"Y" : @""), @"syncB",
        (stFrame.synched ? @"Y" : @""), @"syncC",
        [NSString stringWithFormat: @"%d", gcFrame.esv], @"esvL",
        [NSString stringWithFormat: @"%d", fsFrame.esv], @"esvW",
        (fsFrame.isEncounter ? @"Y" : @""), @"canFish",
        (sdFrame.isEncounter ? @"Y" : @""), @"dustIsPoke",
        (bsFrame.isEncounter ? @"" : @"Y"), @"shadowIsPoke",
        nil];
    
    [rowArray addObject: result];
  }
  
  [pidFrameContentArray addObjects: rowArray];
}

- (IBAction) toggleTID:(id)sender
{
  BOOL enabled = [tidSidEnableDesiredTidButton state];
  [tidSidDesiredTidField setEnabled: enabled];
}

- (IBAction) toggleTime:(id)sender
{
  BOOL  hourEnabled = [idFrameEnableHourButton state];
  BOOL  minuteEnabled = [idFrameEnableMinuteButton state];
  BOOL  secondEnabled = [idFrameEnableSecondButton state];
  
  [idFrameStartHour setEnabled: hourEnabled];
  [idFrameHourStepper setEnabled: hourEnabled];
  [idFrameEnableMinuteButton setEnabled: hourEnabled];
  [idFrameStartMinute setEnabled: hourEnabled && minuteEnabled];
  [idFrameMinuteStepper setEnabled: hourEnabled && minuteEnabled];
  [idFrameEnableSecondButton setEnabled: hourEnabled && minuteEnabled];
  [idFrameStartSecond
   setEnabled: hourEnabled && minuteEnabled && secondEnabled];
  [idFrameSecondStepper
   setEnabled: hourEnabled && minuteEnabled && secondEnabled];
}

// dummy method for error panel
- (void)alertDidEnd:(NSAlert *)alert returnCode:(NSInteger)returnCode
        contextInfo:(void *)contextInfo
{}


- (NSValue*)tidSidSearchGetValidatedSearchCriteria
{
  using namespace boost::gregorian;
  using namespace boost::posix_time;
  
  NSInteger  rowNum = [pidFrameTableView selectedRow];
  if (rowNum < 0)
  {
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Please Select a Target PID Row"];
    [alert setInformativeText:@"You need to choose a row from the upper table to be the PID that you wish to make shiny."];
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
  
  if ([tidSidNoKeyHeldButton state])
  {
    criteria.seedParameters.heldButtons.push_back(0);  // no keys
  }
  if ([tidSidOneKeyHeldButton state])
  {
    criteria.seedParameters.heldButtons.insert
      (criteria.seedParameters.heldButtons.end(),
       Button::SingleButtons().begin(),
       Button::SingleButtons().end());
  }
  if ([tidSidTwoKeysHeldButton state])
  {
    criteria.seedParameters.heldButtons.insert
      (criteria.seedParameters.heldButtons.end(),
       Button::TwoButtonCombos().begin(),
       Button::TwoButtonCombos().end());
  }
  if ([tidSidThreeKeysHeldButton state])
  {
    criteria.seedParameters.heldButtons.insert
      (criteria.seedParameters.heldButtons.end(),
       Button::ThreeButtonCombos().begin(),
       Button::ThreeButtonCombos().end());
  }
  
  criteria.seedParameters.fromTime =
    ptime(NSDateToBoostDate([tidSidFromDateField objectValue]), seconds(0));
  
  criteria.seedParameters.toTime =
    ptime(NSDateToBoostDate([tidSidToDateField objectValue]),
                            hours(23) + minutes(59) + seconds(59));
  
  criteria.frame.min = [tidSidMinFrameField intValue];
  criteria.frame.max = [tidSidMaxFrameField intValue];
  
  criteria.hasTID = [tidSidEnableDesiredTidButton state];
  if (criteria.hasTID)
  {
    criteria.tid = [tidSidDesiredTidField intValue];
  }
  
  NSDictionary  *row =
    [[pidFrameContentArray arrangedObjects] objectAtIndex: rowNum];
  NSNumber  *pid = [row objectForKey: @"pid"];
  criteria.shinyPID = [pid unsignedIntValue];
  criteria.hasShinyPID = true;
  
  criteria.wildShiny = [tidSidWildShinyButton state];
  criteria.giftShiny = [tidSidGiftShinyButton state];
  criteria.eggShiny = [tidSidEggShinyButton state];
  
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
  
  date  d = NSDateToBoostDate([idFrameDateField objectValue]);
  
  time_duration  startTime, endTime;
  
  if ([idFrameEnableHourButton state])
  {
    startTime = hours([idFrameStartHour intValue]);
    if ([idFrameEnableMinuteButton isEnabled] &&
        [idFrameEnableMinuteButton state])
    {
      startTime = startTime + minutes([idFrameStartMinute intValue]);
      if ([idFrameEnableSecondButton isEnabled] &&
          [idFrameEnableSecondButton state])
      {
        startTime = startTime + seconds([idFrameStartSecond intValue]);
        
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
  
  criteria.seedParameters.heldButtons.push_back
    ([[idFrameKeyOnePopUp selectedItem] tag] |
     [[idFrameKeyTwoPopUp selectedItem] tag] |
     [[idFrameKeyThreePopUp selectedItem] tag]);
  
  criteria.frame.min = [idFrameMinFrameField intValue];
  criteria.frame.max = [idFrameMaxFrameField intValue];
  
  criteria.hasTID = true;
  criteria.tid = [idFrameTrainerIDField intValue];
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