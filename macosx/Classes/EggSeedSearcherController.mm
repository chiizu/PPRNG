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
        [NSNumber numberWithUnsignedLongLong: frame.seed.rawSeed], @"seed",
        [NSString stringWithFormat: @"%.4d/%.2d/%.2d",
          frame.seed.year(), frame.seed.month(), frame.seed.day()], @"date",
        [NSString stringWithFormat: @"%.2d:%.2d:%.2d",
          frame.seed.hour, frame.seed.minute, frame.seed.second], @"time",
        [NSNumber numberWithUnsignedInt: frame.seed.timer0], @"timer0",
				[NSString stringWithFormat: @"%s",
          Button::ToString(frame.seed.heldButtons).c_str()], @"keys",
        [NSNumber numberWithUnsignedInt: frame.seed.GetSkippedPIDFrames() + 1],
          @"startFrame",
				[NSNumber numberWithUnsignedInt: frame.number], @"pidFrame",
        frame.pid.IsShiny(m_tid, m_sid) ? @"★" : @"", @"shiny",
        (m_usingEverstone && frame.everstoneActivated) ?
            @"<ES>" :
            [NSString stringWithFormat: @"%s",
              Nature::ToString(frame.nature).c_str()],
          @"nature",
        frame.inheritsHiddenAbility ? @"Y" : @"", @"dw",
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

@synthesize femaleHP, femaleAT, femaleDF, femaleSA, femaleSD, femaleSP;
@synthesize maleHP, maleAT, maleDF, maleSA, maleSD, maleSP;

- (NSString *)windowNibName
{
	return @"EggSeedSearcher";
}

- (void)awakeFromNib
{
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

- (void)windowWillClose:(NSNotification *)notification
{
  if ([searcherController isSearching])
    [searcherController startStop: self];
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
  
  result.hp([femaleHP intValue]);
  result.at([femaleAT intValue]);
  result.df([femaleDF intValue]);
  result.sa([femaleSA intValue]);
  result.sd([femaleSD intValue]);
  result.sp([femaleSP intValue]);
  
  return result;
}

- (IVs)maleParentIVs
{
  IVs  result;
  
  result.hp([maleHP intValue]);
  result.at([maleAT intValue]);
  result.df([maleDF intValue]);
  result.sa([maleSA intValue]);
  result.sd([maleSD intValue]);
  result.sp([maleSP intValue]);
  
  return result;
}

- (NSValue*)getValidatedSearchCriteria
{
  using namespace boost::gregorian;
  using namespace boost::posix_time;

  EggSeedSearcher::Criteria  criteria;
  
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
  
  criteria.frameParameters.usingEverstone = [everstoneButton state];
  criteria.frameParameters.usingDitto = [dittoButton state];
  criteria.frameParameters.internationalParents = [internationalButton state];
  criteria.frameParameters.tid = [gen5ConfigController tid];
  criteria.frameParameters.sid = [gen5ConfigController sid];
  
  criteria.ivFrame.min = [minIVFrameField intValue];
  criteria.ivFrame.max = [maxIVFrameField intValue];
  
  criteria.ivs.min = [ivParameterController minIVs];
  criteria.ivs.max = [ivParameterController maxIVs];
  criteria.ivs.shouldCheckMax =
    (criteria.ivs.max != IVs(31, 31, 31, 31, 31, 31));
  criteria.ivs.isRoamer = false;
  
  if ([ivParameterController considerHiddenPower])
  {
    criteria.ivs.hiddenType = [ivParameterController hiddenType];
    criteria.ivs.minHiddenPower = [ivParameterController minHiddenPower];
  }
  else
  {
    criteria.ivs.hiddenType = Element::UNKNOWN;
  }
  
  criteria.femaleIVs = [self femaleParentIVs];
  criteria.maleIVs = [self maleParentIVs];
  criteria.femaleSpecies =
    FemaleParent::Type([[femaleSpeciesPopUp selectedItem] tag]);
  
  criteria.pid.nature = Nature::Type([[naturePopUp selectedItem] tag]);
  criteria.pid.ability = Ability::Type([[abilityPopUp selectedItem] tag]);
  criteria.pid.gender = Gender::Type([[genderPopUp selectedItem] tag]);
  criteria.pid.genderRatio =
    Gender::Ratio([[genderRatioPopUp selectedItem] tag]);
  criteria.pid.searchFromInitialFrame = [useInitialPIDFrameCheckBox state];
  
  criteria.inheritsHiddenAbility = [dreamworldButton state];
  criteria.shinyOnly = [shinyButton state];
  criteria.childSpecies = [[speciesPopUp selectedItem] tag] & 0x1;
  
  criteria.pidFrame.min = [minPIDFrameField intValue];
  criteria.pidFrame.max = [maxPIDFrameField intValue];
  
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
                                criteria->frameParameters.tid,
                                criteria->frameParameters.sid,
                                criteria->frameParameters.usingEverstone,
                                criteria->femaleSpecies),
                  ProgressHandler(searcherController));
}


@end
