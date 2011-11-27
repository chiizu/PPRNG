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


#import "DSParameterSearcherController.h"
#include "InitialSeedSearcher.h"
#include "LinearCongruentialRNG.h"
#include "Utilities.h"

using namespace pprng;

namespace
{

struct ResultHandler
{
  ResultHandler(SearcherController *c)
    : controller(c)
  {}
  
  void operator()(const HashedIVFrame &frame)
  {
    NSMutableDictionary  *row =
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithUnsignedLongLong: frame.seed.rawSeed], @"seed",
        [NSString stringWithFormat: @"%.2d:%.2d:%.2d",
          frame.seed.hour, frame.seed.minute, frame.seed.second], @"time",
				[NSNumber numberWithUnsignedInt: frame.seed.timer0], @"timer0",
				[NSNumber numberWithUnsignedInt: frame.seed.vcount], @"vcount",
				[NSNumber numberWithUnsignedInt: frame.seed.vframe], @"vframe",
        [NSData dataWithBytes: &frame.seed length: sizeof(HashedSeed)],
          @"fullSeed",
        nil];
    
    [controller performSelectorOnMainThread: @selector(addResult:)
                withObject: row
                waitUntilDone: NO];
  }
  
  SearcherController  *controller;
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

@synthesize useStandardParameterRanges;

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
  
  [startDate setObjectValue: [NSDate date]];
  
  [[[[[searcherController tableView] tableColumnWithIdentifier: @"seed"]
    dataCell] formatter]
   setFormatWidth: 16];
  
  self.useStandardParameterRanges = YES;
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
  
  InitialIVSeedSearcher::Criteria  criteria;
  
  criteria.seedParameters.macAddress.low = [gen5ConfigController macAddressLow];
  criteria.seedParameters.macAddress.high = [gen5ConfigController macAddressHigh];
  
  criteria.seedParameters.version = [gen5ConfigController version];
  criteria.seedParameters.dsType = [gen5ConfigController dsType];
  
  if (useStandardParameterRanges)
  {
    if ((criteria.seedParameters.dsType == DS::DSPhat) ||
        (criteria.seedParameters.dsType == DS::DSLite))
    {
      criteria.seedParameters.timer0Low = 0xC00;
      criteria.seedParameters.timer0High = 0xCFF;
      
      criteria.seedParameters.vcountLow = 0x50;
      criteria.seedParameters.vcountHigh = 0x6F;
      
      criteria.seedParameters.vframeLow = 0x0;
      criteria.seedParameters.vframeHigh = 0xF;
    }
    else
    {
      criteria.seedParameters.timer0Low = 0x1200;
      criteria.seedParameters.timer0High = 0x1300;
      
      criteria.seedParameters.vcountLow = 0x80;
      criteria.seedParameters.vcountHigh = 0x9F;
      
      criteria.seedParameters.vframeLow = 0x0;
      criteria.seedParameters.vframeHigh = 0xF;
    }
  }
  else
  {
    criteria.seedParameters.timer0Low = [timer0LowField intValue];
    criteria.seedParameters.timer0High = [timer0HighField intValue];
    
    criteria.seedParameters.vcountLow = [vcountLowField intValue];
    criteria.seedParameters.vcountHigh = [vcountHighField intValue];
    
    criteria.seedParameters.vframeLow = [frameLowField intValue];
    criteria.seedParameters.vframeHigh = [frameHighField intValue];
  }
  
  ptime  startTime = ptime(NSDateToBoostDate([startDate objectValue]),
                           hours([startHour intValue]) +
                           minutes([startMinute intValue]) +
                           seconds([startSecond intValue]));

  criteria.seedParameters.fromTime = startTime - seconds(5);
  criteria.seedParameters.toTime = startTime + seconds(10);
  
  criteria.seedParameters.heldButtons.push_back
    ([[keyOnePopUp selectedItem] tag] |
     [[keyTwoPopUp selectedItem] tag] |
     [[keyThreePopUp selectedItem] tag]);
  
  criteria.minIVs = [ivParameterController minIVs];
  criteria.maxIVs = [ivParameterController maxIVs];
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
  
  searcher.Search(*criteria, ResultHandler(searcherController),
                  ProgressHandler(searcherController));
}

@end
