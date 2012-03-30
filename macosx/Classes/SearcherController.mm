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


#import "SearcherController.h"
#import "Utilities.h"


@implementation SearcherController

- (void)awakeFromNib
{
  isSearching = NO;
  [startStopButton setTitle: @"Search"];
}

- (NSTableView*)tableView
{
  return resultsTableView;
}

- (NSArrayController*)arrayController
{
  return contentArray;
}


- (void)setGetValidatedSearchCriteriaSelector:(SEL)sel
{
  getValidatedSearchCriteriaSelector = sel;
}

- (void)setDoSearchWithCriteriaSelector:(SEL)sel
{
  doSearchWithCriteriaSelector = sel;
}

- (BOOL)isSearching
{
  return isSearching;
}

- (void)setIsSearching:(BOOL)is
{
  isSearching = is;
}

- (BOOL)searchIsCanceled
{
  return shouldStopSearch;
}

- (void)adjustProgress:(NSNumber*)progressDelta
{
  [searchProgressIndicator setDoubleValue:
   [searchProgressIndicator doubleValue] + [progressDelta doubleValue]];
}

- (void)addResult:(id)result
{
  [contentArray addObject: result];
}

- (void)finishSearch
{
  [searchProgressIndicator setDoubleValue: 0.0];
  [startStopButton setTitle: @"Search"];
  [startStopButton setEnabled: YES];
}

- (void)startSearch:(NSValue*)criteria
{
  [self setIsSearching: YES];
  [delegate performSelector: doSearchWithCriteriaSelector
            withObject: criteria];
  [self setIsSearching: NO];
  [self performSelectorOnMainThread: @selector(finishSearch)
        withObject: nil waitUntilDone: NO];
}

- (IBAction)startStop:(id)sender
{
  if (isSearching)
  {
    if (!shouldStopSearch)
    {
      [startStopButton setEnabled: NO];
      shouldStopSearch = YES;
    }
  }
  else
  {
    NSValue  *criteria =
      [delegate performSelector: getValidatedSearchCriteriaSelector];
    
    if (criteria != nil)
    {
      [contentArray setContent: [NSMutableArray array]];
      shouldStopSearch = NO;
      [self performSelectorInBackground: @selector(startSearch:)
            withObject: criteria];
      
      [startStopButton setTitle: @"Stop"];
    }
  }
}

- (IBAction)save:(id)sender
{
  if ([self isSearching])
    return;
  
  SaveTableContentsToCSV(resultsTableView, contentArray);
}

@end
