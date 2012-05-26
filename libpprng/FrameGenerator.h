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

#ifndef FRAME_GENERATOR_H
#define FRAME_GENERATOR_H

#include "PPRNGTypes.h"
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
  typedef uint32_t                  Seed;
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
  
  void SkipFrames(uint32_t numFrames)
  {
    uint32_t  i = 0;
    while (i++ < numFrames)
      m_RNG.AdvanceBuffer();
    
    m_frame.number += numFrames;
  }
  
  void AdvanceFrame()
  {
    m_RNG.AdvanceBuffer();
    
    m_frame.rngValue = m_RNG.PeekNext();
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
  typedef uint32_t             Seed;
  typedef Gen4EncounterFrame   Frame;
  typedef LCRNG34              RNG;
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
    : m_NextSeed(seed), m_RNG(seed), m_PIDRNG(m_RNG), m_IVRNG(m_RNG),
      m_parameters(parameters)
  {
    m_frame.seed = seed;
    m_frame.number = 0;
    m_frame.synched = false;
    m_frame.isEncounter = true;
    m_frame.esv = ESV::Value(0);
  }
  
  void SkipFrames(uint32_t numFrames)
  {
    uint32_t  i = 0;
    while (i++ < numFrames)
      m_RNG.Next();
    
    m_NextSeed = m_RNG.Seed();
    m_frame.number += numFrames;
  }
  
  void AdvanceFrame()
  {
    ++m_frame.number;
    m_frame.method1Number = m_frame.number;
    
    m_RNG.Seed(m_NextSeed);
    m_frame.rngValue = m_RNG.Next();
    
    m_RNG.Seed(m_NextSeed);
    m_NextSeed = m_frame.rngValue;
    
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
  
private:
  bool IsFishingEncounter(uint32_t threshold)
  {
    return Method::CalculatePercentage(m_RNG.Next()) < threshold;
  }
  
  RNG::SeedType  m_NextSeed;
  RNG            m_RNG;
  PIDRNG         m_PIDRNG;
  IVRNG          m_IVRNG;
  Frame          m_frame;
  Parameters     m_parameters;
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


class Gen4EggPIDFrameGenerator
{
public:
  typedef uint32_t         Seed;
  typedef Gen4EggPIDFrame  Frame;
  typedef MTRNG            RNG;
  
  struct Parameters
  {
    bool      internationalParents;
    uint32_t  tid, sid;
    
    Parameters() : internationalParents(false), tid(0), sid(0) {}
  };
  
  Gen4EggPIDFrameGenerator(uint32_t seed, const Parameters &parameters)
    : m_RNG(seed), m_parameters(parameters)
  {
    m_frame.seed = seed;
    m_frame.number = 0;
  }
  
  void SkipFrames(uint32_t numFrames)
  {
    uint32_t  i = 0;
    while (i++ < numFrames)
      m_RNG.Next();
    
    m_frame.number += numFrames;
  }
  
  void AdvanceFrame()
  {
    ++m_frame.number;
    m_frame.rngValue = m_RNG.Next();
    m_frame.pid = m_frame.rngValue;
    
    if (m_parameters.internationalParents)
    {
      uint32_t  shinyChecks = 0;
      while (!m_frame.pid.IsShiny(m_parameters.tid, m_parameters.sid) &&
             (++shinyChecks < 4))
      {
        m_frame.pid = ARNG::NextForSeed(m_frame.pid.word);
      }
    }
  }
  
  const Frame& CurrentFrame() { return m_frame; }
  
private:
  const Parameters  m_parameters;
  RNG               m_RNG;
  Frame             m_frame;
};


class Gen4BreedingFrameGenerator
{
public:
  typedef uint32_t                 Seed;
  typedef Gen4BreedingFrame        Frame;
  typedef BufferedRNG<LCRNG34, 8>  RNG;
  typedef Gen34IVRNG<1, RNG>       IVRNG;
  
  Gen4BreedingFrameGenerator(uint32_t seed, Game::Version version)
    : m_RNG(seed), m_IVRNG(m_RNG),
      m_inheritanceGenerator(GeneratorForVersion(version))
  {
    m_frame.seed = seed;
    m_frame.number = 0;
  }
  
  void SkipFrames(uint32_t numFrames)
  {
    uint32_t  i = 0;
    while (i++ < numFrames)
      m_RNG.AdvanceBuffer();
    
    m_frame.number += numFrames;
  }
  
  void AdvanceFrame()
  {
    m_RNG.AdvanceBuffer();
    ++m_frame.number;
    m_frame.rngValue = m_RNG.PeekNext();
    
    m_frame.baseIVs = m_IVRNG.NextIVWord();
    
    m_frame.ResetInheritance();
    
    (this->*m_inheritanceGenerator)();
  }
  
  const Frame& CurrentFrame() { return m_frame; }
  
private:
  void GenerateParents(IVs::Type inheritedIV[])
  {
    for (uint32_t i = 0; i < 3; ++i)
    {
      m_frame.inheritance[inheritedIV[i]] =
        (((m_RNG.Next() >> 16) & 1) == 0) ?
          Gen4BreedingFrame::ParentA :
          Gen4BreedingFrame::ParentB;
    }
  }
  
  static const IVs::Type  DPPtRound1[6];
  static const IVs::Type  DPPtRound2[5];
  static const IVs::Type  DPPtRound3[4];
  
  void GenerateDPPt()
  {
    IVs::Type  inheritedIV[3];
    
    inheritedIV[0] = DPPtRound1[(m_RNG.Next() >> 16) % 6];
    inheritedIV[1] = DPPtRound2[(m_RNG.Next() >> 16) % 5];
    inheritedIV[2] = DPPtRound3[(m_RNG.Next() >> 16) % 4];
    
    GenerateParents(inheritedIV);
  }
  
  void GenerateHGSS()
  {
    IVs::Type  inheritedIV[3];
    IVs::Type  ivType[6] =
      { IVs::HP, IVs::AT, IVs::DF, IVs::SP, IVs::SA, IVs::SD };
    for (uint32_t i = 0; i < 3; ++i)
    {
      uint32_t  numTypes = 6 - i;
      uint32_t  idx = (m_RNG.Next() >> 16) % numTypes;
      
      inheritedIV[i] = ivType[idx];
      while (idx < (numTypes - 1))
      {
        ivType[idx] = ivType[idx + 1];
        ++idx;
      }
    }
    
    GenerateParents(inheritedIV);
  }
  
  typedef void (Gen4BreedingFrameGenerator::*InheritanceGenerator)();
  
  static InheritanceGenerator GeneratorForVersion(Game::Version version)
  {
    switch (version)
    {
    case Game::Diamond:
    case Game::Pearl:
    case Game::Platinum:
      return &Gen4BreedingFrameGenerator::GenerateDPPt;
      
    case Game::HeartGold:
    case Game::SoulSilver:
      return &Gen4BreedingFrameGenerator::GenerateHGSS;
    
    default:
      return &Gen4BreedingFrameGenerator::GenerateDPPt;
    }
  }
  
  const InheritanceGenerator  m_inheritanceGenerator;
  RNG                         m_RNG;
  IVRNG                       m_IVRNG;
  Frame                       m_frame;
};


class CGearIVFrameGenerator
{
public:
  typedef uint32_t                 Seed;
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
  
  void SkipFrames(uint32_t numFrames);
  
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
  typedef HashedSeed               Seed;
  typedef HashedIVFrame            Frame;
  typedef MTRNG                    RNG;
  typedef Gen5BufferingIVRNG<RNG>  IVRNG;
  
  enum FrameType
  {
    Normal = IVRNG::Normal,
    Roamer = IVRNG::Roamer
  };
  
  HashedIVFrameGenerator(const HashedSeed &seed, FrameType frameType);
  
  void SkipFrames(uint32_t numFrames);
  
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
  typedef HashedSeed              Seed;
  typedef Gen5PIDFrame            Frame;
  typedef BufferedRNG<LCRNG5, 8>  RNG;
  
  enum FrameType
  {
    GrassCaveFrame = 0,
    SurfingFrame,
    FishingFrame,
    SwarmFrame,
    ShakingGrassFrame,
    SwirlingDustFrame,
    BridgeShadowFrame,
    WaterSpotSurfingFrame,
    WaterSpotFishingFrame,
    EntraLinkFrame,
    StationaryFrame,
    NonShinyStationaryFrame,
    StarterFossilGiftFrame,
    RoamerFrame,
    DoublesFrame,
    
    NumFrameTypes
  };
  
  struct Parameters
  {
    FrameType               frameType;
    EncounterLead::Ability  leadAbility;
    Gender::Type            targetGender;
    Gender::Ratio           targetRatio;
    uint32_t                tid, sid;
    bool                    startFromLowestFrame;
    
    Parameters()
      : frameType(GrassCaveFrame), leadAbility(EncounterLead::SYNCHRONIZE),
        targetGender(Gender::ANY), targetRatio(Gender::ANY_RATIO),
        tid(0), sid(0), startFromLowestFrame(false)
    {}
  };
  
  Gen5PIDFrameGenerator(const HashedSeed &seed, const Parameters &parameters);
  
  void SkipFrames(uint32_t numFrames);
  
  void AdvanceFrame();
  
  const Frame& CurrentFrame() const { return m_frame; }
  
  FrameType GetFrameType() const { return m_parameters.frameType; }
  
private:
  typedef void (Gen5PIDFrameGenerator::*PIDGenerator)();
  
  const PIDGenerator  m_PIDGenerator;
  
  
  typedef void (Gen5PIDFrameGenerator::*PIDFrameGenerator)();
  
  const PIDFrameGenerator  m_PIDFrameGenerator;
  
  
  typedef void (Gen5PIDFrameGenerator::*ESVGenerator)();
  
  const ESVGenerator  m_ESVGenerator;
  
  struct FrameGeneratorInfo
  {
    PIDGenerator       pidGenerator;
    PIDFrameGenerator  pidFrameGenerator;
    ESVGenerator       esvGenerator;
  };
  
  static const FrameGeneratorInfo  s_FrameGeneratorInfo[NumFrameTypes];
  
  void NextWildPID();
  void NextEntraLinkPID();
  void NextNonShinyPID();
  void NextGiftPID();
  void NextRoamerPID();
  
  void NextWildFrame();
  void NextFishingFrame();
  void NextSwarmFrame();
  void NextDoublesFrame();
  void NextDustFrame();
  void NextShadowFrame();
  void NextStationaryFrame();
  void NextEntraLinkFrame();
  void NextSimpleFrame();
  
  void CheckLeadAbility();
  void ApplySync();
  
  void LandESV();
  void SurfESV();
  void FishingESV();
  void NoESV();
  
  void NextHeldItem();
  
  RNG                      m_RNG;
  Frame                    m_frame;
  const Parameters         m_parameters;
};


class WonderCardFrameGenerator
{
public:
  typedef HashedSeed               Seed;
  typedef WonderCardFrame          Frame;
  typedef BufferedRNG<LCRNG5, 7>   RNG;
  typedef Gen5BufferingIVRNG<RNG>  IVRNG;
  
  struct Parameters
  {
    Nature::Type               cardNature;
    Ability::Type              cardAbility;
    Gender::Type               cardGender;
    Gender::Ratio              cardGenderRatio;
    WonderCardShininess::Type  cardShininess;
    uint32_t                   cardTID, cardSID;
    
    bool           startFromLowestFrame;
    
    Parameters()
      : cardNature(Nature::ANY), cardAbility(Ability::ANY),
        cardGender(Gender::ANY), cardGenderRatio(Gender::ANY_RATIO),
        cardShininess(WonderCardShininess::NEVER_SHINY),
        cardTID(0), cardSID(0), startFromLowestFrame(false)
    {}
  };
  
  WonderCardFrameGenerator(const HashedSeed &seed,
                           const Parameters &parameters);
  
  void SkipFrames(uint32_t numFrames);
  
  void AdvanceFrame();
  
  const Frame& CurrentFrame() { return m_frame; }
  
private:
  RNG               m_RNG;
  IVRNG             m_IVRNG;
  Frame             m_frame;
  const Parameters  m_parameters;
  const bool        m_isGLAN;
};


class Gen5TrainerIDFrameGenerator
{
public:
  typedef HashedSeed          Seed;
  typedef Gen5TrainerIDFrame  Frame;
  typedef LCRNG5              RNG;
  
  Gen5TrainerIDFrameGenerator(const HashedSeed &seed, const PID &shinyPID)
    : m_RNG(seed.rawSeed), m_ShinyPID(shinyPID),
      m_EggPID((uint64_t(shinyPID.word) * 0xFFFFFFFFULL) >> 32),
      m_frame(seed)
  {
    m_frame.number = 0;
  }
  
  void SkipFrames(uint32_t numFrames)
  {
    uint32_t  i = 0;
    while (i++ < numFrames)
      m_RNG.Next();
    
    m_frame.number += numFrames;
  }
  
  void AdvanceFrame()
  {
    ++m_frame.number;
    
    uint32_t  fullID = ((m_RNG.Next() >> 32) * 0xFFFFFFFFULL) >> 32;
    m_frame.tid = fullID & 0xffff;
    m_frame.sid = fullID >> 16;
    m_frame.wildShiny =
      PID(Gen5PIDRNG::TIDBitTwiddle(m_ShinyPID.word, m_frame.tid, m_frame.sid))
        .IsShiny(m_frame.tid, m_frame.sid);
    m_frame.giftShiny = m_ShinyPID.IsShiny(m_frame.tid, m_frame.sid);
    m_frame.eggShiny = m_EggPID.IsShiny(m_frame.tid, m_frame.sid);
  }
  
  const Frame& CurrentFrame() { return m_frame; }
  
private:
  RNG        m_RNG;
  const PID  m_ShinyPID;
  const PID  m_EggPID;
  Frame      m_frame;
};

class Gen5BreedingFrameGenerator
{
public:
  typedef HashedSeed         Seed;
  typedef Gen5BreedingFrame  Frame;
  typedef LCRNG5             RNG;
  
  struct Parameters
  {
    FemaleParent::Type  femaleSpecies;
    bool                usingEverstone;
    bool                usingDitto;
    bool                internationalParents;
    uint32_t            tid, sid;
  };
  
  Gen5BreedingFrameGenerator(const HashedSeed &seed,
                             const Parameters &parameters);
  
  void AdvanceFrame();
  
  const Frame& CurrentFrame() { return m_frame; }
  
private:
  const Parameters  m_parameters;
  
  RNG::SeedType  m_NextSeed;
  RNG            m_RNG;
  Frame          m_frame;
};

}

#endif
