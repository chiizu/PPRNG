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

struct ResultHandler
{
  ResultHandler(SearcherController *c,
                const WonderCardSeedSearcher::Criteria &criteria)
    : m_controller(c), m_criteria(criteria)
  {}
  
  void operator()(const WonderCardFrame &frame)
  {
    uint32_t  genderValue = frame.pid.GenderValue();
    bool      showGender = m_criteria.frameParameters.cardGender == Gender::ANY;
    
    NSMutableDictionary  *result =
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
        [NSString stringWithFormat: @"%.4d/%.2d/%.2d",
          frame.seed.year(), frame.seed.month(), frame.seed.day()], @"date",
        [NSString stringWithFormat: @"%.2d:%.2d:%.2d",
          frame.seed.hour, frame.seed.minute, frame.seed.second], @"time",
        [NSNumber numberWithUnsignedInt: frame.seed.timer0], @"timer0",
				[NSString stringWithFormat: @"%s",
          Button::ToString(frame.seed.heldButtons).c_str()], @"keys",
        [NSNumber numberWithUnsignedInt: frame.seed.GetSkippedPIDFrames() + 1],
          @"startFrame",
				[NSNumber numberWithUnsignedInt: frame.number], @"frame",
        [NSString stringWithFormat: @"%s",
          Nature::ToString
            ((m_criteria.frameParameters.cardNature != Nature::ANY) ?
               m_criteria.frameParameters.cardNature : frame.nature).c_str()],
          @"nature",
        ((m_criteria.frameParameters.cardAbility == Ability::HIDDEN) ?
          @"DW" :
          [NSString stringWithFormat: @"%d",
            ((m_criteria.frameParameters.cardAbility == Ability::ANY) ?
               frame.pid.Gen5Ability() :
               m_criteria.frameParameters.cardAbility)]),
          @"ability",
        (showGender ? ((genderValue < 31) ? @"♀" : @"♂") : @""),
          @"gender18",
        (showGender ? ((genderValue < 63) ? @"♀" : @"♂") : @""),
          @"gender14",
        (showGender ? ((genderValue < 127) ? @"♀" : @"♂") : @""),
          @"gender12",
        (showGender ? ((genderValue < 191) ? @"♀" : @"♂") : @""),
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
  
  SearcherController                      *m_controller;
  const WonderCardSeedSearcher::Criteria  &m_criteria;
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

@synthesize cardNature;
@synthesize cardAbility;
@synthesize cardAlwaysShiny;
@synthesize cardGender;
@synthesize cardGenderRatio;

@synthesize natureSearchable;
@synthesize abilitySearchable;
@synthesize genderSearchable;

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
  
  self.cardNature = Nature::ANY;
  self.cardAbility = Ability::ANY;
  self.cardAlwaysShiny = NO;
  self.cardGender = Gender::ANY;
  self.cardGenderRatio = Gender::UNSPECIFIED;
}

- (void)windowWillClose:(NSNotification *)notification
{
  if ([searcherController isSearching])
    [searcherController startStop: self];
}

- (void)setCardNature:(uint32_t)newValue
{
  if (cardNature != newValue)
  {
    cardNature = newValue;
    
    self.natureSearchable = (newValue == Nature::ANY);
    if (!natureSearchable)
    {
      [naturePopUp selectItemWithTag: Nature::ANY];
    }
  }
}

- (void)setCardAbility:(uint32_t)newValue
{
  if (cardAbility != newValue)
  {
    cardAbility = newValue;
    
    self.abilitySearchable = (newValue == Ability::ANY);
    if (!abilitySearchable)
    {
      [abilityPopUp selectItemWithTag: Ability::ANY];
    }
  }
}

- (void)setCardGender:(uint32_t)newValue
{
  if (cardGender != newValue)
  {
    cardGender = newValue;
    
    self.genderSearchable = (newValue == Gender::ANY);
    if (!genderSearchable)
    {
      [genderPopUp selectItemWithTag: Gender::ANY];
      [genderRatioPopUp selectItemWithTag: Gender::UNSPECIFIED];
    }
    
    if (cardGender == Gender::NEUTRAL)
      self.cardGenderRatio = Gender::UNSPECIFIED;
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
      
      inspector.cardNature = cardNature;
      inspector.cardAbility = cardAbility;
      inspector.cardAlwaysShiny = cardAlwaysShiny;
      inspector.cardGender = cardGender;
      inspector.cardGenderRatio = cardGenderRatio;
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
  
  WonderCardSeedSearcher::Criteria  criteria;
  
  criteria.seedParameters.macAddress = [gen5ConfigController macAddress];
  
  criteria.seedParameters.version = [gen5ConfigController version];
  criteria.seedParameters.dsType = [gen5ConfigController dsType];
  
  criteria.seedParameters.timer0Low = [gen5ConfigController timer0Low];
  criteria.seedParameters.timer0High = [gen5ConfigController timer0High];
  
  criteria.seedParameters.vcountLow = [gen5ConfigController vcountLow];
  criteria.seedParameters.vcountHigh = [gen5ConfigController vcountHigh];
  
  criteria.seedParameters.vframeLow = [gen5ConfigController vframeLow];
  criteria.seedParameters.vframeHigh = [gen5ConfigController vframeHigh];
  
  if ([noKeyHeldButton state])
  {
    criteria.seedParameters.heldButtons.push_back(0);  // no keys
  }
  if ([oneKeyHeldButton state])
  {
    criteria.seedParameters.heldButtons.insert
      (criteria.seedParameters.heldButtons.end(),
       Button::SingleButtons().begin(),
       Button::SingleButtons().end());
  }
  if ([twoKeysHeldButton state])
  {
    criteria.seedParameters.heldButtons.insert
      (criteria.seedParameters.heldButtons.end(),
       Button::TwoButtonCombos().begin(),
       Button::TwoButtonCombos().end());
  }
  if ([threeKeysHeldButton state])
  {
    criteria.seedParameters.heldButtons.insert
      (criteria.seedParameters.heldButtons.end(),
       Button::ThreeButtonCombos().begin(),
       Button::ThreeButtonCombos().end());
  }
  
  criteria.seedParameters.fromTime =
    ptime(NSDateToBoostDate([fromDateField objectValue]), seconds(0));
  
  criteria.seedParameters.toTime =
    ptime(NSDateToBoostDate([toDateField objectValue]),
                            hours(23) + minutes(59) + seconds(59));
  
  criteria.frameParameters.startFromLowestFrame = [useInitialPIDButton state];
  criteria.frameParameters.cardNature = Nature::Type(cardNature);
  criteria.frameParameters.cardAbility = cardAbility;
  criteria.frameParameters.cardAlwaysShiny = cardAlwaysShiny;
  criteria.frameParameters.cardGender = Gender::Type(cardGender);
  criteria.frameParameters.cardGenderRatio = Gender::Ratio(cardGenderRatio);
  
  criteria.ivs.min = [ivParameterController minIVs];
  criteria.ivs.max = [ivParameterController maxIVs];
  criteria.ivs.shouldCheckMax =
    (criteria.ivs.max != IVs(31, 31, 31, 31, 31, 31));
  
  if ([ivParameterController considerHiddenPower])
  {
    criteria.ivs.hiddenType = [ivParameterController hiddenType];
    criteria.ivs.minHiddenPower = [ivParameterController minHiddenPower];
  }
  else
  {
    criteria.ivs.hiddenType = Element::UNKNOWN;
  }
  criteria.pid.nature = Nature::Type([[naturePopUp selectedItem] tag]);
  criteria.pid.ability = Ability::Type([[abilityPopUp selectedItem] tag]);
  criteria.pid.gender = Gender::Type([[genderPopUp selectedItem] tag]);
  criteria.pid.genderRatio =
    Gender::Ratio([[genderRatioPopUp selectedItem] tag]);
  
  criteria.frame.min = [minFrameField intValue];
  criteria.frame.max = [maxFrameField intValue];
  
  if (CheckExpectedResults(criteria, 10000,
                           @"The current search parameters are expected to return more than 10,000 results. Please set more specific IVs, limit the date range, use fewer held keys, or other similar settings to reduce the number of expected results.",
                           self,
                           @selector(alertDidEnd:returnCode:contextInfo:)))
  {
    return [NSValue valueWithPointer:
            new WonderCardSeedSearcher::Criteria(criteria)];
  }
  else
  {
    return nil;
  }
}

- (void)doSearchWithCriteria:(NSValue*)criteriaPtr
{
  std::auto_ptr<WonderCardSeedSearcher::Criteria>
    criteria(static_cast<WonderCardSeedSearcher::Criteria*>
      ([criteriaPtr pointerValue]));
  
  WonderCardSeedSearcher  searcher;
  
  searcher.Search(*criteria,
                  ResultHandler(searcherController, *criteria),
                  ProgressHandler(searcherController));
}


@end
