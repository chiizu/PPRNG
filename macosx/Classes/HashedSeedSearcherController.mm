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


#import "HashedSeedSearcherController.h"

#include "HashedSeedSearcher.h"
#include "FrameGenerator.h"
#include "SearchResultProtocols.h"
#include "Utilities.h"

#import "StandardSeedInspectorController.h"

#include <map>
#include <vector>
#include <memory>

using namespace pprng;

@interface HashedSeedSearchResult :
  NSObject <HashedSeedResultParameters, IVResult, PIDResult>
{
  DECLARE_HASHED_SEED_RESULT_PARAMETERS_VARIABLES();
  
  uint32_t       ivFrame;
  DECLARE_IV_RESULT_VARIABLES();
  
  EncounterLead::Ability  leadAbility;
  
  DECLARE_PID_RESULT_VARIABLES();
  
  uint32_t       requiredEncountersMask;
  Gender::Type   targetGender;
  Gender::Ratio  targetRatio;
  
  uint32_t  pidStartFrame, giftFrame, grassCaveSurfSpotFrame;
  uint32_t  swarmFrame, doublesFrame, dustFrame, shadowFrame;
  uint32_t  stationaryFrame, hollowFrame, fishFrame;
  
  ESV::Value  landESV, doublesESV, surfESV, fishESV;
}

@property uint32_t       ivFrame;

@property EncounterLead::Ability  leadAbility;

@property uint32_t  requiredEncountersMask;
@property Gender::Type   targetGender;
@property Gender::Ratio  targetRatio;

@property uint32_t  pidStartFrame, giftFrame, grassCaveSurfSpotFrame;
@property uint32_t  swarmFrame, doublesFrame, dustFrame, shadowFrame;
@property uint32_t  stationaryFrame, hollowFrame, fishFrame;
@property ESV::Value  landESV, doublesESV, surfESV, fishESV;

@end

@implementation HashedSeedSearchResult

- (id)init
{
  if (self = [super init])
  {
    leadAbility = EncounterLead::NONE;
    nature = Nature::NONE;
    ability = Ability::NONE;
    gender18 = Gender::NONE;
    gender14 = Gender::NONE;
    gender12 = Gender::NONE;
    gender34 = Gender::NONE;
    landESV = ESV::NO_SLOT;
    doublesESV = ESV::NO_SLOT;
    surfESV = ESV::NO_SLOT;
    fishESV = ESV::NO_SLOT;
  }
  
  return self;
}

SYNTHESIZE_HASHED_SEED_RESULT_PARAMETERS_PROPERTIES();

@synthesize ivFrame;
SYNTHESIZE_IV_RESULT_PROPERTIES();

@synthesize leadAbility;

SYNTHESIZE_PID_RESULT_PROPERTIES();

@synthesize requiredEncountersMask, targetGender, targetRatio;
@synthesize pidStartFrame, giftFrame, grassCaveSurfSpotFrame;
@synthesize swarmFrame, doublesFrame, dustFrame, shadowFrame;
@synthesize stationaryFrame, hollowFrame, fishFrame;
@synthesize landESV, doublesESV, surfESV, fishESV;

@end


namespace
{

bool CheckBitMask(uint32_t mask, uint32_t bit)
{
  return (mask & (0x1 << bit)) != 0;
}

uint32_t CountBitMaskBits(uint32_t mask)
{
  uint32_t  count = 0;
  for (uint32_t i = 0; i < 32; ++i)
  {
    count += mask & 0x1;
    mask >>= 1;
  }
  
  return count;
}


struct GUICriteria : public HashedSeedSearcher::Criteria
{
  uint32_t       tid, sid;
  bool           shinyOnly;
  bool           hasShinyCharm;
  bool           memoryLinkUsed;
  
  uint32_t       leadAbilityMask;
  
  PIDCriteria    pid;
  FrameRange     pidFrame;
  
  uint32_t       requiredEncountersMask;
  std::map<ESV::Type, uint32_t>  esvMask;
  
  
  uint64_t ExpectedNumberOfResults() const
  {
    uint64_t  result =
      HashedSeedSearcher::Criteria::ExpectedNumberOfResults();
    
    uint64_t  pidFrameMultiplier = 1;
    uint64_t  shinyDivisor = 1;
    uint64_t  natureMultiplier = 1, natureDivisor = 1;
    
    if (shinyOnly)
    {
      shinyDivisor = 8192;
      pidFrameMultiplier = pidFrame.max - pidFrame.min + 1;
      
      natureMultiplier = pid.NumNatures();
      natureDivisor = 25;
    }
    
    return (result  * pidFrameMultiplier * natureMultiplier) /
           (shinyDivisor * natureDivisor);
  }
};

static const uint32_t  OtherFrameTypes =
  (0x1 << Gen5PIDFrameGenerator::GrassCaveFrame) |
  (0x1 << Gen5PIDFrameGenerator::SurfingFrame) |
  (0x1 << Gen5PIDFrameGenerator::DoublesFrame) |
  (0x1 << Gen5PIDFrameGenerator::SwarmFrame) |
  (0x1 << Gen5PIDFrameGenerator::FishingFrame) |
  (0x1 << Gen5PIDFrameGenerator::SwirlingDustFrame) |
  (0x1 << Gen5PIDFrameGenerator::BridgeShadowFrame) |
  (0x1 << Gen5PIDFrameGenerator::WaterSpotFishingFrame) |
  (0x1 << Gen5PIDFrameGenerator::StationaryFrame) |
  (0x1 << Gen5PIDFrameGenerator::StarterFossilGiftFrame);


static const uint32_t  SyncFrameTypes =
  (0x1 << Gen5PIDFrameGenerator::GrassCaveFrame) |
  (0x1 << Gen5PIDFrameGenerator::SurfingFrame) |
  (0x1 << Gen5PIDFrameGenerator::DoublesFrame) |
  (0x1 << Gen5PIDFrameGenerator::SwarmFrame) |
  (0x1 << Gen5PIDFrameGenerator::FishingFrame) |
  (0x1 << Gen5PIDFrameGenerator::SwirlingDustFrame) |
  (0x1 << Gen5PIDFrameGenerator::WaterSpotFishingFrame) |
  (0x1 << Gen5PIDFrameGenerator::BridgeShadowFrame) |
  (0x1 << Gen5PIDFrameGenerator::StationaryFrame);


static const uint32_t  CuteCharmFrameTypes =
  (0x1 << Gen5PIDFrameGenerator::GrassCaveFrame) |
  (0x1 << Gen5PIDFrameGenerator::SurfingFrame) |
  (0x1 << Gen5PIDFrameGenerator::WaterSpotFishingFrame) |
  (0x1 << Gen5PIDFrameGenerator::DoublesFrame) |
  (0x1 << Gen5PIDFrameGenerator::SwarmFrame) |
  (0x1 << Gen5PIDFrameGenerator::FishingFrame) |
  (0x1 << Gen5PIDFrameGenerator::SwirlingDustFrame) |
  (0x1 << Gen5PIDFrameGenerator::BridgeShadowFrame) |
  (0x1 << Gen5PIDFrameGenerator::StationaryFrame);


static const uint32_t  CompoundEyesFrameTypes =
  (0x1 << Gen5PIDFrameGenerator::SwirlingDustFrame) |
  (0x1 << Gen5PIDFrameGenerator::BridgeShadowFrame);


static const uint32_t  SuctionCupsFrameTypes =
  (0x1 << Gen5PIDFrameGenerator::FishingFrame);

static const uint32_t  HiddenHollowFrameType =
  (0x1 << Gen5PIDFrameGenerator::HiddenHollowFrame);


struct ResultHandler
{
  ResultHandler(SearcherController *c, const GUICriteria &criteria)
    : controller(c), m_criteria(criteria)
  {}
  
  struct PIDEncounterData
  {
    Nature::Type            nature;
    EncounterLead::Ability  leadAbility;
    Gender::Type            gender;
    Gender::Ratio           genderRatio;
    
    typedef std::map<ESV::Type, ESV::Value>  ESVMap;
    ESVMap    esv;
    
    typedef std::map<Gen5PIDFrameGenerator::FrameType, uint32_t>  FrameMap;
    FrameMap  frame;
  };
  
  bool AddResults(const std::map<uint32_t, PIDEncounterData> &resultMap,
                  HashedSeedSearchResult *ivResult)
  {
    bool resultAdded = false;
    
    std::map<uint32_t, PIDEncounterData>::const_iterator  eit;
    for (eit = resultMap.begin(); eit != resultMap.end(); ++eit)
    {
      const PIDEncounterData            &data(eit->second);
      const PIDEncounterData::FrameMap  &frameMap(data.frame);
      
      bool hasRequiredEncounters = true;
      
      if (m_criteria.requiredEncountersMask != 0)
      {
        for (uint32_t i = 0; i < Gen5PIDFrameGenerator::NumFrameTypes; ++i)
        {
          if (CheckBitMask(m_criteria.requiredEncountersMask, i) &&
              (frameMap.find(Gen5PIDFrameGenerator::FrameType(i)) ==
               frameMap.end()))
          {
            hasRequiredEncounters = false;
            break;
          }
        }
      }
      
      if (hasRequiredEncounters)
      {
        PID  pid(eit->first);
        
        HashedSeedSearchResult  *pidResult = NSCopyObject(ivResult, 0, NULL);
        
        pidResult.leadAbility = data.leadAbility;
        
        SetPIDResult(pidResult, pid, m_criteria.tid, m_criteria.sid,
                     data.nature, pid.Gen5Ability(),
                     data.gender, data.genderRatio);
        
        pidResult.requiredEncountersMask = m_criteria.requiredEncountersMask;
        pidResult.targetGender = m_criteria.pid.gender;
        pidResult.targetRatio = m_criteria.pid.genderRatio;
        
        PIDEncounterData::FrameMap::const_iterator  it;
        
        if ((it = frameMap.find(Gen5PIDFrameGenerator::GrassCaveFrame)) !=
            frameMap.end())
        {
          pidResult.grassCaveSurfSpotFrame = uint32_t(it->second);
          pidResult.landESV = ESV::Value(data.esv.find(ESV::LAND_TYPE)->second);
          pidResult.surfESV = ESV::Value(data.esv.find(ESV::SURF_TYPE)->second);
          pidResult.fishESV =
            ESV::Value(data.esv.find(ESV::GOOD_ROD_TYPE)->second);
        }
        
        if ((it = frameMap.find(Gen5PIDFrameGenerator::DoublesFrame)) !=
            frameMap.end())
        {
          pidResult.doublesFrame = uint32_t(it->second);
          pidResult.doublesESV =
            ESV::Value(data.esv.find(ESV::DOUBLES_GRASS_DOUBLE_TYPE)->second);
        }
        
        if ((it = frameMap.find(Gen5PIDFrameGenerator::SwarmFrame)) !=
            frameMap.end())
        {
          pidResult.swarmFrame = uint32_t(it->second);
        }
        
        if ((it = frameMap.find(Gen5PIDFrameGenerator::SwirlingDustFrame)) !=
            frameMap.end())
        {
          pidResult.dustFrame = uint32_t(it->second);
        }
        
        if ((it = frameMap.find(Gen5PIDFrameGenerator::BridgeShadowFrame)) !=
            frameMap.end())
        {
          pidResult.shadowFrame = uint32_t(it->second);
        }
        
        if ((it = frameMap.find(Gen5PIDFrameGenerator::StationaryFrame)) !=
            frameMap.end())
        {
          pidResult.stationaryFrame = uint32_t(it->second);
        }
        
        if ((it = frameMap.find(Gen5PIDFrameGenerator::HiddenHollowFrame)) !=
            frameMap.end())
        {
          pidResult.hollowFrame = uint32_t(it->second);
        }
        
        if ((it = frameMap.find(Gen5PIDFrameGenerator::FishingFrame)) !=
            frameMap.end())
        {
          pidResult.fishFrame = uint32_t(it->second);
          pidResult.fishESV =
            ESV::Value(data.esv.find(ESV::GOOD_ROD_TYPE)->second);
        }
        
        if ((it = frameMap.find(Gen5PIDFrameGenerator::StarterFossilGiftFrame)) !=
            frameMap.end())
        {
          pidResult.giftFrame = uint32_t(it->second);
        }
        
        [controller performSelectorOnMainThread: @selector(addResult:)
                    withObject: pidResult
                    waitUntilDone: NO];
        
        resultAdded = true;
      }
    }
    
    return resultAdded;
  }
  
  bool SearchEncounters
    (const HashedSeed &seed,
     const Gen5PIDFrameGenerator::Parameters &frameParameters,
     uint32_t frameTypesMask,
     HashedSeedSearchResult *ivResult)
  {
    std::map<uint32_t, PIDEncounterData>  resultMap;
    Gen5PIDFrameGenerator::Parameters     parameters = frameParameters;
    
    for (uint32_t f = 0; f < 32; ++f)
    {
      if ((frameTypesMask & (0x1 << f)) != 0)
      {
        parameters.frameType = Gen5PIDFrameGenerator::FrameType(f);
        Gen5PIDFrameGenerator  generator(seed, parameters);
        
        uint32_t  minFrame = m_criteria.pid.startFromLowestFrame ?
                               0 : m_criteria.pidFrame.min - 1;
        uint32_t  frameNum = 0;
        
        while (frameNum < minFrame)
        {
          generator.AdvanceFrame();
          ++frameNum;
        }
        
        while (frameNum < m_criteria.pidFrame.max)
        {
          generator.AdvanceFrame();
          ++frameNum;
          
          Gen5PIDFrame  frame = generator.CurrentFrame();
          
          if (frame.isEncounter && frame.abilityActivated &&
              (frame.pid.IsShiny(m_criteria.tid, m_criteria.sid) ||
               !m_criteria.shinyOnly) && 
              ((parameters.leadAbility == EncounterLead::SYNCHRONIZE) ||
               m_criteria.pid.CheckNature(frame.nature)) &&
              ((m_criteria.pid.ability == Ability::ANY) ||
               (m_criteria.pid.ability == frame.pid.Gen5Ability())) &&
              Gender::GenderValueMatches(frame.pid.GenderValue(),
                                         m_criteria.pid.gender,
                                         m_criteria.pid.genderRatio) &&
              ((parameters.frameType ==
                Gen5PIDFrameGenerator::StarterFossilGiftFrame) ||
               (parameters.frameType ==
                Gen5PIDFrameGenerator::StationaryFrame) ||
               (parameters.frameType ==
                Gen5PIDFrameGenerator::HiddenHollowFrame) ||
               ((parameters.frameType == Gen5PIDFrameGenerator::SwarmFrame) &&
                (frame.esv == ESV::SWARM)) ||
               ((m_criteria.esvMask.find(ESV::SlotType(frame.esv))->second &
                 (0x1 << ESV::Slot(frame.esv))) != 0)))
          {
            PIDEncounterData  *data;
            std::map<uint32_t, PIDEncounterData>::iterator  it;
            
            it = resultMap.find(frame.pid.word);
            if (it == resultMap.end())
            {
              data = &resultMap[frame.pid.word];
              data->nature = frame.nature;
              data->leadAbility = frame.leadAbility;
              data->gender = parameters.targetGender;
              data->genderRatio = parameters.targetRatio;
            }
            else
            {
              data = &it->second;
            }
            
            if (data->frame.find(parameters.frameType) == data->frame.end())
            {
              if (parameters.frameType == Gen5PIDFrameGenerator::DoublesFrame)
              {
                data->esv[ESV::DOUBLES_GRASS_DOUBLE_TYPE] = frame.esv;
              }
              else if ((parameters.frameType !=
                        Gen5PIDFrameGenerator::StarterFossilGiftFrame) &&
                       (parameters.frameType !=
                        Gen5PIDFrameGenerator::SwarmFrame) &&
                       (parameters.frameType !=
                        Gen5PIDFrameGenerator::StationaryFrame) &&
                       (parameters.frameType !=
                        Gen5PIDFrameGenerator::HiddenHollowFrame))
              {
                data->esv[ESV::SlotType(frame.esv)] = frame.esv;
              }
              data->frame[parameters.frameType] = frame.number;
            }
            
            // if not searching for shinies, take only first result
            if (!m_criteria.shinyOnly)
              break;
          }
        }
      }
    }
    
    return AddResults(resultMap, ivResult);
  }
  
  bool SearchHiddenHollowFrame(const HashedSeed &seed,
     const Gen5PIDFrameGenerator::Parameters &baseParameters,
     HashedSeedSearchResult *ivResult)
  {
    Gen5PIDFrameGenerator::Parameters  frameParameters = baseParameters;
    bool                               shinyFound = false;
    
    if ((m_criteria.pid.gender != Gender::ANY) &&
        ((m_criteria.pid.genderRatio != Gender::ANY_RATIO) ||
         (m_criteria.pid.gender == Gender::GENDERLESS)))
    {
      shinyFound = SearchEncounters(seed, frameParameters,
                                    HiddenHollowFrameType, ivResult);
    }
    
    return shinyFound;
  }
  
  void operator()(const HashedIVFrame &frame)
  {
    HashedSeedSearchResult  *result = [[HashedSeedSearchResult alloc] init];
    
    SetHashedSeedResultParameters(result, frame.seed);
    
    result.ivFrame = frame.number;
    SetIVResult(result, frame.ivs, m_criteria.ivs.isRoamer);
    
    result.pidStartFrame =
      frame.seed.GetSkippedPIDFrames(m_criteria.memoryLinkUsed) + 1;
    
    bool  shinyFound = false;
    
    Gen5PIDFrameGenerator::Parameters  frameParameters;
    
    frameParameters.targetGender = m_criteria.pid.gender;
    frameParameters.targetRatio = m_criteria.pid.genderRatio;
    frameParameters.tid = m_criteria.tid;
    frameParameters.sid = m_criteria.sid;
    
    frameParameters.isBlack2White2 =
      Game::IsBlack2White2(m_criteria.seedParameters.version);
    frameParameters.hasShinyCharm = m_criteria.hasShinyCharm;
    frameParameters.memoryLinkUsed = m_criteria.memoryLinkUsed;
    
    frameParameters.startFromLowestFrame = m_criteria.pid.startFromLowestFrame;
    
    if (CheckBitMask(m_criteria.leadAbilityMask, EncounterLead::OTHER))
    {
      frameParameters.leadAbility = EncounterLead::OTHER;
      shinyFound = SearchEncounters(frame.seed, frameParameters,
                                    OtherFrameTypes, result);
      shinyFound =
        SearchHiddenHollowFrame(frame.seed, frameParameters, result) ||
          shinyFound;
    }
    
    if (CheckBitMask(m_criteria.leadAbilityMask, EncounterLead::SYNCHRONIZE))
    {
      frameParameters.leadAbility = EncounterLead::SYNCHRONIZE;
      shinyFound = SearchEncounters(frame.seed, frameParameters,
                                    SyncFrameTypes, result) || shinyFound;
      shinyFound =
        SearchHiddenHollowFrame(frame.seed, frameParameters, result) ||
          shinyFound;
    }
    
    if (CheckBitMask(m_criteria.leadAbilityMask, EncounterLead::COMPOUND_EYES))
    {
      frameParameters.leadAbility = EncounterLead::COMPOUND_EYES;
      shinyFound =
        SearchEncounters(frame.seed, frameParameters,
                         CompoundEyesFrameTypes, result) || shinyFound;
    }
    
    if (CheckBitMask(m_criteria.leadAbilityMask, EncounterLead::SUCTION_CUPS))
    {
      frameParameters.leadAbility = EncounterLead::SUCTION_CUPS;
      shinyFound =
        SearchEncounters(frame.seed, frameParameters,
                         SuctionCupsFrameTypes, result) || shinyFound;
    }
    
    if (m_criteria.shinyOnly &&
        CheckBitMask(m_criteria.leadAbilityMask, EncounterLead::CUTE_CHARM))
    {
      frameParameters.leadAbility = EncounterLead::CUTE_CHARM;
      
      if (m_criteria.pid.genderRatio != Gender::ANY_RATIO)
      {
        if (m_criteria.pid.gender != Gender::ANY)
        {
          shinyFound =
            SearchEncounters(frame.seed, frameParameters,
                             CuteCharmFrameTypes, result) || shinyFound;
        }
        else
        {
          for (uint32_t t = Gender::FEMALE; t <= Gender::MALE; ++t)
          {
            frameParameters.targetGender = Gender::Type(t);
            shinyFound =
              SearchEncounters(frame.seed, frameParameters,
                               CuteCharmFrameTypes, result) || shinyFound;
          }
        }
      }
      else
      {
        for (uint32_t t = Gender::FEMALE; t <= Gender::MALE; ++t)
        {
          for (uint32_t r = Gender::ONE_EIGHTH_FEMALE;
               r <= Gender::THREE_FOURTHS_FEMALE;
               ++r)
          {
            frameParameters.targetGender = Gender::Type(t);
            frameParameters.targetRatio = Gender::Ratio(r);
            
            shinyFound =
              SearchEncounters(frame.seed, frameParameters,
                               CuteCharmFrameTypes, result) || shinyFound;
          }
        }
      }
    }
    
    if (!shinyFound && !m_criteria.shinyOnly)
    {
      [controller performSelectorOnMainThread: @selector(addResult:)
                  withObject: result
                  waitUntilDone: NO];
    }
  }
  
  SearcherController  *controller;
  const GUICriteria   &m_criteria;
};

struct ProgressHandler
{
  ProgressHandler(SearcherController *c)
    : controller(c)
  {}
  
  bool operator()(double progressDelta)
  {
    [controller performSelectorOnMainThread: @selector(adjustProgress:)
                withObject: [NSNumber numberWithDouble: progressDelta]
                waitUntilDone: NO];
    
    return ![controller searchIsCanceled];
  }
  
  SearcherController  *controller;
};

}


@implementation HashedSeedSearcherController

@synthesize fromDate, toDate;
@synthesize noButtonHeld, oneButtonHeld, twoButtonsHeld, threeButtonsHeld;

@synthesize minIVFrame, maxIVFrame;

@synthesize showShinyOnly;
@synthesize ability, gender, genderRatio;
@synthesize startFromInitialPIDFrame;
@synthesize minPIDFrame, maxPIDFrame;

- (NSString *)windowNibName
{
	return @"HashedSeedSearcher";
}

- (void)awakeFromNib
{
  [searcherController setGetValidatedSearchCriteriaSelector:
                      @selector(getValidatedSearchCriteria)];
  [searcherController setDoSearchWithCriteriaSelector:
                      @selector(doSearchWithCriteria:)];
  
  [[searcherController tableView] setTarget: self];
  [[searcherController tableView] setDoubleAction: @selector(inspectSeed:)];
  
  NSDate  *now = [NSDate date];
  self.fromDate = now;
  self.toDate = now;
  
  self.noButtonHeld = YES;
  self.oneButtonHeld = YES;
  self.twoButtonsHeld = NO;
  self.threeButtonsHeld = NO;
  
  self.minIVFrame = 1;
  self.maxIVFrame = 6;
  
  self.showShinyOnly = NO;
  self.ability = Ability::ANY;
  self.gender = Gender::ANY;
  self.genderRatio = Gender::ANY_RATIO;
  self.startFromInitialPIDFrame = YES;
  self.minPIDFrame = 50;
  self.maxPIDFrame = 500;
}

- (void)windowWillClose:(NSNotification *)notification
{
  if ([searcherController isSearching])
    [searcherController startStop: self];
}

- (IBAction)toggleDropDownChoice:(id)sender
{
  HandleComboMenuItemChoice(sender);
}

- (void)inspectSeed:(id)sender
{
  NSInteger  rowNum = [sender clickedRow];
  
  if (rowNum >= 0)
  {
    HashedSeedSearchResult  *row =
      [[[searcherController arrayController] arrangedObjects]
        objectAtIndex: rowNum];
    
    if (row != nil)
    {
      StandardSeedInspectorController  *inspector =
        [[StandardSeedInspectorController alloc] init];
      [inspector window];
      
      [inspector setSeedFromResult: row];
      
      HashedSeedInspectorFramesTabController *framesTab =
        inspector.framesTabController;
      HashedSeedInspectorAdjacentsTabController *adjacentsTab =
        inspector.adjacentsTabController;
      
      uint32_t  targetFrame = 0;
      
      if (row.leadAbility == EncounterLead::NONE)
      {
        targetFrame = row.pidStartFrame;
      }
      else
      {
        inspector.tid = [NSNumber numberWithUnsignedInt: row.tid];
        inspector.sid = [NSNumber numberWithUnsignedInt: row.sid];
        
        framesTab.encounterLeadAbility = row.leadAbility;
        adjacentsTab.encounterLeadAbility = row.leadAbility;
        
        if (row.leadAbility == EncounterLead::CUTE_CHARM)
        {
          if (row.gender18 != Gender::NONE)
          {
            framesTab.targetGender = row.gender18;
            framesTab.targetGenderRatio = Gender::ONE_EIGHTH_FEMALE;
            adjacentsTab.targetGender = row.gender18;
            adjacentsTab.targetGenderRatio = Gender::ONE_EIGHTH_FEMALE;
          }
          else if (row.gender14 != Gender::NONE)
          {
            framesTab.targetGender = row.gender14;
            framesTab.targetGenderRatio = Gender::ONE_FOURTH_FEMALE;
            adjacentsTab.targetGender = row.gender14;
            adjacentsTab.targetGenderRatio = Gender::ONE_FOURTH_FEMALE;
          }
          else if (row.gender12 != Gender::NONE)
          {
            framesTab.targetGender = row.gender12;
            framesTab.targetGenderRatio = Gender::ONE_HALF_FEMALE;
            adjacentsTab.targetGender = row.gender12;
            adjacentsTab.targetGenderRatio = Gender::ONE_HALF_FEMALE;
          }
          else if (row.gender34 != Gender::NONE)
          {
            framesTab.targetGender = row.gender34;
            framesTab.targetGenderRatio = Gender::THREE_FOURTHS_FEMALE;
            adjacentsTab.targetGender = row.gender34;
            adjacentsTab.targetGenderRatio = Gender::THREE_FOURTHS_FEMALE;
          }
        }
        
        if (row.requiredEncountersMask != 0)
        {
          uint32_t  f = 0;
          while ((row.requiredEncountersMask & (0x1 << f)) == 0)
            ++f;
          
          framesTab.encounterFrameType = Gen5PIDFrameGenerator::FrameType(f);
          adjacentsTab.encounterFrameType = Gen5PIDFrameGenerator::FrameType(f);
          
          switch (f)
          {
          case Gen5PIDFrameGenerator::GrassCaveFrame:
            targetFrame = row.grassCaveSurfSpotFrame;
            break;
          case Gen5PIDFrameGenerator::FishingFrame:
            targetFrame = row.fishFrame;
            break;
          case Gen5PIDFrameGenerator::DoublesFrame:
            targetFrame = row.doublesFrame;
            break;
          case Gen5PIDFrameGenerator::SwarmFrame:
            targetFrame = row.swarmFrame;
            break;
          case Gen5PIDFrameGenerator::SwirlingDustFrame:
            targetFrame = row.dustFrame;
            break;
          case Gen5PIDFrameGenerator::BridgeShadowFrame:
            targetFrame = row.shadowFrame;
            break;
          case Gen5PIDFrameGenerator::StationaryFrame:
            targetFrame = row.stationaryFrame;
            break;
          case Gen5PIDFrameGenerator::HiddenHollowFrame:
            targetFrame = row.hollowFrame;
            break;
          case Gen5PIDFrameGenerator::StarterFossilGiftFrame:
            targetFrame = row.giftFrame;
            break;
          default:
            break;
          }
        }
        else if (row.grassCaveSurfSpotFrame > 0)
        {
          targetFrame = row.grassCaveSurfSpotFrame;
          framesTab.encounterFrameType = Gen5PIDFrameGenerator::GrassCaveFrame;
          adjacentsTab.encounterFrameType =
            Gen5PIDFrameGenerator::GrassCaveFrame;
        }
        else if (row.doublesFrame > 0)
        {
          targetFrame = row.doublesFrame;
          framesTab.encounterFrameType = Gen5PIDFrameGenerator::DoublesFrame;
          adjacentsTab.encounterFrameType = Gen5PIDFrameGenerator::DoublesFrame;
        }
        else if (row.fishFrame > 0)
        {
          targetFrame = row.fishFrame;
          framesTab.encounterFrameType = Gen5PIDFrameGenerator::FishingFrame;
          adjacentsTab.encounterFrameType = Gen5PIDFrameGenerator::FishingFrame;
        }
        else if (row.swarmFrame > 0)
        {
          targetFrame = row.swarmFrame;
          framesTab.encounterFrameType = Gen5PIDFrameGenerator::SwarmFrame;
          adjacentsTab.encounterFrameType = Gen5PIDFrameGenerator::SwarmFrame;
        }
        else if (row.dustFrame > 0)
        {
          targetFrame = row.dustFrame;
          framesTab.encounterFrameType =
            Gen5PIDFrameGenerator::SwirlingDustFrame;
          adjacentsTab.encounterFrameType =
            Gen5PIDFrameGenerator::SwirlingDustFrame;
        }
        else if (row.shadowFrame > 0)
        {
          targetFrame = row.shadowFrame;
          framesTab.encounterFrameType =
            Gen5PIDFrameGenerator::BridgeShadowFrame;
          adjacentsTab.encounterFrameType =
            Gen5PIDFrameGenerator::BridgeShadowFrame;
        }
        else if (row.stationaryFrame > 0)
        {
          targetFrame = row.stationaryFrame;
          framesTab.encounterFrameType = Gen5PIDFrameGenerator::StationaryFrame;
          adjacentsTab.encounterFrameType =
            Gen5PIDFrameGenerator::StationaryFrame;
        }
        else if (row.hollowFrame > 0)
        {
          targetFrame = row.hollowFrame;
          framesTab.encounterFrameType =
            Gen5PIDFrameGenerator::HiddenHollowFrame;
          framesTab.targetGender = row.targetGender;
          framesTab.targetGenderRatio = row.targetRatio;
          
          adjacentsTab.encounterFrameType =
            Gen5PIDFrameGenerator::HiddenHollowFrame;
          adjacentsTab.targetGender = row.targetGender;
          adjacentsTab.targetGenderRatio = row.targetRatio;
        }
        else if (row.giftFrame > 0)
        {
          targetFrame = row.giftFrame;
          framesTab.encounterFrameType =
            Gen5PIDFrameGenerator::StarterFossilGiftFrame;
          adjacentsTab.encounterFrameType =
            Gen5PIDFrameGenerator::StarterFossilGiftFrame;
        }
        
        if (targetFrame != 0)
        {
          if (targetFrame < row.pidStartFrame)
          {
            framesTab.startFromInitialPIDFrame = NO;
            framesTab.minPIDFrame = 1;
            adjacentsTab.matchOffsetFromInitialPIDFrame = NO;
          }
          
          adjacentsTab.pidFrame = targetFrame;
          framesTab.maxPIDFrame = targetFrame + 20;
        }
      }
      
      [framesTab generatePIDFrames: self];
      [framesTab selectAndShowPIDFrame: targetFrame];
      
      [framesTab.ivParameterController setIsRoamer: row.isRoamer];
      [framesTab generateIVFrames: self];
      [framesTab selectAndShowIVFrame: row.ivFrame];
      
      adjacentsTab.isRoamer = row.isRoamer;
      adjacentsTab.ivFrame = row.ivFrame;
      
      [inspector showWindow: self];
    }
  }
}

// dummy method for error panel
- (void)alertDidEnd:(NSAlert *)alert returnCode:(NSInteger)returnCode
        contextInfo:(void *)contextInfo
{}

- (NSValue*)getValidatedSearchCriteria
{
  using namespace boost::gregorian;
  using namespace boost::posix_time;
  
  if (!EndEditing([self window]))
    return nil;
  
  GUICriteria  criteria;
  
  criteria.seedParameters.macAddress = [gen5ConfigController macAddress];
  
  criteria.seedParameters.version = [gen5ConfigController version];
  criteria.seedParameters.dsType = [gen5ConfigController dsType];
  
  criteria.seedParameters.timer0Low = [gen5ConfigController timer0Low];
  criteria.seedParameters.timer0High = [gen5ConfigController timer0High];
  
  criteria.seedParameters.vcountLow = [gen5ConfigController vcountLow];
  criteria.seedParameters.vcountHigh = [gen5ConfigController vcountHigh];
  
  criteria.seedParameters.vframeLow = [gen5ConfigController vframeLow];
  criteria.seedParameters.vframeHigh = [gen5ConfigController vframeHigh];
  
  if (noButtonHeld)
  {
    criteria.seedParameters.heldButtons.push_back(0);  // no keys
  }
  if (oneButtonHeld)
  {
    criteria.seedParameters.heldButtons.insert
      (criteria.seedParameters.heldButtons.end(),
       Button::SingleButtons().begin(),
       Button::SingleButtons().end());
  }
  if (twoButtonsHeld)
  {
    criteria.seedParameters.heldButtons.insert
      (criteria.seedParameters.heldButtons.end(),
       Button::TwoButtonCombos().begin(),
       Button::TwoButtonCombos().end());
  }
  if (threeButtonsHeld)
  {
    criteria.seedParameters.heldButtons.insert
      (criteria.seedParameters.heldButtons.end(),
       Button::ThreeButtonCombos().begin(),
       Button::ThreeButtonCombos().end());
  }
  
  criteria.seedParameters.fromTime =
    ptime(NSDateToBoostDate(fromDate), seconds(0));
  
  criteria.seedParameters.toTime =
    ptime(NSDateToBoostDate(toDate), hours(23) + minutes(59) + seconds(59));
  
  criteria.ivFrame.min = minIVFrame;
  criteria.ivFrame.max = maxIVFrame;
  
  criteria.ivs.min = ivParameterController.minIVs;
  criteria.ivs.max = ivParameterController.maxIVs;
  criteria.ivs.shouldCheckMax =
    (criteria.ivs.max != IVs(31, 31, 31, 31, 31, 31));
  criteria.ivs.isRoamer = ivParameterController.isRoamer;
  
  if (ivParameterController.considerHiddenPower)
  {
    criteria.ivs.hiddenType = ivParameterController.hiddenType;
    criteria.ivs.minHiddenPower = ivParameterController.minHiddenPower;
  }
  else
  {
    criteria.ivs.hiddenType = Element::NONE;
  }
  
  criteria.tid = [gen5ConfigController tid];
  criteria.sid = [gen5ConfigController sid];
  criteria.shinyOnly = showShinyOnly;
  criteria.hasShinyCharm = [gen5ConfigController hasShinyCharm];
  criteria.memoryLinkUsed = [gen5ConfigController memoryLinkUsed];
  criteria.leadAbilityMask = GetComboMenuBitMask(leadAbilityDropDown);
  
  criteria.pid.natureMask = GetComboMenuBitMask(natureDropDown);
  criteria.pid.ability = ability;
  criteria.pid.gender = gender;
  criteria.pid.genderRatio = genderRatio;
  
  criteria.pid.startFromLowestFrame = startFromInitialPIDFrame;
  criteria.pidFrame.min = minPIDFrame;
  criteria.pidFrame.max = maxPIDFrame;
  
  criteria.requiredEncountersMask =
    GetComboMenuBitMask(requiredEncountersDropDown);
  criteria.esvMask[ESV::LAND_TYPE] = GetComboMenuBitMask(landESVDropDown);
  criteria.esvMask[ESV::DOUBLES_GRASS_DOUBLE_TYPE] =
    criteria.esvMask[ESV::DOUBLES_GRASS_SINGLE_TYPE] =
      GetComboMenuBitMask(doublesESVDropDown);
  criteria.esvMask[ESV::SURF_TYPE] = GetComboMenuBitMask(surfESVDropDown);
  criteria.esvMask[ESV::GOOD_ROD_TYPE] = GetComboMenuBitMask(fishESVDropDown);
  
  if (CheckExpectedResults(criteria, 10000,
                           @"The current search parameters are expected to return more than 10,000 results. Please set more specific IVs, limit the date range, use fewer held keys, or other similar settings to reduce the number of expected results.",
                           self,
                           @selector(alertDidEnd:returnCode:contextInfo:)))
  {
    return [NSValue valueWithPointer: new GUICriteria(criteria)];
  }
  else
  {
    return nil;
  }
}

- (void)doSearchWithCriteria:(NSValue*)criteriaPtr
{
  std::auto_ptr<GUICriteria> 
    criteria(static_cast<GUICriteria*>([criteriaPtr pointerValue]));
  
  HashedSeedSearcher  searcher;
  
  searcher.Search(*criteria, ResultHandler(searcherController, *criteria),
                  ProgressHandler(searcherController));
}


@end
