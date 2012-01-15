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

#import "HashedSeedInspectorEggsTabController.h"

#include "HashedSeed.h"
#include "FrameGenerator.h"
#include "Utilities.h"

using namespace pprng;

static
NSString* GetEggIV(Gen5BreedingFrame::Inheritance inheritance, uint32_t iv,
                   bool showIvs, uint32_t femaleIV, uint32_t maleIV,
                   bool showParentIVs)
{
  switch (inheritance)
  {
  default:
  case Gen5BreedingFrame::NotInherited:
    if (showIvs)
      return [NSString stringWithFormat:@"%d", iv];
    else
      return @"";
  case Gen5BreedingFrame::ParentX:
    if (showParentIVs)
      return [NSString stringWithFormat:@"%d", femaleIV];
    else
      return @"♀";
    break;
  case Gen5BreedingFrame::ParentY:
    if (showParentIVs)
      return [NSString stringWithFormat:@"%d", maleIV];
    else
      return @"♂";
    break;
  }
}

IVs GetEggIVs(const Gen5BreedingFrame &frame, IVs eggIVs,
              IVs femaleIVs, IVs maleIVs)
{
  uint32_t  i;
  
  for (i = 0; i < 6; ++i)
  {
    switch (frame.inheritance[i])
    {
    case Gen5BreedingFrame::ParentX:
      eggIVs.setIV(i, femaleIVs.iv(i));
      break;
      
    case Gen5BreedingFrame::ParentY:
      eggIVs.setIV(i, maleIVs.iv(i));
      break;
      
    default:
    case Gen5BreedingFrame::NotInherited:
      break;
    }
  }
  
  return eggIVs;
}

static
NSString* GetEggHiddenPowers(Gen5BreedingFrame::Inheritance inheritance[6],
                             IVs ivs, bool showIvs)
{
  if (!showIvs)
    return @"";
  
  uint32_t  inheritedIdx[3];
  uint32_t  i, j = 0;
  
  for (i = 0; i < 6; ++i)
  {
    if (inheritance[i] != Gen5BreedingFrame::NotInherited)
    {
      inheritedIdx[j++] = i;
    }
  }
  
  uint32_t  k;
  uint32_t  hps = 0;
  for (i = 30; i < 32; ++i)
  {
    for (j = 30; j < 32; ++j)
    {
      for (k = 30; k < 32; ++k)
      {
        uint32_t  baseIVs[6] =
          { ivs.hp(), ivs.at(), ivs.df(), ivs.sa(), ivs.sd(), ivs.sp() };
        
        baseIVs[inheritedIdx[0]] = i;
        baseIVs[inheritedIdx[1]] = j;
        baseIVs[inheritedIdx[2]] = k;
        IVs temp;
        temp.hp(baseIVs[0]);
        temp.at(baseIVs[1]);
        temp.df(baseIVs[2]);
        temp.sa(baseIVs[3]);
        temp.sd(baseIVs[4]);
        temp.sp(baseIVs[5]);
        
        if (temp.HiddenPower() == 70)
          hps |= 0x1 << temp.HiddenType();
      }
    }
  }
  
  NSString  *result = [NSString string];
  i = 0;
  while (hps != 0)
  {
    if (hps & 0x1)
    {
      result =
        [result stringByAppendingString:
          [NSString stringWithFormat: @"%s ",
            Element::ToString(Element::Type(i)).c_str()]];
    }
    
    hps >>= 1;
    ++i;
  }
  
  return result;
}

@implementation HashedSeedInspectorEggsTabController

@synthesize enableParentIVs;
@synthesize femaleHP, femaleAT, femaleDF, femaleSA, femaleSD, femaleSP;
@synthesize maleHP, maleAT, maleDF, maleSA, maleSD, maleSP;

- (void)awakeFromNib
{
  [[[[eggsTableView tableColumnWithIdentifier: @"pid"] dataCell] formatter]
   setFormatWidth: 8];
  
  enableParentIVs = NO;
}

- (IBAction)toggleUseInitialPID:(id)sender
{
  BOOL enabled = [eggsUseInitialPIDButton state];
  [eggsMinPIDFrameField setEnabled: !enabled];
}

- (IBAction)toggleEggIVs:(id)sender
{
  BOOL enabled = [eggsEnableIVsButton state];
  [eggsIVFrameField setEnabled: enabled];
}

- (IBAction)generateEggs:(id)sender
{
  if ([[seedField stringValue] length] == 0)
  {
    return;
  }
  
  [eggsContentArray setContent: [NSMutableArray array]];
  
  HashedSeed  seed([[seedField objectValue] unsignedLongLongValue]);
  
  bool      showIVs = [eggsEnableIVsButton state];
  uint32_t  ivFrame = showIVs ? [eggsIVFrameField intValue] : 0;
  uint32_t  frameNum = 0;
  
  HashedIVFrameGenerator  ivGenerator(seed, HashedIVFrameGenerator::Normal);
  while (frameNum++ < ivFrame)
    ivGenerator.AdvanceFrame();
  
  IVs  ivs = ivGenerator.CurrentFrame().ivs;
  IVs  femaleIVs, maleIVs;
  
  if (enableParentIVs)
  {
    femaleIVs.hp([femaleHP intValue]);
    femaleIVs.at([femaleAT intValue]);
    femaleIVs.df([femaleDF intValue]);
    femaleIVs.sa([femaleSA intValue]);
    femaleIVs.sd([femaleSD intValue]);
    femaleIVs.sp([femaleSP intValue]);
    
    maleIVs.hp([maleHP intValue]);
    maleIVs.at([maleAT intValue]);
    maleIVs.df([maleDF intValue]);
    maleIVs.sa([maleSA intValue]);
    maleIVs.sd([maleSD intValue]);
    maleIVs.sp([maleSP intValue]);
  }
  
  FemaleParent::Type femaleSpecies =
    FemaleParent::Type([[eggsFemaleSpeciesPopup selectedItem] tag]);
  uint32_t  minPIDFrame = [eggsUseInitialPIDButton state] ?
                          (seed.GetSkippedPIDFrames() + 1) :
                          [eggsMinPIDFrameField intValue];
  uint32_t  maxPIDFrame = [eggsMaxPIDFrameField intValue];
  uint32_t  limitFrame = minPIDFrame - 1;
  
  Gen5BreedingFrameGenerator::Parameters  p;
  p.usingEverstone = [eggsUseEverstoneButton state];
  p.usingDitto = [eggsUseDittoButton state];
  p.internationalParents = [eggsInternationalButton state];
  p.tid = [gen5ConfigController tid];
  p.sid = [gen5ConfigController sid];
  Gen5BreedingFrameGenerator  generator(seed, p);
  
  frameNum = 0;
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
    
    Gen5BreedingFrame  frame = generator.CurrentFrame();
    uint32_t           genderValue = frame.pid.GenderValue();
    id                 hiddenType = @"";
    id                 hiddenPower = @"";
    id                 characteristic = @"";
    
    if (showIVs && enableParentIVs)
    {
      IVs  eggIVs = GetEggIVs(frame, ivs, femaleIVs, maleIVs);
      
      characteristic =
        [NSString stringWithFormat: @"%s",
          Characteristic::ToString
            (Characteristic::Get(frame.pid, eggIVs)).c_str()];
      hiddenType = [NSString stringWithFormat: @"%s",
                     Element::ToString(eggIVs.HiddenType()).c_str()];
      hiddenPower = [NSNumber numberWithUnsignedInt: eggIVs.HiddenPower()];
    }
    
    NSMutableDictionary  *result =
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
				[NSNumber numberWithUnsignedInt: frame.number], @"frame",
        ((p.usingEverstone && frame.everstoneActivated) ? @"<ES>" :
          [NSString stringWithFormat: @"%s",
            Nature::ToString(frame.nature).c_str()]), @"nature",
        [NSNumber numberWithUnsignedInt: frame.pid.word], @"pid",
        (frame.pid.IsShiny(p.tid, p.sid) ? @"★" : @""), @"shiny",
        ((!p.usingDitto && frame.inheritsHiddenAbility) ? @"Y" : @""),
          @"dreamWorld",
        [NSNumber numberWithUnsignedInt: frame.pid.Gen5Ability()], @"ability",
        ((genderValue < 31) ? @"♀" : @"♂"), @"gender18",
        ((genderValue < 63) ? @"♀" : @"♂"), @"gender14",
        ((genderValue < 127) ? @"♀" : @"♂"), @"gender12",
        ((genderValue < 191) ? @"♀" : @"♂"), @"gender34",
        GetEggIV(frame.inheritance[0], ivs.hp(), showIVs,
                 femaleIVs.hp(), maleIVs.hp(), enableParentIVs), @"hp",
        GetEggIV(frame.inheritance[1], ivs.at(), showIVs,
                 femaleIVs.at(), maleIVs.at(), enableParentIVs), @"atk",
        GetEggIV(frame.inheritance[2], ivs.df(), showIVs,
                 femaleIVs.df(), maleIVs.df(), enableParentIVs), @"def",
        GetEggIV(frame.inheritance[3], ivs.sa(), showIVs,
                 femaleIVs.sa(), maleIVs.sa(), enableParentIVs), @"spa",
        GetEggIV(frame.inheritance[4], ivs.sd(), showIVs,
                 femaleIVs.sd(), maleIVs.sd(), enableParentIVs), @"spd",
        GetEggIV(frame.inheritance[5], ivs.sp(), showIVs,
                 femaleIVs.sp(), maleIVs.sp(), enableParentIVs), @"spe",
        hiddenType, @"hiddenType",
        hiddenPower, @"hiddenPower",
        characteristic, @"characteristic",
        SpeciesString(femaleSpecies, frame.species), @"species",
        nil];
    
    [rowArray addObject: result];
  }
  
  [eggsContentArray addObjects: rowArray];
}


@end
