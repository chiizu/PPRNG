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



#import "HashedSeedInspectorController.h"

#include "HashedSeed.h"
#include "FrameGenerator.h"
#include "Utilities.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>

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

static
NSString* GetEggCharacteristic(const Gen5BreedingFrame &frame, IVs eggIVs,
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
  
  return [NSString stringWithFormat: @"%s",
    Characteristic::ToString(Characteristic::Get(frame.pid, eggIVs)).c_str()];
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


@implementation HashedSeedInspectorController

- (NSString *)windowNibName
{
	return @"HashedSeedInspector";
}

- (void)awakeFromNib
{
  [super awakeFromNib];
  
  [[seedField formatter] setFormatWidth: 16];
  [startDate setObjectValue: [NSDate date]];
}

- (IBAction)calculateSeed:(id)sender
{
  using namespace boost::gregorian;
  using namespace boost::posix_time;
  
  const char *dstr = [[[startDate objectValue] description] UTF8String];
  date  d(boost::lexical_cast<uint32_t>(std::string(dstr, 4)),
          boost::lexical_cast<uint32_t>(std::string(dstr + 5, 2)),
          boost::lexical_cast<uint32_t>(std::string(dstr + 8, 2)));
  
  time_duration t
    (hours([startHour intValue]) +
     minutes([startMinute intValue]) +
     seconds([startSecond intValue]));
  
  uint32_t  macAddressLow = [gen5ConfigController macAddressLow];
  uint32_t  macAddressHigh = [gen5ConfigController macAddressHigh];
  
  Game::Version  version = [gen5ConfigController version];
  
  uint32_t  timer0 = [timer0Field intValue];
  uint32_t  vcount = [vcountField intValue];
  uint32_t  vframe = [vframeField intValue];
  
  uint32_t  pressedKeys = [[key1Menu selectedItem] tag] |
                          [[key2Menu selectedItem] tag] |
                          [[key3Menu selectedItem] tag];
  
  HashedSeed  seed(d.year(), d.month(), d.day(), d.day_of_week(),
                   t.hours(), t.minutes(), t.seconds(),
                   macAddressLow, macAddressHigh,
                   HashedSeed::NazoForVersion(version), 0, 0, 0,
                   vcount, timer0, HashedSeed::GxStat, vframe, pressedKeys);
  
  currentSeed = [NSData dataWithBytes: &seed length: sizeof(HashedSeed)];
  
  [seedField setObjectValue:
    [NSNumber numberWithUnsignedLongLong: seed.m_rawSeed]];
}


- (IBAction)generatePIDFrames:(id)sender
{
  if ([[seedField stringValue] length] == 0)
  {
    return;
  }
  
  [pidFrameContentArray setContent: [NSMutableArray array]];
  
  HashedSeed  seed([[seedField objectValue] unsignedLongLongValue]);
  
  uint32_t  minPIDFrame = [minPIDFrameField intValue];
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
  bool  generatesCanFish = generator.GeneratesCanFish();
  bool  generatesFindItem = generator.GeneratesFindItem();
  
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
    
    NSMutableDictionary  *result =
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
				[NSNumber numberWithUnsignedInt: frame.number], @"frame",
        [NSString stringWithFormat: @"%s",
          Nature::ToString(frame.nature).c_str()], @"nature",
        [NSNumber numberWithUnsignedInt: frame.pid.word], @"pid",
        frame.pid.IsShiny(tid, sid) ? @"!!!" : @"", @"shiny",
        [NSNumber numberWithUnsignedInt: frame.pid.Gen5Ability()], @"ability",
        (generatesESV ? [NSString stringWithFormat: @"%d", frame.esv] : @""),
          @"esv",
        (frame.synched ? @"Y" : @""), @"sync",
        ((generatesCanFish && frame.canFish) ? @"Y" : @""), @"canFish",
        ((generatesFindItem && frame.findItem) ? @"Y" : @""), @"findItem",
        GenderString(frame.pid), @"gender",
        HeldItemString(frame.heldItem), @"heldItem",
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


- (IBAction)generateAdjacents:(id)sender
{
  using namespace boost::gregorian;
  using namespace boost::posix_time;
  
  if (currentSeed == nil)
  {
    return;
  }
  
  HashedSeed  targetSeed;
  [currentSeed getBytes: &targetSeed length: sizeof(HashedSeed)];
  
  if (targetSeed.m_rawSeed != [[seedField objectValue] unsignedLongLongValue])
  {
    return;
  }
  
  [adjacentsContentArray setContent: [NSMutableArray array]];
  
  uint32_t  tid = [gen5ConfigController tid];
  uint32_t  sid = [gen5ConfigController sid];
  uint32_t  timer0Low = [gen5ConfigController timer0Low];
  uint32_t  timer0High = [gen5ConfigController timer0High];
  
  if ((targetSeed.m_timer0 < timer0Low) || (targetSeed.m_timer0 > timer0High))
  {
    timer0Low = targetSeed.m_timer0 - 1;
    timer0High = targetSeed.m_timer0 + 1;
  }
  
  uint32_t  secondVariance = [adjacentsTimeVarianceField intValue];
  
  ptime     dt(date(targetSeed.m_year, targetSeed.m_month, targetSeed.m_day),
               hours(targetSeed.m_hour) + minutes(targetSeed.m_minute) +
               seconds(targetSeed.m_second));
  ptime     endTime = dt + seconds(secondVariance);
  dt = dt - seconds(secondVariance);
  
  uint32_t  ivFrameNum = [adjacentsIVFrameField intValue];
  bool      isRoamer = [adjacentsRoamerButton state];
  
  uint32_t  pidFrameNum = [adjacentsPIDFrameField intValue];
  uint32_t  pidFrameVariance = [adjacentsPIDFrameVarianceField intValue];
  uint32_t  pidStartFrameNum;
  if (pidFrameNum < pidFrameVariance)
  {
    pidStartFrameNum = 1;
  }
  else
  {
    pidStartFrameNum = pidFrameNum - pidFrameVariance;
  }
  uint32_t  pidEndFrameNum = pidFrameNum + pidFrameVariance;
  
  Gen5PIDFrameGenerator::FrameType  frameType =
    static_cast<Gen5PIDFrameGenerator::FrameType>
      ([[adjacentsPIDFrameTypeMenu selectedItem] tag]);
  
  NSMutableArray  *rowArray =
    [NSMutableArray arrayWithCapacity:
      (timer0High - timer0Low + 1) * ((2 * secondVariance) + 1)];
  
  for (; dt <= endTime; dt = dt + seconds(1))
  {
    date           d = dt.date();
    time_duration  t = dt.time_of_day();
    
    NSString  *dateStr =
      [NSString stringWithFormat: @"%.4d/%.2d/%.2d",
                uint32_t(d.year()), uint32_t(d.month()), uint32_t(d.day())];
    NSString  *timeStr = [NSString stringWithFormat:@"%.2d:%.2d:%.2d",
                           t.hours(), t.minutes(), t.seconds()];
    
    for (uint32_t timer0 = timer0Low; timer0 <= timer0High; ++timer0)
    {
      HashedSeed  seed(d.year(), d.month(), d.day(), d.day_of_week(),
                       t.hours(), t.minutes(), t.seconds(),
                       targetSeed.m_macAddressLow, targetSeed.m_macAddressHigh,
                       targetSeed.m_nazo, 0, 0, 0,
                       targetSeed.m_vcount, timer0, HashedSeed::GxStat,
                       targetSeed.m_vframe, targetSeed.m_keyInput);
      
      HashedIVFrameGenerator  ivGenerator(seed,
                                          (isRoamer ?
                                           HashedIVFrameGenerator::Roamer :
                                           HashedIVFrameGenerator::Normal));
      
      for (uint32_t j = 0; j < ivFrameNum; ++j)
        ivGenerator.AdvanceFrame();
      
      IVs  ivs = ivGenerator.CurrentFrame().ivs;
      
      Gen5PIDFrameGenerator  pidGenerator(seed, frameType, false, tid, sid);
      
      for (uint32_t j = 0; j < pidStartFrameNum; ++j)
        pidGenerator.AdvanceFrame();
      
      for (pidFrameNum = pidStartFrameNum;
           pidFrameNum <= pidEndFrameNum;
           ++pidFrameNum)
      {
        Gen5PIDFrame  frame = pidGenerator.CurrentFrame();
        
        [rowArray addObject:
        [NSMutableDictionary dictionaryWithObjectsAndKeys:
          dateStr, @"date",
          timeStr, @"time",
          [NSNumber numberWithUnsignedInt: timer0], @"timer0",
          [NSNumber numberWithUnsignedInt: pidFrameNum], @"pidFrame",
          [NSString stringWithFormat: @"%s",
            Nature::ToString(frame.nature).c_str()], @"nature",
          [NSNumber numberWithUnsignedInt: frame.pid.Gen5Ability()], @"ability",
          GenderString(frame.pid), @"gender",
            @"gender",
          [NSNumber numberWithUnsignedInt: ivs.hp()], @"hp",
          [NSNumber numberWithUnsignedInt: ivs.at()], @"atk",
          [NSNumber numberWithUnsignedInt: ivs.df()], @"def",
          [NSNumber numberWithUnsignedInt: ivs.sa()], @"spa",
          [NSNumber numberWithUnsignedInt: ivs.sd()], @"spd",
          [NSNumber numberWithUnsignedInt: ivs.sp()], @"spe",
          nil]];
        
        pidGenerator.AdvanceFrame();
      }
    }
  }
  
  [adjacentsContentArray addObjects: rowArray];
}

- (IBAction)toggleEggIVs:(id)sender
{
  BOOL enabled = [eggsEnableIVsButton state];
  [eggsIVFrameField setEnabled: enabled];
}

- (IBAction)toggleParentIVs:(id)sender
{
  BOOL enabled = [eggsEnableParentIVsCheckBox state];
  
  [eggsFemaleHPField setEnabled: enabled];
  [eggsFemaleHPStepper setEnabled: enabled];
  [eggsFemaleAtkField setEnabled: enabled];
  [eggsFemaleAtkStepper setEnabled: enabled];
  [eggsFemaleDefField setEnabled: enabled];
  [eggsFemaleDefStepper setEnabled: enabled];
  [eggsFemaleSpAField setEnabled: enabled];
  [eggsFemaleSpAStepper setEnabled: enabled];
  [eggsFemaleSpDField setEnabled: enabled];
  [eggsFemaleSpDStepper setEnabled: enabled];
  [eggsFemaleSpeField setEnabled: enabled];
  [eggsFemaleSpeStepper setEnabled: enabled];
  
  [eggsMaleHPField setEnabled: enabled];
  [eggsMaleHPStepper setEnabled: enabled];
  [eggsMaleAtkField setEnabled: enabled];
  [eggsMaleAtkStepper setEnabled: enabled];
  [eggsMaleDefField setEnabled: enabled];
  [eggsMaleDefStepper setEnabled: enabled];
  [eggsMaleSpAField setEnabled: enabled];
  [eggsMaleSpAStepper setEnabled: enabled];
  [eggsMaleSpDField setEnabled: enabled];
  [eggsMaleSpDStepper setEnabled: enabled];
  [eggsMaleSpeField setEnabled: enabled];
  [eggsMaleSpeStepper setEnabled: enabled];
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
  
  bool      parentIVs = [eggsEnableParentIVsCheckBox state];
  IVs       femaleIVs, maleIVs;
  if (parentIVs)
  {
    femaleIVs.hp([eggsFemaleHPField intValue]);
    femaleIVs.at([eggsFemaleAtkField intValue]);
    femaleIVs.df([eggsFemaleDefField intValue]);
    femaleIVs.sa([eggsFemaleSpAField intValue]);
    femaleIVs.sd([eggsFemaleSpDField intValue]);
    femaleIVs.sp([eggsFemaleSpeField intValue]);
    
    maleIVs.hp([eggsMaleHPField intValue]);
    maleIVs.at([eggsMaleAtkField intValue]);
    maleIVs.df([eggsMaleDefField intValue]);
    maleIVs.sa([eggsMaleSpAField intValue]);
    maleIVs.sd([eggsMaleSpDField intValue]);
    maleIVs.sp([eggsMaleSpeField intValue]);
  }
  
  uint32_t  tid = [gen5ConfigController tid];
  uint32_t  sid = [gen5ConfigController sid];
  bool      isInternational = [eggsInternationalButton state];
  bool      hasDitto = [eggsUseDittoButton state];
  bool      hasEverstone = [eggsUseEverstoneButton state];
  uint32_t  minPIDFrame = [eggsMinPIDFrameField intValue];
  uint32_t  maxPIDFrame = [eggsMaxPIDFrameField intValue];
  uint32_t  limitFrame = minPIDFrame - 1;
  
  Gen5BreedingFrameGenerator  generator
      (seed, isInternational, hasEverstone, hasDitto, tid, sid);
  
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
    
    NSMutableDictionary  *result =
      [NSMutableDictionary dictionaryWithObjectsAndKeys:
				[NSNumber numberWithUnsignedInt: frame.number], @"frame",
        ((hasEverstone && frame.everstoneActivated) ? @"<ES>" :
          [NSString stringWithFormat: @"%s",
            Nature::ToString(frame.nature).c_str()]), @"nature",
        [NSNumber numberWithUnsignedInt: frame.pid.word], @"pid",
        (frame.pid.IsShiny(tid, sid) ? @"!!!" : @""), @"shiny",
        ((!hasDitto && frame.dreamWorldAbilityPassed) ? @"Y" : @""),
          @"dreamWorld",
        [NSNumber numberWithUnsignedInt: frame.pid.Gen5Ability()], @"ability",
        GenderString(frame.pid), @"gender",
        GetEggIV(frame.inheritance[0], ivs.hp(), showIVs,
                 femaleIVs.hp(), maleIVs.hp(), parentIVs), @"hp",
        GetEggIV(frame.inheritance[1], ivs.at(), showIVs,
                 femaleIVs.at(), maleIVs.at(), parentIVs), @"atk",
        GetEggIV(frame.inheritance[2], ivs.df(), showIVs,
                 femaleIVs.df(), maleIVs.df(), parentIVs), @"def",
        GetEggIV(frame.inheritance[3], ivs.sa(), showIVs,
                 femaleIVs.sa(), maleIVs.sa(), parentIVs), @"spa",
        GetEggIV(frame.inheritance[4], ivs.sd(), showIVs,
                 femaleIVs.sd(), maleIVs.sd(), parentIVs), @"spd",
        GetEggIV(frame.inheritance[5], ivs.sp(), showIVs,
                 femaleIVs.sp(), maleIVs.sp(), parentIVs), @"spe",
        ((showIVs && parentIVs) ? 
            GetEggCharacteristic(frame, ivs, femaleIVs, maleIVs) : @""),
          @"characteristic",
        nil];
    
    [rowArray addObject: result];
  }
  
  [eggsContentArray addObjects: rowArray];
}

- (void)setSeed:(NSData*)seedData
{
  if (seedData != currentSeed)
  {
    currentSeed = seedData;
    
    HashedSeed  seed;
    [currentSeed getBytes: &seed length: sizeof(HashedSeed)];
    
    NSDate  *now = [NSDate date];
    NSRange  timeZoneOffsetRange;
    timeZoneOffsetRange.location = 20;
    timeZoneOffsetRange.length = 5;
    NSString  *dateTime =
      [NSString stringWithFormat: @"%.4d-%.2d-%.2d %.2d:%.2d:%.2d %@",
        seed.m_year, seed.m_month, seed.m_day,
        seed.m_hour, seed.m_minute, seed.m_second,
        [[now description] substringWithRange: timeZoneOffsetRange]];
    
    [startDate setObjectValue: [NSDate dateWithString: dateTime]];
    [startHour setIntValue: seed.m_hour];
    [startMinute setIntValue: seed.m_minute];
    [startSecond setIntValue: seed.m_second];
    
    [timer0Field setIntValue: seed.m_timer0];
    [vcountField setIntValue: seed.m_vcount];
    [vframeField setIntValue: seed.m_vframe];
    
    NSPopUpButton  *keyMenu[3] = { key1Menu, key2Menu, key3Menu };
    uint32_t  i = 0;
    uint32_t  dpadPress = seed.m_keyInput & Button::DPAD_MASK;
    uint32_t  buttonPress = seed.m_keyInput & Button::SINGLE_BUTTON_MASK;
    
    if (dpadPress != 0)
    {
      [key1Menu selectItemWithTag: dpadPress];
      ++i;
    }
    
    uint32_t  j = 1;
    while ((buttonPress != 0) && (i < 3))
    {
      if (buttonPress & 0x1)
      {
        [keyMenu[i++] selectItemWithTag: j];
      }
      
      buttonPress >>= 1;
      j <<= 1;
    }
    
    [seedField setObjectValue:
      [NSNumber numberWithUnsignedLongLong: seed.m_rawSeed]];
  }
}

@end
