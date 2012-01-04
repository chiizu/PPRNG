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
#import "VertResizeOnlyWindowController.h"
#import "Gen4ConfigurationController.h"

@interface Gen4SeedInspectorController : VertResizeOnlyWindowController
{
  // header
  IBOutlet Gen4ConfigurationController  *gen4ConfigController;
  IBOutlet NSTextField                  *seedField;
  IBOutlet NSTextField                  *baseDelayField;
  IBOutlet NSTabView                    *tabView;
  
  // Frames tab
  IBOutlet NSTextField        *minFrameField;
  IBOutlet NSTextField        *maxFrameField;
  
  IBOutlet NSTableView        *frameTableView;
  IBOutlet NSArrayController  *frameContentArray;
  
  
  // Encounter Slots tab
  IBOutlet NSTextField        *esvMethod1FrameField;
  IBOutlet NSTextField        *esvFrameDescriptionField;
  
  IBOutlet NSTableView        *esvTableView;
  IBOutlet NSArrayController  *esvContentArray;
  
  
  // Time / Adjacents tab
  IBOutlet NSTextField        *timeFinderYearField;
  IBOutlet NSTextField        *timeFinderActualDelayField;
  IBOutlet NSTextField        *timeFinderSecondField;
  
  IBOutlet NSTableView        *timeFinderTableView;
  IBOutlet NSArrayController  *timeFinderContentArray;
  
  IBOutlet NSTextField        *adjacentsDelayVarianceField;
  IBOutlet NSTextField        *adjacentsTimeVarianceField;
  
  IBOutlet NSTableView        *adjacentsTableView;
  IBOutlet NSArrayController  *adjacentsContentArray;
  
  int                         mode;
  uint32_t                    raikouLocation;
  uint32_t                    enteiLocation;
  uint32_t                    latiLocation;
  uint32_t                    nextRaikouLocation;
  uint32_t                    nextEnteiLocation;
  uint32_t                    nextLatiLocation;
  uint32_t                    skippedFrames;
  uint32_t                    seedCoinFlips;
  uint64_t                    seedProfElmResponses;
  
  int32_t                     encounterType;
  int32_t                     syncNature;
  BOOL                        showRealFrame;
  NSMutableArray              *dpptFrames;
  NSMutableArray              *hgssFrames;
  
  NSMutableArray              *dpptESVs;
  NSMutableArray              *hgssESVs;
  
  BOOL                        useSpecifiedSecond;
  BOOL                        matchSeedDelayParity;
  uint32_t                    coinFlipsSearchValue;
  uint64_t                    profElmResponsesSearchValue;
  uint32_t                    raikouLocationSearchValue;
  uint32_t                    enteiLocationSearchValue;
  uint32_t                    latiLocationSearchValue;
}

@property int       mode;
@property uint32_t  raikouLocation;
@property uint32_t  enteiLocation;
@property uint32_t  latiLocation;
@property uint32_t  nextRaikouLocation;
@property uint32_t  nextEnteiLocation;
@property uint32_t  nextLatiLocation;
@property uint32_t  skippedFrames;
@property uint32_t  seedCoinFlips;
@property uint64_t  seedProfElmResponses;
@property BOOL      showRealFrame;
@property int32_t   encounterType;
@property int32_t   syncNature;
@property BOOL      useSpecifiedSecond;
@property BOOL      matchSeedDelayParity;
@property uint32_t  coinFlipsSearchValue;
@property uint64_t  profElmResponsesSearchValue;
@property uint32_t  raikouLocationSearchValue;
@property uint32_t  enteiLocationSearchValue;
@property uint32_t  latiLocationSearchValue;

// for opening with a seed and Method 1 frame already set
- (void)setSeed:(uint32_t)seed;
- (void)setFrame:(uint32_t)frame;

// for updating the delay, coin flips, etc. when the seed text is set
- (IBAction)seedUpdated:(id)sender;

- (IBAction)roamerLocationChanged:(id)sender;

// frames tab
- (IBAction)generateFrames:(id)sender;

// Encounter Slots tab
- (IBAction)esvMethod1FrameUpdated:(id)sender;

// time / adjacents tab
- (IBAction)yearUpdated:(id)sender;
- (IBAction)calculateTimes:(id)sender;
- (IBAction)generateAdjacents:(id)sender;

- (IBAction)addHeads:(id)sender;
- (IBAction)addTails:(id)sender;

- (IBAction)addEResponse:(id)sender;
- (IBAction)addKResponse:(id)sender;
- (IBAction)addPResponse:(id)sender;
- (IBAction)searchRoamerLocation:(id)sender;

- (IBAction)removeLastSearchItem:(id)sender;
- (IBAction)resetSearch:(id)sender;

@end
