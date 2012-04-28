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


#import "Gen4EggPIDSeedSearcherController.h"

#import "SearchResultProtocols.h"
#import "Utilities.h"
#import "Gen4SeedInspectorController.h"

#include "Gen4EggPIDSeedSearcher.h"

using namespace pprng;


@interface Gen4EggPIDSeedSearchResult : NSObject <PIDResult>
{
  uint32_t  seed, delay, frame;
  DECLARE_PID_RESULT_VARIABLES();
  
  uint32_t  initialFlips;
  uint32_t  taps;
  uint32_t  additionalFlips;
}

@property uint32_t  seed, delay, frame;
@property uint32_t  initialFlips;
@property uint32_t  taps;
@property uint32_t  additionalFlips;

@end

@implementation Gen4EggPIDSeedSearchResult

@synthesize seed, delay, frame;
SYNTHESIZE_PID_RESULT_PROPERTIES();
@synthesize initialFlips, taps, additionalFlips;

@end


namespace
{

struct ResultHandler
{
  ResultHandler(SearcherController *controller,
                const Gen4EggPIDSeedSearcher::Criteria &criteria)
    : m_controller(controller), m_criteria(criteria)
  {}
  
  void operator()(const Gen4EggPIDSeedSearcher::ResultType &frame)
  {
    TimeSeed  seed(frame.seed);
    
    Gen4EggPIDSeedSearchResult  *result =
      [[Gen4EggPIDSeedSearchResult alloc] init];
    
    result.seed = frame.seed;
    result.delay = seed.BaseDelay();
    result.frame = frame.number;
    
    SetPIDResult(result, frame.pid,
                 m_criteria.frameParameters.tid,
                 m_criteria.frameParameters.sid, frame.pid.Gen34Nature(),
                 frame.pid.Gen34Ability(), Gender::ANY, Gender::ANY_RATIO);
    
    result.initialFlips = CoinFlips(frame.seed, 10).word;
    result.taps = (frame.number - 22 - 1) / 12;
    result.additionalFlips = (frame.number - 10 - 1) % 12;
    
    [m_controller performSelectorOnMainThread: @selector(addResult:)
                  withObject: result
                  waitUntilDone: NO];
  }
  
  SearcherController                      *m_controller;
  const Gen4EggPIDSeedSearcher::Criteria  &m_criteria;
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


@implementation Gen4EggPIDSeedSearcherController

@synthesize  mode;
@synthesize  internationalParents;
@synthesize  minDelay, maxDelay;
@synthesize  allowTaps, maxTaps, maxFrame;
@synthesize  shinyOnly;
@synthesize  ability;
@synthesize  gender;
@synthesize  genderRatio;

- (NSString *)windowNibName
{
	return @"Gen4EggPIDSeedSearcher";
}

- (void)updateColumnVisibility
{
  NSTableView  *resultsTableView = [searcherController tableView];
  
  [[resultsTableView tableColumnWithIdentifier:@"initialFlips"]
    setHidden: mode];
  [[resultsTableView tableColumnWithIdentifier:@"additionalFlips"]
    setHidden: mode];
  [[resultsTableView tableColumnWithIdentifier:@"taps"]
    setHidden: mode || !allowTaps];
  
  [[resultsTableView tableColumnWithIdentifier:@"frame"]
    setHidden: !mode];
}

- (void)awakeFromNib
{
  [super awakeFromNib];
  
  [searcherController setGetValidatedSearchCriteriaSelector:
                      @selector(getValidatedSearchCriteria)];
  [searcherController setDoSearchWithCriteriaSelector:
                      @selector(doSearchWithCriteria:)];
  
  NSTableView  *resultsTableView = [searcherController tableView];
  [resultsTableView setTarget: self];
  [resultsTableView setDoubleAction: @selector(inspectSeed:)];
  
  Game::Version  version = [gen4ConfigController version];
  BOOL           isDPPt = (version != Game::HeartGold) &&
                          (version != Game::SoulSilver);
  self.mode = isDPPt ? 0 : 1;
  self.minDelay = 600;
  self.maxDelay = 650;
  self.maxTaps = 10;
  self.maxFrame = 10;
  self.shinyOnly = YES;
  self.ability = Ability::ANY;
  self.gender = Gender::ANY;
  self.genderRatio = Gender::ANY_RATIO;
  
  [self updateColumnVisibility];
}

- (void)windowWillClose:(NSNotification *)notification
{
  if ([searcherController isSearching])
    [searcherController startStop: self];
}

- (IBAction)toggleDropDownChoice:(id)sender
{
  HandleComboMenuItemChoice(sender);
}

- (void)inspectSeed:(id)sender
{
  NSInteger  rowNum = [sender clickedRow];
  
  if (rowNum >= 0)
  {
    Gen4EggPIDSeedSearchResult  *row =
      [[[searcherController arrayController] arrangedObjects]
      objectAtIndex: rowNum];
    
    if (row != nil)
    {
      Gen4SeedInspectorController  *inspector =
        [[Gen4SeedInspectorController alloc] init];
      [inspector window];
      
      [inspector setMode: mode];
      [inspector setSeed: row.seed];
      [inspector setFrame: row.frame];
      
      [inspector showWindow: self];
    }
  }
}

// dummy method for error panel
- (void)alertDidEnd:(NSAlert *)alert returnCode:(NSInteger)returnCode
        contextInfo:(void *)contextInfo
{}

- (NSValue*)getValidatedSearchCriteria
{
  if (!EndEditing([self window]))
    return nil;
  
  Gen4EggPIDSeedSearcher::Criteria  criteria;
  
  criteria.delay.min = minDelay;
  criteria.delay.max = maxDelay;
  
  if (mode == 0)
  {
    if (allowTaps)
    {
      criteria.frame.min = 10 + 12 + 1;
      criteria.frame.max = 10 + (maxTaps * 12) + 12 + 11;
    }
    else
    {
      criteria.frame.min = 10 + 1;
      criteria.frame.max = 10 + 11;
    }
  }
  else
  {
    criteria.frame.min = 1;
    criteria.frame.max = maxFrame;
  }
  
  criteria.shinyOnly = shinyOnly;
  criteria.pid.natureMask = GetComboMenuBitMask(natureDropDown);
  criteria.pid.ability = ability;
  criteria.pid.gender = gender;
  criteria.pid.genderRatio = genderRatio;
  
  criteria.frameParameters.internationalParents = internationalParents;
  criteria.frameParameters.tid = [gen4ConfigController tid];
  criteria.frameParameters.sid = [gen4ConfigController sid];
  
  if (criteria.ExpectedNumberOfResults() > 10000)
  {
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Please Limit Search Parameters"];
    [alert setInformativeText:@"The current search parameters are expected to return more than 10,000 results. Please set more specific IVs, limit the delay range, or change other settings to reduce the number of expected results."];
    [alert setAlertStyle:NSWarningAlertStyle];
    
    [alert beginSheetModalForWindow:[self window] modalDelegate:self
           didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
           contextInfo:nil];
    
    return nil;
  }
  else
  {
    return [NSValue
             valueWithPointer: new Gen4EggPIDSeedSearcher::Criteria(criteria)];
  }
}

- (void)doSearchWithCriteria:(NSValue*)criteriaPtr
{
  [self updateColumnVisibility];
  
  std::auto_ptr<Gen4EggPIDSeedSearcher::Criteria> 
    criteria(static_cast<Gen4EggPIDSeedSearcher::Criteria*>
               ([criteriaPtr pointerValue]));
  
  Gen4EggPIDSeedSearcher  searcher;
  
  searcher.Search(*criteria, ResultHandler(searcherController, *criteria),
                  ProgressHandler(searcherController));
}

@end
