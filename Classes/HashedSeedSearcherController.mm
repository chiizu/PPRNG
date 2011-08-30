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


#import "HashedSeedSearcherController.h"

#include "HashedSeedSearcher.h"
#include "FrameGenerator.h"
#include "Utilities.h"

#import "HashedSeedInspectorController.h"

#include <memory>

using namespace pprng;

namespace
{

uint32_t GetESVBitmask(NSPopUpButton *esvMenu)
{
  uint32_t   mask = 0;
  NSInteger  numItems = [esvMenu numberOfItems];
  NSInteger  i;
  
  for (i = 0; i < numItems; ++i)
  {
    NSMenuItem  *item = [esvMenu itemAtIndex: i];
    NSInteger   tag = [item tag];
    
    if ((tag >= 0) && ([item state] == NSOnState))
    {
      mask |= 0x1 << tag;
    }
  }
  
  return mask;
}

struct GUICriteria : public HashedSeedSearcher::Criteria
{
  enum EncounterType
  {
    WildEncounter = 0,
    GiftEncounter,
    BothEncounters,
    AnyEncounter = -1
  };
  
  uint32_t       tid, sid;
  bool           shinyOnly;
  EncounterType  encounterType;
  bool           startFromLowestPID;
  uint32_t       minPIDFrame, maxPIDFrame;
  Nature::Type   nature;
  uint32_t       ability;
  Gender::Type   gender;
  Gender::Ratio  genderRatio;
  bool           syncA;
  bool           syncB;
  bool           syncC;
  uint32_t       esvMaskLand;
  uint32_t       esvMaskWater;
  bool           canFish;
  bool           dustIsPoke;
  bool           shadowIsPoke;
  
  
  uint64_t ExpectedNumberOfResults() const
  {
    uint64_t  result = HashedSeedSearcher::Criteria::ExpectedNumberOfResults();
    uint64_t  pidFrameMultiplier = 1;
    uint64_t  shinyDivisor = 1;
    uint64_t  natureDivisor = 1;
    
    if (shinyOnly)
    {
      shinyDivisor = 8192;
      pidFrameMultiplier = maxPIDFrame - minPIDFrame + 1;
      
      if ((nature != Nature::ANY) && (nature != Nature::UNKNOWN))
      {
        natureDivisor = 25;
      }
    }
    
    return (result  * pidFrameMultiplier) / (shinyDivisor * natureDivisor);
  }
};

struct ResultHandler
{
  ResultHandler(SearcherController *c, const GUICriteria &criteria)
    : controller(c), m_criteria(criteria)
  {}
  
  void operator()(const HashedIVFrame &frame)
  {
    NSMutableDictionary  *pidResult = nil;
    
    Gen5PIDFrameGenerator
      gcGenerator(frame.seed, Gen5PIDFrameGenerator::GrassCaveFrame,
                  false, m_criteria.tid, m_criteria.sid),
      fsGenerator(frame.seed, Gen5PIDFrameGenerator::FishingFrame,
                  false, m_criteria.tid, m_criteria.sid),
      sdGenerator(frame.seed, Gen5PIDFrameGenerator::SwirlingDustFrame,
                  false, m_criteria.tid, m_criteria.sid),
      bsGenerator(frame.seed, Gen5PIDFrameGenerator::BridgeShadowFrame,
                  false, m_criteria.tid, m_criteria.sid),
      stGenerator(frame.seed, Gen5PIDFrameGenerator::StationaryFrame,
                  false, m_criteria.tid, m_criteria.sid),
      pidGenerator(frame.seed, Gen5PIDFrameGenerator::StarterFossilGiftFrame,
                   false, m_criteria.tid, m_criteria.sid);
    
    // get the PIDs in sync
    gcGenerator.AdvanceFrame();
    stGenerator.AdvanceFrame();
    stGenerator.AdvanceFrame();
    stGenerator.AdvanceFrame();
    pidGenerator.AdvanceFrame();
    pidGenerator.AdvanceFrame();
    pidGenerator.AdvanceFrame();
    pidGenerator.AdvanceFrame();
    
    bool      found = false;
    uint32_t  minFrame = m_criteria.minPIDFrame - 1;
    
    if (m_criteria.startFromLowestPID)
      minFrame = frame.seed.GetSkippedPIDFrames();
    
    while (fsGenerator.CurrentFrame().number < minFrame)
    {
      gcGenerator.AdvanceFrame();
      fsGenerator.AdvanceFrame();
      sdGenerator.AdvanceFrame();
      bsGenerator.AdvanceFrame();
      stGenerator.AdvanceFrame();
      pidGenerator.AdvanceFrame();
    }
    
    bool  wildShiny, giftShiny;
    
    while ((fsGenerator.CurrentFrame().number < m_criteria.maxPIDFrame) &&
           !found)
    {
      gcGenerator.AdvanceFrame();
      fsGenerator.AdvanceFrame();
      sdGenerator.AdvanceFrame();
      bsGenerator.AdvanceFrame();
      stGenerator.AdvanceFrame();
      pidGenerator.AdvanceFrame();
      
      Gen5PIDFrame  gcFrame = gcGenerator.CurrentFrame();
      Gen5PIDFrame  fsFrame = fsGenerator.CurrentFrame();
      Gen5PIDFrame  sdFrame = sdGenerator.CurrentFrame();
      Gen5PIDFrame  bsFrame = bsGenerator.CurrentFrame();
      Gen5PIDFrame  stFrame = stGenerator.CurrentFrame();
      Gen5PIDFrame  pidFrame = pidGenerator.CurrentFrame();
      
      wildShiny = gcGenerator.CurrentFrame().pid.IsShiny(m_criteria.tid,
                                                         m_criteria.sid);
      giftShiny = pidGenerator.CurrentFrame().pid.IsShiny(m_criteria.tid,
                                                          m_criteria.sid);
      bool  shinyFound = false;
      
      switch (m_criteria.encounterType)
      {
      case GUICriteria::WildEncounter:
        shinyFound = wildShiny;
        break;
      case GUICriteria::GiftEncounter:
        shinyFound = giftShiny;
        break;
      case GUICriteria::BothEncounters:
        shinyFound = wildShiny && giftShiny;
        break;
      case GUICriteria::AnyEncounter:
      default:
        shinyFound = wildShiny || giftShiny;
        break;
      }
      
      if (shinyFound &&
          ((m_criteria.nature == Nature::ANY) ||
           (pidFrame.nature == m_criteria.nature)) &&
          ((m_criteria.ability > 1) ||
           (m_criteria.ability == pidFrame.pid.Gen5Ability())) &&
          Gender::GenderValueMatches(pidFrame.pid.GenderValue(),
                                     m_criteria.gender,
                                     m_criteria.genderRatio) &&
          (!m_criteria.syncA || gcFrame.synched) &&
          (!m_criteria.syncB || fsFrame.synched) &&
          (!m_criteria.syncC || stFrame.synched) &&
          ((m_criteria.esvMaskLand & (0x1 << gcFrame.esv)) != 0) &&
          ((m_criteria.esvMaskWater & (0x1 << fsFrame.esv)) != 0) &&
          (!m_criteria.canFish || fsFrame.isEncounter) &&
          (!m_criteria.dustIsPoke || !sdFrame.isEncounter) &&
          (!m_criteria.shadowIsPoke || !bsFrame.isEncounter))
      {
        found = true;
      }
    }
    
    if (found)
    {
      Gen5PIDFrame  gcFrame = gcGenerator.CurrentFrame();
      Gen5PIDFrame  fsFrame = fsGenerator.CurrentFrame();
      Gen5PIDFrame  sdFrame = sdGenerator.CurrentFrame();
      Gen5PIDFrame  bsFrame = bsGenerator.CurrentFrame();
      Gen5PIDFrame  stFrame = stGenerator.CurrentFrame();
      Gen5PIDFrame  pidFrame = pidGenerator.CurrentFrame();
      
      uint32_t  genderValue = pidFrame.pid.GenderValue();
      
      pidResult = [NSMutableDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithUnsignedInt: sdFrame.number], @"shinyFrame",
        (wildShiny ? @"Y" : @""), @"wildIsShiny",
        (giftShiny ? @"Y" : @""), @"giftIsShiny",
        [NSString stringWithFormat: @"%s",
          Nature::ToString(pidFrame.nature).c_str()], @"shinyNature",
        [NSNumber numberWithUnsignedInt: pidFrame.pid.Gen5Ability()],
          @"shinyAbility",
        ((genderValue < 31) ? @"♀" : @"♂"), @"gender18",
        ((genderValue < 63) ? @"♀" : @"♂"), @"gender14",
        ((genderValue < 127) ? @"♀" : @"♂"), @"gender12",
        ((genderValue < 191) ? @"♀" : @"♂"), @"gender34",
        (gcFrame.synched ? @"Y" : @""), @"shinySyncA",
        (fsFrame.synched ? @"Y" : @""), @"shinySyncB",
        (stFrame.synched ? @"Y" : @""), @"shinySyncC",
        [NSString stringWithFormat: @"%d", gcFrame.esv], @"shinyLandESV",
        [NSString stringWithFormat: @"%d", fsFrame.esv], @"shinyWaterESV",
        (fsFrame.isEncounter ? @"Y" : @""), @"canFish",
        (sdFrame.isEncounter ? @"" : @"Y"), @"dustIsPoke",
        (bsFrame.isEncounter ? @"" : @"Y"), @"shadowIsPoke",
        nil];
    }
    else if (m_criteria.shinyOnly)
    {
      return;
    }
    
    NSMutableDictionary  *result =
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
        [NSString stringWithFormat: @"%.4d/%.2d/%.2d",
          frame.seed.m_year, frame.seed.m_month, frame.seed.m_day], @"date",
        [NSString stringWithFormat: @"%.2d:%.2d:%.2d",
          frame.seed.m_hour, frame.seed.m_minute, frame.seed.m_second], @"time",
        [NSNumber numberWithUnsignedInt: frame.seed.m_timer0], @"timer0",
				[NSString stringWithFormat: @"%s",
          Button::ToString(frame.seed.m_keyInput).c_str()], @"keys",
				[NSNumber numberWithUnsignedInt: frame.number], @"frame",
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
        [NSNumber numberWithUnsignedInt: frame.seed.GetSkippedPIDFrames() + 1],
          @"startFrame",
        [NSData dataWithBytes: &frame.seed length: sizeof(HashedSeed)],
          @"fullSeed",
        nil];
    
    if (pidResult)
    {
      [result addEntriesFromDictionary: pidResult];
    }
    
    [controller performSelectorOnMainThread: @selector(addResult:)
                withObject: result
                waitUntilDone: NO];
  }
  
  SearcherController  *controller;
  const GUICriteria   &m_criteria;
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


@implementation HashedSeedSearcherController

- (NSString *)windowNibName
{
	return @"HashedSeedSearcher";
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

- (IBAction)toggleSearchFromStartFrame:(id)sender
{
  BOOL  enabled = [sender state];
  
  [minPIDFrameField setEnabled: !enabled];
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
        case -2:
          if (tag & 0x1)
            [item setState: NSOnState];
          else
            [item setState: NSOffState];
          break;
        case -3:
          if (tag & 0x1)
            [item setState: NSOffState];
          else
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
  
  criteria.minIVFrame = [minIVFrameField intValue];
  criteria.maxIVFrame = [maxIVFrameField intValue];
  
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
  
  criteria.tid = [gen5ConfigController tid];
  criteria.sid = [gen5ConfigController sid];
  criteria.shinyOnly = [shinyOnlyCheckbox state];
  criteria.encounterType =
    GUICriteria::EncounterType([[shinyEncounterTypePopUp selectedItem] tag]);
  criteria.startFromLowestPID = [shinyFromFirstPIDCheckBox state];
  criteria.minPIDFrame = [minPIDFrameField intValue];
  criteria.maxPIDFrame = [maxPIDFrameField intValue];
  criteria.nature = Nature::Type([[shinyNaturePopUp selectedItem] tag]);
  criteria.ability = [[shinyAbilityPopUp selectedItem] tag];
  criteria.gender = Gender::Type([[shinyGenderPopUp selectedItem] tag]);
  criteria.genderRatio =
    Gender::Ratio([[shinyGenderRatioPopUp selectedItem] tag]);
  criteria.syncA = [shinySyncACheckBox state];
  criteria.syncB = [shinySyncBCheckBox state];
  criteria.syncC = [shinySyncCCheckBox state];
  criteria.esvMaskLand = GetESVBitmask(shinyLandESVPopUp);
  criteria.esvMaskWater = GetESVBitmask(shinyWaterESVPopUp);
  criteria.canFish = [shinyCanFishCheckBox state];
  criteria.dustIsPoke = [shinyDustIsPokeCheckBox state];
  criteria.shadowIsPoke = [shinyShadowIsPokeCheckBox state];
  
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
  
  HashedSeedSearcher  searcher;
  
  searcher.Search(*criteria, ResultHandler(searcherController, *criteria),
                  ProgressHandler(searcherController));
}


@end
