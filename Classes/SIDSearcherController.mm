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


#import "SIDSearcherController.h"

#include "SIDSearcher.h"
#include <boost/lexical_cast.hpp>


using namespace pprng;

namespace
{

struct ResultHandler
{
  ResultHandler(SearcherController *c)
    : controller(c)
  {}
  
  void operator()(const Gen5TrainerIDFrame &frame)
  {
    NSMutableDictionary  *result =
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithUnsignedLongLong: frame.seed.m_rawSeed], @"seed",
        [NSString stringWithFormat: @"%.2d:%.2d:%.2d",
          frame.seed.m_hour, frame.seed.m_minute, frame.seed.m_second], @"time",
				[NSNumber numberWithUnsignedInt: frame.number], @"frame",
        [NSNumber numberWithUnsignedInt: frame.sid], @"sid",
        [NSNumber numberWithUnsignedInt: frame.seed.m_timer0], @"timer0",
        nil];
    
    [controller performSelectorOnMainThread: @selector(addResult:)
                withObject: result
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


@implementation SIDSearcherController

- (NSString *)windowNibName
{
	return @"SIDSearcher";
}

- (void)awakeFromNib
{
  [super awakeFromNib];
  
  [[[[[searcherController tableView] tableColumnWithIdentifier: @"seed"]
    dataCell] formatter]
   setFormatWidth: 16];
}

- (IBAction) toggleTime:(id)sender
{
  BOOL  hourEnabled = [enableHourButton state];
  BOOL  minuteEnabled = [enableMinuteButton state];
  BOOL  secondEnabled = [enableSecondButton state];
  
  [startHour setEnabled: hourEnabled];
  [hourStepper setEnabled: hourEnabled];
  [enableMinuteButton setEnabled: hourEnabled];
  [startMinute setEnabled: hourEnabled && minuteEnabled];
  [minuteStepper setEnabled: hourEnabled && minuteEnabled];
  [enableSecondButton setEnabled: hourEnabled && minuteEnabled];
  [startSecond setEnabled: hourEnabled && minuteEnabled && secondEnabled];
  [secondStepper setEnabled: hourEnabled && minuteEnabled && secondEnabled];
}

// dummy method for error panel
- (void)alertDidEnd:(NSAlert *)alert returnCode:(NSInteger)returnCode
        contextInfo:(void *)contextInfo
{}

- (NSValue*)getValidatedSearchCriteria
{
  using namespace boost::gregorian;
  using namespace boost::posix_time;

  SIDSearcher::Criteria  criteria;
  
  criteria.macAddressLow = [gen5ConfigController macAddressLow];
  criteria.macAddressHigh = [gen5ConfigController macAddressHigh];
  
  criteria.version = [gen5ConfigController version];
  
  criteria.timer0Low = [gen5ConfigController timer0Low];
  criteria.timer0High = [gen5ConfigController timer0High];
  
  criteria.vcountLow = [gen5ConfigController vcountLow];
  criteria.vcountHigh = [gen5ConfigController vcountHigh];
  
  criteria.vframeLow = [gen5ConfigController vframeLow];
  criteria.vframeHigh = [gen5ConfigController vframeHigh];
  
  const char *dstr = [[[dateField objectValue] description] UTF8String];
  criteria.startDate =
    date(boost::lexical_cast<uint32_t>(std::string(dstr, 4)),
         boost::lexical_cast<uint32_t>(std::string(dstr + 5, 2)),
         boost::lexical_cast<uint32_t>(std::string(dstr + 8, 2)));
  
  if ([enableHourButton state])
  {
    criteria.startTime = hours([startHour intValue]);
    if ([enableMinuteButton isEnabled] && [enableMinuteButton state])
    {
      criteria.startTime = criteria.startTime + minutes([startMinute intValue]);
      if ([enableSecondButton isEnabled] && [enableSecondButton state])
      {
        criteria.startTime = criteria.startTime
          + seconds([startSecond intValue]);
        
        criteria.endTime = criteria.startTime + seconds(5);
        criteria.startTime = criteria.startTime - seconds(5);
      }
      else
      {
        criteria.endTime = criteria.startTime + seconds(59);
      }
    }
    else
    {
      criteria.endTime = criteria.startTime + minutes(59) + seconds(59);
    }
  }
  else
  {
    criteria.startTime = seconds(0);
    criteria.endTime = hours(23) + minutes(59) + seconds(59);
  }
  
  criteria.minFrame = [minFrameField intValue];
  criteria.maxFrame = [maxFrameField intValue];
  
  criteria.tid = [gen5ConfigController tid];
  
  if (criteria.ExpectedNumberOfResults() > 100)
  {
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Please Limit Search Parameters"];
    [alert setInformativeText:@"The current search parameters are expected to return more than 100 results. Please limit the frame range of the search in order to reduce the number of expected results."];
    [alert setAlertStyle:NSWarningAlertStyle];
    
    [alert beginSheetModalForWindow:[self window] modalDelegate:self
           didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
           contextInfo:nil];
    
    return nil;
  }
  else
  {
    return [NSValue valueWithPointer: new SIDSearcher::Criteria(criteria)];
  }
}

- (void)doSearchWithCriteria:(NSValue*)criteriaPtr
{
  std::auto_ptr<SIDSearcher::Criteria> 
    criteria(static_cast<SIDSearcher::Criteria*>([criteriaPtr pointerValue]));
  
  SIDSearcher  searcher;
  
  searcher.Search(*criteria,
                  ResultHandler(searcherController),
                  ProgressHandler(searcherController));
}


@end
