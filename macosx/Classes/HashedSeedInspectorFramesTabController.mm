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

#import "HashedSeedInspectorFramesTabController.h"

#import "StandardSeedInspectorController.h"
#import "SearchResultProtocols.h"

#include "HashedSeed.h"
#include "FrameGenerator.h"
#include "Utilities.h"

using namespace pprng;

@interface HashedSeedInspectorPIDFrame : NSObject <PIDResult>
{
  uint32_t        frame;
  uint32_t        chatotPitch;
  DECLARE_PID_RESULT_VARIABLES();
  ESV::Value      esv;
  HeldItem::Type  heldItem;
  NSString        *details;
}

@property uint32_t         frame;
@property uint32_t         chatotPitch;
@property ESV::Value       esv;
@property HeldItem::Type   heldItem;
@property (copy) NSString  *details;

@end

@implementation HashedSeedInspectorPIDFrame

@synthesize frame;
@synthesize chatotPitch;
SYNTHESIZE_PID_RESULT_PROPERTIES();
@synthesize esv;
@synthesize heldItem;
@synthesize details;

@end


@interface HashedSeedInspectorIVFrame : NSObject <IVResult>
{
  uint32_t  frame;
  DECLARE_IV_RESULT_VARIABLES();
}

@property uint32_t  frame;

@end

@implementation HashedSeedInspectorIVFrame

@synthesize frame;
SYNTHESIZE_IV_RESULT_PROPERTIES();

@end



@implementation HashedSeedInspectorFramesTabController

@synthesize encounterFrameType, encounterLeadAbility;
@synthesize genderRequired, genderlessAllowed, targetGender;
@synthesize genderRatioRequired, targetGenderRatio;
@synthesize startFromInitialPIDFrame;
@synthesize minPIDFrame, maxPIDFrame;
@synthesize isEntralink, cgearStartOffset;
@synthesize minIVFrame, maxIVFrame;
@synthesize ivParameterController;

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
      self.targetGenderRatio = Gender::NO_RATIO;
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
  self.encounterFrameType = Gen5PIDFrameGenerator::GrassCaveFrame;
  self.encounterLeadAbility = EncounterLead::SYNCHRONIZE;
  self.targetGender = Gender::GENDERLESS;
  self.targetGenderRatio = Gender::NO_RATIO;
  self.cgearStartOffset = 3;
  self.startFromInitialPIDFrame = YES;
  self.minPIDFrame = 50;
  self.maxPIDFrame = 500;
  self.minIVFrame = 1;
  self.maxIVFrame = 100;
}

- (IBAction)generatePIDFrames:(id)sender
{
  if (!EndEditing([inspectorController window]))
    return;
  
  if (!inspectorController.rawSeed)
    return;
  
  [pidFrameContentArray setContent: [NSMutableArray array]];
  
  HashedSeed  seed([inspectorController.rawSeed unsignedLongLongValue],
                   inspectorController.version);
  
  uint32_t  minFrame = startFromInitialPIDFrame ? 0 : minPIDFrame - 1;
  uint32_t  frameNum = 0;
  
  Gen5PIDFrameGenerator::Parameters  p;
  
  p.frameType = encounterFrameType;
  p.leadAbility = encounterLeadAbility;
  
  if (genderRequired)
  {
    p.targetGender = targetGender;
    
    if (targetGender == Gender::GENDERLESS)
    {
      p.targetRatio = Gender::NO_RATIO;
    }
    else if (targetGenderRatio == Gender::ANY_RATIO)
    {
      p.targetRatio = (targetGender == Gender::FEMALE) ?
        Gender::FEMALE_ONLY : Gender::MALE_ONLY;
    }
    else
    {
      p.targetRatio = targetGenderRatio;
    }
  }
  else
  {
    p.targetGender = Gender::ANY;
    p.targetRatio = Gender::ANY_RATIO;
  }
  
  p.tid = [inspectorController.tid unsignedIntValue];
  p.sid = [inspectorController.sid unsignedIntValue];
  
  p.isBlack2White2 = Game::IsBlack2White2(inspectorController.version);
  p.hasShinyCharm = inspectorController.hasShinyCharm;
  p.memoryLinkUsed = inspectorController.memoryLinkUsed;
  
  p.startFromLowestFrame = startFromInitialPIDFrame;
  
  Gen5PIDFrameGenerator    generator(seed, p);
  
  if (frameNum < minFrame)
  {
    generator.SkipFrames(minFrame - frameNum);
    frameNum = minFrame;
  }
  
  CGearFrameTime  cgearTime(cgearStartOffset);
  
  NSMutableArray  *rowArray =
    [NSMutableArray arrayWithCapacity: maxPIDFrame - minFrame];
  
  while (frameNum < maxPIDFrame)
  {
    generator.AdvanceFrame();
    ++frameNum;
    
    Gen5PIDFrame  frame = generator.CurrentFrame();
    
    cgearTime.AdvanceFrame(frame.rngValue);
    
    HashedSeedInspectorPIDFrame  *result =
      [[HashedSeedInspectorPIDFrame alloc] init];
    
    result.frame = frame.number;
    result.chatotPitch = Chatot::Gen5Pitch(frame.rngValue);
    SetPIDResult(result, frame.pid, p.tid, p.sid,
                 frame.nature, frame.pid.Gen5Ability(), p.targetGender,
                 ((p.leadAbility == EncounterLead::CUTE_CHARM) &&
                  (p.frameType != Gen5PIDFrameGenerator::EntraLinkFrame)) ?
                    (frame.abilityActivated ? p.targetRatio :
                                              Gender::ANY_RATIO) :
                    p.targetRatio);
    result.esv = frame.esv;
    result.heldItem = frame.heldItem;
    result.details = GetGen5PIDFrameDetails(frame, p, cgearTime.GetTicks());
    
    [rowArray addObject: result];
  }
  
  [pidFrameContentArray addObjects: rowArray];
}

- (void)selectAndShowPIDFrame:(uint32_t)frame
{
  NSArray  *rows = [pidFrameContentArray arrangedObjects];
  if (rows && ([rows count] > 0))
  {
    HashedSeedInspectorPIDFrame  *row = [rows objectAtIndex: 0];
    
    if (row.frame <= frame)
    {
      NSInteger  rowNum = frame - row.frame;
      
      [pidFrameTableView
        selectRowIndexes: [NSIndexSet indexSetWithIndex: rowNum]
        byExtendingSelection: NO];
      [pidFrameTableView scrollRowToVisible: rowNum];
    }
  }
}


- (IBAction)generateIVFrames:(id)sender
{
  if (!EndEditing([inspectorController window]))
    return;
  
  if (!inspectorController.rawSeed)
    return;
  
  [ivFrameContentArray setContent: [NSMutableArray array]];
  
  HashedSeed  seed([inspectorController.rawSeed unsignedLongLongValue],
                   inspectorController.version);
  
  uint32_t  frameNum = 0, limitFrame = minIVFrame - 1;
  bool      isRoamer = [ivParameterController isRoamer];
  
  HashedIVFrameGenerator  generator(seed, isRoamer ?
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
    
    HashedSeedInspectorIVFrame  *result =
      [[HashedSeedInspectorIVFrame alloc] init];
    
    result.frame = frame.number;
    SetIVResult(result, frame.ivs, isRoamer);
    
    [rowArray addObject: result];
  }
  
  [ivFrameContentArray addObjects: rowArray];
}

- (void)selectAndShowIVFrame:(uint32_t)frame
{
  NSArray  *rows = [ivFrameContentArray arrangedObjects];
  if (rows && ([rows count] > 0))
  {
    HashedSeedInspectorIVFrame  *row = [rows objectAtIndex: 0];
    
    if (row.frame <= frame)
    {
      NSInteger  rowNum = frame - row.frame;
      
      [ivFrameTableView
        selectRowIndexes: [NSIndexSet indexSetWithIndex: rowNum]
        byExtendingSelection: NO];
      [ivFrameTableView scrollRowToVisible: rowNum];
    }
  }
}

@end
