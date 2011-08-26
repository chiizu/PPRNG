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


#import "EggSeedSearcherController.h"

#import "HashedSeedInspectorController.h"

#include "EggSeedSearcher.h"
#include "Utilities.h"

using namespace pprng;

namespace
{

struct ResultHandler
{
  ResultHandler(SearcherController *c, uint32_t tid, uint32_t sid,
                bool usingEverstone, FemaleParent::Type femaleSpecies)
    : controller(c), m_tid(tid), m_sid(sid), m_usingEverstone(usingEverstone),
      m_femaleSpecies(femaleSpecies)
  {}
  
  void operator()(const Gen5EggFrame &frame)
  {
    uint32_t  genderValue = frame.pid.GenderValue();
    
    NSMutableDictionary  *result =
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithUnsignedLongLong: frame.seed.m_rawSeed], @"seed",
        [NSString stringWithFormat: @"%.4d/%.2d/%.2d",
          frame.seed.m_year, frame.seed.m_month, frame.seed.m_day], @"date",
        [NSString stringWithFormat: @"%.2d:%.2d:%.2d",
          frame.seed.m_hour, frame.seed.m_minute, frame.seed.m_second], @"time",
        [NSNumber numberWithUnsignedInt: frame.seed.m_timer0], @"timer0",
				[NSString stringWithFormat: @"%s",
          Button::ToString(frame.seed.m_keyInput).c_str()], @"keys",
        [NSNumber numberWithUnsignedInt: frame.seed.GetSkippedPIDFrames() + 1],
          @"startFrame",
				[NSNumber numberWithUnsignedInt: frame.number], @"pidFrame",
        frame.pid.IsShiny(m_tid, m_sid) ? @"★" : @"", @"shiny",
        (m_usingEverstone && frame.everstoneActivated) ?
            @"<ES>" :
            [NSString stringWithFormat: @"%s",
              Nature::ToString(frame.nature).c_str()],
          @"nature",
        frame.dreamWorldAbilityPassed ? @"Y" : @"", @"dw",
        [NSNumber numberWithUnsignedInt: frame.pid.Gen5Ability()], @"ability",
        ((m_femaleSpecies == FemaleParent::OTHER) ?
         ((genderValue < 31) ? @"♀" : @"♂") : @""), @"gender18",
        ((m_femaleSpecies == FemaleParent::OTHER) ?
         ((genderValue < 63) ? @"♀" : @"♂") : @""), @"gender14",
        ((m_femaleSpecies == FemaleParent::OTHER) ?
         ((genderValue < 127) ? @"♀" : @"♂") : @""), @"gender12",
        ((m_femaleSpecies == FemaleParent::OTHER) ?
         ((genderValue < 191) ? @"♀" : @"♂") : @""), @"gender34",
				[NSNumber numberWithUnsignedInt: frame.ivFrameNumber], @"ivFrame",
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
        SpeciesString(m_femaleSpecies, frame.species), @"species",
        [NSData dataWithBytes: &frame.seed length: sizeof(HashedSeed)],
          @"fullSeed",
        nil];
    
    [controller performSelectorOnMainThread: @selector(addResult:)
                withObject: result
                waitUntilDone: NO];
  }
  
  SearcherController  *controller;
  uint32_t            m_tid, m_sid;
  bool                m_usingEverstone;
  FemaleParent::Type  m_femaleSpecies;
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

@implementation EggSeedSearcherController

- (NSString *)windowNibName
{
	return @"EggSeedSearcher";
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
  
  [speciesPopUp setAutoenablesItems: NO];
}

- (IBAction)onFemaleSpeciesChange:(id)sender
{
  FemaleParent::Type  species =
    FemaleParent::Type([[femaleSpeciesPopUp selectedItem] tag]);
  
  if (species == FemaleParent::OTHER)
  {
    [speciesPopUp setEnabled: NO];
    [speciesPopUp selectItemWithTag: -1];
    [genderPopUp setEnabled: YES];
    [genderRatioPopUp setEnabled: YES];
  }
  else
  {
    [speciesPopUp setEnabled: YES];
    [genderPopUp setEnabled: NO];
    [genderPopUp selectItemWithTag: -1];
    [genderRatioPopUp setEnabled: NO];
    [genderRatioPopUp selectItemWithTag: -1];
    
    BOOL  isNidoranFemale = (species == FemaleParent::NIDORAN_FEMALE);
    
    [[speciesPopUp itemAtIndex: [speciesPopUp indexOfItemWithTag: 0]]
      setEnabled: isNidoranFemale];
    [[speciesPopUp itemAtIndex: [speciesPopUp indexOfItemWithTag: 1]]
      setEnabled: isNidoranFemale];
    [[speciesPopUp itemAtIndex: [speciesPopUp indexOfItemWithTag: 2]]
      setEnabled: !isNidoranFemale];
    [[speciesPopUp itemAtIndex: [speciesPopUp indexOfItemWithTag: 3]]
      setEnabled: !isNidoranFemale];
    
    if (![[speciesPopUp selectedItem] isEnabled])
    {
      [speciesPopUp selectItemWithTag: -1];
    }
  }
}

- (IBAction)toggleSearchFromStartFrame:(id)sender
{
  BOOL  enabled = [sender state];
  
  [minPIDFrameField setEnabled: !enabled];
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
      
      HashedSeedInspectorController  *inspector =
        [[HashedSeedInspectorController alloc] init];
      [inspector showWindow: self];
      [inspector setSeed: seed];
    }
  }
}

// dummy method for error panel
- (void)alertDidEnd:(NSAlert *)alert returnCode:(NSInteger)returnCode
        contextInfo:(void *)contextInfo
{}

- (IVs)femaleParentIVs
{
  IVs  result;
  
  result.hp([femaleHPField intValue]);
  result.at([femaleAtkField intValue]);
  result.df([femaleDefField intValue]);
  result.sa([femaleSpAField intValue]);
  result.sd([femaleSpDField intValue]);
  result.sp([femaleSpeField intValue]);
  
  return result;
}

- (IVs)maleParentIVs
{
  IVs  result;
  
  result.hp([maleHPField intValue]);
  result.at([maleAtkField intValue]);
  result.df([maleDefField intValue]);
  result.sa([maleSpAField intValue]);
  result.sd([maleSpDField intValue]);
  result.sp([maleSpeField intValue]);
  
  return result;
}

- (NSValue*)getValidatedSearchCriteria
{
  using namespace boost::gregorian;
  using namespace boost::posix_time;

  EggSeedSearcher::Criteria  criteria;
  
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
  
  criteria.femaleIVs = [self femaleParentIVs];
  criteria.maleIVs = [self maleParentIVs];
  criteria.femaleSpecies =
    FemaleParent::Type([[femaleSpeciesPopUp selectedItem] tag]);
  criteria.usingEverstone = [everstoneButton state];
  criteria.usingDitto = [dittoButton state];
  criteria.internationalParents = [internationalButton state];
  
  criteria.minIVFrame = [minIVFrameField intValue];
  criteria.maxIVFrame = [maxIVFrameField intValue];
  
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
  
  criteria.tid = [gen5ConfigController tid];
  criteria.sid = [gen5ConfigController sid];
  criteria.nature = Nature::Type([[naturePopUp selectedItem] tag]);
  criteria.ability = [[abilityPopUp selectedItem] tag];
  criteria.inheritsDreamworldAbility = [dreamworldButton state];
  criteria.shinyOnly = [shinyButton state];
  criteria.childSpecies = [[speciesPopUp selectedItem] tag] & 0x1;
  criteria.gender = Gender::Type([[genderPopUp selectedItem] tag]);
  criteria.genderRatio =
    Gender::Ratio([[genderRatioPopUp selectedItem] tag]);
  
  criteria.minPIDFrame = [minPIDFrameField intValue];
  criteria.maxPIDFrame = [maxPIDFrameField intValue];
  
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
    [alert setInformativeText:@"The current search parameters are expected to return more than 10,000 results. Please set more specific IVs, limit the date range, use fewer held keys, or other similar settings to reduce the number of expected results."];
    [alert setAlertStyle:NSWarningAlertStyle];
    
    [alert beginSheetModalForWindow:[self window] modalDelegate:self
           didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
           contextInfo:nil];
    
    return nil;
  }
  else
  {
    return [NSValue valueWithPointer: new EggSeedSearcher::Criteria(criteria)];
  }
}

- (void)doSearchWithCriteria:(NSValue*)criteriaPtr
{
  std::auto_ptr<EggSeedSearcher::Criteria> 
    criteria(static_cast<EggSeedSearcher::Criteria*>
      ([criteriaPtr pointerValue]));
  
  EggSeedSearcher  searcher;
  
  searcher.Search(*criteria,
                  ResultHandler(searcherController,
                                criteria->tid, criteria->sid,
                                criteria->usingEverstone,
                                criteria->femaleSpecies),
                  ProgressHandler(searcherController));
}


@end
