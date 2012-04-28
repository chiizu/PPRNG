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

#import "SearchResultProtocols.h"

#import "Utilities.h"

using namespace pprng;

void SetHashedSeedResultParameters(id <HashedSeedResultParameters> result,
                                   const HashedSeed &seed)
{
  result.date = MakeUInt32Date(seed.year(), seed.month(), seed.day());
  result.time = MakeUInt32Time(seed.hour, seed.minute, seed.second);
  result.timer0 = seed.timer0;
  result.vcount = seed.vcount;
  result.vframe = seed.vframe;
  result.heldButtons = seed.heldButtons;
  result.rawSeed = seed.rawSeed;
}

void SetIVResult(id <IVResult> result, const IVs &ivs, BOOL isRoamer)
{
  result.hp = ivs.hp();
  result.atk = ivs.at();
  result.def = ivs.df();
  result.spa = ivs.sa();
  result.spd = ivs.sd();
  result.spe = ivs.sp();
  result.hiddenType = ivs.HiddenType();
  result.hiddenPower = ivs.HiddenPower();
  result.isRoamer = isRoamer;
}

void SetPIDResult(id <PIDResult> result,
                  const PID &pid, uint32_t tid, uint32_t sid,
                  Nature::Type nature, Ability::Type ability,
                  Gender::Type gender, Gender::Ratio genderRatio)
{
  result.pid = pid.word;
  result.shiny = pid.IsShiny(tid, sid);
  result.nature = nature;
  result.ability = ability;
  
  if (genderRatio == Gender::ANY_RATIO)
  {
    uint32_t  genderValue = pid.GenderValue();
    
    result.gender18 = (genderValue < Gender::ONE_EIGHTH_FEMALE_THRESHOLD) ?
                        Gender::FEMALE : Gender::MALE;
    result.gender14 = (genderValue < Gender::ONE_FOURTH_FEMALE_THRESHOLD) ?
                        Gender::FEMALE : Gender::MALE;
    result.gender12 = (genderValue < Gender::ONE_HALF_FEMALE_THRESHOLD) ?
                        Gender::FEMALE : Gender::MALE;
    result.gender34 = (genderValue < Gender::THREE_FOURTHS_FEMALE_THRESHOLD) ?
                        Gender::FEMALE : Gender::MALE;
  }
  else if ((genderRatio == Gender::FEMALE_ONLY) ||
           (genderRatio == Gender::MALE_ONLY) ||
           (genderRatio == Gender::NO_RATIO))
  {
    result.gender18 = Gender::NONE;
    result.gender14 = Gender::NONE;
    result.gender12 = Gender::NONE;
    result.gender34 = Gender::NONE;
  }
  else
  {
    result.gender18 = (genderRatio == Gender::ONE_EIGHTH_FEMALE) ?
                        gender : Gender::NONE;
    result.gender14 = (genderRatio == Gender::ONE_FOURTH_FEMALE) ?
                        gender : Gender::NONE;
    result.gender12 = (genderRatio == Gender::ONE_HALF_FEMALE) ?
                        gender : Gender::NONE;
    result.gender34 = (genderRatio == Gender::THREE_FOURTHS_FEMALE) ?
                        gender : Gender::NONE;
  }
}

