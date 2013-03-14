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



#import "IVParameterController.h"

#import "Utilities.h"

using namespace pprng;

@implementation IVParameterController

@synthesize  ivPattern;
@synthesize  minHP, minAT, minDF, minSA, minSD, minSP;
@synthesize  maxHP, maxAT, maxDF, maxSA, maxSD, maxSP;
@synthesize  considerHiddenPower;
@synthesize  hiddenTypeMask;
@synthesize  minHiddenPower;
@synthesize  isRoamer;

- (void)awakeFromNib
{
  isSettingPattern = NO;
  [self setMinIVs: IVs(0, 0, 0, 0, 0, 0)];
  [self setMaxIVs: IVs(31, 31, 31, 31, 31, 31)];
  
  self.considerHiddenPower = NO;
  self.hiddenTypeMask = 0xffff;
  self.minHiddenPower = 70;
  self.isRoamer = NO;
}

- (void)setIvPattern:(IVPattern::Type)newIvPattern
{
  if (newIvPattern != ivPattern)
  {
    ivPattern = newIvPattern;
    isSettingPattern = YES;
    
    if (ivPattern != IVPattern::CUSTOM)
    {
      BOOL shouldCheckHiddenPower = NO;
      
      switch (ivPattern)
      {
      case IVPattern::HEX_FLAWLESS:
        [self setMinIVs: IVs::Perfect];
        [self setMaxIVs: IVs::Perfect];
        break;
        
      case IVPattern::HEX_FLAWLESS_TRICK:
        [self setMinIVs: IVs::PerfectTrick];
        [self setMaxIVs: IVs::PerfectTrick];
        break;
        
      case IVPattern::PHYSICAL_FLAWLESS:
        [self setMinIVs: IVs::PhysPerfect];
        [self setMaxIVs: IVs::Perfect];
        break;
        
      case IVPattern::PHYSICAL_FLAWLESS_TRICK:
        [self setMinIVs: IVs::PhysPerfectTrick];
        [self setMaxIVs: IVs::PerfectTrick];
        break;
        
      case IVPattern::SPECIAL_FLAWLESS:
        [self setMinIVs: IVs::SpecPerfect];
        [self setMaxIVs: IVs::Perfect];
        break;
        
      case IVPattern::SPECIAL_FLAWLESS_TRICK:
        [self setMinIVs: IVs::SpecPerfectTrick];
        [self setMaxIVs: IVs::PerfectTrick];
        break;
        
      case IVPattern::SPECIAL_HIDDEN_POWER_FLAWLESS:
        [self setMinIVs: IVs::HpPerfectLow];
        [self setMaxIVs: IVs::HpPerfectHigh];
        shouldCheckHiddenPower = YES;
        break;
        
      case IVPattern::SPECIAL_HIDDEN_POWER_FLAWLESS_TRICK:
        [self setMinIVs: IVs::HpPerfectTrickLow];
        [self setMaxIVs: IVs::HpPerfectTrickHigh];
        shouldCheckHiddenPower = YES;
        break;
      
      case IVPattern::CUSTOM:
      default:
        return;
        break;
      }
      
      self.considerHiddenPower = shouldCheckHiddenPower;
      if (shouldCheckHiddenPower)
        self.minHiddenPower = 70;
    }
    isSettingPattern = NO;
  }
}

- (void)updateIVPattern
{
  if (!isSettingPattern)
    self.ivPattern = IVPattern::Get([self minIVs], [self maxIVs],
                                    considerHiddenPower, minHiddenPower);
}

- (void)setMinHP:(uint32_t)newValue
{
  if (newValue != minHP)
  {
    minHP = newValue;
    
    if (maxHP < minHP)
    {
      self.maxHP = newValue;
    }
    
    [self updateIVPattern];
  }
}

- (void)setMinAT:(uint32_t)newValue
{
  if (newValue != minAT)
  {
    minAT = newValue;
    
    if (maxAT < minAT)
    {
      self.maxAT = newValue;
    }
    
    [self updateIVPattern];
  }
}

- (void)setMinDF:(uint32_t)newValue
{
  if (newValue != minDF)
  {
    minDF = newValue;
    
    if (maxDF < minDF)
    {
      self.maxDF = newValue;
    }
    
    [self updateIVPattern];
  }
}

- (void)setMinSA:(uint32_t)newValue
{
  if (newValue != minSA)
  {
    minSA = newValue;
    
    if (maxSA < minSA)
    {
      self.maxSA = newValue;
    }
    
    [self updateIVPattern];
  }
}

- (void)setMinSD:(uint32_t)newValue
{
  if (newValue != minSD)
  {
    minSD = newValue;
    
    if (maxSD < minSD)
    {
      self.maxSD = newValue;
    }
    
    [self updateIVPattern];
  }
}

- (void)setMinSP:(uint32_t)newValue
{
  if (newValue != minSP)
  {
    minSP = newValue;
    
    if (maxSP < minSP)
    {
      self.maxSP = newValue;
    }
    
    [self updateIVPattern];
  }
}

- (IVs)minIVs
{
  IVs  result;
  
  result.hp(minHP);
  result.at(minAT);
  result.df(minDF);
  result.sa(minSA);
  result.sd(minSD);
  result.sp(minSP);
  
  return result;
}

- (void)setMinIVs:(IVs)ivs
{
  self.minHP = ivs.hp();
  self.minAT = ivs.at();
  self.minDF = ivs.df();
  self.minSA = ivs.sa();
  self.minSD = ivs.sd();
  self.minSP = ivs.sp();
}

- (void)setMaxHP:(uint32_t)newValue
{
  if (newValue != maxHP)
  {
    maxHP = newValue;
    
    if (maxHP < minHP)
    {
      self.minHP = newValue;
    }
    
    [self updateIVPattern];
  }
}

- (void)setMaxAT:(uint32_t)newValue
{
  if (newValue != maxAT)
  {
    maxAT = newValue;
    
    if (maxAT < minAT)
    {
      self.minAT = newValue;
    }
    
    [self updateIVPattern];
  }
}

- (void)setMaxDF:(uint32_t)newValue
{
  if (newValue != maxDF)
  {
    maxDF = newValue;
    
    if (maxDF < minDF)
    {
      self.minDF = newValue;
    }
    
    [self updateIVPattern];
  }
}

- (void)setMaxSA:(uint32_t)newValue
{
  if (newValue != maxSA)
  {
    maxSA = newValue;
    
    if (maxSA < minSA)
    {
      self.minSA = newValue;
    }
    
    [self updateIVPattern];
  }
}

- (void)setMaxSD:(uint32_t)newValue
{
  if (newValue != maxSD)
  {
    maxSD = newValue;
    
    if (maxSD < minSD)
    {
      self.minSD = newValue;
    }
    
    [self updateIVPattern];
  }
}

- (void)setMaxSP:(uint32_t)newValue
{
  if (newValue != maxSP)
  {
    maxSP = newValue;
    
    if (maxSP < minSP)
    {
      self.minSP = newValue;
    }
    
    [self updateIVPattern];
  }
}

- (IVs)maxIVs
{
  IVs  result;
  
  result.hp(maxHP);
  result.at(maxAT);
  result.df(maxDF);
  result.sa(maxSA);
  result.sd(maxSD);
  result.sp(maxSP);
  
  return result;
}

- (void)setMaxIVs:(IVs)ivs
{
  self.maxHP = ivs.hp();
  self.maxAT = ivs.at();
  self.maxDF = ivs.df();
  self.maxSA = ivs.sa();
  self.maxSD = ivs.sd();
  self.maxSP = ivs.sp();
}

- (void)setConsiderHiddenPower:(BOOL)newValue
{
  if (considerHiddenPower != newValue)
  {
    considerHiddenPower = newValue;
    
    [self updateIVPattern];
  }
}

- (uint32_t)hiddenTypeMask
{
  return considerHiddenPower ?
    ((hiddenTypeMask == 0) ? 0xffff : hiddenTypeMask) :
    0;
}

- (void)setHiddenTypeMask:(uint32_t)newMask
{
  if (hiddenTypeMask != newMask)
  {
    hiddenTypeMask = newMask;
    SetComboMenuBitMask(hpTypeDropDown, newMask << 1);
    
    [self updateIVPattern];
  }
}

- (void)setMinHiddenPower:(uint32_t)newPower
{
  if (minHiddenPower != newPower)
  {
    minHiddenPower = newPower;
    
    [self updateIVPattern];
  }
}

- (IBAction)handleHPTypeDropDownChoice:(id)sender
{
  HandleComboMenuItemChoice(sender);
  self.hiddenTypeMask = GetComboMenuBitMask(sender) >> 1;
}

- (uint32_t)numberOfIVCombinations
{
  IVs  minIVs = [self minIVs];
  IVs  maxIVs = [self maxIVs];
  
  uint32_t  result = (maxIVs.hp() - minIVs.hp() + 1) *
                     (maxIVs.at() - minIVs.at() + 1) *
                     (maxIVs.df() - minIVs.df() + 1) *
                     (maxIVs.sa() - minIVs.sa() + 1) *
                     (maxIVs.sd() - minIVs.sd() + 1) *
                     (maxIVs.sp() - minIVs.sp() + 1);
  return result;
}

@end
