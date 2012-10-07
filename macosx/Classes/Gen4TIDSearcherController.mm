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


#import "Gen4TIDSearcherController.h"

#import "SearchResultProtocols.h"
#import "Gen4SeedInspectorController.h"

#include "Gen4TIDSearcher.h"
#include "TimeSeed.h"
#include "Utilities.h"


using namespace pprng;


@interface Gen4TIDSeedSearchResult : NSObject
{
  uint32_t  seed, delay;
  uint32_t  tid, sid;
}

@property uint32_t  seed, delay, tid, sid;

@end

@implementation Gen4TIDSeedSearchResult

@synthesize seed, delay, tid, sid;

@end



namespace
{

struct TIDSearchResultHandler
{
  TIDSearchResultHandler(SearcherController *c)
    : controller(c)
  {}
  
  void operator()(const Gen4TrainerIDFrame &frame)
  {
    Gen4TIDSeedSearchResult  *result = [[Gen4TIDSeedSearchResult alloc] init];
    
    result.seed = frame.seed;
    result.delay = TimeSeed(frame.seed).BaseDelay();
    result.tid = frame.tid;
    result.sid = frame.sid;
    
    [controller performSelectorOnMainThread: @selector(addResult:)
                withObject: result
                waitUntilDone: NO];
  }
  
  SearcherController  *controller;
};

struct TIDSearchProgressHandler
{
  TIDSearchProgressHandler(SearcherController *c)
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

struct FoundIDSearchCriteria : public SearchCriteria
{
  uint32_t                  tid;
  boost::posix_time::ptime  startTime;
  uint32_t                  minDelay, maxDelay;
  
  uint64_t ExpectedNumberOfResults() const
  {
    uint64_t  delays = maxDelay - minDelay + 1;
    
    return delays / 65536;
  }
};

struct FoundIDSeedSearchResultHandler
{
  FoundIDSeedSearchResultHandler(SearcherController *c,
                                 uint32_t delayAdjustment)
    : m_controller(c), m_delayAdjustment(delayAdjustment)
  {}
  
  void operator()(const Gen4TrainerIDFrame &frame)
  {
    Gen4TIDSeedSearchResult  *result = [[Gen4TIDSeedSearchResult alloc] init];
    
    result.seed = frame.seed;
    result.delay = TimeSeed(frame.seed).BaseDelay() - m_delayAdjustment;
    result.tid = frame.tid;
    result.sid = frame.sid;
    
    [m_controller performSelectorOnMainThread: @selector(addResult:)
                  withObject: result
                  waitUntilDone: NO];
  }
  
  SearcherController  *m_controller;
  const uint32_t      m_delayAdjustment;
};

struct FoundIDSeedSearchProgressHandler
{
  FoundIDSeedSearchProgressHandler(SearcherController *c)
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


@implementation Gen4TIDSearcherController

@synthesize  shinyPID, desiredTID, desiredSID;
@synthesize  minSearchDelay, maxSearchDelay;
@synthesize  foundTID;
@synthesize  startDate;
@synthesize  startHour, startMinute, startSecond;
@synthesize  minFoundDelay, maxFoundDelay;

- (NSString *)windowNibName
{
	return @"Gen4TIDSearcher";
}

- (void)awakeFromNib
{
  [super awakeFromNib];
  
  [tidSidSearcherController setGetValidatedSearchCriteriaSelector:
                            @selector(tidSidSearchGetValidatedSearchCriteria)];
  [tidSidSearcherController setDoSearchWithCriteriaSelector:
                            @selector(tidSidSearchDoSearchWithCriteria:)];
  
  NSTableView  *resultsTableView = [tidSidSearcherController tableView];
  [resultsTableView setTarget: self];
  [resultsTableView setDoubleAction: @selector(inspectSeed:)];
  
  self.shinyPID = nil;
  self.desiredTID = nil;
  self.desiredSID = nil;
  self.minSearchDelay = 5000;
  self.maxSearchDelay = 65535;
  
  [foundTIDSearcherController setGetValidatedSearchCriteriaSelector:
                              @selector(idSearchGetValidatedSearchCriteria)];
  [foundTIDSearcherController setDoSearchWithCriteriaSelector:
                              @selector(idSearchDoSearchWithCriteria:)];
  
  self.foundTID = nil;
  self.startDate = [NSDate date];
  self.startHour = 0;
  self.startMinute = 0;
  self.startSecond = 0;
  self.minSearchDelay = 5000;
  self.maxSearchDelay = 65535;
}

- (void)windowWillClose:(NSNotification *)notification
{
  if ([tidSidSearcherController isSearching])
    [tidSidSearcherController startStop: self];
  if ([foundTIDSearcherController isSearching])
    [foundTIDSearcherController startStop: self];
}

- (void)inspectSeed:(id)sender
{
  NSInteger  rowNum = [sender clickedRow];
  
  if (rowNum >= 0)
  {
    Gen4TIDSeedSearchResult  *row =
      [[[tidSidSearcherController arrayController] arrangedObjects]
        objectAtIndex: rowNum];
    
    if (row != nil)
    {
      Gen4SeedInspectorController  *inspector =
        [[Gen4SeedInspectorController alloc] init];
      [inspector window];
      
      inspector.seed = [NSNumber numberWithUnsignedInt: row.seed];
      
      inspector.selectedTabId = @"timeAndAdjacents";
      [inspector calculateTimes: self];
      
      [inspector showWindow: self];
    }
  }
}

// dummy method for error panel
- (void)alertDidEnd:(NSAlert *)alert returnCode:(NSInteger)returnCode
        contextInfo:(void *)contextInfo
{}


- (NSValue*)tidSidSearchGetValidatedSearchCriteria
{
  if (!EndEditing([self window]))
    return nil;
  
  Gen4TIDSearcher::Criteria  criteria;
  
  criteria.minDelay = minSearchDelay;
  criteria.maxDelay = maxSearchDelay;
  
  if (desiredTID != nil)
  {
    criteria.considerTID = true;
    criteria.tid = [desiredTID unsignedIntValue];
  }
  else
  {
    criteria.considerTID = false;
  }
  
  if (desiredSID != nil)
  {
    criteria.considerSID = true;
    criteria.sid = [desiredSID unsignedIntValue];
  }
  else
  {
    criteria.considerSID = false;
  }
  
  if (shinyPID != nil)
  {
    criteria.considerPID = true;
    criteria.pid = [shinyPID unsignedIntValue];
  }
  else
  {
    criteria.considerPID = false;
  }
  
  if (criteria.considerTID && criteria.considerSID && criteria.considerPID &&
      !criteria.pid.IsShiny(criteria.tid, criteria.sid))
  {
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Invalid Search Parameters"];
    [alert setInformativeText:@"The desired shiny PID cannot be shiny using the given TID and SID combination.  Please change or remove one of the parameters (for example the SID)."];
    [alert setAlertStyle:NSWarningAlertStyle];
    
    [alert beginSheetModalForWindow:[self window] modalDelegate:self
           didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
           contextInfo:nil];
    
    return nil;
  }
  else if (criteria.ExpectedNumberOfResults() > 10000)
  {
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Please Limit Search Parameters"];
    [alert setInformativeText:@"The current parameters are expected to return more than 10,000 results.  Set a TID, SID, a PID to make shiny, or narrow the range of delays being considered in order to run the search."];
    [alert setAlertStyle:NSWarningAlertStyle];
    
    [alert beginSheetModalForWindow:[self window] modalDelegate:self
           didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
           contextInfo:nil];
    
    return nil;
  }
  else
  {
    return [NSValue
             valueWithPointer: new Gen4TIDSearcher::Criteria(criteria)];
  }
}

- (void)tidSidSearchDoSearchWithCriteria:(NSValue*)criteriaPtr
{
  std::auto_ptr<Gen4TIDSearcher::Criteria> 
    criteria(static_cast<Gen4TIDSearcher::Criteria*>
      ([criteriaPtr pointerValue]));
  
  Gen4TIDSearcher  searcher;
  
  searcher.Search(*criteria,
                  TIDSearchResultHandler(tidSidSearcherController),
                  TIDSearchProgressHandler(tidSidSearcherController));
}

- (NSValue*)idSearchGetValidatedSearchCriteria
{
  using namespace boost::posix_time;

  if (!EndEditing([self window]))
    return nil;
  
  if (!foundTID)
    return nil;
  
  FoundIDSearchCriteria  criteria;
  
  criteria.tid = [foundTID unsignedIntValue];
  
  criteria.startTime = ptime(NSDateToBoostDate(startDate),
                             hours(startHour) + minutes(startMinute) +
                             seconds(startSecond));
  
  criteria.minDelay = minSearchDelay;
  criteria.maxDelay = maxSearchDelay;
  
  if (criteria.ExpectedNumberOfResults() > 100)
  {
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Please Limit Search Parameters"];
    [alert setInformativeText:@"The current search parameters are expected to return more than 100 results. Please limit the delay range of the search in order to reduce the number of expected results."];
    [alert setAlertStyle:NSWarningAlertStyle];
    
    [alert beginSheetModalForWindow:[self window] modalDelegate:self
           didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
           contextInfo:nil];
    
    return nil;
  }
  else
  {
    return [NSValue valueWithPointer:
            new FoundIDSearchCriteria(criteria)];
  }
}

- (void)idSearchDoSearchWithCriteria:(NSValue*)criteriaPtr
{
  using namespace boost::gregorian;
  using namespace boost::posix_time;
  
  std::auto_ptr<FoundIDSearchCriteria> 
    criteria(static_cast<FoundIDSearchCriteria*>
      ([criteriaPtr pointerValue]));
  
  date  d = criteria->startTime.date();
  time_duration  t = criteria->startTime.time_of_day();
  
  uint32_t  year = d.year();
  uint32_t  month = d.month();
  uint32_t  day = d.day();
  uint32_t  hour = t.hours();
  uint32_t  minute = t.minutes();
  uint32_t  second = t.seconds();
  
  FoundIDSeedSearchResultHandler  resultHandler(foundTIDSearcherController,
                                                year - 2000);
  
  for (uint32_t delay = criteria->minDelay;
       delay <= criteria->maxDelay;
       ++delay)
  {
    TimeSeed  ts(year, month, day, hour, minute, second, delay);
    
    Gen4TrainerIDFrameGenerator  frameGen(ts.m_seed);
    frameGen.AdvanceFrame();
    
    Gen4TrainerIDFrame  frame = frameGen.CurrentFrame();
    
    if (frame.tid == criteria->tid)
      resultHandler(frame);
  }
}


@end
