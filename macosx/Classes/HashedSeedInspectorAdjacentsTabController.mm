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

#import "HashedSeedInspectorAdjacentsTabController.h"

#import "StandardSeedInspectorController.h"

#include "HashedSeed.h"
#include "FrameGenerator.h"
#include "SearchResultProtocols.h"
#include "Utilities.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>

using namespace pprng;


@interface HashedSeedInspectorAdjacentFrame :
  NSObject <HashedSeedResultParameters, IVResult, PIDResult>
{
  DECLARE_HASHED_SEED_RESULT_PARAMETERS_VARIABLES();
  
  DECLARE_IV_RESULT_VARIABLES();
  
  uint32_t              startFrame, pidFrame;
  DECLARE_PID_RESULT_VARIABLES();
  ESV::Value            esv;
  HeldItem::Type        heldItem;
  Characteristic::Type  characteristic;
  NSString              *details;
}

@property uint32_t              startFrame, pidFrame;
@property ESV::Value            esv;
@property HeldItem::Type        heldItem;
@property Characteristic::Type  characteristic;
@property (copy) NSString       *details;

@end

@implementation HashedSeedInspectorAdjacentFrame

SYNTHESIZE_HASHED_SEED_RESULT_PARAMETERS_PROPERTIES();
SYNTHESIZE_IV_RESULT_PROPERTIES();
@synthesize startFrame, pidFrame;
SYNTHESIZE_PID_RESULT_PROPERTIES();
@synthesize esv;
@synthesize heldItem;
@synthesize characteristic;
@synthesize details;

@end


@implementation HashedSeedInspectorAdjacentsTabController

@synthesize secondsVariance, timer0Variance;
@synthesize matchOffsetFromInitialPIDFrame;
@synthesize ivFrame, isRoamer;
@synthesize pidFrame, pidFrameVariance;
@synthesize encounterFrameType, encounterLeadAbility;
@synthesize genderRequired, genderlessAllowed, targetGender;
@synthesize genderRatioRequired, targetGenderRatio;
@synthesize isEntralink, cgearStartOffset;

- (void)checkGenderSettingsRequired
{
  self.genderRequired =
    ((encounterLeadAbility == EncounterLead::CUTE_CHARM) &&
     (encounterFrameType != Gen5PIDFrameGenerator::StarterFossilGiftFrame) &&
     (encounterFrameType != Gen5PIDFrameGenerator::RoamerFrame)) ||
    (encounterFrameType == Gen5PIDFrameGenerator::EntraLinkFrame) ||
    (encounterFrameType == Gen5PIDFrameGenerator::HiddenHollowFrame);
  
  if (!genderRequired)
    self.targetGender = Gender::GENDERLESS;
  else if (!isEntralink && (targetGender == Gender::GENDERLESS))
    self.targetGender = Gender::FEMALE;
  
  self.genderRatioRequired = genderRequired &&
                             (targetGender != Gender::GENDERLESS);
}

- (void)setEncounterFrameType:(Gen5PIDFrameGenerator::FrameType)newFrameType
{
  if (newFrameType != encounterFrameType)
  {
    encounterFrameType = newFrameType;
    self.isEntralink = (newFrameType == Gen5PIDFrameGenerator::EntraLinkFrame);
    self.genderlessAllowed = isEntralink ||
      (newFrameType == Gen5PIDFrameGenerator::HiddenHollowFrame);
    [self checkGenderSettingsRequired];
  }
}

- (void)setEncounterLeadAbility:(EncounterLead::Ability)newAbility
{
  if (newAbility != encounterLeadAbility)
  {
    encounterLeadAbility = newAbility;
    [self checkGenderSettingsRequired];
  }
}

- (void)setTargetGender:(Gender::Type)newGender
{
  if (newGender != targetGender)
  {
    targetGender = newGender;
    if (targetGender == Gender::GENDERLESS)
    {
      self.targetGenderRatio = Gender::ANY_RATIO;
      self.genderRatioRequired = NO;
    }
    else
    {
      self.genderRatioRequired = YES;
    }
  }
}

- (void)awakeFromNib
{
  self.secondsVariance = self.timer0Variance = 1;
  self.matchOffsetFromInitialPIDFrame = YES;
  self.ivFrame = 1;
  self.isRoamer = NO;
  self.pidFrame = 50;
  self.pidFrameVariance = 10;
  self.encounterFrameType = Gen5PIDFrameGenerator::GrassCaveFrame;
  self.encounterLeadAbility = EncounterLead::SYNCHRONIZE;
  self.targetGender = Gender::GENDERLESS;
  self.targetGenderRatio = Gender::NO_RATIO;
  self.cgearStartOffset = 3;
}

- (IBAction)generateAdjacents:(id)sender
{
  using namespace boost::gregorian;
  using namespace boost::posix_time;
  
  if (!EndEditing([inspectorController window]))
    return;
  
  if (!inspectorController.startDate || !inspectorController.startHour ||
      !inspectorController.startMinute || !inspectorController.startSecond ||
      !inspectorController.timer0 || !inspectorController.vcount ||
      !inspectorController.vframe)
    return;
  
  HashedSeed::Parameters  targetSeedParams;
  
  targetSeedParams.version = inspectorController.version;
  targetSeedParams.dsType = inspectorController.dsType;
  targetSeedParams.macAddress = [inspectorController macAddress];
  targetSeedParams.gxStat = HashedSeed::HardResetGxStat;
  targetSeedParams.vcount = [inspectorController.vcount unsignedIntValue];
  targetSeedParams.vframe = [inspectorController.vframe unsignedIntValue];
  targetSeedParams.timer0 = [inspectorController.timer0 unsignedIntValue];
  targetSeedParams.date = NSDateToBoostDate(inspectorController.startDate);
  targetSeedParams.hour = [inspectorController.startHour unsignedIntValue];
  targetSeedParams.minute = [inspectorController.startMinute unsignedIntValue];
  targetSeedParams.second = [inspectorController.startSecond unsignedIntValue];
  targetSeedParams.heldButtons = inspectorController.button1 |
                                 inspectorController.button2 |
                                 inspectorController.button3;
  HashedSeed  targetSeed(targetSeedParams);
  
  [adjacentsContentArray setContent: [NSMutableArray array]];
  
  uint32_t  timer0Low = targetSeedParams.timer0 - timer0Variance;
  uint32_t  timer0High = targetSeedParams.timer0 + timer0Variance;
  
  if (targetSeedParams.timer0 < timer0Variance)
    timer0Low = 0;
  
  if (targetSeedParams.timer0 > (0xffffffff - timer0Variance))
    timer0High = 0xffffffff;
  
  ptime     seedTime(targetSeedParams.date, hours(targetSeedParams.hour) +
                                            minutes(targetSeedParams.minute) +
                                            seconds(targetSeedParams.second));
  ptime     dt = seedTime;
  ptime     endTime = dt + seconds(secondsVariance);
  dt = dt - seconds(secondsVariance);
  
  uint32_t  pidFrameOffset = matchOffsetFromInitialPIDFrame ?
    (pidFrame -
     targetSeed.GetSkippedPIDFrames(inspectorController.memoryLinkUsed) - 1) :
    pidFrame;
  
  if (pidFrameOffset > pidFrame)
    pidFrameOffset = pidFrame;
  
  HashedSeed::Parameters  seedParams = targetSeedParams;
  
  Gen5PIDFrameGenerator::Parameters  pidFrameParams;
  pidFrameParams.frameType = encounterFrameType;
  pidFrameParams.leadAbility = encounterLeadAbility;
  pidFrameParams.targetGender = targetGender;
  
  pidFrameParams.targetRatio = genderRequired ?
    targetGenderRatio : Gender::ANY_RATIO;
  
  pidFrameParams.tid = [inspectorController.tid unsignedIntValue];
  pidFrameParams.sid = [inspectorController.sid unsignedIntValue];
  
  pidFrameParams.isBlack2White2 =
    Game::IsBlack2White2(inspectorController.version);
  pidFrameParams.hasShinyCharm = inspectorController.hasShinyCharm;
  pidFrameParams.memoryLinkUsed = inspectorController.memoryLinkUsed;
  
  pidFrameParams.startFromLowestFrame = matchOffsetFromInitialPIDFrame;
  
  NSMutableArray  *rowArray =
    [NSMutableArray arrayWithCapacity:
      (timer0High - timer0Low + 1) * ((2 * secondsVariance) + 1)];
  
  for (; dt <= endTime; dt = dt + seconds(1))
  {
    seedParams.date = dt.date();
    
    time_duration  t = dt.time_of_day();
    
    seedParams.hour = t.hours();
    seedParams.minute = t.minutes();
    seedParams.second = t.seconds();
    
    for (uint32_t timer0 = timer0Low; timer0 <= timer0High; ++timer0)
    {
      seedParams.timer0 = timer0;
      
      HashedSeed  seed(seedParams);
      
      HashedIVFrameGenerator  ivGenerator(seed,
                                          (isRoamer ?
                                           HashedIVFrameGenerator::Roamer :
                                           HashedIVFrameGenerator::Normal));
      
      for (uint32_t j = 0; j < ivFrame; ++j)
        ivGenerator.AdvanceFrame();
      
      IVs  ivs = ivGenerator.CurrentFrame().ivs;
      
      uint32_t  adjacentPIDFrameNum = matchOffsetFromInitialPIDFrame ?
        (seed.GetSkippedPIDFrames(inspectorController.memoryLinkUsed) + 1 +
         pidFrameOffset) :
        pidFrame;
      
      uint32_t  skippedFrames;
      if (matchOffsetFromInitialPIDFrame)
      {
        if (pidFrameOffset < (pidFrameVariance + 1))
          skippedFrames = 0;
        else
          skippedFrames = pidFrameOffset - pidFrameVariance - 1;
      }
      else
      {
        if (adjacentPIDFrameNum < (pidFrameVariance + 1))
          skippedFrames = 0;
        else
          skippedFrames = adjacentPIDFrameNum - pidFrameVariance - 1;
      }
      
      uint32_t  pidEndFrameNum = adjacentPIDFrameNum + pidFrameVariance;
      
      Gen5PIDFrameGenerator  pidGenerator(seed, pidFrameParams);
      
      for (uint32_t j = 0; j < skippedFrames; ++j)
        pidGenerator.AdvanceFrame();
      
      CGearFrameTime  cgearTime(cgearStartOffset);
      
      for (uint32_t f = pidGenerator.CurrentFrame().number;
           f < pidEndFrameNum;
           ++f)
      {
        pidGenerator.AdvanceFrame();
        
        Gen5PIDFrame  frame = pidGenerator.CurrentFrame();
        
        cgearTime.AdvanceFrame(frame.rngValue);
        
        HashedSeedInspectorAdjacentFrame  *result =
          [[HashedSeedInspectorAdjacentFrame alloc] init];
        
        SetHashedSeedResultParameters(result, seed);
        
        SetIVResult(result, ivs, isRoamer);
        
        result.startFrame =
          seed.GetSkippedPIDFrames(inspectorController.memoryLinkUsed) + 1;
        result.pidFrame = frame.number;
        
        SetPIDResult(result, frame.pid, pidFrameParams.tid, pidFrameParams.sid,
          frame.nature, frame.pid.Gen5Ability(),
          pidFrameParams.targetGender,
          ((pidFrameParams.leadAbility == EncounterLead::CUTE_CHARM) &&
           (pidFrameParams.frameType != Gen5PIDFrameGenerator::EntraLinkFrame))?
             (frame.abilityActivated ? pidFrameParams.targetRatio :
                                       Gender::ANY_RATIO) :
             pidFrameParams.targetRatio);
        
        result.esv = frame.esv;
        result.heldItem = frame.heldItem;
        result.characteristic = Characteristic::Get(frame.pid, ivs);
        result.details = GetGen5PIDFrameDetails(frame, pidFrameParams,
                                                cgearTime.GetTicks());
        
        [rowArray addObject: result];
      }
    }
  }
  
  [adjacentsContentArray addObjects: rowArray];
}

@end
