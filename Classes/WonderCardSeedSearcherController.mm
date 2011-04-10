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



#import "WonderCardSeedSearcherController.h"

#include "WonderCardSeedSearcher.h"

#import "WonderCardSeedInspectorController.h"

#include <boost/lexical_cast.hpp>

using namespace pprng;

namespace
{

struct ResultHandler
{
  ResultHandler(SearcherController *c)
    : controller(c)
  {}
  
  void operator()(const WonderCardFrame &frame)
  {
    NSMutableDictionary  *result =
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithUnsignedLongLong: frame.seed.m_rawSeed], @"seed",
        [NSString stringWithFormat: @"%.4d/%.2d/%.2d",
          frame.seed.m_year, frame.seed.m_month, frame.seed.m_day], @"date",
        [NSString stringWithFormat: @"%.2d:%.2d:%.2d",
          frame.seed.m_hour, frame.seed.m_minute, frame.seed.m_second], @"time",
        [NSNumber numberWithUnsignedInt: frame.seed.m_timer0], @"timer0",
				[NSString stringWithFormat: @"%s",
          Button::ToString(frame.seed.m_keyInput).c_str()], @"keys",
				[NSNumber numberWithUnsignedInt: frame.number], @"frame",
        [NSNumber numberWithUnsignedInt: frame.pid.word], @"pid",
        [NSString stringWithFormat: @"%s",
          Nature::ToString(frame.nature).c_str()], @"nature",
        [NSNumber numberWithUnsignedInt: frame.pid.Gen5Ability()], @"ability",
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
        [NSNumber numberWithUnsignedInt: frame.seed.m_vcount], @"vcount",
        [NSNumber numberWithUnsignedInt: frame.seed.m_vframe], @"vframe",
        [NSData dataWithBytes: &frame.seed length: sizeof(HashedSeed)],
          @"fullSeed",
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


@implementation WonderCardSeedSearcherController

- (NSString *)windowNibName
{
	return @"WonderCardSeedSearcher";
}

- (void)awakeFromNib
{
  [super awakeFromNib];
  
  [[[[[searcherController tableView] tableColumnWithIdentifier: @"seed"]
    dataCell] formatter]
   setFormatWidth: 16];
  [[[[[searcherController tableView] tableColumnWithIdentifier: @"pid"]
    dataCell] formatter]
   setFormatWidth: 8];
  
  [[searcherController tableView] setDoubleAction: @selector(inspectSeed:)];
  
  NSDate  *now = [NSDate date];
  [fromDateField setObjectValue: now];
  [toDateField setObjectValue: now];
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
      
      WonderCardSeedInspectorController  *inspector =
        [[WonderCardSeedInspectorController alloc] init];
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
  
  WonderCardSeedSearcher::Criteria  criteria;
  
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
  
  const char *dstr = [[[fromDateField objectValue] description] UTF8String];
  criteria.fromTime =
    ptime(date(boost::lexical_cast<uint32_t>(std::string(dstr, 4)),
               boost::lexical_cast<uint32_t>(std::string(dstr + 5, 2)),
               boost::lexical_cast<uint32_t>(std::string(dstr + 8, 2))),
          seconds(0));
  
  dstr = [[[toDateField objectValue] description] UTF8String];
  criteria.toTime =
    ptime(date(boost::lexical_cast<uint32_t>(std::string(dstr, 4)),
               boost::lexical_cast<uint32_t>(std::string(dstr + 5, 2)),
               boost::lexical_cast<uint32_t>(std::string(dstr + 8, 2))),
          hours(23) + minutes(59) + seconds(59));
  
  criteria.minFrame = [minFrameField intValue];
  criteria.maxFrame = [maxFrameField intValue];
  
  criteria.minIVs = [ivParameterController minIVs];
  criteria.shouldCheckMaxIVs = [ivParameterController shouldCheckMaxIVs];
  criteria.maxIVs = [ivParameterController maxIVs];
  
  if ([ivParameterController shouldCheckHiddenPower])
  {
    criteria.hiddenType = [ivParameterController hiddenType];
    criteria.minHiddenPower = [ivParameterController minHiddenPower];
  }
  else
  {
    criteria.hiddenType = Element::UNKNOWN;
  }
  criteria.nature = static_cast<Nature::Type>([[natureMenu selectedItem] tag]);
  criteria.canBeShiny = false;
  
  if (criteria.ExpectedNumberOfResults() > 10000)
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
    return [NSValue
             valueWithPointer: new WonderCardSeedSearcher::Criteria(criteria)];
  }
}

- (void)doSearchWithCriteria:(NSValue*)criteriaPtr
{
  std::auto_ptr<WonderCardSeedSearcher::Criteria> 
    criteria(static_cast<WonderCardSeedSearcher::Criteria*>
               ([criteriaPtr pointerValue]));
  
  WonderCardSeedSearcher  searcher;
  
  searcher.Search(*criteria,
                  ResultHandler(searcherController),
                  ProgressHandler(searcherController));
}


@end
