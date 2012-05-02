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

#include "PPRNGTypes.h"

@interface Gen4SeedInspectorController : VertResizeOnlyWindowController
{
  // header
  IBOutlet Gen4ConfigurationController  *gen4ConfigController;
  
  NSNumber  *seed;
  NSNumber  *baseDelay;
  int       mode;
  uint32_t  raikouLocation, enteiLocation, latiLocation;
  uint32_t  nextRaikouLocation, nextEnteiLocation, nextLatiLocation;
  uint32_t  skippedFrames;
  uint32_t  seedCoinFlips;
  uint64_t  seedProfElmResponses;
  
  NSString  *selectedTabId;
  
  // Frames tab
  uint32_t             minFrame, maxFrame;
  int32_t              encounterType;
  pprng::Nature::Type  syncNature;
  BOOL                 showRealFrame;
  NSMutableArray       *dpptFrames, *hgssFrames;
  
  IBOutlet NSTableView        *frameTableView;
  IBOutlet NSArrayController  *frameContentArray;
  
  
  // Encounter Slots tab
  uint32_t        esvMethod1Frame;
  NSString        *esvFrameDescription;
  NSMutableArray  *dpptESVs, *hgssESVs;
  
  IBOutlet NSTableView        *esvTableView;
  IBOutlet NSArrayController  *esvContentArray;
  
  
  // Time / Adjacents tab
  uint32_t  year;
  NSNumber  *actualDelay;
  BOOL      useSpecifiedSecond;
  uint32_t  second;
  
  IBOutlet NSTableView        *timeFinderTableView;
  IBOutlet NSArrayController  *timeFinderContentArray;
  
  uint32_t  secondVariance, delayVariance;
  BOOL      matchSeedDelayParity;
  
  uint32_t  coinFlipsSearchValue;
  uint64_t  profElmResponsesSearchValue;
  uint32_t  raikouLocationSearchValue;
  uint32_t  enteiLocationSearchValue;
  uint32_t  latiLocationSearchValue;
  
  IBOutlet NSTableView        *adjacentsTableView;
  IBOutlet NSArrayController  *adjacentsContentArray;
  
  
  // Egg PID Frames tab
  uint32_t  minEggPIDFrame, maxEggPIDFrame;
  BOOL      internationalParents;
  
  IBOutlet NSTableView        *eggPIDsTableView;
  IBOutlet NSArrayController  *eggPIDsContentArray;
  
  
  // Egg IV Frames tab
  uint32_t                    minEggIVFrame, maxEggIVFrame;
  
  BOOL                        enableParentIVs;
  NSNumber                    *aHP, *aAT, *aDF, *aSA, *aSD, *aSP;
  NSNumber                    *bHP, *bAT, *bDF, *bSA, *bSD, *bSP;
  
  IBOutlet NSTableView        *eggIVsTableView;
  IBOutlet NSArrayController  *eggIVsContentArray;
}

@property (copy) NSNumber  *seed;
@property (copy) NSNumber  *baseDelay;

@property int       mode;
@property uint32_t  raikouLocation, enteiLocation, latiLocation;
@property uint32_t  nextRaikouLocation, nextEnteiLocation, nextLatiLocation;
@property uint32_t  skippedFrames;
@property uint32_t  seedCoinFlips;
@property uint64_t  seedProfElmResponses;

@property (copy) NSString  *selectedTabId;

@property uint32_t             minFrame, maxFrame;
@property int32_t              encounterType;
@property pprng::Nature::Type  syncNature;
@property BOOL                 showRealFrame;

@property uint32_t         esvMethod1Frame;
@property (copy) NSString  *esvFrameDescription;

@property uint32_t         year;
@property (copy) NSNumber  *actualDelay;
@property BOOL             useSpecifiedSecond;
@property uint32_t         second;

@property uint32_t  secondVariance, delayVariance;
@property BOOL      matchSeedDelayParity;

@property uint32_t  coinFlipsSearchValue;
@property uint64_t  profElmResponsesSearchValue;
@property uint32_t  raikouLocationSearchValue;
@property uint32_t  enteiLocationSearchValue;
@property uint32_t  latiLocationSearchValue;

@property uint32_t  minEggPIDFrame, maxEggPIDFrame;
@property BOOL      internationalParents;

@property uint32_t  minEggIVFrame, maxEggIVFrame;
@property BOOL      enableParentIVs;

@property (copy) NSNumber  *aHP, *aAT, *aDF, *aSA, *aSD, *aSP;
@property (copy) NSNumber  *bHP, *bAT, *bDF, *bSA, *bSD, *bSP;

// frames tab
- (IBAction)generateFrames:(id)sender;
- (void)selectAndShowFrame:(uint32_t)frame;

// time / adjacents tab
- (IBAction)calculateTimes:(id)sender;
- (IBAction)generateAdjacents:(id)sender;

- (IBAction)addHeads:(id)sender;
- (IBAction)addTails:(id)sender;

- (IBAction)addEResponse:(id)sender;
- (IBAction)addKResponse:(id)sender;
- (IBAction)addPResponse:(id)sender;

- (IBAction)removeLastSearchItem:(id)sender;
- (IBAction)resetSearch:(id)sender;

// eggs
- (IBAction)generateEggPIDFrames:(id)sender;
- (void)selectAndShowEggPIDFrame:(uint32_t)frame;

- (void)setAIVs:(const pprng::OptionalIVs&)ivs;
- (void)setBIVs:(const pprng::OptionalIVs&)ivs;

- (IBAction)generateEggIVFrames:(id)sender;
- (void)selectAndShowEggIVFrame:(uint32_t)frame;

@end
