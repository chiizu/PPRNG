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

#import "HashedSeedInspectorFramesTabController.h"

#include "HashedSeed.h"
#include "FrameGenerator.h"
#include "Utilities.h"

using namespace pprng;

@implementation HashedSeedInspectorFramesTabController

- (void)awakeFromNib
{
  [[[[pidFrameTableView tableColumnWithIdentifier: @"pid"] dataCell] formatter]
   setFormatWidth: 8];
}

- (IBAction)toggleUseInitialPID:(id)sender
{
  BOOL enabled = [useInitialPIDButton state];
  [minPIDFrameField setEnabled: !enabled];
}

- (IBAction)generatePIDFrames:(id)sender
{
  if ([[seedField stringValue] length] == 0)
  {
    return;
  }
  
  [pidFrameContentArray setContent: [NSMutableArray array]];
  
  HashedSeed  seed([[seedField objectValue] unsignedLongLongValue]);
  
  uint32_t  minPIDFrame = [useInitialPIDButton state] ?
                          (seed.GetSkippedPIDFrames() + 1) :
                          [minPIDFrameField intValue];
  uint32_t  maxPIDFrame = [maxPIDFrameField intValue];
  uint32_t  frameNum = 0, limitFrame = minPIDFrame - 1;
  uint32_t  tid = [gen5ConfigController tid];
  uint32_t  sid = [gen5ConfigController sid];
  
  Gen5PIDFrameGenerator::FrameType  frameType =
    static_cast<Gen5PIDFrameGenerator::FrameType>
      ([[pidFrameTypeMenu selectedItem] tag]);
  
  Gen5PIDFrameGenerator  generator(seed, frameType,
                                   [useCompoundEyesCheckBox state], tid, sid);
  
  bool  generatesESV = generator.GeneratesESV();
  bool  generatesIsEncounter = generator.GeneratesIsEncounter();
  
  while (frameNum < limitFrame)
  {
    generator.AdvanceFrame();
    ++frameNum;
  }
  
  NSMutableArray  *rowArray =
    [NSMutableArray arrayWithCapacity: maxPIDFrame - minPIDFrame + 1];
  
  while (frameNum < maxPIDFrame)
  {
    generator.AdvanceFrame();
    ++frameNum;
    
    Gen5PIDFrame  frame = generator.CurrentFrame();
    uint32_t      genderValue = frame.pid.GenderValue();
    
    NSMutableDictionary  *result =
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
				[NSNumber numberWithUnsignedInt: frame.number], @"frame",
        [NSNumber numberWithUnsignedInt: frame.pid.word], @"pid",
        (frame.pid.IsShiny(tid, sid) ? @"★" : @""), @"shiny",
        [NSString stringWithFormat: @"%s",
          Nature::ToString(frame.nature).c_str()], @"nature",
        [NSNumber numberWithUnsignedInt: frame.pid.Gen5Ability()], @"ability",
        ((genderValue < 31) ? @"♀" : @"♂"), @"gender18",
        ((genderValue < 63) ? @"♀" : @"♂"), @"gender14",
        ((genderValue < 127) ? @"♀" : @"♂"), @"gender12",
        ((genderValue < 191) ? @"♀" : @"♂"), @"gender34",
        (frame.synched ? @"Y" : @""), @"sync",
        (generatesESV ? [NSString stringWithFormat: @"%d", frame.esv] : @""),
          @"esv",
        HeldItemString(frame.heldItem), @"heldItem",
        ((generatesIsEncounter && frame.isEncounter) ? @"Y" : @""),
          @"isEncounter",
        nil];
    
    [rowArray addObject: result];
  }
  
  [pidFrameContentArray addObjects: rowArray];
}


- (IBAction)generateIVFrames:(id)sender
{
  if ([[seedField stringValue] length] == 0)
  {
    return;
  }
  
  [ivFrameContentArray setContent: [NSMutableArray array]];
  
  HashedSeed  seed([[seedField objectValue] unsignedLongLongValue]);
  
  uint32_t  minIVFrame = [minIVFrameField intValue];
  uint32_t  maxIVFrame = [maxIVFrameField intValue];
  uint32_t  frameNum = 0, limitFrame = minIVFrame - 1;
  
  HashedIVFrameGenerator  generator(seed, [ivParameterController isRoamer] ?
                                          HashedIVFrameGenerator::Roamer :
                                          HashedIVFrameGenerator::Normal);
  
  while (frameNum < limitFrame)
  {
    generator.AdvanceFrame();
    ++frameNum;
  }
  
  NSMutableArray  *rowArray =
    [NSMutableArray arrayWithCapacity: maxIVFrame - minIVFrame + 1];
  
  while (frameNum < maxIVFrame)
  {
    generator.AdvanceFrame();
    ++frameNum;
    
    HashedIVFrame  frame = generator.CurrentFrame();
    
    NSMutableDictionary  *result =
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
				[NSNumber numberWithUnsignedInt: frame.number], @"frame",
        [NSNumber numberWithUnsignedInt: frame.ivs.hp()], @"hp",
        [NSNumber numberWithUnsignedInt: frame.ivs.at()], @"atk",
        [NSNumber numberWithUnsignedInt: frame.ivs.df()], @"def",
        [NSNumber numberWithUnsignedInt: frame.ivs.sa()], @"spa",
        [NSNumber numberWithUnsignedInt: frame.ivs.sd()], @"spd",
        [NSNumber numberWithUnsignedInt: frame.ivs.sp()], @"spe",
        [NSString stringWithFormat: @"%s",
          Element::ToString(frame.ivs.HiddenType()).c_str()], @"hiddenType",
        [NSNumber numberWithUnsignedInt: frame.ivs.HiddenPower()],
          @"hiddenPower",
        nil];
    
    [rowArray addObject: result];
  }
  
  [ivFrameContentArray addObjects: rowArray];
}

@end
