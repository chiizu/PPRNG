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

#import "Gen5ConfigurationController.h"
#import "IVParameterController.h"

@class StandardSeedInspectorController;

@interface HashedSeedInspectorFramesTabController : NSObject
{
  IBOutlet StandardSeedInspectorController  *inspectorController;
  
  pprng::Gen5PIDFrameGenerator::FrameType  encounterFrameType;
  pprng::EncounterLead::Ability            encounterLeadAbility;
  
  BOOL                  genderRequired;
  pprng::Gender::Type   targetGender;
  BOOL                  genderRatioRequired;
  pprng::Gender::Ratio  targetGenderRatio;
  
  BOOL      startFromInitialPIDFrame;
  uint32_t  minPIDFrame, maxPIDFrame;
  
  BOOL      isEntralink;
  uint32_t  cgearStartOffset;
  
  IBOutlet NSTableView                  *pidFrameTableView;
  IBOutlet NSArrayController            *pidFrameContentArray;
  
  uint32_t  minIVFrame, maxIVFrame;
  
  IBOutlet NSTableView                  *ivFrameTableView;
  IBOutlet NSArrayController            *ivFrameContentArray;
  
  IBOutlet IVParameterController        *ivParameterController;
}

@property pprng::Gen5PIDFrameGenerator::FrameType  encounterFrameType;
@property pprng::EncounterLead::Ability            encounterLeadAbility;

@property BOOL                  genderRequired;
@property pprng::Gender::Type   targetGender;
@property BOOL                  genderRatioRequired;
@property pprng::Gender::Ratio  targetGenderRatio;

@property BOOL      startFromInitialPIDFrame;
@property uint32_t  minPIDFrame, maxPIDFrame;

@property BOOL      isEntralink;
@property uint32_t  cgearStartOffset;

@property uint32_t   minIVFrame, maxIVFrame;
@property (readonly) IVParameterController  *ivParameterController;

- (IBAction)generatePIDFrames:(id)sender;
- (IBAction)generateIVFrames:(id)sender;

- (void)selectAndShowPIDFrame:(uint32_t)frame;
- (void)selectAndShowIVFrame:(uint32_t)frame;

@end
