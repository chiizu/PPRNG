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

#import "HashedSeedInspectorEggsTabController.h"

#import "HashedSeedInspectorController.h"
#import "SearchResultProtocols.h"

#include "HashedSeed.h"
#include "FrameGenerator.h"
#include "Utilities.h"

using namespace pprng;

static
NSString* GetEggIV(Gen5BreedingFrame::Inheritance inheritance, uint32_t iv,
                   bool showIvs, bool isFemaleSet, uint32_t femaleIV,
                   bool isMaleSet, uint32_t maleIV, bool showParentIVs)
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
    if (showParentIVs && isFemaleSet)
      return [NSString stringWithFormat:@"%d", femaleIV];
    else
      return @"♀";
    break;
  case Gen5BreedingFrame::ParentY:
    if (showParentIVs && isMaleSet)
      return [NSString stringWithFormat:@"%d", maleIV];
    else
      return @"♂";
    break;
  }
}

static
OptionalIVs GetEggIVs(const Gen5BreedingFrame &frame, IVs baseIVs,
                      const OptionalIVs &femaleIVs, const OptionalIVs &maleIVs)
{
  OptionalIVs  eggIVs;
  uint32_t     i;
  
  for (i = 0; i < 6; ++i)
  {
    switch (frame.inheritance[i])
    {
    case Gen5BreedingFrame::ParentX:
      if (femaleIVs.isSet(i))
        eggIVs.setIV(i, femaleIVs.values.iv(i));
      break;
      
    case Gen5BreedingFrame::ParentY:
      if (maleIVs.isSet(i))
        eggIVs.setIV(i, maleIVs.values.iv(i));
      break;
      
    default:
    case Gen5BreedingFrame::NotInherited:
      eggIVs.setIV(i, baseIVs.iv(i));
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


@interface HashedSeedInspectorEggFrame : NSObject <PIDResult>
{
  uint32_t              frame;
  uint32_t              chatotPitch;
  DECLARE_PID_RESULT_VARIABLES();
  BOOL                  inheritsHiddenAbility;
  NSString              *hp, *atk, *def, *spa, *spd, *spe;
  Element::Type         hiddenType;
  NSNumber              *hiddenPower;
  Characteristic::Type  characteristic;
  NSString              *species;
}

@property uint32_t              frame;
@property uint32_t              chatotPitch;
@property BOOL                  inheritsHiddenAbility;
@property (copy) NSString       *hp, *atk, *def, *spa, *spd, *spe;
@property Element::Type         hiddenType;
@property (copy) NSNumber       *hiddenPower;
@property Characteristic::Type  characteristic;
@property (copy) NSString       *species;

@end

@implementation HashedSeedInspectorEggFrame

@synthesize frame;
@synthesize chatotPitch;
SYNTHESIZE_PID_RESULT_PROPERTIES();
@synthesize inheritsHiddenAbility;
@synthesize hp, atk, def, spa, spd, spe;
@synthesize hiddenType, hiddenPower;
@synthesize characteristic;
@synthesize species;

@end


@implementation HashedSeedInspectorEggsTabController

@synthesize internationalParents;
@synthesize usingEverstone;
@synthesize usingDitto;

@synthesize enableIVs;
@synthesize ivFrame;

@synthesize startFromInitialPIDFrame;
@synthesize minPIDFrame, maxPIDFrame;

@synthesize femaleSpecies;

@synthesize enableParentIVs;
@synthesize femaleHP, femaleAT, femaleDF, femaleSA, femaleSD, femaleSP;
@synthesize maleHP, maleAT, maleDF, maleSA, maleSD, maleSP;

- (void)awakeFromNib
{
  self.internationalParents = NO;
  self.usingEverstone = NO;
  self.usingDitto = NO;
  self.enableIVs = NO;
  self.ivFrame = 8;
  self.startFromInitialPIDFrame = YES;
  self.minPIDFrame = 50;
  self.maxPIDFrame = 100;
  self.femaleSpecies = FemaleParent::OTHER;
  self.enableParentIVs = NO;
  self.femaleHP = nil;
  self.femaleAT = nil;
  self.femaleDF = nil;
  self.femaleSA = nil;
  self.femaleSD = nil;
  self.femaleSP = nil;
  self.maleHP = nil;
  self.maleAT = nil;
  self.maleDF = nil;
  self.maleSA = nil;
  self.maleSD = nil;
  self.maleSP = nil;
}

- (void)setFemaleIVs:(const pprng::OptionalIVs&)ivs
{
  self.femaleHP = ivs.isSet(IVs::HP) ?
    [NSNumber numberWithUnsignedInt: ivs.hp()] : nil;
  self.femaleAT = ivs.isSet(IVs::AT) ?
    [NSNumber numberWithUnsignedInt: ivs.at()] : nil;
  self.femaleDF = ivs.isSet(IVs::DF) ?
    [NSNumber numberWithUnsignedInt: ivs.df()] : nil;
  self.femaleSA = ivs.isSet(IVs::SA) ?
    [NSNumber numberWithUnsignedInt: ivs.sa()] : nil;
  self.femaleSD = ivs.isSet(IVs::SD) ?
    [NSNumber numberWithUnsignedInt: ivs.sd()] : nil;
  self.femaleSP = ivs.isSet(IVs::SP) ?
    [NSNumber numberWithUnsignedInt: ivs.sp()] : nil;
}

- (void)setMaleIVs:(const pprng::OptionalIVs&)ivs
{
  self.maleHP = ivs.isSet(IVs::HP) ?
    [NSNumber numberWithUnsignedInt: ivs.hp()] : nil;
  self.maleAT = ivs.isSet(IVs::AT) ?
    [NSNumber numberWithUnsignedInt: ivs.at()] : nil;
  self.maleDF = ivs.isSet(IVs::DF) ?
    [NSNumber numberWithUnsignedInt: ivs.df()] : nil;
  self.maleSA = ivs.isSet(IVs::SA) ?
    [NSNumber numberWithUnsignedInt: ivs.sa()] : nil;
  self.maleSD = ivs.isSet(IVs::SD) ?
    [NSNumber numberWithUnsignedInt: ivs.sd()] : nil;
  self.maleSP = ivs.isSet(IVs::SP) ?
    [NSNumber numberWithUnsignedInt: ivs.sp()] : nil;
}

- (IBAction)generateEggs:(id)sender
{
  if (!EndEditing([inspectorController window]))
    return;
  
  if (!inspectorController.rawSeed)
    return;
  
  [eggsContentArray setContent: [NSMutableArray array]];
  
  HashedSeed  seed([inspectorController.rawSeed unsignedLongLongValue]);
  
  uint32_t  ivFrameNum = enableIVs ? ivFrame : 0;
  uint32_t  frameNum = 0;
  
  HashedIVFrameGenerator  ivGenerator(seed, HashedIVFrameGenerator::Normal);
  while (frameNum++ < ivFrameNum)
    ivGenerator.AdvanceFrame();
  
  IVs  ivs = ivGenerator.CurrentFrame().ivs;
  OptionalIVs  femaleIVs, maleIVs;
  
  if (enableParentIVs)
  {
    if (femaleHP)
      femaleIVs.hp([femaleHP unsignedIntValue]);
    if (femaleAT)
      femaleIVs.at([femaleAT unsignedIntValue]);
    if (femaleDF)
      femaleIVs.df([femaleDF unsignedIntValue]);
    if (femaleSA)
      femaleIVs.sa([femaleSA unsignedIntValue]);
    if (femaleSD)
      femaleIVs.sd([femaleSD unsignedIntValue]);
    if (femaleSP)
      femaleIVs.sp([femaleSP unsignedIntValue]);
    
    if (maleHP)
      maleIVs.hp([maleHP unsignedIntValue]);
    if (maleAT)
      maleIVs.at([maleAT unsignedIntValue]);
    if (maleDF)
      maleIVs.df([maleDF unsignedIntValue]);
    if (maleSA)
      maleIVs.sa([maleSA unsignedIntValue]);
    if (maleSD)
      maleIVs.sd([maleSD unsignedIntValue]);
    if (maleSP)
      maleIVs.sp([maleSP unsignedIntValue]);
  }
  
  uint32_t  minFrameNum = startFromInitialPIDFrame ?
                          (seed.GetSkippedPIDFrames() + 1) :
                          minPIDFrame;
  uint32_t  limitFrame = minFrameNum - 1;
  
  Gen5BreedingFrameGenerator::Parameters  p;
  p.femaleSpecies = femaleSpecies;
  p.usingEverstone = usingEverstone;
  p.usingDitto = usingDitto;
  p.internationalParents = internationalParents;
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
    [NSMutableArray arrayWithCapacity: maxPIDFrame - minFrameNum + 1];
  
  while (frameNum < maxPIDFrame)
  {
    generator.AdvanceFrame();
    ++frameNum;
    
    Gen5BreedingFrame  frame = generator.CurrentFrame();
    
    HashedSeedInspectorEggFrame  *result =
      [[HashedSeedInspectorEggFrame alloc] init];
    
    result.frame = frame.number;
    result.chatotPitch = frame.chatotPitch;
    
    SetGen5PIDResult(result, frame.nature, frame.pid, p.tid, p.sid,
                     Gender::ANY, Gender::ANY_RATIO);
    result.inheritsHiddenAbility = frame.inheritsHiddenAbility;
    
    result.hp = GetEggIV(frame.inheritance[0], ivs.hp(), enableIVs,
                          femaleIVs.isSet(IVs::HP), femaleIVs.hp(),
                          maleIVs.isSet(IVs::HP), maleIVs.hp(), enableParentIVs);
    result.atk = GetEggIV(frame.inheritance[1], ivs.at(), enableIVs,
                          femaleIVs.isSet(IVs::AT), femaleIVs.at(),
                          maleIVs.isSet(IVs::AT), maleIVs.at(), enableParentIVs);
    result.def = GetEggIV(frame.inheritance[2], ivs.df(), enableIVs,
                          femaleIVs.isSet(IVs::DF), femaleIVs.df(),
                          maleIVs.isSet(IVs::DF), maleIVs.df(), enableParentIVs);
    result.spa = GetEggIV(frame.inheritance[3], ivs.sa(), enableIVs,
                          femaleIVs.isSet(IVs::SA), femaleIVs.sa(),
                          maleIVs.isSet(IVs::SA), maleIVs.sa(), enableParentIVs);
    result.spd = GetEggIV(frame.inheritance[4], ivs.sd(), enableIVs,
                          femaleIVs.isSet(IVs::SD), femaleIVs.sd(),
                          maleIVs.isSet(IVs::SD), maleIVs.sd(), enableParentIVs);
    result.spe = GetEggIV(frame.inheritance[5], ivs.sp(), enableIVs,
                          femaleIVs.isSet(IVs::SP), femaleIVs.sp(),
                          maleIVs.isSet(IVs::SP), maleIVs.sp(), enableParentIVs);
    
    result.hiddenType = Element::NONE;
    result.hiddenPower = nil;
    result.characteristic = Characteristic::NONE;
    
    if (enableIVs && enableParentIVs)
    {
      OptionalIVs  eggIVs = GetEggIVs(frame, ivs, femaleIVs, maleIVs);
      
      if (eggIVs.allSet())
      {
        result.characteristic = Characteristic::Get(frame.pid, eggIVs.values);
        result.hiddenType = eggIVs.values.HiddenType();
        result.hiddenPower =
          [NSNumber numberWithUnsignedInt: eggIVs.values.HiddenPower()];
      }
    }
    
    result.species = SpeciesString(frame.species);
    
    [rowArray addObject: result];
  }
  
  [eggsContentArray addObjects: rowArray];
}


- (void)selectAndShowEggFrame:(uint32_t)frame
{
  NSArray  *rows = [eggsContentArray arrangedObjects];
  if (rows && ([rows count] > 0))
  {
    HashedSeedInspectorEggFrame  *row = [rows objectAtIndex: 0];
    
    if (row.frame <= frame)
    {
      NSInteger  rowNum = frame - row.frame;
      
      [eggsTableView
        selectRowIndexes: [NSIndexSet indexSetWithIndex: rowNum]
        byExtendingSelection: NO];
      [eggsTableView scrollRowToVisible: rowNum];
    }
  }
}


@end
