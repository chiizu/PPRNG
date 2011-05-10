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

CGearIVFrameGenerator::CGearIVFrameGenerator(uint32_t seed, FrameType frameType)
  : m_RNG(seed), m_IVRNG(m_RNG, IVRNG::FrameType(frameType))
{
  m_frame.seed = seed;
  m_frame.number = 0;
  
  // C-Gear starts on 'frame 3'
  m_RNG.AdvanceBuffer();
  m_RNG.AdvanceBuffer();
}

void CGearIVFrameGenerator::AdvanceFrame()
{
  m_RNG.AdvanceBuffer();
  
  ++m_frame.number;
  m_frame.ivs = m_IVRNG.NextIVWord();
}


HashedIVFrameGenerator::HashedIVFrameGenerator
    (const HashedSeed &seed, FrameType frameType)
  : m_RNG(seed.m_rawSeed >> 32), m_IVRNG(m_RNG, IVRNG::FrameType(frameType)),
    m_frame(seed)
{
  m_frame.number = 0;
}

void HashedIVFrameGenerator::AdvanceFrame()
{
  m_RNG.AdvanceBuffer();
  
  ++m_frame.number;
  m_frame.ivs = m_IVRNG.NextIVWord();
}


Gen5PIDFrameGenerator::Gen5PIDFrameGenerator
    (const HashedSeed &seed, FrameType frameType, bool useCompoundEyes,
     uint32_t tid, uint32_t sid)
  : m_PIDFrameGenerator(s_FrameGeneratorInfo[frameType].pidFrameGenerator),
    m_ESVGenerator(s_FrameGeneratorInfo[frameType].esvGenerator),
    m_RNG(seed.m_rawSeed),
    m_PIDRNG(m_RNG, s_FrameGeneratorInfo[frameType].pidType, tid, sid),
    m_frame(seed), m_useCompoundEyes(useCompoundEyes)
{
  m_frame.number = 0;
  m_frame.synched = false;
  m_frame.esv = 0;
  m_frame.heldItem = HeldItem::NO_ITEM;
  m_frame.canFish = 0;
  m_frame.findItem = 0;
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
  // ShakingGrassFrame
  { PIDRNG::WildPID,
    &Gen5PIDFrameGenerator::NextWildFrame,
    &Gen5PIDFrameGenerator::LandESV },
  // SwirlingDustFrame
  { PIDRNG::WildPID,
    &Gen5PIDFrameGenerator::NextDustOrShadowFrame,
    &Gen5PIDFrameGenerator::LandESV },
  // BridgeShadowFrame
  { PIDRNG::WildPID,
    &Gen5PIDFrameGenerator::NextDustOrShadowFrame,
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

bool Gen5PIDFrameGenerator::GeneratesCanFish() const
{
  return m_PIDFrameGenerator == &Gen5PIDFrameGenerator::NextFishingFrame;
}

bool Gen5PIDFrameGenerator::GeneratesFindItem() const
{
  return m_PIDFrameGenerator == &Gen5PIDFrameGenerator::NextDustOrShadowFrame;
}


void Gen5PIDFrameGenerator::LandESV()
{
  static const uint32_t  LandESVTable[100] =
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9,10,11 };
  
  // uint32_t  raw_esv = ((m_RNG.Next() >> 32) * 0x64) >> 32;
  // above is wrong for 0xB34F765F546EFDAC
  uint32_t  raw_esv = (m_RNG.Next() >> 48) / 0x290;
  
  m_frame.esv = LandESVTable[raw_esv];
}

void Gen5PIDFrameGenerator::WaterESV()
{
  static const uint32_t  WaterESVTable[100] =
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4 };
  
  // uint32_t  raw_esv = ((m_RNG.Next() >> 32) * 0x64) >> 32;
  // above is wrong for 0xB34F765F546EFDAC
  uint32_t  raw_esv = (m_RNG.Next() >> 48) / 0x290;
  
  m_frame.esv = WaterESVTable[raw_esv];
}

void Gen5PIDFrameGenerator::NoESV()
{
  m_frame.esv = 0;
}


void Gen5PIDFrameGenerator::NextWildFrame()
{
  if (!m_useCompoundEyes)
    NextSync();
  
  (this->*m_ESVGenerator)();
  
  // unknown
  m_RNG.Next();
  
  NextSimpleFrame();
  NextHeldItem();
}

void Gen5PIDFrameGenerator::NextFishingFrame()
{
  if (!m_useCompoundEyes)
    NextSync();
  
  m_frame.canFish = (m_RNG.Next() >> 63) == 0;
  WaterESV();
  
  // unknown
  m_RNG.Next();
  
  NextSimpleFrame();
  NextHeldItem();
}

void Gen5PIDFrameGenerator::NextDustOrShadowFrame()
{
  m_frame.findItem = (((m_RNG.Next() >> 32) * 1000) >> 32) >= 400;
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


Gen5BreedingFrameGenerator::Gen5BreedingFrameGenerator
    (const HashedSeed &seed, bool isInternational,
     bool hasEverstone, bool hasDitto,
     uint32_t tid, uint32_t sid)
  : m_hasDitto(hasDitto), m_hasEverstone(hasEverstone),
    m_isInternational(isInternational), m_tid(tid), m_sid(sid),
    m_RNG(seed.m_rawSeed),
    m_PIDRNG(m_RNG, PIDRNG::EggPID, tid, sid),
    m_frame(seed)
{
  m_frame.number = 0;
}

void Gen5BreedingFrameGenerator::AdvanceFrame()
{
  m_frame.ResetInheritance();
  
  m_RNG.AdvanceQueue();
  
  ++m_frame.number;
  
  m_frame.nature = static_cast<Nature::Type>(((m_RNG.Next() >> 32) * 25) >> 32);
  
  if (m_hasEverstone)
    m_frame.everstoneActivated = (m_RNG.Next() >> 63) == 1;
  
  m_frame.dreamWorldAbilityPassed = (((m_RNG.Next() >> 32) * 5) >> 32) >= 2;
  
  if (m_hasDitto)
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
  if (m_isInternational)
  {
    uint32_t  shinyChecks = 0;
    while (!m_frame.pid.IsShiny(m_tid, m_sid) && (++shinyChecks < 6))
    {
      m_frame.pid = m_PIDRNG.NextPIDWord();
    }
  }
}

}
