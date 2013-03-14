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


#import "Gen4SeedSearcherController.h"

#import "Utilities.h"

#include "Gen4QuickSeedSearcher.h"
#import "Gen4SeedInspectorController.h"

using namespace pprng;

namespace
{

uint32_t GetESVBitmaskForTypeMask(uint32_t typeMask, NSPopUpButton *esvMenu)
{
  uint32_t   mask = 0;
  NSInteger  numItems = [esvMenu numberOfItems];
  NSInteger  i;
  
  for (i = 0; i < numItems; ++i)
  {
    NSMenuItem  *item = [esvMenu itemAtIndex: i];
    NSInteger   tag = [item tag];
    
    if ((tag >= 0) && ((tag & 0xf0) == typeMask) && ([item state] == NSOnState))
    {
      mask |= 0x1 << (tag & 0xf);
    }
  }
  
  return mask;
}

NSString* GetFrameTypeResult(const Gen4Frame::EncounterData &data,
                             Gen4Frame::EncounterData::FrameType frameType)
{
  uint32_t  frameNumber = data.lowestFrames.number[frameType];
  
  return (frameNumber > 0) ?
    [NSString stringWithFormat: @"%d", frameNumber] :
    @"None";
}

struct ResultHandler
{
  ResultHandler(SearcherController *c, uint32_t tid, uint32_t sid)
    : controller(c), tid(tid), sid(sid)
  {}
  
  void operator()(const Gen4QuickSeedSearcher::ResultType &frame)
  {
    TimeSeed  seed(frame.seed);
    uint32_t  genderValue = frame.pid.GenderValue();
    
    NSMutableDictionary  *result =
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithUnsignedInt: seed.m_seed], @"seed",
				[NSNumber numberWithUnsignedInt: seed.BaseDelay()], @"delay",
        [NSNumber numberWithUnsignedInt: frame.pid.word], @"pid",
        frame.pid.IsShiny(tid, sid) ? @"★" : @"", @"shiny",
        [NSString stringWithFormat: @"%s",
          Nature::ToString(frame.pid.Gen34Nature()).c_str()], @"nature",
        [NSNumber numberWithUnsignedInt: frame.pid.Gen34Ability()], @"ability",
        ((genderValue < 31) ? @"♀" : @"♂"), @"gender18",
        ((genderValue < 63) ? @"♀" : @"♂"), @"gender14",
        ((genderValue < 127) ? @"♀" : @"♂"), @"gender12",
        ((genderValue < 191) ? @"♀" : @"♂"), @"gender34",
        [NSNumber numberWithUnsignedInt: frame.ivs.hp()], @"hp",
        [NSNumber numberWithUnsignedInt: frame.ivs.at()], @"atk",
        [NSNumber numberWithUnsignedInt: frame.ivs.df()], @"def",
        [NSNumber numberWithUnsignedInt: frame.ivs.sa()], @"spa",
        [NSNumber numberWithUnsignedInt: frame.ivs.sd()], @"spd",
        [NSNumber numberWithUnsignedInt: frame.ivs.sp()], @"spe",
        [NSString stringWithFormat: @"%s",
          Element::ToString(frame.ivs.HiddenType()).c_str()], @"hiddenType",
        [NSNumber numberWithUnsignedInt: frame.ivs.HiddenPower()],
          @"hiddenPower",
				[NSNumber numberWithUnsignedInt: frame.number], @"frame",
        GetFrameTypeResult(frame.methodJ, Gen4Frame::EncounterData::NoSync),
          @"frameJ",
        GetFrameTypeResult(frame.methodJ, Gen4Frame::EncounterData::Sync),
          @"frameJSync",
        GetFrameTypeResult(frame.methodJ, Gen4Frame::EncounterData::FailedSync),
          @"frameJSyncFail",
        GetFrameTypeResult(frame.methodK, Gen4Frame::EncounterData::NoSync),
          @"frameK",
        GetFrameTypeResult(frame.methodK, Gen4Frame::EncounterData::Sync),
          @"frameKSync",
        GetFrameTypeResult(frame.methodK, Gen4Frame::EncounterData::FailedSync),
          @"frameKSyncFail",
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

@synthesize  mode;
@synthesize  shinyOnly;
@synthesize  ability;
@synthesize  gender;
@synthesize  genderRatio;
@synthesize  minFrame;
@synthesize  maxFrame;
@synthesize  minDelay;
@synthesize  maxDelay;

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
  
  NSTableView  *resultsTableView = [searcherController tableView];
  [resultsTableView setTarget: self];
  [resultsTableView setDoubleAction: @selector(inspectSeed:)];
  
  [[resultsTableView tableColumnWithIdentifier:@"frameJ"]
    setHidden: NO];
  [[resultsTableView tableColumnWithIdentifier:@"frameJSync"]
    setHidden: NO];
  [[resultsTableView tableColumnWithIdentifier:@"frameJSyncFail"]
    setHidden: NO];
  [[resultsTableView tableColumnWithIdentifier:@"frameK"]
    setHidden: YES];
  [[resultsTableView tableColumnWithIdentifier:@"frameKSync"]
    setHidden: YES];
  [[resultsTableView tableColumnWithIdentifier:@"frameKSyncFail"]
    setHidden: YES];
  
  Game::Version  version = [gen4ConfigController version];
  BOOL           isDPPt = (version != Game::HeartGoldVersion) &&
                          (version != Game::SoulSilverVersion);
  self.mode = isDPPt ? 0 : 1;
  self.shinyOnly = NO;
  self.ability = Ability::ANY;
  self.gender = Gender::ANY;
  self.genderRatio = Gender::ANY_RATIO;
  self.minFrame = 1;
  self.maxFrame = 120;
  self.minDelay = 600;
  self.maxDelay = 750;
}

- (void)setMode:(int)newMode
{
  if (newMode != mode)
  {
    mode = newMode;
    
    // change which table columns are displayed
    NSTableView  *resultsTableView = [searcherController tableView];
    
    [[resultsTableView tableColumnWithIdentifier:@"frameJ"]
      setHidden: newMode];
    [[resultsTableView tableColumnWithIdentifier:@"frameJSync"]
      setHidden: newMode];
    [[resultsTableView tableColumnWithIdentifier:@"frameJSyncFail"]
      setHidden: newMode];
    [[resultsTableView tableColumnWithIdentifier:@"frameK"]
      setHidden: !newMode];
    [[resultsTableView tableColumnWithIdentifier:@"frameKSync"]
      setHidden: !newMode];
    [[resultsTableView tableColumnWithIdentifier:@"frameKSyncFail"]
      setHidden: !newMode];
  }
}

- (void)windowWillClose:(NSNotification *)notification
{
  if ([searcherController isSearching])
    [searcherController startStop: self];
}

- (IBAction)toggleNatureChoice:(id)sender
{
  HandleComboMenuItemChoice(sender);
}

- (IBAction)toggleESVChoice:(id)sender
{
  NSMenuItem  *selectedItem = [sender selectedItem];
  
  if ([selectedItem tag] >= 0)
  {
    [selectedItem setState: ![selectedItem state]];
  }
  else if ([selectedItem tag] != -5)
  {
    NSInteger  action = [selectedItem tag];
    NSInteger  numItems = [sender numberOfItems];
    NSInteger  i;
    
    for (i = 0; i < numItems; ++i)
    {
      NSMenuItem  *item = [sender itemAtIndex: i];
      NSInteger   tag = [item tag];
      
      if (tag >= 0)
      {
        switch (action)
        {
        case -1:
          [item setState: NSOnState];
          break;
        case -4:
          [item setState: NSOffState];
          break;
        default:
          break;
        }
      }
    }
  }
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
      NSNumber  *seed = [row objectForKey: @"seed"];
      NSNumber  *frame = [row objectForKey: @"frame"];
      uint32_t  frameNum = [frame unsignedIntValue];
      
      Gen4SeedInspectorController  *inspector =
        [[Gen4SeedInspectorController alloc] init];
      [inspector window];
      
      inspector.mode = mode;
      inspector.seed = seed;
      
      inspector.selectedTabId = @"frames";
      
      inspector.maxFrame = frameNum + 20;
      inspector.encounterType = -1;
      
      [inspector generateFrames: self];
      [inspector selectAndShowFrame: frameNum];
      
      inspector.esvMethod1Frame = frameNum;
      
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
  
  Gen4QuickSeedSearcher::Criteria  criteria;
  
  criteria.version = [gen4ConfigController version];
  criteria.tid = [gen4ConfigController tid];
  criteria.sid = [gen4ConfigController sid];
  
  criteria.delay.min = minDelay;
  criteria.delay.max = maxDelay;
  criteria.frame.min = minFrame;
  criteria.frame.max = maxFrame;
  
  criteria.pid.natureMask = GetComboMenuBitMask(naturePopup);
  criteria.pid.ability = Ability::Type([[abilityPopUp selectedItem] tag]);
  criteria.pid.gender = Gender::Type([[genderPopUp selectedItem] tag]);
  criteria.pid.genderRatio =
    Gender::Ratio([[genderRatioPopUp selectedItem] tag]);
  criteria.shinyOnly = shinyOnly;
  criteria.landESVs = GetESVBitmaskForTypeMask(0x00, esvPopUp);
  criteria.surfESVs = GetESVBitmaskForTypeMask(0x10, esvPopUp);
  criteria.oldRodESVs = GetESVBitmaskForTypeMask(0x20, esvPopUp);
  criteria.goodRodESVs = GetESVBitmaskForTypeMask(0x30, esvPopUp);
  criteria.superRodESVs = GetESVBitmaskForTypeMask(0x40, esvPopUp);
  
  criteria.ivs.min = ivParameterController.minIVs;
  criteria.ivs.max = ivParameterController.maxIVs;
  
  criteria.ivs.hiddenTypeMask = ivParameterController.hiddenTypeMask;
  criteria.ivs.minHiddenPower = ivParameterController.minHiddenPower;
  
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
             valueWithPointer: new Gen4QuickSeedSearcher::Criteria(criteria)];
  }
}

- (void)doSearchWithCriteria:(NSValue*)criteriaPtr
{
  std::auto_ptr<Gen4QuickSeedSearcher::Criteria> 
    criteria(static_cast<Gen4QuickSeedSearcher::Criteria*>
               ([criteriaPtr pointerValue]));
  
  Gen4QuickSeedSearcher  searcher;
  
  searcher.Search(*criteria,
                  ResultHandler(searcherController,
                                criteria->tid, criteria->sid),
                  ProgressHandler(searcherController));
}

@end
