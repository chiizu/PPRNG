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


#import <Cocoa/Cocoa.h>


@interface SearcherController : NSObject
{
  IBOutlet id                   delegate;
  
  IBOutlet NSButton             *startStopButton;
  IBOutlet NSProgressIndicator  *searchProgressIndicator;
  
  IBOutlet NSTableView          *resultsTableView;
  IBOutlet NSArrayController    *contentArray;
  
  BOOL                          isSearching;
  BOOL                          shouldStopSearch;
}

- (IBAction)startStop:(id)sender;
- (IBAction)save:(id)sender;

- (NSTableView*)tableView;
- (NSArrayController*)arrayController;

- (BOOL)isSearching;
- (void)setIsSearching:(BOOL)is;
- (BOOL)searchIsCanceled;
- (void)adjustProgress:(NSNumber*)progressDelta;
- (void)addResult:(NSMutableDictionary*)result;

@end
