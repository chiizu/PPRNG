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
#include "BasicTypes.h"

@interface IVParameterController : NSObject
{
  IBOutlet NSPopUpButton  *ivPatternMenu;
  
  IBOutlet NSTextField    *minHPField;
  IBOutlet NSTextField    *minAtkField;
  IBOutlet NSTextField    *minDefField;
  IBOutlet NSTextField    *minSpAField;
  IBOutlet NSTextField    *minSpDField;
  IBOutlet NSTextField    *minSpeField;
  
  IBOutlet NSButton       *enableMaxIVsButton;
  IBOutlet NSTextField    *maxHPField;
  IBOutlet NSStepper      *maxHPStepper;
  IBOutlet NSTextField    *maxAtkField;
  IBOutlet NSStepper      *maxAtkStepper;
  IBOutlet NSTextField    *maxDefField;
  IBOutlet NSStepper      *maxDefStepper;
  IBOutlet NSTextField    *maxSpAField;
  IBOutlet NSStepper      *maxSpAStepper;
  IBOutlet NSTextField    *maxSpDField;
  IBOutlet NSStepper      *maxSpDStepper;
  IBOutlet NSTextField    *maxSpeField;
  IBOutlet NSStepper      *maxSpeStepper;
  
  IBOutlet NSButton       *enableHiddenPowerButton;
  IBOutlet NSPopUpButton  *hiddenTypeMenu;
  IBOutlet NSTextField    *minHiddenPowerField;
  IBOutlet NSStepper      *minHiddenPowerStepper;
  
  IBOutlet NSButton       *isRoamerButton;
}

- (IBAction)switchIVPattern:(id)sender;
- (IBAction)toggleMaxIVs:(id)sender;
- (IBAction)toggleHiddenPower:(id)sender;

- (pprng::IVs)minIVs;
- (void)setMinIVs:(pprng::IVs)ivs;

- (BOOL)shouldCheckMaxIVs;
- (void)setShouldCheckMaxIVs:(BOOL)s;

- (pprng::IVs)maxIVs;
- (void)setMaxIVs:(pprng::IVs)ivs;

- (BOOL)shouldCheckHiddenPower;
- (void)setShouldCheckHiddenPower:(BOOL)s;

- (pprng::Element::Type)hiddenType;
- (void)setHiddenType:(pprng::Element::Type)type;

- (uint32_t)minHiddenPower;
- (void)setMinHiddenPower:(uint32_t)power;

- (BOOL)isRoamer;
- (void)setIsRoamer:(BOOL)ir;

- (uint32_t)numberOfIVCombinations;

- (void)controlTextDidChange:(NSNotification*)notification;

@end
