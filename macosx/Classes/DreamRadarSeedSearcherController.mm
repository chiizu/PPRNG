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



#import "DreamRadarSeedSearcherController.h"

#include "DreamRadarSeedSearcher.h"
#include "Utilities.h"

#import "DreamRadarSeedInspectorController.h"

#include <boost/lexical_cast.hpp>

using namespace pprng;

@interface DreamRadarSeedSearchResult :
  NSObject <HashedSeedResultParameters, IVResult, PIDResult>
{
  DreamRadarFrameGenerator::FrameType  type;
  
  Gender::Type   targetGender;
  Gender::Ratio  targetGenderRatio;
  
  uint32_t       slot, numPrecedingGenderless;
  
  DECLARE_HASHED_SEED_RESULT_PARAMETERS_VARIABLES();
  
  uint32_t  frame;
  DECLARE_PID_RESULT_VARIABLES();
  DECLARE_IV_RESULT_VARIABLES();
  Characteristic::Type  characteristic;
}

@property DreamRadarFrameGenerator::FrameType  type;

@property Gender::Type   targetGender;
@property Gender::Ratio  targetGenderRatio;
@property uint32_t       slot, numPrecedingGenderless;

@property uint32_t  frame;

@property Characteristic::Type  characteristic;

@end

@implementation DreamRadarSeedSearchResult

@synthesize type;
@synthesize targetGender, targetGenderRatio;
@synthesize slot, numPrecedingGenderless;

SYNTHESIZE_HASHED_SEED_RESULT_PARAMETERS_PROPERTIES();

@synthesize frame;
SYNTHESIZE_PID_RESULT_PROPERTIES();
SYNTHESIZE_IV_RESULT_PROPERTIES();
@synthesize characteristic;

@end


namespace
{

struct ResultHandler
{
  ResultHandler(SearcherController *c,
                const DreamRadarSeedSearcher::Criteria &criteria)
    : m_controller(c), m_criteria(criteria)
  {}
  
  void operator()(const DreamRadarFrame &frame)
  {
    DreamRadarSeedSearchResult  *result =
      [[DreamRadarSeedSearchResult alloc] init];
    
    result.type = m_criteria.frameParameters.frameType;
    result.targetGender = m_criteria.frameParameters.targetGender;
    result.targetGenderRatio = m_criteria.frameParameters.targetRatio;
    result.slot = m_criteria.frameParameters.slot;
    result.numPrecedingGenderless =
      m_criteria.frameParameters.numPrecedingGenderless;
    
    SetHashedSeedResultParameters(result, frame.seed);
    
    result.frame = frame.number;
    
    SetPIDResult(result, frame.pid,
                 m_criteria.frameParameters.tid,
                 m_criteria.frameParameters.sid,
                 frame.nature, Ability::HIDDEN,
                 m_criteria.frameParameters.targetGender,
                 m_criteria.frameParameters.targetRatio);
    
    SetIVResult(result, frame.ivs, NO);
    
    result.characteristic = Characteristic::Get(frame.pid, frame.ivs);
    
    [m_controller performSelectorOnMainThread: @selector(addResult:)
                  withObject: result
                  waitUntilDone: NO];
  }
  
  SearcherController                      *m_controller;
  const DreamRadarSeedSearcher::Criteria  &m_criteria;
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


@implementation DreamRadarSeedSearcherController

@synthesize fromDate, toDate;
@synthesize noButtonHeld, oneButtonHeld, twoButtonsHeld, threeButtonsHeld;
@synthesize minFrame, maxFrame;

@synthesize type;

@synthesize genderRequired;
@synthesize gender, genderRatio;

@synthesize numPrecedingGenderlessRequired;
@synthesize slot, numPrecedingGenderless, maxNumPrecedingGenderless;

- (NSString *)windowNibName
{
	return @"DreamRadarSeedSearcher";
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
  
  NSDate  *now = [NSDate date];
  self.fromDate = now;
  self.toDate = now;
  
  self.noButtonHeld = YES;
  self.oneButtonHeld = YES;
  self.twoButtonsHeld = NO;
  self.threeButtonsHeld = NO;
  
  self.type = DreamRadarFrameGenerator::NonLegendaryFrame;
  self.genderRequired = YES;
  self.gender = Gender::FEMALE;
  self.genderRatio = Gender::ONE_HALF_FEMALE;
  
  self.slot = 1;
  self.numPrecedingGenderless = 0;
  
  self.minFrame = 4;
  self.maxFrame = 35;
}

- (IBAction)toggleDropDownChoice:(id)sender
{
  HandleComboMenuItemChoice(sender);
}

- (void)windowWillClose:(NSNotification *)notification
{
  if ([searcherController isSearching])
    [searcherController startStop: self];
}

- (void)setType:(DreamRadarFrameGenerator::FrameType)newValue
{
  if (type != newValue)
  {
    type = newValue;
    
    self.genderRequired = (type == DreamRadarFrameGenerator::NonLegendaryFrame);
  }
}

- (void)setGender:(Gender::Type)newValue
{
  if (gender != newValue)
  {
    gender = newValue;
    
    if (gender == Gender::GENDERLESS)
      self.genderRatio = Gender::NO_RATIO;
  }
}

- (void)setSlot:(uint32_t)newValue
{
  if (slot != newValue)
  {
    slot = newValue;
    self.maxNumPrecedingGenderless = slot - 1;
    
    self.numPrecedingGenderlessRequired = (slot > 1);
    if (numPrecedingGenderless >= slot)
      self.numPrecedingGenderless = slot - 1;
  }
}

- (void)setNumPrecedingGenderlessRequired:(BOOL)newValue
{
  if (numPrecedingGenderlessRequired != newValue)
  {
    numPrecedingGenderlessRequired = newValue;
    if (newValue)
    {
      self.numPrecedingGenderless = 0;
    }
  }
}

- (void)inspectSeed:(id)sender
{
  NSInteger  rowNum = [sender clickedRow];
  
  if (rowNum >= 0)
  {
    DreamRadarSeedSearchResult  *row =
      [[[searcherController arrayController] arrangedObjects]
        objectAtIndex: rowNum];
    
    if (row != nil)
    {
      DreamRadarSeedInspectorController  *inspector =
        [[DreamRadarSeedInspectorController alloc] init];
      [inspector window];
      
      [inspector setSeedFromResult: row];
      
      inspector.type = row.type;
      inspector.gender = row.targetGender;
      inspector.genderRatio = row.targetGenderRatio;
      
      inspector.slot = row.slot;
      inspector.numPrecedingGenderless = row.numPrecedingGenderless;
      
      inspector.maxFrame = row.frame + 5;
      
      [inspector generateFrames: self];
      [inspector selectAndShowFrame: row.frame];
      
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
  using namespace boost::gregorian;
  using namespace boost::posix_time;
  
  if (!EndEditing([self window]))
    return nil;
  
  DreamRadarSeedSearcher::Criteria  criteria;
  
  criteria.seedParameters.macAddress = [gen5ConfigController macAddress];
  
  criteria.seedParameters.version = [gen5ConfigController version];
  criteria.seedParameters.dsType = [gen5ConfigController dsType];
  
  criteria.seedParameters.timer0Low = [gen5ConfigController timer0Low];
  criteria.seedParameters.timer0High = [gen5ConfigController timer0High];
  
  criteria.seedParameters.vcountLow = [gen5ConfigController vcountLow];
  criteria.seedParameters.vcountHigh = [gen5ConfigController vcountHigh];
  
  criteria.seedParameters.vframeLow = [gen5ConfigController vframeLow];
  criteria.seedParameters.vframeHigh = [gen5ConfigController vframeHigh];
  
  if (noButtonHeld)
  {
    criteria.seedParameters.heldButtons.push_back(0);  // no buttons
  }
  if (oneButtonHeld)
  {
    criteria.seedParameters.heldButtons.insert
      (criteria.seedParameters.heldButtons.end(),
       Button::SingleButtons().begin(),
       Button::SingleButtons().end());
  }
  if (twoButtonsHeld)
  {
    criteria.seedParameters.heldButtons.insert
      (criteria.seedParameters.heldButtons.end(),
       Button::TwoButtonCombos().begin(),
       Button::TwoButtonCombos().end());
  }
  if (threeButtonsHeld)
  {
    criteria.seedParameters.heldButtons.insert
      (criteria.seedParameters.heldButtons.end(),
       Button::ThreeButtonCombos().begin(),
       Button::ThreeButtonCombos().end());
  }
  
  criteria.seedParameters.fromTime =
    ptime(NSDateToBoostDate(fromDate), seconds(0));
  
  criteria.seedParameters.toTime =
    ptime(NSDateToBoostDate(toDate), hours(23) + minutes(59) + seconds(59));
  
  criteria.frameParameters.frameType = type;
  if (type == DreamRadarFrameGenerator::NonLegendaryFrame)
  {
    criteria.frameParameters.targetGender = gender;
    criteria.frameParameters.targetRatio = genderRatio;
  }
  else
  {
    criteria.frameParameters.targetGender = Gender::MALE;
    criteria.frameParameters.targetRatio = Gender::MALE_ONLY;
  }
  criteria.frameParameters.slot = slot;
  criteria.frameParameters.numPrecedingGenderless = numPrecedingGenderless;
  criteria.frameParameters.tid = [gen5ConfigController tid];
  criteria.frameParameters.sid = [gen5ConfigController sid];
  criteria.frameParameters.memoryLinkUsed =
    [gen5ConfigController memoryLinkUsed];
  
  criteria.ivs.min = ivParameterController.minIVs;
  criteria.ivs.max = ivParameterController.maxIVs;
  criteria.ivs.shouldCheckMax =
    (criteria.ivs.max != IVs(31, 31, 31, 31, 31, 31));
  
  if (ivParameterController.considerHiddenPower)
  {
    criteria.ivs.hiddenType = ivParameterController.hiddenType;
    criteria.ivs.minHiddenPower = ivParameterController.minHiddenPower;
  }
  else
  {
    criteria.ivs.hiddenType = Element::NONE;
  }
  criteria.pid.natureMask = GetComboMenuBitMask(natureDropDown);
  
  criteria.frame.min = minFrame;
  criteria.frame.max = maxFrame;
  
  if (CheckExpectedResults(criteria, 10000,
                           @"The current search parameters are expected to return more than 10,000 results. Please set more specific IVs, limit the date range, use fewer held keys, or other similar settings to reduce the number of expected results.",
                           self,
                           @selector(alertDidEnd:returnCode:contextInfo:)))
  {
    return [NSValue valueWithPointer:
            new DreamRadarSeedSearcher::Criteria(criteria)];
  }
  else
  {
    return nil;
  }
}

- (void)doSearchWithCriteria:(NSValue*)criteriaPtr
{
  std::auto_ptr<DreamRadarSeedSearcher::Criteria>
    criteria(static_cast<DreamRadarSeedSearcher::Criteria*>
      ([criteriaPtr pointerValue]));
  
  DreamRadarSeedSearcher  searcher;
  
  searcher.Search
    (*criteria,
     ResultHandler(searcherController, *criteria),
     ProgressHandler(searcherController));
}


@end
