/*
  Copyright (C) 2011-2012 chiizu
  chiizu.pprng@gmail.com
  
  This file is part of libpprng.
  
  libpprng is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  libpprng is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with libpprng.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "FrameGenerator.h"

using namespace pprng;

namespace pprng
{


const IVs::Type  Gen4BreedingFrameGenerator::DPPtRound1[6] =
  { IVs::HP, IVs::AT, IVs::DF, IVs::SP, IVs::SA, IVs::SD };
const IVs::Type  Gen4BreedingFrameGenerator::DPPtRound2[5] =
  { IVs::AT, IVs::DF, IVs::SP, IVs::SA, IVs::SD };
const IVs::Type  Gen4BreedingFrameGenerator::DPPtRound3[4] =
  { IVs::AT, IVs::SP, IVs::SA, IVs::SD };


CGearIVFrameGenerator::CGearIVFrameGenerator(uint32_t seed, FrameType frameType,
                                             bool skipFirstTwoFrames)
  : m_RNG(seed), m_IVRNG(m_RNG, IVRNG::FrameType(frameType))
{
  m_frame.seed = seed;
  m_frame.number = 0;
  
  if (skipFirstTwoFrames)
  {
    // C-Gear starts on 'frame 3'
    m_IVRNG.NextIVWord();
    m_IVRNG.NextIVWord();
  }
}

void CGearIVFrameGenerator::SkipFrames(uint32_t numFrames)
{
  uint32_t  i = 0;
  while (i++ < numFrames)
    m_IVRNG.NextIVWord();
  
  m_frame.number += numFrames;
}

void CGearIVFrameGenerator::AdvanceFrame()
{
  ++m_frame.number;
  m_frame.ivs = m_IVRNG.NextIVWord();
}


HashedIVFrameGenerator::HashedIVFrameGenerator
    (const HashedSeed &seed, FrameType frameType)
  : m_RNG(seed.rawSeed >> 32), m_IVRNG(m_RNG, IVRNG::FrameType(frameType)),
    m_frame(seed)
{
  if (Game::IsBlack2White2(seed.parameters.version))
  {
    // b2w2 skips first 2 frames
    m_IVRNG.NextIVWord();
    m_IVRNG.NextIVWord();
  }
  
  m_frame.number = 0;
}

void HashedIVFrameGenerator::SkipFrames(uint32_t numFrames)
{
  uint32_t  i = 0;
  while (i++ < numFrames)
    m_IVRNG.NextIVWord();
  
  m_frame.number += numFrames;
}

void HashedIVFrameGenerator::AdvanceFrame()
{
  ++m_frame.number;
  m_frame.ivs = m_IVRNG.NextIVWord();
}


Gen5PIDFrameGenerator::Gen5PIDFrameGenerator
  (const HashedSeed &seed, const Gen5PIDFrameGenerator::Parameters &parameters)
  : m_PIDGenerator(s_FrameGeneratorInfo[parameters.frameType].pidGenerator),
    m_PIDFrameGenerator
      (s_FrameGeneratorInfo[parameters.frameType].pidFrameGenerator),
    m_ESVGenerator(parameters.isBlack2White2 ?
                   s_FrameGeneratorInfo[parameters.frameType].b2w2EsvGenerator :
                   s_FrameGeneratorInfo[parameters.frameType].bwEsvGenerator),
    m_RNG(seed.rawSeed), m_frame(seed), m_parameters(parameters),
    m_shinyChances((m_parameters.hasShinyCharm &&
                    Game::IsBlack2White2(m_frame.seed.parameters.version)) ?
                      3 : 1)
{
  m_frame.number = 0;
  m_frame.rngValue = 0;
  m_frame.leadAbility = parameters.leadAbility;
  m_frame.isEncounter = true;
  m_frame.encounterItem = EncounterItem::NONE;
  m_frame.abilityActivated = (parameters.frameType != EntraLinkFrame);
  m_frame.esv = ESV::NO_SLOT;
  m_frame.heldItem = HeldItem::NO_ITEM;
  
  if (parameters.startFromLowestFrame)
  {
    uint32_t  skippedFrames =
      seed.GetSkippedPIDFrames(parameters.memoryLinkUsed);
    m_RNG.AdvanceBuffer(skippedFrames);
    m_frame.number += skippedFrames;
  }
}

void Gen5PIDFrameGenerator::SkipFrames(uint32_t numFrames)
{
  m_RNG.AdvanceBuffer(numFrames);
  m_frame.number += numFrames;
}

void Gen5PIDFrameGenerator::AdvanceFrame()
{
  m_RNG.AdvanceBuffer();
  
  ++m_frame.number;
  m_frame.rngValue = m_RNG.PeekNext();
  
  (this->*m_PIDFrameGenerator)();
}


const Gen5PIDFrameGenerator::FrameGeneratorInfo
  Gen5PIDFrameGenerator::s_FrameGeneratorInfo
    [Gen5PIDFrameGenerator::NumFrameTypes] =
{
  // GrassCaveFrame
  { &Gen5PIDFrameGenerator::NextWildPID,
    &Gen5PIDFrameGenerator::NextWildFrame,
    &Gen5PIDFrameGenerator::BWLandESV,
    &Gen5PIDFrameGenerator::B2W2LandESV },
  // SurfingFrame
  { &Gen5PIDFrameGenerator::NextWildPID,
    &Gen5PIDFrameGenerator::NextWildFrame,
    &Gen5PIDFrameGenerator::BWSurfESV,
    &Gen5PIDFrameGenerator::B2W2SurfESV },
  // FishingFrame
  { &Gen5PIDFrameGenerator::NextWildPID,
    &Gen5PIDFrameGenerator::NextFishingFrame,
    &Gen5PIDFrameGenerator::BWFishingESV,
    &Gen5PIDFrameGenerator::B2W2FishingESV },
  // SwarmFrame
  { &Gen5PIDFrameGenerator::NextWildPID,
    &Gen5PIDFrameGenerator::NextSwarmFrame,
    &Gen5PIDFrameGenerator::BWLandESV,
    &Gen5PIDFrameGenerator::B2W2LandESV },
  // ShakingGrassFrame
  { &Gen5PIDFrameGenerator::NextWildPID,
    &Gen5PIDFrameGenerator::NextWildFrame,
    &Gen5PIDFrameGenerator::BWLandESV,
    &Gen5PIDFrameGenerator::B2W2LandESV },
  // SwirlingDustFrame
  { &Gen5PIDFrameGenerator::NextWildPID,
    &Gen5PIDFrameGenerator::NextDustFrame,
    &Gen5PIDFrameGenerator::BWLandESV,
    &Gen5PIDFrameGenerator::B2W2LandESV },
  // BridgeShadowFrame
  { &Gen5PIDFrameGenerator::NextWildPID,
    &Gen5PIDFrameGenerator::NextShadowFrame,
    &Gen5PIDFrameGenerator::BWLandESV,
    &Gen5PIDFrameGenerator::B2W2LandESV },
  // WaterSpotSurfingFrame
  { &Gen5PIDFrameGenerator::NextWildPID,
    &Gen5PIDFrameGenerator::NextWildFrame,
    &Gen5PIDFrameGenerator::BWSurfESV,
    &Gen5PIDFrameGenerator::B2W2SurfESV },
  // WaterSpotFishingFrame
  { &Gen5PIDFrameGenerator::NextWildPID,
    &Gen5PIDFrameGenerator::NextWildFrame,
    &Gen5PIDFrameGenerator::BWFishingESV,
    &Gen5PIDFrameGenerator::B2W2FishingESV },
  // EntraLinkFrame
  { &Gen5PIDFrameGenerator::NextEntraLinkPID,
    &Gen5PIDFrameGenerator::NextEntraLinkFrame,
    &Gen5PIDFrameGenerator::NoESV,
    &Gen5PIDFrameGenerator::NoESV },
  // StationaryFrame
  { &Gen5PIDFrameGenerator::NextWildPID,
    &Gen5PIDFrameGenerator::NextStationaryFrame,
    &Gen5PIDFrameGenerator::NoESV,
    &Gen5PIDFrameGenerator::NoESV },
  // ZekReshVicFrame
  { &Gen5PIDFrameGenerator::NextNonShinyPID,
    &Gen5PIDFrameGenerator::NextStationaryFrame,
    &Gen5PIDFrameGenerator::NoESV,
    &Gen5PIDFrameGenerator::NoESV },
  // StarterFossilGiftFrame
  { &Gen5PIDFrameGenerator::NextGiftPID,
    &Gen5PIDFrameGenerator::NextSimpleFrame,
    &Gen5PIDFrameGenerator::NoESV,
    &Gen5PIDFrameGenerator::NoESV },
  // RoamerFrame
  { &Gen5PIDFrameGenerator::NextRoamerPID,
    &Gen5PIDFrameGenerator::NextSimpleFrame,
    &Gen5PIDFrameGenerator::NoESV,
    &Gen5PIDFrameGenerator::NoESV },
  // DoublesFrame
  { &Gen5PIDFrameGenerator::NextWildPID,
    &Gen5PIDFrameGenerator::NextDoublesFrame,
    &Gen5PIDFrameGenerator::BWLandESV,
    &Gen5PIDFrameGenerator::B2W2LandESV },
  // HiddenHollowFrame
  { &Gen5PIDFrameGenerator::NextHiddenHollowPID,
    &Gen5PIDFrameGenerator::NextHiddenHollowFrame,
    &Gen5PIDFrameGenerator::NoESV,
    &Gen5PIDFrameGenerator::NoESV },
  // LarvestaEggFrame
  { &Gen5PIDFrameGenerator::NextRoamerPID,
    &Gen5PIDFrameGenerator::NextLarvestaEggFrame,
    &Gen5PIDFrameGenerator::NoESV,
    &Gen5PIDFrameGenerator::NoESV }
};

void Gen5PIDFrameGenerator::BWLandESV()
{
  uint32_t  raw_esv = (m_RNG.Next() >> 48) / 0x290;
  
  m_frame.esv = ESV::Gen5Land(raw_esv);
}

void Gen5PIDFrameGenerator::B2W2LandESV()
{
  uint32_t  raw_esv = ((m_RNG.Next() >> 32) * 100) >> 32;
  
  m_frame.esv = ESV::Gen5Land(raw_esv);
}

void Gen5PIDFrameGenerator::BWSurfESV()
{
  uint32_t  raw_esv = (m_RNG.Next() >> 48) / 0x290;
  
  m_frame.esv = ESV::Gen5Surfing(raw_esv);
}

void Gen5PIDFrameGenerator::B2W2SurfESV()
{
  uint32_t  raw_esv = ((m_RNG.Next() >> 32) * 100) >> 32;
  
  m_frame.esv = ESV::Gen5Surfing(raw_esv);
}

void Gen5PIDFrameGenerator::BWFishingESV()
{
  uint32_t  raw_esv = (m_RNG.Next() >> 48) / 0x290;
  
  m_frame.esv = ESV::Gen5Fishing(raw_esv);
}

void Gen5PIDFrameGenerator::B2W2FishingESV()
{
  uint32_t  raw_esv = ((m_RNG.Next() >> 32) * 100) >> 32;
  
  m_frame.esv = ESV::Gen5Fishing(raw_esv);
}

void Gen5PIDFrameGenerator::NoESV()
{
  m_frame.esv = ESV::Value(0);
}

void Gen5PIDFrameGenerator::NextWildPID()
{
  uint32_t  shinyChances = m_shinyChances;
  
  do
  {
    if ((m_parameters.leadAbility == EncounterLead::CUTE_CHARM) &&
        m_frame.abilityActivated)
    {
      m_frame.pid = Gen5PIDRNG::NextCuteCharmPIDWord
                      (m_RNG, m_parameters.targetGender,
                       m_parameters.targetRatio,
                       m_parameters.tid, m_parameters.sid);
    }
    else
    {
      m_frame.pid =
        Gen5PIDRNG::NextWildPIDWord(m_RNG, m_parameters.tid, m_parameters.sid);
    }
  }
  while (!m_frame.pid.IsShiny(m_parameters.tid, m_parameters.sid) &&
         (--shinyChances > 0));
}

void Gen5PIDFrameGenerator::NextEntraLinkPID()
{
  m_frame.pid = Gen5PIDRNG::NextEntraLinkPIDWord
                  (m_RNG, m_parameters.targetGender, m_parameters.targetRatio,
                   m_parameters.tid, m_parameters.sid);
}

void Gen5PIDFrameGenerator::NextHiddenHollowPID()
{
  m_frame.pid = Gen5PIDRNG::NextDreamRadarPIDWord
                  (m_RNG, m_parameters.targetGender, m_parameters.targetRatio,
                   m_parameters.tid, m_parameters.sid);
}

void Gen5PIDFrameGenerator::NextNonShinyPID()
{
  m_frame.pid = Gen5PIDRNG::NextNonShinyPIDWord
                  (m_RNG, m_parameters.tid, m_parameters.sid);
}

void Gen5PIDFrameGenerator::NextGiftPID()
{
  uint32_t  shinyChances = m_shinyChances;
  
  do
  {
    m_frame.pid = Gen5PIDRNG::NextGiftPIDWord(m_RNG);
  }
  while (!m_frame.pid.IsShiny(m_parameters.tid, m_parameters.sid) &&
         (--shinyChances > 0));
}

void Gen5PIDFrameGenerator::NextRoamerPID()
{
  uint32_t  shinyChances = m_shinyChances;
  
  do
  {
    m_frame.pid = Gen5PIDRNG::NextRoamerPIDWord(m_RNG);
  }
  while (!m_frame.pid.IsShiny(m_parameters.tid, m_parameters.sid) &&
         (--shinyChances > 0));
}

void Gen5PIDFrameGenerator::NextWildFrame()
{
  CheckLeadAbility();
  
  (this->*m_ESVGenerator)();
  
  // level
  m_RNG.Next();
  
  NextSimpleFrame();
  ApplySync();
  NextHeldItem();
}

void Gen5PIDFrameGenerator::NextFishingFrame()
{
  CheckLeadAbility();
  
  if (m_parameters.leadAbility == EncounterLead::SUCTION_CUPS)
    m_frame.isEncounter = true;
  else
    m_frame.isEncounter = (((m_RNG.Next() >> 32) * 100) >> 32) < 50;
  
  (this->*m_ESVGenerator)();
  
  // level
  m_RNG.Next();
  
  NextSimpleFrame();
  ApplySync();
  NextHeldItem();
}


void Gen5PIDFrameGenerator::NextSwarmFrame()
{
  CheckLeadAbility();
  
  bool  isSwarm = (((m_RNG.Next() >> 32) * 100) >> 32) < 40;
  
  (this->*m_ESVGenerator)();
  
  if (isSwarm)
    m_frame.esv = ESV::SWARM;
  
  // level
  m_RNG.Next();
  
  NextSimpleFrame();
  ApplySync();
  NextHeldItem();
}


void Gen5PIDFrameGenerator::NextDoublesFrame()
{
  CheckLeadAbility();
  
  bool isDoubleBattle = (((m_RNG.Next() >> 32) * 100) >> 32) < 40;
  if (isDoubleBattle)
  {
    // right ESV
    (this->*m_ESVGenerator)();
    uint32_t  rightSlot = ESV::Slot(m_frame.esv);
    
    // right level
    m_RNG.Next();
    
    // left ESV
    (this->*m_ESVGenerator)();
    uint32_t  leftSlot = ESV::Slot(m_frame.esv);
    
    m_frame.esv =
      ESV::MakeDoublesESV(ESV::DOUBLES_GRASS_DOUBLE_TYPE, rightSlot, leftSlot);
    
    // left level
    m_RNG.Next();
    
    NextSimpleFrame();
    ApplySync();
    NextHeldItem();
  }
  else
  {
    (this->*m_ESVGenerator)();
    uint32_t  rightSlot = ESV::Slot(m_frame.esv);
    
    m_frame.esv =
      ESV::MakeDoublesESV(ESV::DOUBLES_GRASS_SINGLE_TYPE, rightSlot, 0);
    
    // level
    m_RNG.Next();
    
    NextSimpleFrame();
    ApplySync();
    NextHeldItem();
  }
}


void Gen5PIDFrameGenerator::NextDustFrame()
{
  uint64_t  rawRNGValue = m_RNG.Next();
  
  m_frame.isEncounter = (((rawRNGValue >> 32) * 1000) >> 32) < 400;
  if (!m_frame.isEncounter)
  {
    LCRNG5    itemRNG(rawRNGValue);
    uint32_t  testValue = ((itemRNG.Next() >> 32) * 1000) >> 32;
    uint64_t  itemRange = 0;
    EncounterItem::Type  itemBase;
    
    if (testValue < 100)
    {
      itemRange = 1000;
      itemBase = EncounterItem::SWIRLING_DUST_STONE_START;
    }
    else if (testValue < 950)
    {
      itemRange = 1700;
      itemBase = EncounterItem::SWIRLING_DUST_GEM_START;
    }
    else
    {
      itemRange = 200;
      itemBase = EncounterItem::SWIRLING_DUST_RARE_ITEM_START;
    }
    
    uint32_t  itemIndex = (((itemRNG.Next() >> 32) * itemRange) >> 32) / 100;
    m_frame.encounterItem = EncounterItem::Type(itemBase + itemIndex);
  }
  else
  {
    m_frame.encounterItem = EncounterItem::NONE;
  }
  
  NextWildFrame();
}

void Gen5PIDFrameGenerator::NextShadowFrame()
{
  uint64_t  rawRNGValue = m_RNG.Next();
  
  m_frame.isEncounter = (((rawRNGValue >> 32) * 1000) >> 32) < 200;
  if (!m_frame.isEncounter)
  {
    LCRNG5    itemRNG(rawRNGValue);
    uint32_t  testValue = ((itemRNG.Next() >> 32) * 1000) >> 32;
    
    if (testValue < 900)
    {
      uint32_t  itemIndex = (((itemRNG.Next() >> 32) * 600) >> 32) / 100;
      
      m_frame.encounterItem =
        EncounterItem::Type(EncounterItem::BRIDGE_SHADOW_ITEM_START +
                            itemIndex);
    }
    else
    {
      m_frame.encounterItem = EncounterItem::PRETTY_WING;
    }
  }
  else
  {
    m_frame.encounterItem = EncounterItem::NONE;
  }
  
  NextWildFrame();
}

void Gen5PIDFrameGenerator::NextStationaryFrame()
{
  CheckLeadAbility();
  NextSimpleFrame();
  ApplySync();
  NextHeldItem();
}

void Gen5PIDFrameGenerator::NextEntraLinkFrame()
{
  NextEntraLinkPID();
  
  // skip 3 frames for something...
  m_RNG.Next();
  m_RNG.Next();
  m_RNG.Next();
  
  m_frame.nature = Nature::Type(((m_RNG.Next() >> 32) * 25) >> 32);
}

void Gen5PIDFrameGenerator::NextHiddenHollowFrame()
{
  // level?
  m_RNG.Next();
  
  CheckLeadAbility();
  NextSimpleFrame();
  ApplySync();
  NextHeldItem();
}

void Gen5PIDFrameGenerator::NextLarvestaEggFrame()
{
  NextRoamerPID();
  m_RNG.Next();
  m_frame.nature = Nature::Type(((m_RNG.Next() >> 32) * 25) >> 32);
}

void Gen5PIDFrameGenerator::NextSimpleFrame()
{
  (this->*m_PIDGenerator)();
  m_frame.nature = Nature::Type(((m_RNG.Next() >> 32) * 25) >> 32);
}

void Gen5PIDFrameGenerator::CheckLeadAbility()
{
  switch (m_parameters.leadAbility)
  {
  case EncounterLead::SYNCHRONIZE:
    m_frame.abilityActivated = (m_RNG.Next() >> 63) == 0x1;
    break;
    
  case EncounterLead::CUTE_CHARM:
    m_frame.abilityActivated =
      ((((m_RNG.Next() >> 32) * 0xFFFF) >> 32) / 0x290) < 67;
    break;
    
  case EncounterLead::OTHER:
  case EncounterLead::SUCTION_CUPS:
    m_RNG.Next();  // still consumed
  case EncounterLead::COMPOUND_EYES:
  default:
    break;
  }
}

void Gen5PIDFrameGenerator::ApplySync()
{
  if ((m_parameters.leadAbility == EncounterLead::SYNCHRONIZE) &&
      m_frame.abilityActivated)
    m_frame.nature = Nature::SYNCHRONIZE;
}

void Gen5PIDFrameGenerator::NextHeldItem()
{
  uint32_t  heldItemPercent = ((m_RNG.Next() >> 32) * 100) >> 32;
  
  if (m_parameters.leadAbility == EncounterLead::COMPOUND_EYES)
  {
    if (heldItemPercent < 60)
    {
      m_frame.heldItem = HeldItem::FIFTY_PERCENT_ITEM;
    }
    else if (heldItemPercent < 80)
    {
      m_frame.heldItem = HeldItem::FIVE_PERCENT_ITEM;
    }
    else if (heldItemPercent < 85)
    {
      m_frame.heldItem = HeldItem::ONE_PERCENT_ITEM;
    }
    else
    {
      m_frame.heldItem = HeldItem::NO_ITEM;
    }
  }
  else
  {
    if (heldItemPercent < 50)
    {
      m_frame.heldItem = HeldItem::FIFTY_PERCENT_ITEM;
    }
    else if (heldItemPercent < 55)
    {
      m_frame.heldItem = HeldItem::FIVE_PERCENT_ITEM;
    }
    else if (heldItemPercent == 55)
    {
      m_frame.heldItem = HeldItem::ONE_PERCENT_ITEM;
    }
    else
    {
      m_frame.heldItem = HeldItem::NO_ITEM;
    }
  }
}


WonderCardFrameGenerator::WonderCardFrameGenerator(const HashedSeed &seed,
                                                   const Parameters &parameters)
  : m_initialValueRNG(seed.rawSeed),
    m_RNG(seed.rawSeed), m_IVRNG(m_RNG, IVRNG::Normal),
    m_frame(seed),
    m_parameters(parameters),
    m_isGLAN((parameters.cardNature == Nature::ANY) &&
             ((parameters.cardGender == Gender::FEMALE) ||
              (parameters.cardGender == Gender::MALE)))
{
  // skip over IVs buffered in IVRNG
  for (uint32_t i = 0; i < 5; ++i)
    m_RNG.AdvanceBuffer();
  
  // skip over 'unused' frames
  uint32_t  ivSkip = m_isGLAN ? 24 : 22;
  for (uint32_t i = 0; i < ivSkip; ++i)
  {
    m_RNG.AdvanceBuffer();
    m_IVRNG.NextIVWord();
  }
  
  m_frame.number = 0;
  m_frame.hasHiddenAbility = m_parameters.cardAbility == Ability::HIDDEN;
  
  if (parameters.startFromLowestFrame)
  {
    uint32_t  skippedFrames =
      seed.GetSkippedPIDFrames(parameters.memoryLinkUsed);
    SkipFrames(skippedFrames);
  }
}

void WonderCardFrameGenerator::SkipFrames(uint32_t numFrames)
{
  uint32_t  i = 0;
  while (i++ < numFrames)
  {
    m_initialValueRNG.Next();
    m_RNG.AdvanceBuffer();
    m_IVRNG.NextIVWord();
  }
  m_frame.number += numFrames;
}

void WonderCardFrameGenerator::AdvanceFrame()
{
  ++m_frame.number;
  m_frame.rngValue = m_initialValueRNG.Next();
  
  m_RNG.AdvanceBuffer();
  m_frame.ivs = m_IVRNG.NextIVWord();
  
  // 'unused' frames
  m_RNG.Next();
  m_RNG.Next();
  
  uint32_t  pid = Gen5PIDRNG::NextRawPIDWord(m_RNG);
  if (m_parameters.cardGender != Gender::ANY)
  {
    pid = Gen5PIDRNG::ForceGender(pid, m_parameters.cardGender,
                                  m_parameters.cardGenderRatio, m_RNG);
  }
  
  switch (m_parameters.cardShininess)
  {
  case WonderCardShininess::NEVER_SHINY:
    pid = Gen5PIDRNG::ForceNonShiny(pid, m_parameters.cardTID,
                                    m_parameters.cardSID);
    break;
  
  case WonderCardShininess::ALWAYS_SHINY:
    pid = Gen5PIDRNG::ForceShiny(pid, m_parameters.cardTID,
                                 m_parameters.cardSID);
    break;
  
  case WonderCardShininess::MAY_BE_SHINY:
  default:
    break;
  }
  
  if (m_parameters.cardAbility == Ability::ANY)
  {
    pid = Gen5PIDRNG::FlipAbility(pid);
  }
  else if (m_parameters.cardAbility == Ability::HIDDEN)
  {
    pid = Gen5PIDRNG::ClearAbility(pid);
  }
  else
  {
    pid = Gen5PIDRNG::ForceAbility(pid, m_parameters.cardAbility);
  }
  
  m_frame.pid = pid;
  
  if (m_parameters.cardNature == Nature::ANY)
  {
    // skip 'unused' frames
    m_RNG.Next();
    
    m_frame.nature = Nature::Type(((m_RNG.Next() >> 32) * 25) >> 32);
  }
  else
  {
    m_frame.nature = m_parameters.cardNature;
  }
}


Gen5BreedingFrameGenerator::Gen5BreedingFrameGenerator
    (const HashedSeed &seed, const Parameters &parameters)
  : m_parameters(parameters),
    m_NextSeed(seed.rawSeed),
    m_RNG(seed.rawSeed),
    m_frame(seed)
{
  m_frame.number = 0;
}

void Gen5BreedingFrameGenerator::AdvanceFrame()
{
  m_frame.ResetInheritance();
  
  m_RNG.Seed(m_NextSeed);
  
  ++m_frame.number;
  
  m_frame.rngValue = m_RNG.Next();
  
  switch (m_parameters.femaleSpecies)
  {
  case FemaleParent::OTHER:
    default:
    m_frame.species = EggSpecies::OTHER;
    break;
    
  case FemaleParent::NIDORAN_FEMALE:
    m_frame.species = (m_frame.rngValue >> 63) ?
      EggSpecies::NIDORAN_M : EggSpecies::NIDORAN_F;
    break;
    
  case FemaleParent::ILLUMISE:
    m_frame.species = (m_frame.rngValue >> 63) ?
      EggSpecies::ILLUMISE : EggSpecies::VOLBEAT;
    break;
  }
  
  m_NextSeed = m_RNG.Seed();
  
  m_frame.nature = Nature::Type(((m_RNG.Next() >> 32) * 25) >> 32);
  
  if (m_parameters.usingEverstone)
  {
    m_frame.everstoneActivated = (m_RNG.Next() >> 63) == 1;
    if (m_frame.everstoneActivated)
      m_frame.nature = Nature::EVERSTONE;
  }
  
  m_frame.inheritsHiddenAbility = (((m_RNG.Next() >> 32) * 5) >> 32) >= 2;
  
  if (m_parameters.usingDitto)
    m_RNG.Next();
  
  uint32_t  numInherited = 0;
  while (numInherited < 3)
  {
    uint32_t  ivIndex = ((m_RNG.Next() >> 32) * 6) >> 32;
    uint32_t  parent = m_RNG.Next() >> 63;
    
    if (m_frame.inheritance[ivIndex] == Gen5BreedingFrame::NotInherited)
    {
      m_frame.inheritance[ivIndex] = (parent == 1) ?
          Gen5BreedingFrame::ParentX :
          Gen5BreedingFrame::ParentY;
      
      ++numInherited;
    }
  }
  
  m_frame.pid = Gen5PIDRNG::NextEggPIDWord(m_RNG);
  if (m_parameters.internationalParents)
  {
    uint32_t  shinyChecks = 0;
    while (!m_frame.pid.IsShiny(m_parameters.tid, m_parameters.sid) &&
           (++shinyChecks < 6))
    {
      m_frame.pid = Gen5PIDRNG::NextEggPIDWord(m_RNG);
    }
  }
}


DreamRadarFrameGenerator::DreamRadarFrameGenerator(const HashedSeed &seed,
                                                   const Parameters &parameters)
  : m_PIDRNG(seed.rawSeed),
    m_MTRNG(seed.rawSeed >> 32), m_IVRNG(m_MTRNG, IVRNG::Normal),
    m_frame(seed),
    m_parameters(parameters),
    m_pidAdvancements(parameters.GetPIDRNGAdvancements())
{
  m_frame.number = 0;
  
  int32_t  skippedFrames = seed.GetSkippedPIDFrames(parameters.memoryLinkUsed);
  m_PIDRNG.AdvanceBuffer(skippedFrames);
  
  if (!parameters.memoryLinkUsed)
    m_PIDRNG.AdvanceBuffer();
  
  // start from (b2w2) IV frame 8 (bw frame 10)
  skippedFrames = 9;
  while (skippedFrames-- > 0)
    m_IVRNG.NextIVWord();
  
  switch (parameters.frameType)
  {
  case GenieFrame:
    // genies use stats from the second slot
    m_parameters.slot += 1;
    
    // fall through
    
  case NonGenieLegendaryFrame:
    // legendaries and genies have 100% male PIDs
    m_parameters.targetGender = Gender::MALE;
    m_parameters.targetRatio = Gender::MALE_ONLY;
    break;
    
  default:
    break;
  }
  
  // skip over IV frames
  skippedFrames = (m_parameters.slot - 1) * 13;
  while (skippedFrames-- > 0)
    m_IVRNG.NextIVWord();
}

void DreamRadarFrameGenerator::SkipFrames(uint32_t numFrames)
{
  m_PIDRNG.AdvanceBuffer(numFrames * 2);
  
  uint32_t  i = 0;
  while (i++ < numFrames)
  {
    m_IVRNG.NextIVWord();
    m_IVRNG.NextIVWord();
  }
  
  m_frame.number += numFrames;
}

void DreamRadarFrameGenerator::AdvanceFrame()
{
  m_frame.ivs = m_IVRNG.NextIVWord();
  
  m_PIDRNG.AdvanceBuffer();
  
  ++m_frame.number;
  
  // skip 1 frame for some reason...
  m_frame.rngValue = m_PIDRNG.Next();
  
  // skip PID frames of earlier slots
  int32_t  skippedFrames = m_pidAdvancements;
  while (skippedFrames-- > 0)
    m_PIDRNG.Next();
  
  m_frame.pid =
    Gen5PIDRNG::NextDreamRadarPIDWord(m_PIDRNG, m_parameters.targetGender,
                                      m_parameters.targetRatio,
                                      m_parameters.tid, m_parameters.sid);
  
  // skip 2 frames for something...
  m_PIDRNG.Next();
  m_PIDRNG.Next();
  
  m_frame.nature = Nature::Type(((m_PIDRNG.Next() >> 32) * 25) >> 32);
  
  // skipped when advancing via spinner
  m_PIDRNG.AdvanceBuffer();
  m_IVRNG.NextIVWord();
}

DreamRadarFrame DreamRadarFrameGenerator::FrameFromIVFrame
  (const HashedIVFrame &ivFrame, const Parameters &parameters)
{
  DreamRadarFrame  result(ivFrame.seed);
  
  result.ivs = ivFrame.ivs;
  result.number = parameters.GetDRFrameNumber(ivFrame.number);
  
  LCRNG5  pidRNG(0);
  result.seed.SeedAndSkipPIDFrames(pidRNG, parameters.memoryLinkUsed);
  if (!parameters.memoryLinkUsed)
    pidRNG.Next();
  
  result.rngValue = pidRNG.Next();
  
  uint32_t  skipped = parameters.GetPIDRNGAdvancements();
  skipped += (result.number - 1) * 2;
  while (skipped-- > 0)
    pidRNG.Next();
  
  result.pid =
    Gen5PIDRNG::NextDreamRadarPIDWord(pidRNG, parameters.targetGender,
                                      parameters.targetRatio,
                                      parameters.tid, parameters.sid);
  
  // skip 2 frames for something...
  pidRNG.Next();
  pidRNG.Next();
  
  result.nature = Nature::Type(((pidRNG.Next() >> 32) * 25) >> 32);
  
  return result;
}


namespace
{

static const uint32_t HiddenHollowSpawnSlotThreshold[] =
{ 0, 4, 19, 20, 24, 34, 59, 60, 64, 74, 99 };

static uint32_t GetHiddenHollowSpawnSlot(uint32_t value)
{
  uint32_t  slot = 0;
  while (value > HiddenHollowSpawnSlotThreshold[slot])
    ++slot;
  
  return slot;
}

}

HiddenHollowSpawnFrameGenerator::HiddenHollowSpawnFrameGenerator
  (const HashedSeed &seed, bool memoryLinkUsed)
  : m_PIDRNG(seed.rawSeed),
    m_frame(seed)
{
  m_frame.number = 0;
  
  int32_t  skippedFrames = seed.GetSkippedPIDFrames(memoryLinkUsed);
  m_PIDRNG.AdvanceBuffer(skippedFrames);
}

void HiddenHollowSpawnFrameGenerator::SkipFrames(uint32_t numFrames)
{
  m_PIDRNG.AdvanceBuffer(numFrames);
  m_frame.number += numFrames;
}

void HiddenHollowSpawnFrameGenerator::AdvanceFrame()
{
  m_PIDRNG.AdvanceBuffer();
  m_frame.rngValue = m_PIDRNG.PeekNext();
  
  ++m_frame.number;
  
  m_frame.isSpawn = (((m_PIDRNG.Next() >> 32) * 100) >> 32) < 5;
  m_frame.group = m_PIDRNG.Next() >> 62;
  m_frame.slot =
    GetHiddenHollowSpawnSlot(((m_PIDRNG.Next() >> 32) * 100) >> 32);
  m_frame.genderPercentage = ((m_PIDRNG.Next() >> 32) * 100) >> 32;
}

}
