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



#import "WonderCardSeedSearcherController.h"

#include "WonderCardSeedSearcher.h"
#include "Utilities.h"

#import "WonderCardSeedInspectorController.h"

#include <boost/lexical_cast.hpp>

using namespace pprng;

@interface WonderCardSeedSearchResult :
  NSObject <HashedSeedResultParameters, IVResult, PIDResult>
{
  pprng::Nature::Type               cardNature;
  pprng::Ability::Type              cardAbility;
  pprng::Gender::Type               cardGender;
  pprng::Gender::Ratio              cardGenderRatio;
  pprng::WonderCardShininess::Type  cardShininess;
  NSNumber                          *cardTID, *cardSID;
  
  DECLARE_HASHED_SEED_RESULT_PARAMETERS_VARIABLES();
  
  uint32_t  startFrame, frame;
  DECLARE_PID_RESULT_VARIABLES();
  
  DECLARE_IV_RESULT_VARIABLES();
}

@property pprng::Nature::Type               cardNature;
@property pprng::Ability::Type              cardAbility;
@property pprng::Gender::Type               cardGender;
@property pprng::Gender::Ratio              cardGenderRatio;
@property pprng::WonderCardShininess::Type  cardShininess;
@property (copy) NSNumber                   *cardTID, *cardSID;

@property uint32_t  startFrame, frame;

@end

@implementation WonderCardSeedSearchResult

@synthesize cardNature, cardAbility;
@synthesize cardGender, cardGenderRatio;
@synthesize cardShininess, cardTID, cardSID;

SYNTHESIZE_HASHED_SEED_RESULT_PARAMETERS_PROPERTIES();

@synthesize startFrame, frame;
SYNTHESIZE_PID_RESULT_PROPERTIES();

SYNTHESIZE_IV_RESULT_PROPERTIES();

@end


namespace
{

struct ResultHandler
{
  ResultHandler(SearcherController *c,
                const WonderCardSeedSearcher::Criteria &criteria,
                NSNumber *cardTID, NSNumber *cardSID)
    : m_controller(c), m_criteria(criteria),
      m_cardTID(cardTID), m_cardSID(cardSID)
  {}
  
  void operator()(const WonderCardFrame &frame)
  {
    WonderCardSeedSearchResult  *result =
      [[WonderCardSeedSearchResult alloc] init];
    
    result.cardNature = m_criteria.frameParameters.cardNature;
    result.cardAbility = m_criteria.frameParameters.cardAbility;
    result.cardGender = m_criteria.frameParameters.cardGender;
    result.cardGenderRatio = m_criteria.frameParameters.cardGenderRatio;
    result.cardShininess = m_criteria.frameParameters.cardShininess;
    result.cardTID = m_cardTID;
    result.cardSID = m_cardSID;
    
    SetHashedSeedResultParameters(result, frame.seed);
    
    result.startFrame = frame.seed.GetSkippedPIDFrames() + 1;
    result.frame = frame.number;
    
    SetGen5PIDResult(result, frame.nature, frame.pid,
                     m_criteria.frameParameters.cardTID,
                     m_criteria.frameParameters.cardSID,
                     m_criteria.frameParameters.cardGender,
                     m_criteria.frameParameters.cardGenderRatio);
    if (frame.hasHiddenAbility)
      result.ability = Ability::HIDDEN;
    
    SetIVResult(result, frame.ivs, NO);
    
    [m_controller performSelectorOnMainThread: @selector(addResult:)
                  withObject: result
                  waitUntilDone: NO];
  }
  
  SearcherController                      *m_controller;
  const WonderCardSeedSearcher::Criteria  &m_criteria;
  NSNumber                                *m_cardTID, *m_cardSID;
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

@synthesize fromDate, toDate;
@synthesize noButtonHeld, oneButtonHeld, twoButtonsHeld, threeButtonsHeld;

@synthesize cardNature, cardAbility;
@synthesize cardGender, cardGenderRatio;
@synthesize cardShininess, cardTID, cardSID;

@synthesize natureSearchable, abilitySearchable;
@synthesize shininessSearchable, genderSearchable;

@synthesize showShinyOnly;
@synthesize ability, gender;

@synthesize startFromInitialFrame;
@synthesize minFrame, maxFrame;

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
  
  [[searcherController tableView] setTarget: self];
  [[searcherController tableView] setDoubleAction: @selector(inspectSeed:)];
  
  NSDate  *now = [NSDate date];
  self.fromDate = now;
  self.toDate = now;
  
  self.noButtonHeld = YES;
  self.oneButtonHeld = YES;
  self.twoButtonsHeld = NO;
  self.threeButtonsHeld = NO;
  
  self.cardNature = Nature::ANY;
  self.cardAbility = Ability::ANY;
  self.cardGender = Gender::ANY;
  self.cardGenderRatio = Gender::ANY_RATIO;
  self.cardShininess = WonderCardShininess::NEVER_SHINY;
  self.cardTID = nil;
  self.cardSID = [NSNumber numberWithUnsignedInt: 0];
  
  self.showShinyOnly = NO;
  self.ability = Ability::ANY;
  self.gender = Gender::ANY;
  
  self.startFromInitialFrame = YES;
  self.minFrame = 50;
  self.maxFrame = 200;
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

- (void)setCardNature:(Nature::Type)newValue
{
  if (cardNature != newValue)
  {
    cardNature = newValue;
    
    self.natureSearchable = (newValue == Nature::ANY);
  }
}

- (void)setCardAbility:(Ability::Type)newValue
{
  if (cardAbility != newValue)
  {
    cardAbility = newValue;
    
    self.abilitySearchable = (newValue == Ability::ANY);
    if (!abilitySearchable)
    {
      self.ability = newValue;
    }
  }
}

- (void)setCardShininess:(WonderCardShininess::Type)newValue
{
  if (cardShininess != newValue)
  {
    cardShininess = newValue;
    
    self.shininessSearchable = (newValue == 1);
    if (shininessSearchable)
    {
      self.showShinyOnly = YES;
    }
    else
    {
      self.showShinyOnly = (newValue == 2);
    }
  }
}

- (void)setCardGender:(Gender::Type)newValue
{
  if (cardGender != newValue)
  {
    cardGender = newValue;
    
    self.genderSearchable = (newValue == Gender::ANY);
    if (!genderSearchable)
    {
      self.gender = newValue;
    }
    
    if (cardGender == Gender::GENDERLESS)
      self.cardGenderRatio = Gender::ANY_RATIO;
  }
}

- (void)inspectSeed:(id)sender
{
  NSInteger  rowNum = [sender clickedRow];
  
  if (rowNum >= 0)
  {
    WonderCardSeedSearchResult  *row =
      [[[searcherController arrayController] arrangedObjects]
        objectAtIndex: rowNum];
    
    if (row != nil)
    {
      WonderCardSeedInspectorController  *inspector =
        [[WonderCardSeedInspectorController alloc] init];
      [inspector window];
      
      [inspector setSeedFromResult: row];
      
      inspector.cardNature = row.cardNature;
      inspector.cardAbility = row.cardAbility;
      inspector.cardGender = row.cardGender;
      inspector.cardGenderRatio = row.cardGenderRatio;
      inspector.cardShininess = row.cardShininess;
      inspector.cardTID = row.cardTID;
      inspector.cardSID = row.cardSID;
      
      if (row.frame < row.startFrame)
      {
        inspector.startFromInitialFrame = NO;
        inspector.minFrame = 1;
        
        inspector.matchOffsetFromInitialFrame = NO;
      }
      inspector.maxFrame = row.frame + 20;
      inspector.targetFrame = row.frame;
      
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
  
  criteria.frameParameters.startFromLowestFrame = startFromInitialFrame;
  criteria.frameParameters.cardNature = cardNature;
  criteria.frameParameters.cardAbility = cardAbility;
  criteria.frameParameters.cardGender = Gender::Type(cardGender);
  criteria.frameParameters.cardGenderRatio = Gender::Ratio(cardGenderRatio);
  criteria.frameParameters.cardShininess = cardShininess;
  criteria.frameParameters.cardTID = [cardTID unsignedIntValue];
  criteria.frameParameters.cardSID = [cardSID unsignedIntValue];
  
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
  criteria.pid.ability = ability;
  criteria.pid.gender = gender;
  criteria.pid.genderRatio = cardGenderRatio;
  criteria.shinyOnly = showShinyOnly;
  
  criteria.frame.min = minFrame;
  criteria.frame.max = maxFrame;
  
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
  
  searcher.Search
    (*criteria,
     ResultHandler(searcherController, *criteria, cardTID, cardSID),
     ProgressHandler(searcherController));
}


@end
