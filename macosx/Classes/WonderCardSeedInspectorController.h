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
#import "HashedSeedInspectorController.h"
#import "IVParameterController.h"

@interface WonderCardSeedInspectorController : HashedSeedInspectorController
{
  pprng::Nature::Type               cardNature;
  pprng::Ability::Type              cardAbility;
  pprng::Gender::Type               cardGender;
  pprng::Gender::Ratio              cardGenderRatio;
  pprng::WonderCardShininess::Type  cardShininess;
  NSNumber                          *cardTID, *cardSID;
  
  BOOL  natureSearchable, abilitySearchable;
  BOOL  genderSearchable, shininessSearchable;
  
  // frames tab
  BOOL      startFromInitialFrame;
  uint32_t  minFrame, maxFrame;
  
  IBOutlet NSTableView            *frameTableView;
  IBOutlet NSArrayController      *frameContentArray;
  
  IBOutlet IVParameterController  *ivParameterController;
  
  // adjacents tab
  uint32_t  secondsVariance;
  uint32_t  timer0Variance;
  
  BOOL      matchOffsetFromInitialFrame;
  uint32_t  targetFrame;
  uint32_t  targetFrameVariance;
  
  IBOutlet NSTableView            *adjacentsTableView;
  IBOutlet NSArrayController      *adjacentsContentArray;
}

@property pprng::Nature::Type               cardNature;
@property pprng::Ability::Type              cardAbility;
@property pprng::Gender::Type               cardGender;
@property pprng::Gender::Ratio              cardGenderRatio;
@property pprng::WonderCardShininess::Type  cardShininess;
@property (copy) NSNumber                   *cardTID, *cardSID;

@property BOOL  natureSearchable, abilitySearchable;
@property BOOL  genderSearchable, shininessSearchable;

@property BOOL      startFromInitialFrame;
@property uint32_t  minFrame, maxFrame;

@property uint32_t  secondsVariance;
@property uint32_t  timer0Variance;

@property BOOL      matchOffsetFromInitialFrame;
@property uint32_t  targetFrame;
@property uint32_t  targetFrameVariance;


- (IBAction)generateFrames:(id)sender;
- (IBAction)generateAdjacents:(id)sender;

- (void)selectAndShowFrame:(uint32_t)frame;

@end
