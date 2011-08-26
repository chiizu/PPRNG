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


#include "BasicTypes.h"
#include <list>
#include <map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <ctype.h>


namespace pprng
{

namespace
{

static const char *DSTypeNameArray[] =
{ "DS", "DS Lite", "DSi", "DSi XL/LL", "3DS", "Unknown DS Type" };

static const std::vector<std::string>  DSTypeName
  (DSTypeNameArray, DSTypeNameArray +
                    (sizeof(DSTypeNameArray) / sizeof(const char*)));

static const char *GameVersionNameArray[] =
{ "Emerald", "Diamond", "Pearl", "Platinum", "HeartGold", "SoulSilver",
  "Black (English)", "Black (French)", "Black (German)", "Black (Italian)",
  "Black (Japanese)", "Black (Spanish)",
  "White (English)", "White (French)", "White (German)", "White (Italian)",
  "White (Japanese)", "White (Spanish)", "Black (Korean)", "White (Korean)",
  "Unknown Version"
};

static const std::vector<std::string>  GameVersionName
  (GameVersionNameArray, GameVersionNameArray + 
                    (sizeof(GameVersionNameArray) / sizeof(const char *)));

static const char *NatureNameArray[] =
{ "HARDY", "LONELY", "BRAVE", "ADAMANT", "NAUGHTY", "BOLD", "DOCILE", "RELAXED",
  "IMPISH", "LAX", "TIMID", "HASTY", "SERIOUS", "JOLLY", "NAIVE", "MODEST",
  "MILD", "QUIET", "BASHFUL", "RASH", "CALM", "GENTLE", "SASSY", "CAREFUL",
  "QUIRKY",
  "ANY", "UNKNOWN" };

static const std::vector<std::string>  NatureName
  (NatureNameArray, NatureNameArray +
                    (sizeof(NatureNameArray) / sizeof(const char *)));

static const char *ElementNameArray[] =
{ "NORMAL", "FIGHTING", "FLYING", "POISON", "GROUND", "ROCK", "BUG", "GHOST",
  "STEEL", "FIRE", "WATER", "GRASS", "ELECTRIC", "PSYCHIC", "ICE", "DRAGON",
  "DARK",
  "ANY", "UNKNOWN" };

static const std::vector<std::string>  ElementName
  (ElementNameArray, ElementNameArray +
                     (sizeof(ElementNameArray) / sizeof(const char *)));

static const char *ButtonNameArray[] =
{ "A", "B", "Sel", "St", "Right", "Left", "Up", "Down", "R", "L",
  "X", "Y" };

static const std::vector<std::string>  ButtonName
  (ButtonNameArray, ButtonNameArray +
                    (sizeof(ButtonNameArray) / sizeof(const char *)));

static const uint32_t  SingleButtons[] =
{ Button::A_BUTTON, Button::B_BUTTON, Button::SELECT_BUTTON,
  Button::START_BUTTON, Button::R_BUTTON, Button::L_BUTTON,
  Button::X_BUTTON, Button::Y_BUTTON };

static const char *DPadDirectionNameeArray[] =
{ "", "Right", "Left", "Right-Left", "Up", "Up-Rt", "Up-Lf",
  "Right-Left-Up", "Down", "Dn-Rt", "Dn-Lf"  };

static const std::vector<std::string>  DPadDirectionName
  (DPadDirectionNameeArray,
   DPadDirectionNameeArray +
     (sizeof(DPadDirectionNameeArray) / sizeof(const char *)));

static const uint32_t  DPadDirections[] =
{ Button::RIGHT_BUTTON, Button::LEFT_BUTTON, Button::UP_BUTTON,
  Button::DOWN_BUTTON, Button::UP_RIGHT, Button::UP_LEFT,
  Button::DOWN_RIGHT, Button::DOWN_LEFT };

static const char *CharacteristicNameArray[] =
{ "Loves to eat", "Often dozes off", "Often scatters things",
  "Scatters things often", "Likes to relax", "Proud of its power",
  "Likes to thrash about", "A little quick tempered", "Likes to fight",
  "Quick tempered", "Sturdy body", "Capable of taking hits",
  "Highly persistent", "Good endurance", "Good perseverance", "Highly curious",
  "Mischievous", "Thoroughly cunning", "Often lost in thought", "Very finicky",
  "Strong willed", "Somewhat vain", "Strongly defiant", "Hates to lose",
  "Somewhat stubborn", "Likes to run", "Alert to sounds", "Impetuous and silly",
  "Somewhat of a clown", "Quick to flee", "Unknown Characteristic" };

static const std::vector<std::string>  CharacteristicName
  (CharacteristicNameArray,
   CharacteristicNameArray +
     (sizeof(CharacteristicNameArray) / sizeof(const char *)));

static std::string MakeBadIVIndexExceptionString(int i)
{
  std::ostringstream  os;
  os << "Bad IV Index: " << i;
  return os.str();
}

}


std::string DS::ToString(DS::Type t)
{
  if ((t >= DS::Phat) && (t <= DS::_3DS))
  {
    return DSTypeName[t];
  }
  else
  {
    return DSTypeName[UnknownDSType];
  }
}

std::string Button::ToString(uint32_t keys)
{
  std::string  result;
  
  if (keys != 0)
  {
    uint32_t  dpad = (keys & DPAD_MASK) >> DPAD_SHIFT;
    
    if (dpad != 0)
    {
      result = DPadDirectionName[dpad];
    }
    
    uint32_t  buttons = keys & SINGLE_BUTTON_MASK;
    if (buttons != 0)
    {
      if (!result.empty())
      {
        result += " + ";
      }
      
      uint32_t  i = 0;
      while ((buttons & 0x1) == 0)
      {
        buttons >>= 1;
        ++i;
      }
      result += ButtonName[i++];
      
      while ((buttons >>= 1) != 0)
      {
        if ((buttons & 0x1) != 0)
        {
          result += '-' + ButtonName[i];
        }
        ++i;
      }
    }
  }
  
  return result;
}

std::vector<uint32_t> MakeSingleButtons()
{
  std::vector<uint32_t>  result
  (SingleButtons, SingleButtons + (sizeof(SingleButtons) / sizeof(uint32_t)));
  
  result.insert(result.end(), DPadDirections,
    DPadDirections + (sizeof(DPadDirections) / sizeof(uint32_t)));
  
  return result;
}

const std::vector<uint32_t>& Button::SingleButtons()
{
  static std::vector<uint32_t>  s_SingleButtonVector = MakeSingleButtons();
  
  return s_SingleButtonVector;
}

static std::vector<uint32_t> MakeTwoButtonCombos()
{
  std::vector<uint32_t>  result;
  
  for (uint32_t i = 0; i < Button::NUM_BINARY_BUTTONS; ++i)
  {
    for (uint32_t j = 0; j < Button::NUM_DPAD_DIRECTIONS; ++j)
    {
      result.push_back(SingleButtons[i] | DPadDirections[j]);
    }
    
    for (uint32_t j = i + 1; j < Button::NUM_BINARY_BUTTONS; ++j)
    {
      result.push_back(SingleButtons[i] | SingleButtons[j]);
    }
  }
  
  return result;
}

const std::vector<uint32_t>& Button::TwoButtonCombos()
{
  static std::vector<uint32_t>  s_TwoButtonComboVector = MakeTwoButtonCombos();
  
  return s_TwoButtonComboVector;
}

static std::vector<uint32_t> MakeThreeButtonCombos()
{
  std::vector<uint32_t>  result;
  
  for (uint32_t i = 0; i < Button::NUM_BINARY_BUTTONS; ++i)
  {
    for (uint32_t j = i + 1; j < Button::NUM_BINARY_BUTTONS; ++j)
    {
      for (uint32_t k = 0; k < Button::NUM_DPAD_DIRECTIONS; ++k)
      {
        result.push_back(SingleButtons[i] | SingleButtons[j] |
                         DPadDirections[k]);
      }
      
      for (uint32_t k = j + 1; k < Button::NUM_BINARY_BUTTONS; ++k)
      {
        result.push_back(SingleButtons[i] | SingleButtons[j] |
                         SingleButtons[k]);
      }
    }
  }
  
  return result;
}

const std::vector<uint32_t>& Button::ThreeButtonCombos()
{
  static std::vector<uint32_t>  s_ThreeButtonComboVector =
    MakeThreeButtonCombos();
  
  return s_ThreeButtonComboVector;
}

std::string Game::ToString(Game::Version v)
{
  if ((v >= Game::ThirdGenStart) && (v < Game::FifthGenEnd))
  {
    return GameVersionName[v];
  }
  else
  {
    return GameVersionName[Game::UnknownVersion];
  }
}

const std::string& Nature::ToString(Nature::Type t)
{
  if ((t >= HARDY) && (t <= UNKNOWN))
  {
    return NatureName[t];
  }
  else
  {
    return NatureName[UNKNOWN];
  }
}

Nature::Type Nature::FromString(const std::string &name)
{
  std::string  upperName = name;
  std::transform(upperName.begin(), upperName.end(), upperName.begin(),
                 ::toupper);
  
  std::vector<std::string>::const_iterator  i =
    std::find(NatureName.begin(), NatureName.end(), upperName);
  
  if (i != NatureName.end())
  {
    return static_cast<Type>(i - NatureName.begin());
  }
  else
  {
    return UNKNOWN;
  }
}

const std::string& Element::ToString(Element::Type t)
{
  if ((t >= NORMAL) && (t <= UNKNOWN))
  {
    return ElementName[t];
  }
  else
  {
    return ElementName[UNKNOWN];
  }
}

Element::Type Element::FromString(const std::string &name)
{
  std::string  upperName = name;
  std::transform(upperName.begin(), upperName.end(), upperName.begin(),
                 ::toupper);
  
  std::vector<std::string>::const_iterator  i =
    std::find(ElementName.begin(), ElementName.end(), upperName);
  
  if (i != ElementName.end())
  {
    return static_cast<Type>(i - ElementName.begin());
  }
  else
  {
    return UNKNOWN;
  }
}


const IVs::Shift  IVs::IVShift[6] =
{
  HP_SHIFT,
  AT_SHIFT,
  DF_SHIFT,
  SA_SHIFT,
  SD_SHIFT,
  SP_SHIFT
};

const IVs::Mask  IVs::IVMask[6] =
{
  HP_MASK,
  AT_MASK,
  DF_MASK,
  SA_MASK,
  SD_MASK,
  SP_MASK
};

IVs::BadIVIndexException::BadIVIndexException(int i)
  : Exception(MakeBadIVIndexExceptionString(i))
{}

Element::Type IVs::HiddenType() const
{
  uint32_t  typeSum;
  
  typeSum = (word & (0x1 << HP_SHIFT)) >> HP_SHIFT;
  typeSum += (word & (0x1 << AT_SHIFT)) >> (AT_SHIFT - 1);
  typeSum += (word & (0x1 << DF_SHIFT)) >> (DF_SHIFT - 2);
  typeSum += (word & (0x1 << SP_SHIFT)) >> (SP_SHIFT - 3);
  typeSum += (word & (0x1 << SA_SHIFT)) >> (SA_SHIFT - 4);
  typeSum += (word & (0x1 << SD_SHIFT)) >> (SD_SHIFT - 5);
  
  return Element::Type(((typeSum * 15) / 63) + 1);
}

uint32_t IVs::HiddenPower() const
{
  uint32_t  powerSum;
  
  powerSum = (word & (0x2 << HP_SHIFT)) >> (HP_SHIFT + 1);
  powerSum += (word & (0x2 << AT_SHIFT)) >> (AT_SHIFT);
  powerSum += (word & (0x2 << DF_SHIFT)) >> (DF_SHIFT - 1);
  powerSum += (word & (0x2 << SP_SHIFT)) >> (SP_SHIFT - 2);
  powerSum += (word & (0x2 << SA_SHIFT)) >> (SA_SHIFT - 3);
  powerSum += (word & (0x2 << SD_SHIFT)) >> (SD_SHIFT - 4);
  
  return ((powerSum * 40) / 63) + 30;
}


namespace
{

struct HPSumCount { uint32_t  sum, count; };
typedef std::list<HPSumCount>  HPSumCountList;

static void UpdateHPSumsAndCounts(HPSumCountList &sumList, uint32_t addend,
                                  uint32_t numEvenIVs, uint32_t numOddIVs)
{
  HPSumCountList::iterator it;
  
  if (numEvenIVs > 0)
  {
    HPSumCountList  newOddSums;
    
    if (numOddIVs > 0)
    {
      // new sums to be added because the interesting bit is set
      newOddSums = sumList;
      
      for (it = newOddSums.begin(); it != newOddSums.end(); ++it)
      {
        it->sum += addend;  // add this bit's value
        it->count *= numOddIVs;
      }
    }
    
    if (numEvenIVs > 1)
    {
      // update counts for the IV values with unset bits
      for (it = sumList.begin(); it != sumList.end(); ++it)
      {
        it->count *= numEvenIVs;
      }
    }
    
    // add in odd counts with new sums
    sumList.splice(sumList.end(), newOddSums);
  }
  else
  {
    // no even IVs, so just update sums and counts in current list
    for (it = sumList.begin(); it != sumList.end(); ++it)
    {
      it->sum += addend;
      it->count *= numOddIVs;
    }
  }
}

template <typename KeyType, uint32_t Multiplier, uint32_t Addend>
void BuildHPMap(std::map<KeyType, uint32_t> &map,
                const HPSumCountList &sumList)
{
  HPSumCountList::const_iterator  lit;
  for (lit = sumList.begin(); lit != sumList.end(); ++lit)
  {
    KeyType  key = KeyType(((lit->sum * Multiplier) / 63) + Addend);
    
    typename std::map<KeyType, uint32_t>::iterator  mit = map.find(key);
    if (mit == map.end())
    {
      map[key] = lit->count;
    }
    else
    {
      mit->second += lit->count;
    }
  }
}

std::string MakeImpossibleMinMaxIVRangeExceptionString
  (IndividualValues minIVs, IndividualValues maxIVs)
{
  return "Min IVs are not all less than or equal to max IVs";
}

std::string MakeImpossibleHiddenTypeExceptionString
  (IndividualValues minIVs, IndividualValues maxIVs, Element::Type type)
{
  std::ostringstream  os;
  os << "Impossible Hidden Power Type: " << ElementName[type];
  return os.str();
}

std::string MakeImpossibleMinHiddenPowerExceptionString
  (IndividualValues minIVs, IndividualValues maxIVs, uint32_t minPower)
{
  std::ostringstream  os;
  os << "Impossible Minimum Hidden Power: " << minPower;
  return os.str();
}

}

IVs::ImpossibleMinMaxIVRangeException::ImpossibleMinMaxIVRangeException
    (IndividualValues minIVs, IndividualValues maxIVs)
: Exception(MakeImpossibleMinMaxIVRangeExceptionString(minIVs, maxIVs))
{}

IVs::ImpossibleHiddenTypeException::ImpossibleHiddenTypeException
    (IndividualValues minIVs, IndividualValues maxIVs, Element::Type type)
: Exception(MakeImpossibleHiddenTypeExceptionString(minIVs, maxIVs, type))
{}

IVs::ImpossibleMinHiddenPowerException::ImpossibleMinHiddenPowerException
    (IndividualValues minIVs, IndividualValues maxIVs, uint32_t minPower)
: Exception(MakeImpossibleMinHiddenPowerExceptionString(minIVs, maxIVs, minPower))
{}

// This complicated function (including helper function above) is used to
// properly adjust the number of expected results that will be returned based
// on the desired hidden power and type.
uint64_t IVs::AdjustExpectedResultsForHiddenPower
  (uint64_t numResults, IndividualValues minIVs, IndividualValues maxIVs,
   Element::Type type, uint32_t minPower)
      throw (ImpossibleHiddenTypeException, ImpossibleMinHiddenPowerException)
{
  static const Type  IVOrdering[] = { HP, AT, DF, SP, SA, SD };
  
  if (type == Element::UNKNOWN)  return numResults;
  
  uint32_t  addend = 0x1;
  uint64_t  numIVCombos = 1;
  
  HPSumCountList  typeSumList, powerSumList;
  HPSumCount      dummy; dummy.sum = 0; dummy.count = 1;
  
  typeSumList.push_back(dummy);  powerSumList.push_back(dummy);
  
  // For both the hidden type and its power, generate each possible sum and
  // the number of times it can appear in all possible IV combinations produced
  // by the given min and max IVs.
  for (uint32_t i = 0; i < NUM_IVS; ++i)
  {
    uint32_t  minIV = minIVs.iv(IVOrdering[i]);
    uint32_t  maxIV = maxIVs.iv(IVOrdering[i]);
    uint32_t  numEvenTypeIVs = 0, numOddTypeIVs = 0;
    uint32_t  numEvenPwrIVs = 0, numOddPwrIVs = 0;
    
    // Count how many set and unset first and second bits there are
    // in this IV's range of values.
    // This could perhaps be done with some smarter math, but it would end
    // up being more complicated, and the rest is already complicated enough.
    for (uint32_t j = minIV; j <= maxIV; ++j)
    {
      numEvenTypeIVs += (j & 0x1) ^ 0x1;
      numOddTypeIVs += j & 0x1;
      numEvenPwrIVs += ((j & 0x2) >> 1) ^ 0x1;
      numOddPwrIVs += (j & 0x2) >> 1;
    }
    
    UpdateHPSumsAndCounts(typeSumList, addend, numEvenTypeIVs, numOddTypeIVs);
    UpdateHPSumsAndCounts(powerSumList, addend, numEvenPwrIVs, numOddPwrIVs);
    
    addend <<= 1;
    numIVCombos *= numEvenTypeIVs + numOddTypeIVs;
  }
  
  // Now that all of the sums and counts are determined, do the final
  // calculations to determine the real distributions
  std::map<Element::Type, uint32_t>  typeCountMap;
  std::map<uint32_t, uint32_t>       powerCountMap;
  
  BuildHPMap<Element::Type, 15, 1>(typeCountMap, typeSumList);
  BuildHPMap<uint32_t, 40, 30>(powerCountMap, powerSumList);
  
  // specified power is minimum
  std::map<uint32_t, uint32_t>::const_iterator  pit;
  pit = powerCountMap.lower_bound(minPower);
  
  uint64_t  hpMultiplier = 0;
  
  while (pit != powerCountMap.end())
  {
    hpMultiplier += pit->second;
    ++pit;
  }
  
  if (hpMultiplier == 0)
    throw ImpossibleMinHiddenPowerException(minIVs, maxIVs, minPower);
  
  uint64_t  hpDivisor = numIVCombos;
  
  if (type != Element::ANY)
  {
    std::map<Element::Type, uint32_t>::const_iterator  it;
    it = typeCountMap.find(type);
    
    if (it == typeCountMap.end())
      throw ImpossibleHiddenTypeException(minIVs, maxIVs, type);
    
    hpMultiplier *= it->second;
    hpDivisor *= numIVCombos;
  }
  
  return numResults * hpMultiplier / hpDivisor;
}

bool IVs::betterThan(const IndividualValues &ivs) const
{
  return ((word & IVs::HP_MASK) > (ivs.word & IVs::HP_MASK)) &&
         ((word & IVs::AT_MASK) > (ivs.word & IVs::AT_MASK)) &&
         ((word & IVs::DF_MASK) > (ivs.word & IVs::DF_MASK)) &&
         ((word & IVs::SA_MASK) > (ivs.word & IVs::SA_MASK)) &&
         ((word & IVs::SD_MASK) > (ivs.word & IVs::SD_MASK)) &&
         ((word & IVs::SP_MASK) > (ivs.word & IVs::SP_MASK));
}

bool IVs::betterThanOrEqual(const IndividualValues &ivs) const
{
  return ((word & IVs::HP_MASK) >= (ivs.word & IVs::HP_MASK)) &&
         ((word & IVs::AT_MASK) >= (ivs.word & IVs::AT_MASK)) &&
         ((word & IVs::DF_MASK) >= (ivs.word & IVs::DF_MASK)) &&
         ((word & IVs::SA_MASK) >= (ivs.word & IVs::SA_MASK)) &&
         ((word & IVs::SD_MASK) >= (ivs.word & IVs::SD_MASK)) &&
         ((word & IVs::SP_MASK) >= (ivs.word & IVs::SP_MASK));
}

Characteristic::Type Characteristic::Get(PID pid, IVs ivs)
{
  uint32_t      maxIVValue = 0;
  unsigned int  maxes = 0;
  
  if (ivs.hp() >= maxIVValue)
  {
    maxIVValue = ivs.hp();
    maxes = 0x1 << IVs::HP;
  }
  
  if (ivs.at() > maxIVValue)
  {
    maxIVValue = ivs.at();
    maxes = 0x1 << IVs::AT;
  }
  else if (ivs.at() == maxIVValue)
  {
    maxes |= 0x1 << IVs::AT;
  }
  
  if (ivs.df() > maxIVValue)
  {
    maxIVValue = ivs.df();
    maxes = 0x1 << IVs::DF;
  }
  else if (ivs.df() == maxIVValue)
  {
    maxes |= 0x1 << IVs::DF;
  }
  
  if (ivs.sa() > maxIVValue)
  {
    maxIVValue = ivs.sa();
    maxes = 0x1 << IVs::SA;
  }
  else if (ivs.sa() == maxIVValue)
  {
    maxes |= 0x1 << IVs::SA;
  }
  
  if (ivs.sd() > maxIVValue)
  {
    maxIVValue = ivs.sd();
    maxes = 0x1 << IVs::SD;
  }
  else if (ivs.sd() == maxIVValue)
  {
    maxes |= 0x1 << IVs::SD;
  }
  
  if (ivs.sp() > maxIVValue)
  {
    maxIVValue = ivs.sp();
    maxes = 0x1 << IVs::SP;
  }
  else if (ivs.sp() == maxIVValue)
  {
    maxes |= 0x1 << IVs::SP;
  }
  
  uint32_t  offset = maxIVValue % 10;
  if (offset > 4)
    offset -= 5;
  
  const IVs::Type  ivOrder[6] =
    { IVs::HP, IVs::AT, IVs::DF, IVs::SP, IVs::SA, IVs::SD };
  
  const Type  start[6] =
    { LOVES_TO_EAT, PROUD_OF_ITS_POWER, STURDY_BODY,
      LIKES_TO_RUN, HIGHLY_CURIOUS, STRONG_WILLED };
  
  uint32_t   index = pid.word % 6;
  uint32_t   count = 0;
  
  while (count < 6)
  {
    if (maxes & (0x1 << ivOrder[index]))
    {
      return Type(start[index] + offset);
    }
    
    if (++index > 5) index = 0;
    ++count;
  }
  
  return UNKNOWN;
}

const std::string& Characteristic::ToString(Characteristic::Type c)
{
  if ((c >= LOVES_TO_EAT) && (c <= UNKNOWN))
  {
    return CharacteristicName[c];
  }
  else
  {
    return CharacteristicName[UNKNOWN];
  }
}

}
