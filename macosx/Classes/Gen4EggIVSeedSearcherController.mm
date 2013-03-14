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


#import "Gen4EggIVSeedSearcherController.h"

#import "SearchResultProtocols.h"
#import "Utilities.h"
#import "Gen4SeedInspectorController.h"

#include "Gen4EggIVSeedSearcher.h"

using namespace pprng;


@interface Gen4EggIVSeedSearchResult : NSObject
{
  uint32_t       seed, delay, frame;
  OptionalIVs    aIVs, bIVs;
  id             hp, atk, def, spa, spd, spe;
  Element::Type  hiddenType;
  NSNumber       *hiddenPower;
}

@property uint32_t                 seed, delay, frame;
@property (nonatomic) OptionalIVs  aIVs, bIVs;
@property (copy) id                hp, atk, def, spa, spd, spe;
@property Element::Type            hiddenType;
@property (copy) NSNumber          *hiddenPower;

@end

@implementation Gen4EggIVSeedSearchResult

@synthesize seed, delay, frame;
@synthesize aIVs, bIVs;
@synthesize hp, atk, def, spa, spd, spe;
@synthesize hiddenType, hiddenPower;

@end


namespace
{

static id GetEggIV(const Gen4EggIVFrame &frame, IVs::Type iv)
{
  if (frame.ivs.isSet(iv))
    return [NSNumber numberWithUnsignedInt: frame.ivs.iv(iv)];
  
  switch (frame.inheritance[iv])
  {
  default:
  case Gen4EggIVFrame::NotInherited:
    return @"??";
    
  case Gen4EggIVFrame::ParentA:
    return @"A";
    
  case Gen4EggIVFrame::ParentB:
    return @"B";
  }
}

struct ResultHandler
{
  ResultHandler(SearcherController *controller,
                const Gen4EggIVSeedSearcher::Criteria &criteria)
    : m_controller(controller), m_criteria(criteria)
  {}
  
  void operator()(const Gen4EggIVSeedSearcher::ResultType &frame)
  {
    TimeSeed  seed(frame.seed);
    
    Gen4EggIVSeedSearchResult  *result =
      [[Gen4EggIVSeedSearchResult alloc] init];
    
    result.seed = frame.seed;
    result.delay = seed.BaseDelay();
    result.frame = frame.number;
    
    result.aIVs = m_criteria.aIVs;
    result.bIVs = m_criteria.bIVs;
    
    result.hp = GetEggIV(frame, IVs::HP);
    result.atk = GetEggIV(frame, IVs::AT);
    result.def = GetEggIV(frame, IVs::DF);
    result.spa = GetEggIV(frame, IVs::SA);
    result.spd = GetEggIV(frame, IVs::SD);
    result.spe = GetEggIV(frame, IVs::SP);
    
    if (frame.ivs.allSet())
    {
      result.hiddenType = frame.ivs.values.HiddenType();
      result.hiddenPower =
        [NSNumber numberWithUnsignedInt: frame.ivs.values.HiddenPower()];
    }
    else
    {
      result.hiddenType = Element::NONE;
      result.hiddenPower = nil;
    }
    
    [m_controller performSelectorOnMainThread: @selector(addResult:)
                  withObject: result
                  waitUntilDone: NO];
  }
  
  SearcherController                     *m_controller;
  const Gen4EggIVSeedSearcher::Criteria  &m_criteria;
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


@implementation Gen4EggIVSeedSearcherController

@synthesize  mode;
@synthesize  minDelay, maxDelay;
@synthesize  minFrame, maxFrame;

@synthesize aHP, aAT, aDF, aSA, aSD, aSP;
@synthesize bHP, bAT, bDF, bSA, bSD, bSP;

- (NSString *)windowNibName
{
	return @"Gen4EggIVSeedSearcher";
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
  BOOL           isDPPt = (version != Game::HeartGoldVersion) &&
                          (version != Game::SoulSilverVersion);
  self.mode = isDPPt ? 0 : 1;
  self.minDelay = 600;
  self.maxDelay = 650;
  self.minFrame = 5;
  self.maxFrame = 5;
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
    Gen4EggIVSeedSearchResult  *row =
      [[[searcherController arrayController] arrangedObjects]
      objectAtIndex: rowNum];
    
    if (row != nil)
    {
      Gen4SeedInspectorController  *inspector =
        [[Gen4SeedInspectorController alloc] init];
      [inspector window];
      
      inspector.mode = mode;
      inspector.seed = [NSNumber numberWithUnsignedInt: row.seed];
      
      inspector.selectedTabId = @"eggIVs";
      
      inspector.maxEggIVFrame = row.frame + 20;
      
      inspector.enableParentIVs = YES;
      [inspector setAIVs: row.aIVs];
      [inspector setBIVs: row.bIVs];
      
      [inspector generateEggIVFrames: self];
      [inspector selectAndShowEggIVFrame: row.frame];
      
      [inspector showWindow: self];
    }
  }
}

// dummy method for error panel
- (void)alertDidEnd:(NSAlert *)alert returnCode:(NSInteger)returnCode
        contextInfo:(void *)contextInfo
{}

- (OptionalIVs)aParentIVs
{
  OptionalIVs  result;
  
  if (aHP)
    result.hp([aHP unsignedIntValue]);
  if (aAT)
    result.at([aAT unsignedIntValue]);
  if (aDF)
    result.df([aDF unsignedIntValue]);
  if (aSA)
    result.sa([aSA unsignedIntValue]);
  if (aSD)
    result.sd([aSD unsignedIntValue]);
  if (aSP)
    result.sp([aSP unsignedIntValue]);
  
  return result;
}

- (OptionalIVs)bParentIVs
{
  OptionalIVs  result;
  
  if (bHP)
    result.hp([bHP unsignedIntValue]);
  if (bAT)
    result.at([bAT unsignedIntValue]);
  if (bDF)
    result.df([bDF unsignedIntValue]);
  if (bSA)
    result.sa([bSA unsignedIntValue]);
  if (bSD)
    result.sd([bSD unsignedIntValue]);
  if (bSP)
    result.sp([bSP unsignedIntValue]);
  
  return result;
}

- (NSValue*)getValidatedSearchCriteria
{
  if (!EndEditing([self window]))
    return nil;
  
  Gen4EggIVSeedSearcher::Criteria  criteria;
  
  criteria.version = mode ? Game::HeartGoldVersion : Game::DiamondVersion;
  
  criteria.delay.min = minDelay;
  criteria.delay.max = maxDelay;
  
  criteria.frame.min = minFrame;
  criteria.frame.max = maxFrame;
  
  criteria.aIVs = [self aParentIVs];
  criteria.bIVs = [self bParentIVs];
  
  criteria.ivs.min = ivParameterController.minIVs;
  criteria.ivs.max = ivParameterController.maxIVs;
  
  criteria.ivs.hiddenTypeMask = ivParameterController.hiddenTypeMask;
  criteria.ivs.minHiddenPower = ivParameterController.minHiddenPower;
  
  uint64_t  numResults = criteria.ExpectedNumberOfResults();
  
  if (numResults == 0)
  {
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Invalid Search Parameters"];
    [alert setInformativeText:@"The Parent IVs specified cannot produce the desired Egg IVs."];
    [alert setAlertStyle:NSWarningAlertStyle];
    
    [alert beginSheetModalForWindow:[self window] modalDelegate:self
           didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
           contextInfo:nil];
    
    return nil;
  }
  else if (numResults > 10000)
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
             valueWithPointer: new Gen4EggIVSeedSearcher::Criteria(criteria)];
  }
}

- (void)doSearchWithCriteria:(NSValue*)criteriaPtr
{
  std::auto_ptr<Gen4EggIVSeedSearcher::Criteria> 
    criteria(static_cast<Gen4EggIVSeedSearcher::Criteria*>
               ([criteriaPtr pointerValue]));
  
  Gen4EggIVSeedSearcher  searcher;
  
  searcher.Search(*criteria, ResultHandler(searcherController, *criteria),
                  ProgressHandler(searcherController));
}

@end
