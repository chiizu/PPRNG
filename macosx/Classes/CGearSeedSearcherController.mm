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


#import "CGearSeedSearcherController.h"

#import "SearchResultProtocols.h"
#import "Utilities.h"

#include "CGearSeedSearcher.h"
#include "CGearSeedInspectorController.h"

using namespace pprng;

@interface CGearSeedSearchResult : NSObject <IVResult>
{
  uint32_t  seed, delay, frame;
  DECLARE_IV_RESULT_VARIABLES();
}

@property uint32_t  seed, delay, frame;

@end

@implementation CGearSeedSearchResult

@synthesize seed, delay, frame;
SYNTHESIZE_IV_RESULT_PROPERTIES();

@end

namespace
{

struct ResultHandler
{
  ResultHandler(SearcherController *c, uint32_t macAddrLow, bool isRoamer)
    : m_controller(c), m_macAddressLow(macAddrLow), m_isRoamer(isRoamer)
  {}
  
  void operator()(const CGearIVFrame &frame)
  {
    CGearSeed  seed(frame.seed, m_macAddressLow);
    
    CGearSeedSearchResult  *result = [[CGearSeedSearchResult alloc] init];
    
    result.seed = seed.m_rawSeed;
    result.delay = seed.BaseDelay();
    result.frame = frame.number;
    SetIVResult(result, frame.ivs, m_isRoamer);
    
    [m_controller performSelectorOnMainThread: @selector(addResult:)
                withObject: result
                waitUntilDone: NO];
  }
  
  SearcherController  *m_controller;
  const uint32_t      m_macAddressLow;
  const bool          m_isRoamer;
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

@synthesize minFrame, maxFrame, minDelay, maxDelay;

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
  
  [[searcherController tableView] setTarget: self];
  [[searcherController tableView] setDoubleAction: @selector(inspectSeed:)];
  
  self.minFrame = 21;
  self.maxFrame = 26;
  self.minDelay = 1800;
  self.maxDelay = 3000;
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
    CGearSeedSearchResult  *row =
      [[[searcherController arrayController] arrangedObjects]
        objectAtIndex: rowNum];
    
    if (row != nil)
    {
      CGearSeedInspectorController  *inspector =
        [[CGearSeedInspectorController alloc] init];
      [inspector window];
      
      inspector.seed = [NSNumber numberWithUnsignedInt: row.seed];
      inspector.maxIVFrame = row.frame + 20;
      inspector.ivFrameParameterController.isRoamer = row.isRoamer;
      [inspector generateIVFrames: self];
      [inspector selectAndShowFrame: row.frame];
      
      if (row.frame <= 5)
        inspector.adjacentsMinIVFrame = 1;
      else
        inspector.adjacentsMinIVFrame = row.frame - 5;
      
      inspector.adjacentsMaxIVFrame = row.frame + 5;
      inspector.adjacentsIVParameterController.isRoamer = row.isRoamer;
      
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
  
  CGearSeedSearcher::Criteria  criteria;
    
  criteria.macAddressLow = [gen5ConfigController macAddressLow];
  
  criteria.minDelay = minDelay;
  criteria.maxDelay = maxDelay;
  criteria.frameRange.min = minFrame;
  criteria.frameRange.max = maxFrame;
  
  criteria.ivs.min = ivParameterController.minIVs;
  criteria.ivs.max = ivParameterController.maxIVs;
  criteria.ivs.shouldCheckMax =
    (criteria.ivs.max != IVs(31, 31, 31, 31, 31, 31));
  criteria.ivs.isRoamer = ivParameterController.isRoamer;
  
  if (ivParameterController.considerHiddenPower)
  {
    criteria.ivs.hiddenType = ivParameterController.hiddenType;
    criteria.ivs.minHiddenPower = ivParameterController.minHiddenPower;
  }
  else
  {
    criteria.ivs.hiddenType = Element::NONE;
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
                  ResultHandler(searcherController,
                                criteria->macAddressLow,
                                criteria->ivs.isRoamer),
                  ProgressHandler(searcherController));
}

@end
