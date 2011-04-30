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



#import "IVParameterController.h"

using namespace pprng;

namespace
{

enum Pattern
{
  Custom = 0,
  HexFlawless,
  PhysicalFlawless,
  SpecialFlawless,
  SpecialHiddenPowerFlawless
};

}

@implementation IVParameterController

- (IBAction)switchIVPattern:(id)sender
{
  IVs  ivs;
  
  ivs.hp(31); ivs.at(31); ivs.df(31); ivs.sa(31); ivs.sd(31); ivs.sp(31);
  
  uint32_t  patternNum = [[sender selectedItem] tag];
  
  if (patternNum != Custom)
  {
    [self setMaxIVs: ivs];
    [self setShouldCheckMaxIVs: NO];
  }
  
  BOOL shouldCheckHiddenPower = NO;
  
  switch (patternNum)
  {
  case HexFlawless:
    break;
    
  case PhysicalFlawless:
    ivs.sa(0);
    break;
    
  case SpecialFlawless:
    ivs.at(0);
    break;
    
  case SpecialHiddenPowerFlawless:
    ivs.hp(30); ivs.at(0); ivs.df(30); ivs.sa(30); ivs.sd(30); ivs.sp(30);
    shouldCheckHiddenPower = YES;
    break;
  
  case Custom:
  default:
    return;
    break;
  }
  
  [self setMinIVs: ivs];
  [self setShouldCheckHiddenPower: shouldCheckHiddenPower];
  [self setMinHiddenPower: 70];
}

- (void)checkIVPattern
{
  if (![self shouldCheckMaxIVs])
  {
    IVs  ivs, minIVs = [self minIVs];
    
    if ([self shouldCheckHiddenPower])
    {
      ivs.hp(30); ivs.at(0); ivs.df(30); ivs.sa(30); ivs.sd(30); ivs.sp(30);
      uint32_t  minHiddenPower = [self minHiddenPower];
      
      if ((minHiddenPower == 70) && (minIVs == ivs))
      {
        [ivPatternMenu selectItemWithTag: SpecialHiddenPowerFlawless];
        return;
      }
    }
    else
    {
      ivs.hp(31); ivs.at(31); ivs.df(31); ivs.sa(31); ivs.sd(31); ivs.sp(31);
      
      if (minIVs == ivs)
      {
        [ivPatternMenu selectItemWithTag: HexFlawless];
        return;
      }
      
      ivs.sa(0);
      if (minIVs == ivs)
      {
        [ivPatternMenu selectItemWithTag: PhysicalFlawless];
        return;
      }
      
      ivs.at(0);
      ivs.sa(31);
      if (minIVs == ivs)
      {
        [ivPatternMenu selectItemWithTag: SpecialFlawless];
        return;
      }
    }
  }
  
  [ivPatternMenu selectItemWithTag: Custom];
}

- (IBAction)toggleMaxIVs:(id)sender
{
  BOOL  enabled = [sender state];
  
  [maxHPField setEnabled: enabled];
  [maxHPStepper setEnabled: enabled];
  [maxAtkField setEnabled: enabled];
  [maxAtkStepper setEnabled: enabled];
  [maxDefField setEnabled: enabled];
  [maxDefStepper setEnabled: enabled];
  [maxSpAField setEnabled: enabled];
  [maxSpAStepper setEnabled: enabled];
  [maxSpDField setEnabled: enabled];
  [maxSpDStepper setEnabled: enabled];
  [maxSpeField setEnabled: enabled];
  [maxSpeStepper setEnabled: enabled];
  
  [self checkIVPattern];
}

- (IBAction)toggleHiddenPower:(id)sender
{
  BOOL  enabled = [sender state];
  
  [hiddenTypeMenu setEnabled: enabled];
  [minHiddenPowerField setEnabled: enabled];
  [minHiddenPowerStepper setEnabled: enabled];
  
  [self checkIVPattern];
}

- (IVs)minIVs
{
  IVs  result;
  
  result.hp([minHPField intValue]);
  result.at([minAtkField intValue]);
  result.df([minDefField intValue]);
  result.sa([minSpAField intValue]);
  result.sd([minSpDField intValue]);
  result.sp([minSpeField intValue]);
  
  return result;
}

- (void)setMinIVs:(IVs)ivs
{
  [minHPField setIntValue: ivs.hp()];
  [minAtkField setIntValue: ivs.at()];
  [minDefField setIntValue: ivs.df()];
  [minSpAField setIntValue: ivs.sa()];
  [minSpDField setIntValue: ivs.sd()];
  [minSpeField setIntValue: ivs.sp()];
}

- (BOOL)shouldCheckMaxIVs
{
  return [enableMaxIVsButton state];
}

- (void)setShouldCheckMaxIVs:(BOOL)s
{
  [enableMaxIVsButton setState: s];
  [self toggleMaxIVs: enableMaxIVsButton];
}

- (IVs)maxIVs
{
  IVs  result;
  
  result.hp([maxHPField intValue]);
  result.at([maxAtkField intValue]);
  result.df([maxDefField intValue]);
  result.sa([maxSpAField intValue]);
  result.sd([maxSpDField intValue]);
  result.sp([maxSpeField intValue]);
  
  return result;
}

- (void)setMaxIVs:(IVs)ivs
{
  [maxHPField setIntValue: ivs.hp()];
  [maxAtkField setIntValue: ivs.at()];
  [maxDefField setIntValue: ivs.df()];
  [maxSpAField setIntValue: ivs.sa()];
  [maxSpDField setIntValue: ivs.sd()];
  [maxSpeField setIntValue: ivs.sp()];
}

- (BOOL)shouldCheckHiddenPower
{
  return [enableHiddenPowerButton state];
}

- (void)setShouldCheckHiddenPower:(BOOL)s
{
  [enableHiddenPowerButton setState: s];
  [self toggleHiddenPower: enableHiddenPowerButton];
}

- (Element::Type)hiddenType
{
  return static_cast<Element::Type>([[hiddenTypeMenu selectedItem] tag]);
}

- (void)setHiddenType:(Element::Type)type
{
  [hiddenTypeMenu selectItemWithTag: type];
}

- (uint32_t)minHiddenPower
{
  return [minHiddenPowerField intValue];
}

- (void)setMinHiddenPower:(uint32_t)power
{
  [minHiddenPowerField setIntValue: power];
}

- (BOOL)isRoamer
{
  return [isRoamerButton state];
}

- (void)setIsRoamer:(BOOL)ir
{
  [isRoamerButton setState: ir];
}

- (uint32_t)numberOfIVCombinations
{
  IVs  minIVs = [self minIVs];
  IVs  maxIVs = [self shouldCheckMaxIVs] ? [self maxIVs] : IVs(0x7FFF7FFF);
  
  uint32_t  result = (maxIVs.hp() - minIVs.hp() + 1) *
                     (maxIVs.at() - minIVs.at() + 1) *
                     (maxIVs.df() - minIVs.df() + 1) *
                     (maxIVs.sa() - minIVs.sa() + 1) *
                     (maxIVs.sd() - minIVs.sd() + 1) *
                     (maxIVs.sp() - minIVs.sp() + 1);
  return result;
}

- (void)controlTextDidChange:(NSNotification*)notification
{
  [self checkIVPattern];
}

@end
