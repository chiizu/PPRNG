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

#ifndef FRAME_H
#define FRAME_H

#include "BasicTypes.h"
#include "TimeSeed.h"
#include "CGearSeed.h"
#include "HashedSeed.h"

namespace pprng
{

struct Gen34Frame
{
  uint32_t  seed;
  uint32_t  number;
  uint32_t  frameSeed;
  PID       pid;
  IVs       ivs;
};

struct Gen4Frame
{
  Gen4Frame(const Gen34Frame &baseFrame);
  
  uint32_t  seed;
  uint32_t  number;
  PID       pid;
  IVs       ivs;
  uint32_t  methodJNumber;
  uint32_t  methodJSyncNumber;
  uint32_t  methodJFailedSyncNumber;
  uint32_t  methodKNumber;
  uint32_t  methodKSyncNumber;
  uint32_t  methodKFailedSyncNumber;
};

struct Gen5PIDFrame
{
  Gen5PIDFrame(const HashedSeed &s) : seed(s) {}
  
  const HashedSeed  seed;
  uint32_t          number;
  
  PID               pid;
  Nature::Type      nature;
  bool              synched;
  uint32_t          esv;
  HeldItem::Type    heldItem;
  bool              canFish;
  bool              findItem;
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
  PID               pid;
  Nature::Type      nature;
  IVs               ivs;
};

struct Gen5TrainerIDFrame
{
  Gen5TrainerIDFrame(const HashedSeed &s) : seed(s) {}
  
  const HashedSeed  seed;
  uint32_t          number;
  uint32_t          tid;
  uint32_t          sid;
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
  
  bool              everstoneActivated;
  bool              dreamWorldAbilityPassed;
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

}

#endif
