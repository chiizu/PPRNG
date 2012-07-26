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



#import "HiddenHollowSpawnSeedSearcherController.h"

#include "HiddenHollowSpawnSeedSearcher.h"
#include "Utilities.h"

#import "SearchResultProtocols.h"

#include <boost/lexical_cast.hpp>

using namespace pprng;

@interface HiddenHollowSpawnSeedSearchResult :
  NSObject <HashedSeedResultParameters>
{
  DECLARE_HASHED_SEED_RESULT_PARAMETERS_VARIABLES();
  
  uint32_t      group, slot;
  Gender::Type  gender5, gender10, gender30, gender60;
}

@property uint32_t      group, slot;
@property Gender::Type  gender5, gender10, gender30, gender60;

@end

@implementation HiddenHollowSpawnSeedSearchResult

SYNTHESIZE_HASHED_SEED_RESULT_PARAMETERS_PROPERTIES();

@synthesize group, slot;
@synthesize gender5, gender10, gender30, gender60;

@end


namespace
{

struct ResultHandler
{
  ResultHandler(SearcherController *c)
    : m_controller(c)
  {}
  
  void operator()(const HiddenHollowSpawnFrame &frame)
  {
    HiddenHollowSpawnSeedSearchResult  *result =
      [[HiddenHollowSpawnSeedSearchResult alloc] init];
    
    SetHashedSeedResultParameters(result, frame.seed);
    
    result.group = frame.group;
    result.slot = frame.slot;
    result.gender5 =
      (frame.genderPercentage < 5) ? Gender::FEMALE : Gender::MALE;
    result.gender10 =
      (frame.genderPercentage < 10) ? Gender::FEMALE : Gender::MALE;
    result.gender30 =
      (frame.genderPercentage < 30) ? Gender::FEMALE : Gender::MALE;
    result.gender60 =
      (frame.genderPercentage < 60) ? Gender::FEMALE : Gender::MALE;
    
    [m_controller performSelectorOnMainThread: @selector(addResult:)
                  withObject: result
                  waitUntilDone: NO];
  }
  
  SearcherController  *m_controller;
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


@implementation HiddenHollowSpawnSeedSearcherController

@synthesize searchDate;

@synthesize group, slot;

@synthesize genderRequired;
@synthesize gender, femalePercentage;

- (NSString *)windowNibName
{
	return @"HiddenHollowSpawnSeedSearcher";
}

- (void)awakeFromNib
{
  [super awakeFromNib];
  
  [searcherController setGetValidatedSearchCriteriaSelector:
                      @selector(getValidatedSearchCriteria)];
  [searcherController setDoSearchWithCriteriaSelector:
                      @selector(doSearchWithCriteria:)];
  
  //[[searcherController tableView] setTarget: self];
  //[[searcherController tableView] setDoubleAction: @selector(inspectSeed:)];
  
  self.searchDate = [NSDate date];
  
  self.group = 0;
  self.slot = 0;
  self.genderRequired = YES;
  self.gender = Gender::FEMALE;
  self.femalePercentage = 30;
}

- (void)windowWillClose:(NSNotification *)notification
{
  if ([searcherController isSearching])
    [searcherController startStop: self];
}

- (void)setGender:(Gender::Type)newValue
{
  if (gender != newValue)
  {
    gender = newValue;
    
    if (gender == Gender::GENDERLESS)
      self.femalePercentage = 0;
  }
}

- (void)setSlot:(uint32_t)newValue
{
  if (slot != newValue)
  {
    slot = newValue;
    
    self.genderRequired = (newValue < 3);
  }
}

/*
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
      
      [inspector generateFrames: self];
      [inspector selectAndShowFrame: row.frame];
      
      [inspector showWindow: self];
    }
  }
}
*/

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
  
  HiddenHollowSpawnSeedSearcher::Criteria  criteria;
  
  criteria.seedParameters.macAddress = [gen5ConfigController macAddress];
  
  criteria.seedParameters.version = [gen5ConfigController version];
  criteria.seedParameters.dsType = [gen5ConfigController dsType];
  
  criteria.seedParameters.timer0Low = [gen5ConfigController timer0Low];
  criteria.seedParameters.timer0High = [gen5ConfigController timer0High];
  
  criteria.seedParameters.vcountLow = [gen5ConfigController vcountLow];
  criteria.seedParameters.vcountHigh = [gen5ConfigController vcountHigh];
  
  criteria.seedParameters.vframeLow = [gen5ConfigController vframeLow];
  criteria.seedParameters.vframeHigh = [gen5ConfigController vframeHigh];
  
  criteria.seedParameters.heldButtons.push_back(0);  // no buttons
  
  criteria.seedParameters.fromTime =
    ptime(NSDateToBoostDate(searchDate), seconds(0));
  
  criteria.seedParameters.toTime =
    ptime(NSDateToBoostDate(searchDate), hours(23) + minutes(59) + seconds(59));
  
  criteria.frame.min = 1;
  criteria.frame.max = 1;
  
  criteria.group = group;
  criteria.slot = slot;
  criteria.gender = gender;
  criteria.femalePercentage = femalePercentage;
  criteria.memoryLinkUsed = [gen5ConfigController memoryLinkUsed];
  
  if (CheckExpectedResults(criteria, 10000,
                           @"The current search parameters are expected to return more than 10,000 results. Please limit the date range to reduce the number of expected results.",
                           self,
                           @selector(alertDidEnd:returnCode:contextInfo:)))
  {
    return [NSValue valueWithPointer:
            new HiddenHollowSpawnSeedSearcher::Criteria(criteria)];
  }
  else
  {
    return nil;
  }
}

- (void)doSearchWithCriteria:(NSValue*)criteriaPtr
{
  std::auto_ptr<HiddenHollowSpawnSeedSearcher::Criteria>
    criteria(static_cast<HiddenHollowSpawnSeedSearcher::Criteria*>
      ([criteriaPtr pointerValue]));
  
  HiddenHollowSpawnSeedSearcher  searcher;
  
  searcher.Search
    (*criteria,
     ResultHandler(searcherController),
     ProgressHandler(searcherController));
}


@end
