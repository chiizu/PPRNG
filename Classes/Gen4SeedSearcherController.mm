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


#import "Gen4SeedSearcherController.h"

#include "Gen34SeedSearcher.h"

using namespace pprng;

namespace
{

struct ResultHandler
{
  ResultHandler(SearcherController *c, uint32_t tid, uint32_t sid)
    : controller(c), tid(tid), sid(sid)
  {}
  
  void operator()(const Gen34SeedSearcher::Frame &frame)
  {
    Gen4Frame  fullFrame(frame);
    TimeSeed   seed(fullFrame.seed);
    
    NSMutableDictionary  *result =
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithUnsignedInt: seed.m_seed], @"seed",
				[NSNumber numberWithUnsignedInt: seed.BaseDelay()], @"delay",
        [NSNumber numberWithUnsignedInt: fullFrame.pid.word], @"pid",
        [NSString stringWithFormat: @"%s",
          Nature::ToString(fullFrame.pid.Gen34Nature()).c_str()], @"nature",
        [NSNumber numberWithUnsignedInt: fullFrame.pid.Gen34Ability()],
          @"ability",
        frame.pid.IsShiny(tid, sid) ? @"!!!" : @"", @"shiny",
        [NSNumber numberWithUnsignedInt: fullFrame.ivs.hp()], @"hp",
        [NSNumber numberWithUnsignedInt: fullFrame.ivs.at()], @"atk",
        [NSNumber numberWithUnsignedInt: fullFrame.ivs.df()], @"def",
        [NSNumber numberWithUnsignedInt: fullFrame.ivs.sa()], @"spa",
        [NSNumber numberWithUnsignedInt: fullFrame.ivs.sd()], @"spd",
        [NSNumber numberWithUnsignedInt: fullFrame.ivs.sp()], @"spe",
        [NSString stringWithFormat: @"%s",
          Element::ToString(fullFrame.ivs.HiddenType()).c_str()], @"hiddenType",
        [NSNumber numberWithUnsignedInt: fullFrame.ivs.HiddenPower()],
          @"hiddenPower",
				[NSNumber numberWithUnsignedInt: fullFrame.number], @"frame",
        (fullFrame.methodJNumber > 0) ?
          [NSString stringWithFormat: @"%d", fullFrame.methodJNumber] :
          @"None", @"frameJ",
        (fullFrame.methodJSyncNumber > 0) ?
          [NSString stringWithFormat: @"%d", fullFrame.methodJSyncNumber] :
          @"None", @"frameJSync",
        (fullFrame.methodJFailedSyncNumber > 0) ?
          [NSString stringWithFormat: @"%d", fullFrame.methodJFailedSyncNumber] :
          @"None", @"frameJSyncFail",
        (fullFrame.methodJNumber > 0) ?
          [NSString stringWithFormat: @"%d", fullFrame.methodKNumber] :
          @"None", @"frameK",
        (fullFrame.methodJSyncNumber > 0) ?
          [NSString stringWithFormat: @"%d", fullFrame.methodKSyncNumber] :
          @"None", @"frameKSync",
        (fullFrame.methodJFailedSyncNumber > 0) ?
          [NSString stringWithFormat: @"%d", fullFrame.methodKFailedSyncNumber] :
          @"None", @"frameKSyncFail",
        nil];
    
    [controller performSelectorOnMainThread: @selector(addResult:)
                withObject: result
                waitUntilDone: NO];
  }
  
  SearcherController  *controller;
  const uint32_t      tid, sid;
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


@implementation Gen4SeedSearcherController

- (NSString *)windowNibName
{
	return @"Gen4SeedSearcher";
}

- (void)awakeFromNib
{
  [super awakeFromNib];
  
  [searcherController setGetValidatedSearchCriteriaSelector:
                      @selector(getValidatedSearchCriteria)];
  [searcherController setDoSearchWithCriteriaSelector:
                      @selector(doSearchWithCriteria:)];
  
  [[[[[searcherController tableView] tableColumnWithIdentifier: @"seed"]
    dataCell] formatter]
   setFormatWidth: 8];
  [[[[[searcherController tableView] tableColumnWithIdentifier: @"pid"]
    dataCell] formatter]
   setFormatWidth: 8];
}

- (void)windowWillClose:(NSNotification *)notification
{
  if ([searcherController isSearching])
    [searcherController startStop: self];
}

- (IBAction)switchSearchType:(id)sender
{}

// dummy method for error panel
- (void)alertDidEnd:(NSAlert *)alert returnCode:(NSInteger)returnCode
        contextInfo:(void *)contextInfo
{}

- (NSValue*)getValidatedSearchCriteria
{
  Gen34SeedSearcher::Criteria  criteria;
  
  criteria.tid = [tidField intValue];
  criteria.sid = [sidField intValue];
  
  criteria.minDelay = [minDelayField intValue];
  criteria.maxDelay = [maxDelayField intValue];
  criteria.minFrame = [minFrameField intValue];
  criteria.maxFrame = [maxFrameField intValue];
  
  criteria.nature = static_cast<Nature::Type>([[natureMenu selectedItem] tag]);
  
  criteria.minIVs = [ivParameterController minIVs];
  criteria.shouldCheckMaxIVs = [ivParameterController shouldCheckMaxIVs];
  criteria.maxIVs = [ivParameterController maxIVs];
  
  if ([ivParameterController shouldCheckHiddenPower])
  {
    criteria.hiddenType = [ivParameterController hiddenType];
    criteria.minHiddenPower = [ivParameterController minHiddenPower];
    criteria.maxHiddenPower = 70;
  }
  else
  {
    criteria.hiddenType = Element::UNKNOWN;
  }
  
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
             valueWithPointer: new Gen34SeedSearcher::Criteria(criteria)];
  }
}

- (void)doSearchWithCriteria:(NSValue*)criteriaPtr
{
  std::auto_ptr<Gen34SeedSearcher::Criteria> 
    criteria(static_cast<Gen34SeedSearcher::Criteria*>
               ([criteriaPtr pointerValue]));
  
  Gen34SeedSearcher  searcher;
  
  searcher.Search(*criteria,
                  ResultHandler(searcherController,
                                [tidField intValue], [sidField intValue]),
                  ProgressHandler(searcherController));
}

@end
