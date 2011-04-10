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


#import "SearcherController.h"


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

- (void)addResult:(NSMutableDictionary*)result
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
  [delegate performSelector: @selector(doSearchWithCriteria:)
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
      [delegate performSelector: @selector(getValidatedSearchCriteria)];
    
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
  
  NSSavePanel  *sp = [NSSavePanel savePanel];
  
  [sp setRequiredFileType: @"csv"];
  
  int          runResult = [sp runModal];
  
  if (runResult == NSOKButton)
  {
    NSString             *result = @"";
    
    NSArray              *columns = [resultsTableView tableColumns];
    NSEnumerator         *columnEnumerator = [columns objectEnumerator];
    NSTableColumn        *column;
    
    /* output header row */
    column = [columnEnumerator nextObject];
    while ((column != nil) && [column isHidden])
    {
      column = [columnEnumerator nextObject];
    }
    
    if (column != nil)
    {
      NSString  *header = [[column headerCell] stringValue];
      
      result = [result stringByAppendingString: header];
      
      while (column = [columnEnumerator nextObject])
      {
        if (![column isHidden])
        {
          header = [[column headerCell] stringValue];
          result = [result stringByAppendingFormat: @",%@", header];
        }
      }
      
      result = [result stringByAppendingString: @"\n"];
    }
    
    NSArray              *rows = [contentArray arrangedObjects];
    NSEnumerator         *rowEnumerator = [rows objectEnumerator];
    NSMutableDictionary  *row;
    
    /* output data rows */
    while (row = [rowEnumerator nextObject])
    {
      columnEnumerator = [columns objectEnumerator];
      column = [columnEnumerator nextObject];
      
      while ((column != nil) && [column isHidden])
      {
        column = [columnEnumerator nextObject];
      }
      
      if (column != nil)
      {
        NSCell    *cell = [column dataCell];
        id        dataObject = [row objectForKey: [column identifier]];
        
        [cell setObjectValue: dataObject];
        
        NSString  *data = [cell stringValue];
        
        result = [result stringByAppendingString: data];
        
        while (column = [columnEnumerator nextObject])
        {
          if (![column isHidden])
          {
            cell = [column dataCell];
            dataObject = [row objectForKey: [column identifier]];
            [cell setObjectValue: dataObject];
            data = [cell stringValue];
            
            result = [result stringByAppendingFormat: @",%@", data];
          }
        }
        
        result = [result stringByAppendingString: @"\n"];
      }
    }
    
    NSError  *error;
    [result writeToFile: [sp filename] atomically: YES
            encoding: NSUTF8StringEncoding error: &error];
  }
}

@end
