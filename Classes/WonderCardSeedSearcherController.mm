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



#import "WonderCardSeedSearcherController.h"

#include "WonderCardSeedSearcher.h"
#include "Utilities.h"

#import "WonderCardSeedInspectorController.h"

#include <boost/lexical_cast.hpp>

using namespace pprng;

namespace
{

struct GUICriteria : public WonderCardSeedSearcher::Criteria
{
  bool  showNature;
  bool  showAbility;
  bool  showGender;
};

struct ResultHandler
{
  ResultHandler(SearcherController *c, const GUICriteria &criteria)
    : m_controller(c), m_criteria(criteria)
  {}
  
  void operator()(const WonderCardFrame &frame)
  {
    uint32_t  genderValue = frame.pid.GenderValue();
    
    NSMutableDictionary  *result =
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
        [NSString stringWithFormat: @"%.4d/%.2d/%.2d",
          frame.seed.m_year, frame.seed.m_month, frame.seed.m_day], @"date",
        [NSString stringWithFormat: @"%.2d:%.2d:%.2d",
          frame.seed.m_hour, frame.seed.m_minute, frame.seed.m_second], @"time",
        [NSNumber numberWithUnsignedInt: frame.seed.m_timer0], @"timer0",
				[NSString stringWithFormat: @"%s",
          Button::ToString(frame.seed.m_keyInput).c_str()], @"keys",
        [NSNumber numberWithUnsignedInt: frame.seed.GetSkippedPIDFrames() + 1],
          @"startFrame",
				[NSNumber numberWithUnsignedInt: frame.number], @"frame",
        (m_criteria.showNature ?
            [NSString stringWithFormat: @"%s",
             Nature::ToString(frame.nature).c_str()] : @""),
          @"nature",
        (m_criteria.showAbility ?
         [NSString stringWithFormat: @"%d", frame.pid.Gen5Ability()] : @""),
          @"ability",
        (m_criteria.showGender ? ((genderValue < 31) ? @"♀" : @"♂") : @""),
          @"gender18",
        (m_criteria.showGender ? ((genderValue < 63) ? @"♀" : @"♂") : @""),
          @"gender14",
        (m_criteria.showGender ? ((genderValue < 127) ? @"♀" : @"♂") : @""),
          @"gender12",
        (m_criteria.showGender ? ((genderValue < 191) ? @"♀" : @"♂") : @""),
          @"gender34",
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
        [NSData dataWithBytes: &frame.seed length: sizeof(HashedSeed)],
          @"fullSeed",
        nil];
    
    [m_controller performSelectorOnMainThread: @selector(addResult:)
                  withObject: result
                  waitUntilDone: NO];
  }
  
  SearcherController  *m_controller;
  const               GUICriteria &m_criteria;
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


@implementation WonderCardSeedSearcherController

- (NSString *)windowNibName
{
	return @"WonderCardSeedSearcher";
}

- (void)awakeFromNib
{
  [super awakeFromNib];
  
  [searcherController setGetValidatedSearchCriteriaSelector:
                      @selector(getValidatedSearchCriteria)];
  [searcherController setDoSearchWithCriteriaSelector:
                      @selector(doSearchWithCriteria:)];
  
  [[searcherController tableView] setDoubleAction: @selector(inspectSeed:)];
  
  NSDate  *now = [NSDate date];
  [fromDateField setObjectValue: now];
  [toDateField setObjectValue: now];
}

- (void)windowWillClose:(NSNotification *)notification
{
  if ([searcherController isSearching])
    [searcherController startStop: self];
}

- (IBAction)onTypeChange:(id)sender
{
  NSInteger  selection = [[sender selectedItem] tag];
  
  if (selection == -1)
  {
    [ivSkipField setEnabled: YES];
    [pidSkipField setEnabled: YES];
    [natureSkipField setEnabled: YES];
  }
  else
  {
    [ivSkipField setEnabled: NO];
    [pidSkipField setEnabled: NO];
    [natureSkipField setEnabled: NO];
    
    if (selection == 0)
    {
      [ivSkipField setIntValue: 22];
      [pidSkipField setIntValue: 2];
      [natureSkipField setIntValue: 1];
    }
    else
    {
      [ivSkipField setIntValue: 24];
      [pidSkipField setIntValue: 2];
      [natureSkipField setIntValue: 3];
    }
  }
}

- (IBAction)toggleFixedNature:(id)sender
{
  BOOL  checked = [fixedNatureCheckBox state];
  [naturePopUp setEnabled: !checked];
  if (checked)
  {
    [naturePopUp selectItemWithTag: -1];
  }
}

- (IBAction)toggleFixedAbility:(id)sender
{
  BOOL  checked = [fixedAbilityCheckBox state];
  [abilityPopUp setEnabled: !checked];
  if (checked)
  {
    [abilityPopUp selectItemWithTag: -1];
  }
}

- (IBAction)toggleFixedGender:(id)sender
{
  BOOL  checked = [fixedGenderCheckBox state];
  [genderPopUp setEnabled: !checked];
  [genderRatioPopUp setEnabled: !checked];
  if (checked)
  {
    [genderPopUp selectItemWithTag: -1];
    [genderRatioPopUp selectItemWithTag: -1];
  }
}

- (IBAction)toggleUseInitialPID:(id)sender
{
  BOOL enabled = [useInitialPIDButton state];
  [minFrameField setEnabled: !enabled];
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
      NSData  *seed = [row objectForKey: @"fullSeed"];
      
      WonderCardSeedInspectorController  *inspector =
        [[WonderCardSeedInspectorController alloc] init];
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
  using namespace boost::gregorian;
  using namespace boost::posix_time;
  
  GUICriteria  criteria;
  
  criteria.macAddressLow = [gen5ConfigController macAddressLow];
  criteria.macAddressHigh = [gen5ConfigController macAddressHigh];
  
  criteria.version = [gen5ConfigController version];
  
  criteria.timer0Low = [gen5ConfigController timer0Low];
  criteria.timer0High = [gen5ConfigController timer0High];
  
  criteria.vcountLow = [gen5ConfigController vcountLow];
  criteria.vcountHigh = [gen5ConfigController vcountHigh];
  
  criteria.vframeLow = [gen5ConfigController vframeLow];
  criteria.vframeHigh = [gen5ConfigController vframeHigh];
  
  if ([noKeyHeldButton state])
  {
    criteria.buttonPresses.push_back(0);  // no keys
  }
  if ([oneKeyHeldButton state])
  {
    criteria.buttonPresses.insert(criteria.buttonPresses.end(),
                                  Button::SingleButtons().begin(),
                                  Button::SingleButtons().end());
  }
  if ([twoKeysHeldButton state])
  {
    criteria.buttonPresses.insert(criteria.buttonPresses.end(),
                                  Button::TwoButtonCombos().begin(),
                                  Button::TwoButtonCombos().end());
  }
  if ([threeKeysHeldButton state])
  {
    criteria.buttonPresses.insert(criteria.buttonPresses.end(),
                                  Button::ThreeButtonCombos().begin(),
                                  Button::ThreeButtonCombos().end());
  }
  
  criteria.fromTime = ptime(NSDateToBoostDate([fromDateField objectValue]),
                            seconds(0));
  
  criteria.toTime   = ptime(NSDateToBoostDate([toDateField objectValue]),
                            hours(23) + minutes(59) + seconds(59));
  
  criteria.startFromLowestFrame = [useInitialPIDButton state];
  criteria.minFrame = [minFrameField intValue];
  criteria.maxFrame = [maxFrameField intValue];
  
  criteria.ivSkip = [ivSkipField intValue];
  criteria.pidSkip = [pidSkipField intValue];
  criteria.natureSkip = [natureSkipField intValue];
  
  criteria.minIVs = [ivParameterController minIVs];
  criteria.shouldCheckMaxIVs = [ivParameterController shouldCheckMaxIVs];
  criteria.maxIVs = [ivParameterController maxIVs];
  
  if ([ivParameterController shouldCheckHiddenPower])
  {
    criteria.hiddenType = [ivParameterController hiddenType];
    criteria.minHiddenPower = [ivParameterController minHiddenPower];
  }
  else
  {
    criteria.hiddenType = Element::UNKNOWN;
  }
  criteria.nature = static_cast<Nature::Type>([[naturePopUp selectedItem] tag]);
  criteria.ability = [[abilityPopUp selectedItem] tag];
  criteria.gender = Gender::Type([[genderPopUp selectedItem] tag]);
  criteria.genderRatio =
    Gender::Ratio([[genderRatioPopUp selectedItem] tag]);
  criteria.canBeShiny = false;
  
  criteria.showNature = ![fixedNatureCheckBox state];
  criteria.showAbility = ![fixedAbilityCheckBox state];
  criteria.showGender = ![fixedGenderCheckBox state];
  
  if (CheckExpectedResults(criteria, 10000,
                           @"The current search parameters are expected to return more than 10,000 results. Please set more specific IVs, limit the date range, use fewer held keys, or other similar settings to reduce the number of expected results.",
                           self,
                           @selector(alertDidEnd:returnCode:contextInfo:)))
  {
    return [NSValue valueWithPointer: new GUICriteria(criteria)];
  }
  else
  {
    return nil;
  }
}

- (void)doSearchWithCriteria:(NSValue*)criteriaPtr
{
  std::auto_ptr<GUICriteria>
    criteria(static_cast<GUICriteria*>([criteriaPtr pointerValue]));
  
  WonderCardSeedSearcher  searcher;
  
  searcher.Search(*criteria,
                  ResultHandler(searcherController, *criteria),
                  ProgressHandler(searcherController));
}


@end
