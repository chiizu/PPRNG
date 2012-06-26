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

#import <Cocoa/Cocoa.h>

#include "PPRNGTypes.h"
#include "FrameGenerator.h"

@class StandardSeedInspectorController;

@interface HashedSeedInspectorAdjacentsTabController : NSObject
{
  IBOutlet StandardSeedInspectorController  *inspectorController;
  
  uint32_t  secondsVariance;
  uint32_t  timer0Variance;
  BOOL      matchOffsetFromInitialPIDFrame;
  
  uint32_t  ivFrame;
  BOOL      isRoamer;
  
  uint32_t  pidFrame;
  uint32_t  pidFrameVariance;
  
  pprng::Gen5PIDFrameGenerator::FrameType  encounterFrameType;
  pprng::EncounterLead::Ability            encounterLeadAbility;
  
  BOOL                  genderRequired;
  pprng::Gender::Type   targetGender;
  BOOL                  genderRatioRequired;
  pprng::Gender::Ratio  targetGenderRatio;
  
  BOOL      isEntralink;
  uint32_t  cgearStartOffset;
  
  IBOutlet NSTableView            *adjacentsTableView;
  IBOutlet NSArrayController      *adjacentsContentArray;
}

@property uint32_t  secondsVariance;
@property uint32_t  timer0Variance;
@property BOOL      matchOffsetFromInitialPIDFrame;

@property uint32_t  ivFrame;
@property BOOL      isRoamer;

@property uint32_t  pidFrame;
@property uint32_t  pidFrameVariance;

@property pprng::Gen5PIDFrameGenerator::FrameType  encounterFrameType;
@property pprng::EncounterLead::Ability            encounterLeadAbility;

@property BOOL                  genderRequired;
@property pprng::Gender::Type   targetGender;
@property BOOL                  genderRatioRequired;
@property pprng::Gender::Ratio  targetGenderRatio;

@property BOOL      isEntralink;
@property uint32_t  cgearStartOffset;

- (IBAction)generateAdjacents:(id)sender;

@end
