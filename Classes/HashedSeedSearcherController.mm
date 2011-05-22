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
#include <boost/lexical_cast.hpp>

using namespace pprng;

namespace
{

struct GUICriteria : public HashedSeedSearcher::Criteria
{
  uint32_t      tid, sid;
  bool          shinyOnly;
  Nature::Type  nature;
  uint32_t      minPIDFrame, maxPIDFrame;
  
  uint64_t ExpectedNumberOfResults()
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
  ResultHandler(SearcherController *c, uint32_t tid, uint32_t sid,
                BOOL shinyOnly, Nature::Type nature,
                uint32_t minPIDFrame, uint32_t maxPIDFrame)
    : controller(c), m_tid(tid), m_sid(sid),
      m_shinyOnly(shinyOnly), m_nature(nature),
      m_minFrame(minPIDFrame), m_maxFrame(maxPIDFrame)
  {}
  
  void operator()(const HashedIVFrame &frame)
  {
    NSMutableDictionary  *pidResult = nil;
    
    Gen5PIDFrameGenerator  frameGen(frame.seed,
                                    Gen5PIDFrameGenerator::GrassCaveFrame,
                                    false, m_tid, m_sid);
    bool                   found = false;
    
    while (frameGen.CurrentFrame().number < (m_minFrame - 1))
      frameGen.AdvanceFrame();
    
    while (frameGen.CurrentFrame().number < m_maxFrame)
    {
      frameGen.AdvanceFrame();
      if (frameGen.CurrentFrame().pid.IsShiny(m_tid, m_sid) &&
          ((m_nature == Nature::ANY) ||
           (frameGen.CurrentFrame().nature == m_nature)))
      {
        found = true;
        break;
      }
    }
    
    if (found)
    {
      Gen5PIDFrame  pidFrame = frameGen.CurrentFrame();
      
      pidResult = [NSMutableDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithUnsignedInt: pidFrame.number], @"shinyFrame",
        [NSString stringWithFormat: @"%s",
          Nature::ToString(pidFrame.nature).c_str()], @"shinyNature",
        (pidFrame.synched ? @"Y" : @""), @"shinySync",
        [NSNumber numberWithUnsignedInt: pidFrame.pid.Gen5Ability()],
          @"shinyAbility",
        [NSNumber numberWithUnsignedInt: pidFrame.esv], @"shinyESV",
        GenderString(pidFrame.pid), @"shinyGender",
        nil];
    }
    else if (m_shinyOnly)
    {
      return;
    }
    
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
  uint32_t            m_tid, m_sid;
  BOOL                m_shinyOnly;
  Nature::Type        m_nature;
  uint32_t            m_minFrame, m_maxFrame;
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
  
  [[[[[searcherController tableView] tableColumnWithIdentifier: @"seed"]
    dataCell] formatter]
   setFormatWidth: 16];
  
  [[searcherController tableView] setDoubleAction: @selector(inspectSeed:)];
  
  NSDate  *now = [NSDate date];
  [fromDateField setObjectValue: now];
  [toDateField setObjectValue: now];
}

- (IBAction)toggleShinyOnly:(id)sender
{
  BOOL  enabled = [sender state];
  
  [shinyNaturePopUp setEnabled: enabled];
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
  
  criteria.minFrame = [minIVFrameField intValue];
  criteria.maxFrame = [maxIVFrameField intValue];
  
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
  criteria.shinyOnly = [shinyOnlyButton state];
  criteria.nature = Nature::Type([[shinyNaturePopUp selectedItem] tag]);
  criteria.minPIDFrame = [minPIDFrameField intValue];
  criteria.maxPIDFrame = [maxPIDFrameField intValue];
  
  uint32_t  numResults = criteria.ExpectedNumberOfResults();
  
  if (numResults > 10000)
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
    return [NSValue valueWithPointer: new GUICriteria(criteria)];
  }
}

- (void)doSearchWithCriteria:(NSValue*)criteriaPtr
{
  std::auto_ptr<GUICriteria> 
    criteria(static_cast<GUICriteria*>([criteriaPtr pointerValue]));
  
  HashedSeedSearcher  searcher;
  
  searcher.Search(*criteria,
    ResultHandler(searcherController,
                  criteria->tid, criteria->sid,
                  criteria->shinyOnly,
                  criteria->nature,
                  criteria->minPIDFrame, criteria->maxPIDFrame),
                  ProgressHandler(searcherController));
}


@end
