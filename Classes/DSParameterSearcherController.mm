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
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/lexical_cast.hpp>

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
        [NSNumber numberWithUnsignedLongLong: frame.seed.m_rawSeed], @"seed",
        [NSString stringWithFormat: @"%.2d:%.2d:%.2d",
          frame.seed.m_hour, frame.seed.m_minute, frame.seed.m_second], @"time",
				[NSNumber numberWithUnsignedInt: frame.seed.m_timer0], @"timer0",
				[NSNumber numberWithUnsignedInt: frame.seed.m_vcount], @"vcount",
				[NSNumber numberWithUnsignedInt: frame.seed.m_vframe], @"vframe",
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

- (NSString *)windowNibName
{
	return @"DSParameterSearcher";
}

- (void)awakeFromNib
{
  [super awakeFromNib];
  
  [[[[[searcherController tableView] tableColumnWithIdentifier: @"seed"]
    dataCell] formatter]
   setFormatWidth: 16];
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
  
  criteria.macAddressLow = [gen5ConfigController macAddressLow];
  criteria.macAddressHigh = [gen5ConfigController macAddressHigh];
  
  criteria.version =
    static_cast<Game::Version>([[versionMenu selectedItem] tag]);
  
  criteria.timer0Low = [timer0LowField intValue];
  criteria.timer0High = [timer0HighField intValue];
  
  criteria.vcountLow = [vcountLowField intValue];
  criteria.vcountHigh = [vcountHighField intValue];
  
  criteria.vframeLow = [frameLowField intValue];
  criteria.vframeHigh = [frameHighField intValue];
  
  criteria.minIVs = [ivParameterController minIVs];
  criteria.maxIVs = [ivParameterController maxIVs];
  criteria.maxSkippedFrames = 50;
  
  const char *dstr = [[[startDate objectValue] description] UTF8String];
  criteria.startTime =
    ptime(date(boost::lexical_cast<uint32_t>(std::string(dstr, 4)),
               boost::lexical_cast<uint32_t>(std::string(dstr + 5, 2)),
               boost::lexical_cast<uint32_t>(std::string(dstr + 8, 2))),
          hours([startHour intValue]) +
          minutes([startMinute intValue]) +
          seconds([startSecond intValue]));
  
  if (criteria.ExpectedNumberOfResults() > 100)
  {
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Please Limit Search Parameters"];
    [alert setInformativeText:@"The current search parameters are expected to return more than 100 results. Please catch a higher level Pokémon, use rare candies to determine more specific IVs, or limit the search ranges of the various DS parameters in order to reduce the number of expected results."];
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
