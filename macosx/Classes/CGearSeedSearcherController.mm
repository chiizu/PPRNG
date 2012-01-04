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


#import "CGearSeedSearcherController.h"

#include "CGearSeedSearcher.h"
#include "CGearSeedInspectorController.h"

using namespace pprng;

namespace
{

struct ResultHandler
{
  ResultHandler(SearcherController *c, uint32_t macAddrLow)
    : controller(c), macAddressLow(macAddrLow)
  {}
  
  void operator()(const CGearIVFrame &frame)
  {
    CGearSeed  seed(frame.seed, macAddressLow);
    
    NSMutableDictionary  *result =
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithUnsignedInt: seed.m_rawSeed], @"seed",
				[NSNumber numberWithUnsignedInt: seed.BaseDelay()], @"delay",
				[NSNumber numberWithUnsignedInt: frame.number], @"frame",
        [NSNumber numberWithUnsignedInt: frame.ivs.hp()], @"hp",
        [NSNumber numberWithUnsignedInt: frame.ivs.at()], @"atk",
        [NSNumber numberWithUnsignedInt: frame.ivs.df()], @"def",
        [NSNumber numberWithUnsignedInt: frame.ivs.sa()], @"spa",
        [NSNumber numberWithUnsignedInt: frame.ivs.sd()], @"spd",
        [NSNumber numberWithUnsignedInt: frame.ivs.sp()], @"spe",
        [NSString stringWithFormat: @"%s",
          Element::ToString(frame.ivs.HiddenType()).c_str()], @"hiddenType",
        [NSNumber numberWithUnsignedInt: frame.ivs.HiddenPower()], @"hiddenPower",
        nil];
    
    [controller performSelectorOnMainThread: @selector(addResult:)
                withObject: result
                waitUntilDone: NO];
  }
  
  SearcherController  *controller;
  const uint32_t      macAddressLow;
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

@implementation CGearSeedSearcherController

- (NSString *)windowNibName
{
	return @"CGearSeedSearcher";
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
  
  [[searcherController tableView] setDoubleAction: @selector(inspectSeed:)];
}

- (void)windowWillClose:(NSNotification *)notification
{
  if ([searcherController isSearching])
    [searcherController startStop: self];
}

- (void)inspectSeed:(id)sender
{
  NSInteger  rowNum = [sender clickedRow];
  
  if (rowNum >= 0)
  {
    NSDictionary  *row =
      [[[searcherController arrayController] arrangedObjects]
        objectAtIndex: rowNum];
    
    if (row != nil)
    {
      uint32_t  seed = [[row objectForKey: @"seed"] unsignedIntValue];
      
      CGearSeedInspectorController  *inspector =
        [[CGearSeedInspectorController alloc] init];
      [inspector showWindow: self];
      [inspector setSeed: seed];
    }
  }
}

// dummy method for error panel
- (void)alertDidEnd:(NSAlert *)alert returnCode:(NSInteger)returnCode
        contextInfo:(void *)contextInfo
{}

- (NSValue*)getValidatedSearchCriteria
{
  CGearSeedSearcher::Criteria  criteria;
    
  criteria.macAddressLow = [gen5ConfigController macAddressLow];
  
  criteria.minDelay = [minDelayField intValue];
  criteria.maxDelay = [maxDelayField intValue];
  criteria.minFrame = [minFrameField intValue];
  criteria.maxFrame = [maxFrameField intValue];
  
  criteria.minIVs = [ivParameterController minIVs];
  criteria.shouldCheckMaxIVs = [ivParameterController shouldCheckMaxIVs];
  criteria.maxIVs = [ivParameterController maxIVs];
  criteria.isRoamer = [ivParameterController isRoamer];
  
  if ([ivParameterController shouldCheckHiddenPower])
  {
    criteria.hiddenType = [ivParameterController hiddenType];
    criteria.minHiddenPower = [ivParameterController minHiddenPower];
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
             valueWithPointer: new CGearSeedSearcher::Criteria(criteria)];
  }
}

- (void)doSearchWithCriteria:(NSValue*)criteriaPtr
{
  std::auto_ptr<CGearSeedSearcher::Criteria> 
    criteria(static_cast<CGearSeedSearcher::Criteria*>
               ([criteriaPtr pointerValue]));
  
  CGearSeedSearcher  searcher;
  
  searcher.Search(*criteria,
                  ResultHandler(searcherController, criteria->macAddressLow),
                  ProgressHandler(searcherController));
}

@end
