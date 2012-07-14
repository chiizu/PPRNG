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


#import "DSParameterSearcherController.h"

#import "SearchResultProtocols.h"

#include "InitialSeedSearcher.h"
#include "LinearCongruentialRNG.h"
#include "Utilities.h"

using namespace pprng;

@interface DSParameterSearchResult :
  NSObject <HashedSeedResultParameters, IVResult>
{
  DECLARE_HASHED_SEED_RESULT_PARAMETERS_VARIABLES();
  
  uint32_t       ivFrame;
  DECLARE_IV_RESULT_VARIABLES();
}

@property uint32_t       ivFrame;

@end

@implementation DSParameterSearchResult

SYNTHESIZE_HASHED_SEED_RESULT_PARAMETERS_PROPERTIES();

@synthesize ivFrame;
SYNTHESIZE_IV_RESULT_PROPERTIES();

@end

namespace
{

struct ResultHandler
{
  ResultHandler(SearcherController *c, bool isRoamer)
    : m_controller(c), m_isRoamer(isRoamer)
  {}
  
  void operator()(const HashedIVFrame &frame)
  {
    DSParameterSearchResult  *row = [[DSParameterSearchResult alloc] init];
    
    SetHashedSeedResultParameters(row, frame.seed);
    row.ivFrame = frame.number;
    SetIVResult(row, frame.ivs, m_isRoamer);
    
    [m_controller performSelectorOnMainThread: @selector(addResult:)
                  withObject: row
                  waitUntilDone: NO];
  }
  
  SearcherController  *m_controller;
  const bool          m_isRoamer;
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

@implementation DSParameterSearcherController

@synthesize timer0Low, timer0High, vcountLow, vcountHigh, vframeLow, vframeHigh;
@synthesize startDate, startHour, startMinute, startSecond;
@synthesize button1, button2, button3;

- (NSString *)windowNibName
{
	return @"DSParameterSearcher";
}

- (void)awakeFromNib
{
  [super awakeFromNib];
  
  [searcherController setGetValidatedSearchCriteriaSelector:
                      @selector(getValidatedSearchCriteria)];
  [searcherController setDoSearchWithCriteriaSelector:
                      @selector(doSearchWithCriteria:)];
  
  DS::Type       dsType = [gen5ConfigController dsType];
  Game::Version  version = [gen5ConfigController version];
  if ((dsType == DS::DSPhat) || (dsType == DS::DSLite))
  {
    if (Game::IsBlack2White2(version))
    {
      self.timer0Low = 0x1000;
      self.timer0High = 0x11FF;
      self.vcountLow = 0x78;
      self.vcountHigh = 0x97;
    }
    else
    {
      self.timer0Low = 0xC00;
      self.timer0High = 0xCFF;
      self.vcountLow = 0x50;
      self.vcountHigh = 0x6F;
    }
  }
  else
  {
    if (version == Game::Black2Japanese)
    {
      self.timer0Low = 0x1480;
      self.timer0High = 0x167F;
      self.vcountLow = 0x98;
      self.vcountHigh = 0xB7;
    }
    else if (version == Game::White2Japanese)
    {
      self.timer0Low = 0x1780;
      self.timer0High = 0x197F;
      self.vcountLow = 0xA8;
      self.vcountHigh = 0xC7;
    }
    else
    {
      self.timer0Low = 0x1100;
      self.timer0High = 0x12FF;
      self.vcountLow = 0x78;
      self.vcountHigh = 0x97;
    }
  }
  self.vframeLow = 0x0;
  self.vframeHigh = 0xF;
  
  self.startDate = [NSDate date];
  self.startHour = 0;
  self.startMinute = 0;
  self.startSecond = 0;
  self.button1 = 0;
  self.button2 = 0;
  self.button3 = 0;
}

- (void)windowWillClose:(NSNotification *)notification
{
  if ([searcherController isSearching])
    [searcherController startStop: self];
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
  
  InitialIVSeedSearcher::Criteria  criteria;
  
  criteria.seedParameters.macAddress.low = [gen5ConfigController macAddressLow];
  criteria.seedParameters.macAddress.high = [gen5ConfigController macAddressHigh];
  
  criteria.seedParameters.version = [gen5ConfigController version];
  criteria.seedParameters.dsType = [gen5ConfigController dsType];
  
  criteria.seedParameters.timer0Low = timer0Low;
  criteria.seedParameters.timer0High = timer0High;
  
  criteria.seedParameters.vcountLow = vcountLow;
  criteria.seedParameters.vcountHigh = vcountHigh;
  
  criteria.seedParameters.vframeLow = vframeLow;
  criteria.seedParameters.vframeHigh = vframeHigh;
  
  ptime  startTime = ptime(NSDateToBoostDate(startDate),
                           hours(startHour) + minutes(startMinute) +
                           seconds(startSecond));

  criteria.seedParameters.fromTime = startTime - seconds(5);
  criteria.seedParameters.toTime = startTime + seconds(10);
  
  criteria.seedParameters.heldButtons.push_back(button1 | button2 | button3);
  
  criteria.minIVs = ivParameterController.minIVs;
  criteria.maxIVs = ivParameterController.maxIVs;
  criteria.isRoamer = ivParameterController.isRoamer;
  criteria.maxSkippedFrames = 50;
  
  if (criteria.ExpectedNumberOfResults() > 1000)
  {
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Please Limit Search Parameters"];
    [alert setInformativeText:@"The current search parameters are expected to return more than 1000 results. Please catch a higher level Pokémon, use rare candies to determine more specific IVs, or limit the search ranges of the various DS parameters in order to reduce the number of expected results."];
    [alert setAlertStyle:NSWarningAlertStyle];
    
    [alert beginSheetModalForWindow:[self window] modalDelegate:self
           didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
           contextInfo:nil];
    
    return nil;
  }
  else
  {
    return [NSValue
             valueWithPointer: new InitialIVSeedSearcher::Criteria(criteria)];
  }
}

- (void)doSearchWithCriteria:(NSValue*)criteriaPtr
{
  std::auto_ptr<InitialIVSeedSearcher::Criteria> 
    criteria(static_cast<InitialIVSeedSearcher::Criteria*>
               ([criteriaPtr pointerValue]));
  
  InitialIVSeedSearcher  searcher;
  
  searcher.Search(*criteria,
                  ResultHandler(searcherController, criteria->isRoamer),
                  ProgressHandler(searcherController));
}

@end
