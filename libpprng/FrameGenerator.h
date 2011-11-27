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
  
private:
  RNG     m_RNG;
  PIDRNG  m_PIDRNG;
  IVRNG   m_IVRNG;
  Frame   m_frame;
};

typedef Gen34FrameGenerator<1> Method1FrameGenerator;
typedef Gen34FrameGenerator<2> Method2FrameGenerator;
typedef Gen34FrameGenerator<3> Method3FrameGenerator;
typedef Gen34FrameGenerator<4> Method4FrameGenerator;

template <class Method>
class Gen4EncounterFrameGenerator
{
public:
  typedef Gen4EncounterFrame   Frame;
  typedef QueuedRNG<LCRNG34>   RNG;
  typedef Gen34PIDRNG<1, RNG>  PIDRNG;
  typedef Gen34IVRNG<1, RNG>   IVRNG;
  
  enum EncounterType
  {
    GrassCaveEncounter = 0,
    SurfingEncounter,
    OldRodFishingEncounter,
    GoodRodFishingEncounter,
    SuperRodFishingEncounter,
    StationaryEncounter,
    
    NumEncounterTypes
  };
  
  static uint32_t MonsterFrameForEncounterType(EncounterType t)
  {
    uint32_t  offsets[NumEncounterTypes] = { 2, 3, 4, 4, 4, 1 };
    return offsets[t];
  }
  
  struct Parameters
  {
    EncounterType  encounterType;
    Nature::Type   syncNature;
    
    Parameters()
      : encounterType(GrassCaveEncounter), syncNature(Nature::ANY)
    {}
  };
  
  Gen4EncounterFrameGenerator(uint32_t seed, const Parameters &parameters)
    : m_RNG(seed), m_PIDRNG(m_RNG), m_IVRNG(m_RNG), m_ProfElmResponseRNG(seed),
      m_parameters(parameters)
  {
    m_frame.seed = seed;
    m_frame.number = 0;
    m_frame.synched = false;
    m_frame.isEncounter = true;
    m_frame.esv = ESV::Value(0);
    m_ProfElmResponseRNG.Next();
  }
  
  void AdvanceFrame()
  {
    m_RNG.AdvanceBuffer();
    
    ++m_frame.number;
    m_frame.method1Number = m_frame.number;
    m_frame.profElmResponse =
      ProfElmResponses::CalcResponse(m_ProfElmResponseRNG.Next());
    
    switch (m_parameters.encounterType)
    {
    case GrassCaveEncounter:
      m_frame.esv = ESV::Gen4Land(Method::CalculatePercentage(m_RNG.Next()));
      break;
    case SurfingEncounter:
      m_frame.esv = ESV::Gen4Surfing(Method::CalculatePercentage(m_RNG.Next()));
      m_RNG.Next(); // level calc?
      break;
    case OldRodFishingEncounter:
      m_frame.isEncounter = IsFishingEncounter(Method::OldRodThreshold);
      m_frame.esv =
        Method::OldRodESV(Method::CalculatePercentage(m_RNG.Next()));
      m_RNG.Next(); // level calc?
      break;
    case GoodRodFishingEncounter:
      m_frame.isEncounter = IsFishingEncounter(Method::GoodRodThreshold);
      m_frame.esv =
        Method::GoodRodESV(Method::CalculatePercentage(m_RNG.Next()));
      m_RNG.Next(); // level calc?
      break;
    case SuperRodFishingEncounter:
      m_frame.isEncounter = IsFishingEncounter(Method::SuperRodThreshold);
      m_frame.esv =
        Method::SuperRodESV(Method::CalculatePercentage(m_RNG.Next()));
      m_RNG.Next(); // level calc?
      break;
    case StationaryEncounter:
      // no esv
    default:
      break;
    }
    
    Nature::Type  targetNature;
    
    if (m_parameters.syncNature != Nature::ANY)
    {
      m_frame.synched = Method::DetermineSync(m_RNG.Next());
      ++m_frame.method1Number;
    }
    if (m_frame.synched)
    {
      targetNature = m_parameters.syncNature;
    }
    else
    {
      targetNature = Method::DetermineNature(m_RNG.Next());
      ++m_frame.method1Number;
    }
    
    m_frame.pid = m_PIDRNG.NextPIDWord();
    while (m_frame.pid.Gen34Nature() != targetNature)
    {
      m_frame.pid = m_PIDRNG.NextPIDWord();
      m_frame.method1Number += 2;
    }
    m_frame.ivs = m_IVRNG.NextIVWord();
  }
  
  const Frame& CurrentFrame() { return m_frame; }
  
  //uint32_t CurrentSeed() { return m_RNG.PeekNext(); }
  
  bool GeneratesESV() const
  { return m_parameters.encounterType != StationaryEncounter; }
  
  bool GeneratesIsEncounter() const
  {
    return (m_parameters.encounterType == OldRodFishingEncounter) ||
           (m_parameters.encounterType == GoodRodFishingEncounter) ||
           (m_parameters.encounterType == SuperRodFishingEncounter);
  }
  
private:
  bool IsFishingEncounter(uint32_t threshold)
  {
    return Method::CalculatePercentage(m_RNG.Next()) < threshold;
  }
  
  RNG         m_RNG;
  PIDRNG      m_PIDRNG;
  IVRNG       m_IVRNG;
  LCRNG34     m_ProfElmResponseRNG;
  Frame       m_frame;
  Parameters  m_parameters;
};

struct MethodJ
{
  static uint32_t CalculatePercentage(uint32_t rawRNG)
  { return (rawRNG >> 16) / 656; }
  
  static Nature::Type DetermineNature(uint32_t rawRNG)
  { return Nature::Type((rawRNG >> 16) / 0xA3E); }
  
  static bool DetermineSync(uint32_t rawRNG)
  { return (rawRNG >> 31) == 0; }
  
  static const uint32_t OldRodThreshold = 25;
  static ESV::Value OldRodESV(uint32_t percentage)
  { return ESV::Gen4OldRod(percentage); }
  
  static const uint32_t GoodRodThreshold = 50;
  static ESV::Value GoodRodESV(uint32_t percentage)
  { return ESV::Gen4GoodRodJ(percentage); }
  
  static const uint32_t SuperRodThreshold = 75;
  static ESV::Value SuperRodESV(uint32_t percentage)
  { return ESV::Gen4SuperRodJ(percentage); }
};

struct MethodK
{
  static uint32_t CalculatePercentage(uint32_t rawRNG)
  { return (rawRNG >> 16) % 100; }
  
  static Nature::Type DetermineNature(uint32_t rawRNG)
  { return Nature::Type((rawRNG >> 16) % 25); }
  
  static bool DetermineSync(uint32_t rawRNG)
  { return (rawRNG & 0x00010000) == 0; }
  
  static const uint32_t OldRodThreshold = 0x2D;
  static ESV::Value OldRodESV(uint32_t rawRNG)
  { return ESV::Gen4OldRod(CalculatePercentage(rawRNG)); }
  
  static const uint32_t GoodRodThreshold = 0x46;
  static ESV::Value GoodRodESV(uint32_t rawRNG)
  { return ESV::Gen4GoodRodK(CalculatePercentage(rawRNG)); }
  
  static const uint32_t SuperRodThreshold = 0x5F;
  static ESV::Value SuperRodESV(uint32_t rawRNG)
  { return ESV::Gen4SuperRodK(CalculatePercentage(rawRNG)); }
};

typedef Gen4EncounterFrameGenerator<MethodJ> DPPtEncounterFrameGenerator;
typedef Gen4EncounterFrameGenerator<MethodK> HGSSEncounterFrameGenerator;


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
  
  struct Parameters
  {
    FrameType  frameType;
    bool       useCompoundEyes;
    uint32_t   tid, sid;
    
    Parameters()
      : frameType(GrassCaveFrame), useCompoundEyes(false), tid(0), sid(0)
    {}
  };
  
  Gen5PIDFrameGenerator(const HashedSeed &seed, const Parameters &parameters);
  
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
  
  struct Parameters
  {
    bool      startFromLowestFrame;
    uint32_t  ivSkip, pidSkip, natureSkip;
    bool      canBeShiny;
    uint32_t  tid, sid;
    
    Parameters()
      : startFromLowestFrame(false),
        ivSkip(0), pidSkip(0), natureSkip(0),
        canBeShiny(false), tid(0), sid(0)
    {}
  };
  
  WonderCardFrameGenerator(const HashedSeed &seed, const Parameters &parameters)
    : m_RNG(seed.rawSeed), m_IVRNG(m_RNG, IVRNG::Normal),
      m_PIDRNG(m_RNG, (parameters.canBeShiny ?
                       PIDRNG::GiftPID : PIDRNG::GiftNoShinyPID),
               parameters.tid, parameters.sid),
      m_frame(seed),
      m_pidSkip(parameters.pidSkip), m_natureSkip(parameters.natureSkip)
  {
    // skip over 'unused' frames
    for (uint32_t i = 0; i < parameters.ivSkip; ++i)
    {
      m_RNG.AdvanceBuffer();
    }
    m_frame.number = 0;
    
    if (parameters.startFromLowestFrame)
    {
      uint32_t  skippedFrames = seed.GetSkippedPIDFrames();
      while (skippedFrames-- > 0)
      {
        m_RNG.AdvanceBuffer();
        ++m_frame.number;
      }
    }
  }
  
  void AdvanceFrame()
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
  
  const Frame& CurrentFrame() { return m_frame; }
  
private:
  RNG             m_RNG;
  IVRNG           m_IVRNG;
  PIDRNG          m_PIDRNG;
  Frame           m_frame;
  const uint32_t  m_pidSkip;
  const uint32_t  m_natureSkip;
};

class Gen5TrainerIDFrameGenerator
{
public:
  typedef Gen5TrainerIDFrame  Frame;
  typedef LCRNG5              RNG;
  
  Gen5TrainerIDFrameGenerator(const HashedSeed &seed)
    : m_RNG(seed.rawSeed),
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
  
  struct Parameters
  {
    bool      usingEverstone;
    bool      usingDitto;
    bool      internationalParents;
    uint32_t  tid, sid;
  };
  
  Gen5BreedingFrameGenerator(const HashedSeed &seed,
                             const Parameters &parameters);
  
  void AdvanceFrame();
  
  const Frame& CurrentFrame() { return m_frame; }
  
private:
  const Parameters  m_parameters;
  
  RNG       m_RNG;
  PIDRNG    m_PIDRNG;
  Frame     m_frame;
};

}

#endif
