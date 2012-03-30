/*
  Copyright (C) 2012 chiizu
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

#import <Cocoa/Cocoa.h>

#include "PPRNGTypes.h"
#include "HashedSeed.h"

@protocol HashedSeedResultParameters

@property uint32_t  date, time, timer0, vcount, vframe, heldButtons;
@property uint64_t  rawSeed;

@end

#define DECLARE_HASHED_SEED_RESULT_PARAMETERS_VARIABLES() \
  uint32_t  date, time, timer0, vcount, vframe, heldButtons; \
  uint64_t  rawSeed

#define SYNTHESIZE_HASHED_SEED_RESULT_PARAMETERS_PROPERTIES() \
  @synthesize date, time, timer0, vcount, vframe, heldButtons, rawSeed

extern void SetHashedSeedResultParameters
  (id <HashedSeedResultParameters> result, const pprng::HashedSeed &seed);


@protocol IVResult

@property uint32_t              hp, atk, def, spa, spd, spe;
@property pprng::Element::Type  hiddenType;
@property uint32_t              hiddenPower;
@property BOOL                  isRoamer;

@end

#define DECLARE_IV_RESULT_VARIABLES() \
  uint32_t              hp, atk, def, spa, spd, spe; \
  pprng::Element::Type  hiddenType; \
  uint32_t              hiddenPower; \
  BOOL                  isRoamer

#define SYNTHESIZE_IV_RESULT_PROPERTIES() \
  @synthesize hp, atk, def, spa, spd, spe; \
  @synthesize hiddenType; \
  @synthesize hiddenPower; \
  @synthesize isRoamer

extern void SetIVResult(id <IVResult> result, const pprng::IVs &ivs,
                        BOOL isRoamer);


@protocol PIDResult

@property uint32_t              pid;
@property BOOL                  shiny;
@property pprng::Nature::Type   nature;
@property pprng::Ability::Type  ability;
@property pprng::Gender::Type   gender18, gender14, gender12, gender34;

@end

#define DECLARE_PID_RESULT_VARIABLES() \
  uint32_t              pid; \
  BOOL                  shiny; \
  pprng::Nature::Type   nature; \
  pprng::Ability::Type  ability; \
  pprng::Gender::Type   gender18, gender14, gender12, gender34

#define SYNTHESIZE_PID_RESULT_PROPERTIES() \
  @synthesize pid; \
  @synthesize shiny; \
  @synthesize nature; \
  @synthesize ability; \
  @synthesize gender18, gender14, gender12, gender34

extern void SetGen5PIDResult(id <PIDResult> result, pprng::Nature::Type nature,
                             const pprng::PID &pid, uint32_t tid, uint32_t sid,
                             pprng::Gender::Type gender,
                             pprng::Gender::Ratio genderRatio);
