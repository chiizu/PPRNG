/*
  Copyright (C) 2011 chiizu
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

namespace pprng
{

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
  m_frame.number = 0;
}

void HashedIVFrameGenerator::AdvanceFrame()
{
  ++m_frame.number;
  m_frame.ivs = m_IVRNG.NextIVWord();
}


Gen5PIDFrameGenerator::Gen5PIDFrameGenerator
  (const HashedSeed &seed, const Gen5PIDFrameGenerator::Parameters &parameters)
  : m_PIDFrameGenerator
      (s_FrameGeneratorInfo[parameters.frameType].pidFrameGenerator),
    m_ESVGenerator(s_FrameGeneratorInfo[parameters.frameType].esvGenerator),
    m_RNG(seed.rawSeed),
    m_PIDRNG(m_RNG, s_FrameGeneratorInfo[parameters.frameType].pidType,
             parameters.tid, parameters.sid),
    m_frame(seed), m_useCompoundEyes(parameters.useCompoundEyes)
{
  m_frame.number = 0;
  m_frame.synched = false;
  m_frame.isSwarm = false;
  m_frame.esv = ESV::Value(0);
  m_frame.heldItem = HeldItem::NO_ITEM;
  m_frame.isEncounter = 0;
}

void Gen5PIDFrameGenerator::AdvanceFrame()
{
  m_RNG.AdvanceBuffer();
  
  ++m_frame.number;
  
  (this->*m_PIDFrameGenerator)();
}


const Gen5PIDFrameGenerator::FrameGeneratorInfo
  Gen5PIDFrameGenerator::s_FrameGeneratorInfo
    [Gen5PIDFrameGenerator::NumFrameTypes] =
{
  // GrassCaveFrame
  { PIDRNG::WildPID,
    &Gen5PIDFrameGenerator::NextWildFrame,
    &Gen5PIDFrameGenerator::LandESV },
  // SurfingFrame
  { PIDRNG::WildPID,
    &Gen5PIDFrameGenerator::NextWildFrame,
    &Gen5PIDFrameGenerator::WaterESV },
  // FishingFrame
  { PIDRNG::WildPID,
    &Gen5PIDFrameGenerator::NextFishingFrame,
    &Gen5PIDFrameGenerator::WaterESV },
  // SwarmFrame
  { PIDRNG::WildPID,
    &Gen5PIDFrameGenerator::NextSwarmFrame,
    &Gen5PIDFrameGenerator::LandESV },
  // ShakingGrassFrame
  { PIDRNG::WildPID,
    &Gen5PIDFrameGenerator::NextWildFrame,
    &Gen5PIDFrameGenerator::LandESV },
  // SwirlingDustFrame
  { PIDRNG::WildPID,
    &Gen5PIDFrameGenerator::NextDustFrame,
    &Gen5PIDFrameGenerator::LandESV },
  // BridgeShadowFrame
  { PIDRNG::WildPID,
    &Gen5PIDFrameGenerator::NextShadowFrame,
    &Gen5PIDFrameGenerator::LandESV },
  // WaterSpotSurfingFrame
  { PIDRNG::WildPID,
    &Gen5PIDFrameGenerator::NextWildFrame,
    &Gen5PIDFrameGenerator::WaterESV },
  // WaterSpotFishingFrame
  { PIDRNG::WildPID,
    &Gen5PIDFrameGenerator::NextWildFrame,
    &Gen5PIDFrameGenerator::WaterESV },
  // EntraLinkFrame
  { PIDRNG::EntraLinkPID,
    &Gen5PIDFrameGenerator::NextSimpleFrame,
    &Gen5PIDFrameGenerator::NoESV },
  // StationaryFrame
  { PIDRNG::WildPID,
    &Gen5PIDFrameGenerator::NextStationaryFrame,
    &Gen5PIDFrameGenerator::NoESV },
  // ZekReshVicFrame
  { PIDRNG::ZekReshVicPID,
    &Gen5PIDFrameGenerator::NextStationaryFrame,
    &Gen5PIDFrameGenerator::NoESV },
  // StarterFossilGiftFrame
  { PIDRNG::GiftPID,
    &Gen5PIDFrameGenerator::NextSimpleFrame,
    &Gen5PIDFrameGenerator::NoESV },
  // RoamerFrame
  { PIDRNG::RoamerPID,
    &Gen5PIDFrameGenerator::NextSimpleFrame,
    &Gen5PIDFrameGenerator::NoESV }
};


bool Gen5PIDFrameGenerator::GeneratesESV() const
{
  return (m_PIDFrameGenerator != &Gen5PIDFrameGenerator::NextStationaryFrame) &&
         (m_PIDFrameGenerator != &Gen5PIDFrameGenerator::NextSimpleFrame);
}

bool Gen5PIDFrameGenerator::GeneratesIsEncounter() const
{
  return (m_PIDFrameGenerator == &Gen5PIDFrameGenerator::NextFishingFrame) ||
         (m_PIDFrameGenerator == &Gen5PIDFrameGenerator::NextDustFrame) ||
         (m_PIDFrameGenerator == &Gen5PIDFrameGenerator::NextShadowFrame);
}

void Gen5PIDFrameGenerator::LandESV()
{
  uint32_t  raw_esv = (m_RNG.Next() >> 48) / 0x290;
  
  m_frame.esv = ESV::Gen5Land(raw_esv);
}

void Gen5PIDFrameGenerator::WaterESV()
{
  uint32_t  raw_esv = (m_RNG.Next() >> 48) / 0x290;
  
  m_frame.esv = ESV::Gen5Surfing(raw_esv);
}

void Gen5PIDFrameGenerator::NoESV()
{
  m_frame.esv = ESV::Value(0);
}


void Gen5PIDFrameGenerator::NextWildFrame()
{
  if (!m_useCompoundEyes)
    NextSync();
  
  (this->*m_ESVGenerator)();
  
  // level
  m_RNG.Next();
  
  NextSimpleFrame();
  NextHeldItem();
}

void Gen5PIDFrameGenerator::NextFishingFrame()
{
  if (!m_useCompoundEyes)
    NextSync();
  
  m_frame.isEncounter = ((m_RNG.Next() >> 48) / 0x290) < 50;
  WaterESV();
  
  // level
  m_RNG.Next();
  
  NextSimpleFrame();
  NextHeldItem();
}


void Gen5PIDFrameGenerator::NextSwarmFrame()
{
  if (!m_useCompoundEyes)
    NextSync();
  
  m_frame.isSwarm = (((m_RNG.Next() >> 32) * 100) >> 32) < 40;
  LandESV();
  
  // level
  m_RNG.Next();
  
  NextSimpleFrame();
  NextHeldItem();
}

void Gen5PIDFrameGenerator::NextDustFrame()
{
  m_frame.isEncounter = (((m_RNG.Next() >> 32) * 1000) >> 32) < 400;
  NextWildFrame();
}

void Gen5PIDFrameGenerator::NextShadowFrame()
{
  m_frame.isEncounter = (((m_RNG.Next() >> 32) * 1000) >> 32) < 200;
  NextWildFrame();
}

void Gen5PIDFrameGenerator::NextStationaryFrame()
{
  if (!m_useCompoundEyes)
    NextSync();
  
  NextSimpleFrame();
  NextHeldItem();
}

void Gen5PIDFrameGenerator::NextSimpleFrame()
{
  m_frame.pid = m_PIDRNG.NextPIDWord();
  m_frame.nature = static_cast<Nature::Type>(((m_RNG.Next() >> 32) * 25) >> 32);
}

void Gen5PIDFrameGenerator::NextSync()
{
  m_frame.synched = (m_RNG.Next() >> 63) == 0x1;
}

void Gen5PIDFrameGenerator::NextHeldItem()
{
  uint32_t  heldItemPercent = ((m_RNG.Next() >> 32) * 100) >> 32;
  
  if (m_useCompoundEyes)
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
  : m_RNG(seed.rawSeed), m_IVRNG(m_RNG, IVRNG::Normal),
    m_PIDRNG(m_RNG, PIDRNG::GiftNoShinyPID, parameters.tid, parameters.sid),
    m_frame(seed),
    m_ivSkip(((parameters.cardNature == Nature::ANY) &&
              ((parameters.cardGender == Gender::FEMALE) ||
               (parameters.cardGender == Gender::MALE))) ? 24 : 22),
    m_pidSkip((m_ivSkip == 22) ? 2 : 0),
    m_natureSkip((m_ivSkip == 22) ? 1 : 4)
{
  // skip over IVs buffered in IVRNG
  for (uint32_t i = 0; i < 5; ++i)
    m_RNG.AdvanceBuffer();
  
  // skip over 'unused' frames
  for (uint32_t i = 0; i < m_ivSkip; ++i)
  {
    m_RNG.AdvanceBuffer();
    m_IVRNG.NextIVWord();
  }
  m_frame.number = 0;
  
  if (parameters.startFromLowestFrame)
  {
    uint32_t  skippedFrames = seed.GetSkippedPIDFrames();
    while (skippedFrames-- > 0)
    {
      m_RNG.AdvanceBuffer();
      m_IVRNG.NextIVWord();
      ++m_frame.number;
    }
  }
}

void WonderCardFrameGenerator::AdvanceFrame()
{
  m_RNG.AdvanceBuffer();
  
  ++m_frame.number;
  m_frame.ivs = m_IVRNG.NextIVWord();
  
  // 'unused' frames
  uint32_t  i = m_pidSkip;
  while (i-- > 0)
    m_RNG.Next();
  
  m_frame.pid = m_PIDRNG.NextPIDWord();
  
  // skip 'unused' frames
  i = m_natureSkip;
  while (i-- > 0)
    m_RNG.Next();
  
  m_frame.nature =
    static_cast<Nature::Type>(((m_RNG.Next() >> 32) * 25) >> 32);
}


Gen5BreedingFrameGenerator::Gen5BreedingFrameGenerator
    (const HashedSeed &seed, const Parameters &parameters)
  : m_parameters(parameters),
    m_RNG(seed.rawSeed),
    m_PIDRNG(m_RNG, PIDRNG::EggPID, parameters.tid, parameters.sid),
    m_frame(seed)
{
  m_frame.number = 0;
}

void Gen5BreedingFrameGenerator::AdvanceFrame()
{
  m_frame.ResetInheritance();
  
  m_RNG.AdvanceBuffer();
  
  ++m_frame.number;
  
  m_frame.species = m_RNG.Next() >> 63;
  
  m_frame.nature = static_cast<Nature::Type>(((m_RNG.Next() >> 32) * 25) >> 32);
  
  if (m_parameters.usingEverstone)
    m_frame.everstoneActivated = (m_RNG.Next() >> 63) == 1;
  
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
  
  m_frame.pid = m_PIDRNG.NextPIDWord();
  if (m_parameters.internationalParents)
  {
    uint32_t  shinyChecks = 0;
    while (!m_frame.pid.IsShiny(m_parameters.tid, m_parameters.sid) &&
           (++shinyChecks < 6))
    {
      m_frame.pid = m_PIDRNG.NextPIDWord();
    }
  }
}

}
