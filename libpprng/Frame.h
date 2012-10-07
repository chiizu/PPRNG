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

#ifndef FRAME_H
#define FRAME_H

#include "PPRNGTypes.h"
#include "TimeSeed.h"
#include "CGearSeed.h"
#include "HashedSeed.h"

namespace pprng
{

struct Gen34Frame
{
  uint32_t  seed;
  uint32_t  number;
  uint32_t  rngValue;
  PID       pid;
  IVs       ivs;
};

struct Gen4Frame
{
  Gen4Frame(const Gen34Frame &baseFrame);
  
  uint32_t  seed;
  uint32_t  number;
  uint32_t  rngValue;
  PID       pid;
  IVs       ivs;
  
  struct EncounterData
  {
    EncounterData()
      : lowestFrames(), landESVs(0), surfESVs(0),
        oldRodESVs(0), goodRodESVs(0), superRodESVs(0), esvFrames()
    {}
    
    enum FrameType
    { NoSync = 0, Sync, FailedSync, NumFrameTypes };
    
    struct Frames
    {
      Frames() { number[NoSync] = number[Sync] = number[FailedSync] = 0; }
      
      uint32_t  number[NumFrameTypes];
    };
    
    Frames    lowestFrames;
    uint32_t  landESVs;
    uint32_t  surfESVs;
    uint32_t  oldRodESVs;
    uint32_t  goodRodESVs;
    uint32_t  superRodESVs;
    
    std::map<ESV::Value, Frames>  esvFrames;
  };
  
  EncounterData  methodJ;
  EncounterData  methodK;
};

struct Gen4EncounterFrame
{
  uint32_t                    seed;
  uint32_t                    number;
  uint32_t                    method1Number;
  uint32_t                    rngValue;
  bool                        isEncounter;
  ESV::Value                  esv;
  bool                        synched;
  PID                         pid;
  IVs                         ivs;
};

struct Gen4EggPIDFrame
{
  uint32_t  seed;
  uint32_t  number;
  uint32_t  rngValue;
  PID       pid;
};

struct Gen4BreedingFrame
{
  Gen4BreedingFrame()
  {
    ResetInheritance();
  }
  
  uint32_t  seed;
  uint32_t  number;
  uint32_t  rngValue;
  IVs       baseIVs;
  
  enum Inheritance
  {
    NotInherited = 0,
    ParentA,
    ParentB
  };
  
  Inheritance  inheritance[6];
  
  void ResetInheritance()
  {
    for (uint32_t i = 0; i < 6; ++i)
      inheritance[i] = NotInherited;
  }
};

struct Gen4EggIVFrame : public Gen4BreedingFrame
{
  Gen4EggIVFrame(const Gen4BreedingFrame &f, const OptionalIVs &parentAIVs,
                 const OptionalIVs &parentBIVs);
  
  typedef Gen4BreedingFrame::Inheritance  Inheritance;
  
  OptionalIVs  ivs;
};

struct Gen4TrainerIDFrame
{
  uint32_t  seed;
  uint32_t  number;
  uint32_t  tid, sid;
};

struct Gen5PIDFrame
{
  Gen5PIDFrame(const HashedSeed &s) : seed(s) {}
  
  const HashedSeed        seed;
  uint32_t                number;
  uint64_t                rngValue;
  bool                    isEncounter;
  EncounterItem::Type     encounterItem;
  EncounterLead::Ability  leadAbility;
  bool                    abilityActivated;
  PID                     pid;
  Nature::Type            nature;
  ESV::Value              esv;
  HeldItem::Type          heldItem;
};

struct CGearIVFrame
{
  uint32_t  seed;
  uint32_t  number;
  IVs       ivs;
};

struct HashedIVFrame
{
  HashedIVFrame(const HashedSeed &s) : seed(s) {}
  
  const HashedSeed  seed;
  uint32_t          number;
  IVs               ivs;
};

struct WonderCardFrame
{
  WonderCardFrame(const HashedSeed &s) : seed(s) {}
  
  const HashedSeed  seed;
  uint32_t          number;
  uint64_t          rngValue;
  PID               pid;
  Nature::Type      nature;
  bool              hasHiddenAbility;
  IVs               ivs;
};

struct Gen5TrainerIDFrame
{
  Gen5TrainerIDFrame(const HashedSeed &s) : seed(s) {}
  
  const HashedSeed  seed;
  uint32_t          number;
  uint32_t          tid, sid;
  bool              wildShiny, giftShiny, eggShiny;
};

struct Gen5BreedingFrame
{
  Gen5BreedingFrame(const HashedSeed &s)
    : seed(s)
  {
    ResetInheritance();
  }
  
  const HashedSeed  seed;
  uint32_t          number;
  uint64_t          rngValue;
  bool              everstoneActivated;
  bool              inheritsHiddenAbility;
  EggSpecies::Type  species;
  Nature::Type      nature;
  PID               pid;
  
  enum Inheritance
  {
    NotInherited = 0,
    ParentX,
    ParentY
  };
  
  Inheritance      inheritance[6];
  
  void ResetInheritance()
  {
    for (uint32_t i = 0; i < 6; ++i)
      inheritance[i] = NotInherited;
  }
};

struct Gen5EggFrame : public Gen5BreedingFrame
{
  Gen5EggFrame(const Gen5BreedingFrame &f, uint32_t ivFrame, IVs baseIVs,
               const OptionalIVs &parentXIVs, const OptionalIVs &parentYIVs);
  
  typedef Gen5BreedingFrame::Inheritance  Inheritance;
  
  uint32_t              ivFrameNumber;
  OptionalIVs           ivs;
  Characteristic::Type  characteristic;
};

struct DreamRadarFrame
{
  DreamRadarFrame(const HashedSeed &s) : seed(s) {}
  
  const HashedSeed  seed;
  uint32_t          number;
  uint64_t          rngValue;
  PID               pid;
  Nature::Type      nature;
  IVs               ivs;
};

struct HiddenHollowSpawnFrame
{
  HiddenHollowSpawnFrame(const HashedSeed &s) : seed(s) {}
  
  const HashedSeed  seed;
  uint32_t          number;
  uint64_t          rngValue;
  bool              isSpawn;
  uint32_t          group;
  uint32_t          slot;
  uint32_t          genderPercentage;
};

}

#endif
