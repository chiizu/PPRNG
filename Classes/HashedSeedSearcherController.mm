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
    
    Gen5PIDFrameGenerator::Parameters  frameParameters;
    
    frameParameters.useCompoundEyes = false;
    frameParameters.tid = m_criteria.tid;
    frameParameters.sid = m_criteria.sid;
    
    frameParameters.frameType = Gen5PIDFrameGenerator::GrassCaveFrame;
    Gen5PIDFrameGenerator  gcGenerator(frame.seed, frameParameters);
    
    frameParameters.frameType = Gen5PIDFrameGenerator::FishingFrame;
    Gen5PIDFrameGenerator  fsGenerator(frame.seed, frameParameters);
    
    frameParameters.frameType = Gen5PIDFrameGenerator::SwirlingDustFrame;
    Gen5PIDFrameGenerator  sdGenerator(frame.seed, frameParameters);
    
    frameParameters.frameType = Gen5PIDFrameGenerator::BridgeShadowFrame;
    Gen5PIDFrameGenerator  bsGenerator(frame.seed, frameParameters);
    
    frameParameters.frameType = Gen5PIDFrameGenerator::StationaryFrame;
    Gen5PIDFrameGenerator  stGenerator(frame.seed, frameParameters);
    
    frameParameters.frameType = Gen5PIDFrameGenerator::StarterFossilGiftFrame;
    Gen5PIDFrameGenerator  pidGenerator(frame.seed, frameParameters);
    
        
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
          ((m_criteria.esvMaskLand & (0x1 << ESV::Slot(gcFrame.esv))) != 0) &&
          ((m_criteria.esvMaskWater & (0x1 << ESV::Slot(fsFrame.esv))) != 0) &&
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
        [NSString stringWithFormat: @"%d", ESV::Slot(gcFrame.esv)],
          @"shinyLandESV",
        [NSString stringWithFormat: @"%d", ESV::Slot(fsFrame.esv)],
          @"shinyWaterESV",
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
          frame.seed.year(), frame.seed.month(), frame.seed.day()], @"date",
        [NSString stringWithFormat: @"%.2d:%.2d:%.2d",
          frame.seed.hour, frame.seed.minute, frame.seed.second], @"time",
        [NSNumber numberWithUnsignedInt: frame.seed.timer0], @"timer0",
				[NSString stringWithFormat: @"%s",
          Button::ToString(frame.seed.heldButtons).c_str()], @"keys",
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
  
  criteria.ivFrame.min = [minIVFrameField intValue];
  criteria.ivFrame.max = [maxIVFrameField intValue];
  
  criteria.ivs.min = [ivParameterController minIVs];
  criteria.ivs.shouldCheckMax = [ivParameterController shouldCheckMaxIVs];
  criteria.ivs.max = [ivParameterController maxIVs];
  criteria.ivs.isRoamer = [ivParameterController isRoamer];
  
  if ([ivParameterController shouldCheckHiddenPower])
  {
    criteria.ivs.hiddenType = [ivParameterController hiddenType];
    criteria.ivs.minHiddenPower = [ivParameterController minHiddenPower];
  }
  else
  {
    criteria.ivs.hiddenType = Element::UNKNOWN;
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
