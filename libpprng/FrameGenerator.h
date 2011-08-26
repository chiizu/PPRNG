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

#ifndef FRAME_GENERATOR_H
#define FRAME_GENERATOR_H

#include "BasicTypes.h"
#include "Frame.h"
#include "LinearCongruentialRNG.h"
#include "MersenneTwisterRNG.h"
#include "IVRNG.h"
#include "PIDRNG.h"
#include "RNGWrappers.h"

namespace pprng
{

template <int Method = 1>
class Gen34FrameGenerator
{
public:
  typedef Gen34Frame                Frame;
  typedef BufferedRNG<LCRNG34, 5>   RNG;
  typedef Gen34PIDRNG<Method, RNG>  PIDRNG;
  typedef Gen34IVRNG<Method, RNG>   IVRNG;
  
  Gen34FrameGenerator(uint32_t seed)
    : m_RNG(seed), m_PIDRNG(m_RNG), m_IVRNG(m_RNG)
  {
    m_frame.seed = seed;
    m_frame.number = 0;
  }
  
  void AdvanceFrame()
  {
    m_RNG.AdvanceBuffer();
    
    m_frame.frameSeed = m_RNG.PeekNext();
    ++m_frame.number;
    
    m_frame.pid = m_PIDRNG.NextPIDWord();
    m_frame.ivs = m_IVRNG.NextIVWord();
  }
  
  const Frame& CurrentFrame() { return m_frame; }
  
  //uint32_t CurrentSeed() { return m_RNG.PeekNext(); }
  
private:
  RNG             m_RNG;
  PIDRNG          m_PIDRNG;
  IVRNG           m_IVRNG;
  Frame           m_frame;
};

typedef Gen34FrameGenerator<1> Method1FrameGenerator;
typedef Gen34FrameGenerator<2> Method2FrameGenerator;
typedef Gen34FrameGenerator<3> Method3FrameGenerator;
typedef Gen34FrameGenerator<4> Method4FrameGenerator;

class CGearIVFrameGenerator
{
public:
  typedef CGearIVFrame             Frame;
  typedef MTRNG                    RNG;
  typedef Gen5BufferingIVRNG<RNG>  IVRNG;
  
  enum FrameType
  {
    Normal = IVRNG::Normal,
    Roamer = IVRNG::Roamer
  };
  
  CGearIVFrameGenerator(uint32_t seed, FrameType frameType,
                        bool skipFirstTwoFrames = true);
  
  void AdvanceFrame();
  
  const Frame& CurrentFrame() { return m_frame; }
  
private:
  RNG             m_RNG;
  IVRNG           m_IVRNG;
  Frame           m_frame;
};

class HashedIVFrameGenerator
{
public:
  typedef HashedIVFrame            Frame;
  typedef MTRNG                    RNG;
  typedef Gen5BufferingIVRNG<RNG>  IVRNG;
  
  enum FrameType
  {
    Normal = IVRNG::Normal,
    Roamer = IVRNG::Roamer
  };
  
  HashedIVFrameGenerator(const HashedSeed &seed, FrameType frameType);
  
  void AdvanceFrame();
  
  const Frame& CurrentFrame() { return m_frame; }
  
private:
  RNG       m_RNG;
  IVRNG     m_IVRNG;
  Frame     m_frame;
};


class Gen5PIDFrameGenerator
{
public:
  typedef Gen5PIDFrame            Frame;
  typedef BufferedRNG<LCRNG5, 6>  RNG;
  typedef Gen5PIDRNG<RNG>         PIDRNG;
  
  enum FrameType
  {
    GrassCaveFrame = 0,
    SurfingFrame,
    FishingFrame,
    ShakingGrassFrame,
    SwirlingDustFrame,
    BridgeShadowFrame,
    WaterSpotSurfingFrame,
    WaterSpotFishingFrame,
    EntraLinkFrame,
    StationaryFrame,
    ZekReshVicFrame,
    StarterFossilGiftFrame,
    RoamerFrame,
    
    NumFrameTypes
  };
  
  Gen5PIDFrameGenerator(const HashedSeed &seed, FrameType frameType,
                        bool useCompoundEyes, uint32_t tid, uint32_t sid);
  
  void AdvanceFrame();
  
  const Frame& CurrentFrame() const { return m_frame; }
  
  bool GeneratesESV() const;
  bool GeneratesIsEncounter() const;
  
private:
  typedef void (Gen5PIDFrameGenerator::*PIDFrameGenerator)();
  
  const PIDFrameGenerator  m_PIDFrameGenerator;
  
  
  typedef void (Gen5PIDFrameGenerator::*ESVGenerator)();
  
  const ESVGenerator  m_ESVGenerator;
  
  struct FrameGeneratorInfo
  {
    PIDRNG::Type       pidType;
    PIDFrameGenerator  pidFrameGenerator;
    ESVGenerator       esvGenerator;
  };
  
  static const FrameGeneratorInfo  s_FrameGeneratorInfo[NumFrameTypes];
  
  void NextWildFrame();
  void NextFishingFrame();
  void NextDustFrame();
  void NextShadowFrame();
  void NextStationaryFrame();
  void NextSimpleFrame();
  
  void NextSync();
  
  void LandESV();
  void WaterESV();
  void NoESV();
  
  void NextHeldItem();
  
  RNG         m_RNG;
  PIDRNG      m_PIDRNG;
  Frame       m_frame;
  const bool  m_useCompoundEyes;
};

class WonderCardFrameGenerator
{
public:
  typedef WonderCardFrame            Frame;
  typedef BufferedRNG<LCRNG5, 11>    RNG;
  typedef Gen5NonBufferingIVRNG<RNG> IVRNG;
  typedef Gen5PIDRNG<RNG>            PIDRNG;
  
  WonderCardFrameGenerator(const HashedSeed &seed, bool canBeShiny,
                           uint32_t tid, uint32_t sid)
    : m_RNG(seed.m_rawSeed), m_IVRNG(m_RNG, IVRNG::Normal),
      m_PIDRNG(m_RNG, (canBeShiny ? PIDRNG::GiftPID : PIDRNG::GiftNoShinyPID),
               tid, sid),
      m_frame(seed)
  {
    // skip over 'unused' frames
    for (uint32_t i = 0; i < 22; ++i)
    {
      m_RNG.AdvanceBuffer();
    }
    m_frame.number = 0;
  }
  
  void AdvanceFrame()
  {
    m_RNG.AdvanceBuffer();
    
    ++m_frame.number;
    m_frame.ivs = m_IVRNG.NextIVWord();
    
    // skip 2 'unused' frames
    m_RNG.Next();
    m_RNG.Next();
    
    m_frame.pid = m_PIDRNG.NextPIDWord();
    
    // skip 1 'unused' frame
    m_RNG.Next();
    
    m_frame.nature =
      static_cast<Nature::Type>(((m_RNG.Next() >> 32) * 25) >> 32);
  }
  
  const Frame& CurrentFrame() { return m_frame; }
  
private:
  RNG       m_RNG;
  IVRNG     m_IVRNG;
  PIDRNG    m_PIDRNG;
  Frame     m_frame;
};

class Gen5TrainerIDFrameGenerator
{
public:
  typedef Gen5TrainerIDFrame  Frame;
  typedef LCRNG5              RNG;
  
  Gen5TrainerIDFrameGenerator(const HashedSeed &seed)
    : m_RNG(seed.m_rawSeed),
      m_frame(seed)
  {
    m_frame.number = 0;
  }
  
  void AdvanceFrame()
  {
    ++m_frame.number;
    
    uint32_t  fullID = ((m_RNG.Next() >> 32) * 0xFFFFFFFFULL) >> 32;
    m_frame.tid = fullID & 0xffff;
    m_frame.sid = fullID >> 16;
  }
  
  const Frame& CurrentFrame() { return m_frame; }
  
private:
  RNG       m_RNG;
  Frame     m_frame;
};

class Gen5BreedingFrameGenerator
{
public:
  typedef Gen5BreedingFrame  Frame;
  typedef QueuedRNG<LCRNG5>  RNG;
  typedef Gen5PIDRNG<RNG>    PIDRNG;
  
  Gen5BreedingFrameGenerator(const HashedSeed &seed, bool isInternational,
                             bool hasEverstone, bool hasDitto,
                             uint32_t tid, uint32_t sid);
  
  void AdvanceFrame();
  
  const Frame& CurrentFrame() { return m_frame; }
  
private:
  const bool      m_hasDitto, m_hasEverstone, m_isInternational;
  const uint32_t  m_tid, m_sid;
  
  RNG       m_RNG;
  PIDRNG    m_PIDRNG;
  Frame     m_frame;
};

}

#endif
