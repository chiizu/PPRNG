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


#import "B2W2ParameterSearcherController.h"

#import "SearchResultProtocols.h"

#include "InitialSeedSearcher.h"
#include "LinearCongruentialRNG.h"
#include "Utilities.h"

using namespace pprng;

@interface B2W2ParameterSearchResult : NSObject <HashedSeedResultParameters>
{
  DECLARE_HASHED_SEED_RESULT_PARAMETERS_VARIABLES();
  
  uint64_t  spinnerSequence;
}

@property uint64_t  spinnerSequence;

@end

@implementation B2W2ParameterSearchResult

SYNTHESIZE_HASHED_SEED_RESULT_PARAMETERS_PROPERTIES();

@synthesize spinnerSequence;

@end

namespace
{

struct ResultHandler
{
  ResultHandler(SearcherController *c, bool memoryLinkUsed)
    : m_controller(c), m_memoryLinkUsed(memoryLinkUsed)
  {}
  
  void operator()(const HashedSeed &seed)
  {
    B2W2ParameterSearchResult  *row = [[B2W2ParameterSearchResult alloc] init];
    
    SetHashedSeedResultParameters(row, seed);
    row.spinnerSequence = SpinnerPositions(seed, m_memoryLinkUsed,
                                           SpinnerPositions::MAX_SPINS).word;
    
    [m_controller performSelectorOnMainThread: @selector(addResult:)
                  withObject: row
                  waitUntilDone: NO];
  }
  
  SearcherController  *m_controller;
  const bool          m_memoryLinkUsed;
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

@implementation B2W2ParameterSearcherController

@synthesize timer0Low, timer0High, vcountLow, vcountHigh, vframeLow, vframeHigh;
@synthesize startDate, startHour, startMinute, startSecond;
@synthesize button1, button2, button3;
@synthesize spinnerSequenceSearchValue;

- (NSString *)windowNibName
{
	return @"B2W2ParameterSearcher";
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
    else if ((version == Game::White2Japanese) ||
             (version == Game::Black2French) ||
             (version == Game::Black2German))
    {
      self.timer0Low = 0x1780;
      self.timer0High = 0x197F;
      self.vcountLow = 0xA8;
      self.vcountHigh = 0xC7;
    }
    else if (Game::IsBlack2White2(version))
    {
      self.timer0Low = 0x1580;
      self.timer0High = 0x177F;
      self.vcountLow = 0x98;
      self.vcountHigh = 0xb7;
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
  
  self.spinnerSequenceSearchValue = 0;
}

- (void)windowWillClose:(NSNotification *)notification
{
  if ([searcherController isSearching])
    [searcherController startStop: self];
}


- (void)addSpin:(SpinnerPositions::Position)position
{
  SpinnerPositions  spins(spinnerSequenceSearchValue);
  
  if (spins.NumSpins() < SpinnerPositions::MAX_SPINS)
  {
    spins.AddSpin(position);
    
    self.spinnerSequenceSearchValue = spins.word;
  }
}

- (IBAction)addUpPosition:(id)sender
{
  [self addSpin: SpinnerPositions::UP];
}

- (IBAction)addUpRightPosition:(id)sender
{
  [self addSpin: SpinnerPositions::UP_RIGHT];
}

- (IBAction)addRightPosition:(id)sender
{
  [self addSpin: SpinnerPositions::RIGHT];
}

- (IBAction)addDownRightPosition:(id)sender
{
  [self addSpin: SpinnerPositions::DOWN_RIGHT];
}

- (IBAction)addDownPosition:(id)sender
{
  [self addSpin: SpinnerPositions::DOWN];
}

- (IBAction)addDownLeftPosition:(id)sender
{
  [self addSpin: SpinnerPositions::DOWN_LEFT];
}

- (IBAction)addLeftPosition:(id)sender
{
  [self addSpin: SpinnerPositions::LEFT];
}

- (IBAction)addUpLeftPosition:(id)sender
{
  [self addSpin: SpinnerPositions::UP_LEFT];
}

- (IBAction)removeLastSearchItem:(id)sender
{
  uint32_t  numSpins = SpinnerPositions(spinnerSequenceSearchValue).NumSpins();
  if (numSpins > 0)
  {
    SpinnerPositions  spins(spinnerSequenceSearchValue);
    spins.RemoveSpin();
    
    self.spinnerSequenceSearchValue = spins.word;
  }
}

- (IBAction)resetSearch:(id)sender
{
  self.spinnerSequenceSearchValue = 0ULL;
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
  
  B2W2InitialSeedSearcher::Criteria  criteria;
  
  criteria.seedParameters.macAddress.low = [gen5ConfigController macAddressLow];
  criteria.seedParameters.macAddress.high =
    [gen5ConfigController macAddressHigh];
  
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
  
  criteria.memoryLinkUsed = [gen5ConfigController memoryLinkUsed];
  
  criteria.spins = SpinnerPositions(spinnerSequenceSearchValue);
  
  if ((criteria.ExpectedNumberOfResults() > 10) &&
      (criteria.spins.NumSpins() < SpinnerPositions::MAX_SPINS))
  {
    NSAlert *alert = [[NSAlert alloc] init];
    
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Too many results expected"];
    [alert setInformativeText:@"Please enter more spins or limit the search ranges of the various DS parameters in order to reduce the number of expected results."];
    [alert setAlertStyle:NSWarningAlertStyle];
    
    [alert beginSheetModalForWindow:[self window] modalDelegate:self
           didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:)
           contextInfo:nil];
    
    return nil;
  }
  else
  {
    return [NSValue
             valueWithPointer: new B2W2InitialSeedSearcher::Criteria(criteria)];
  }
}

- (void)doSearchWithCriteria:(NSValue*)criteriaPtr
{
  std::auto_ptr<B2W2InitialSeedSearcher::Criteria> 
    criteria(static_cast<B2W2InitialSeedSearcher::Criteria*>
               ([criteriaPtr pointerValue]));
  
  B2W2InitialSeedSearcher  searcher;
  
  searcher.Search(*criteria,
                  ResultHandler(searcherController, criteria->memoryLinkUsed),
                  ProgressHandler(searcherController));
}

@end
