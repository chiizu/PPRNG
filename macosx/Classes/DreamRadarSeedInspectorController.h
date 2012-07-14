/*
  Copyright (C) 2012 chiizu
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

#include "PPRNGTypes.h"
#include "FrameGenerator.h"

@interface DreamRadarSeedInspectorController : HashedSeedInspectorController
{
  uint64_t  spinnerSequence;
  
  pprng::DreamRadarFrameGenerator::FrameType  type;
  
  BOOL                  genderRequired;
  pprng::Gender::Type   gender;
  pprng::Gender::Ratio  genderRatio;
  
  BOOL                  numPrecedingGenderlessRequired;
  uint32_t              slot, numPrecedingGenderless, maxNumPrecedingGenderless;
  
  // frames tab
  uint32_t  minFrame, maxFrame;
  
  IBOutlet NSTableView            *frameTableView;
  IBOutlet NSArrayController      *frameContentArray;
  
  IBOutlet IVParameterController  *ivParameterController;
  
  // adjacents tab
  uint32_t  secondsVariance;
  uint32_t  timer0Variance;
  
  uint64_t  spinnerSequenceSearchValue;
  
  IBOutlet NSTableView            *adjacentsTableView;
  IBOutlet NSArrayController      *adjacentsContentArray;
}

@property uint64_t  spinnerSequence;

@property pprng::DreamRadarFrameGenerator::FrameType  type;

@property BOOL                  genderRequired;
@property pprng::Gender::Type   gender;
@property pprng::Gender::Ratio  genderRatio;

@property BOOL      numPrecedingGenderlessRequired;
@property uint32_t  slot, numPrecedingGenderless, maxNumPrecedingGenderless;

@property uint32_t  minFrame, maxFrame;

@property uint32_t  secondsVariance;
@property uint32_t  timer0Variance;

@property uint64_t  spinnerSequenceSearchValue;


- (IBAction)generateFrames:(id)sender;
- (void)selectAndShowFrame:(uint32_t)frame;

- (IBAction)generateAdjacents:(id)sender;

- (IBAction)addUpPosition:(id)sender;
- (IBAction)addUpRightPosition:(id)sender;
- (IBAction)addRightPosition:(id)sender;
- (IBAction)addDownRightPosition:(id)sender;
- (IBAction)addDownPosition:(id)sender;
- (IBAction)addDownLeftPosition:(id)sender;
- (IBAction)addLeftPosition:(id)sender;
- (IBAction)addUpLeftPosition:(id)sender;

- (IBAction)removeLastSearchItem:(id)sender;
- (IBAction)resetSearch:(id)sender;

@end
