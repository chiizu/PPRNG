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


#include "PPRNGTypes.h"
#include <list>
#include <map>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctype.h>

#include "LinearCongruentialRNG.h"
#include "MersenneTwisterRNG.h"
#include "HashedSeed.h"

namespace pprng
{

namespace
{

static const std::string  EmptyString;

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
  "Black 2 (Japanese)", "White 2 (Japanese)",
  "Black 2 (English)", "White 2 (English)",
  "Black 2 (French)", "White 2 (French)",
  "Black 2 (German)", "White 2 (German)",
  "Black 2 (Italian)", "White 2 (Italian)",
  "Black 2 (Korean)", "White 2 (Korean)",
  "Black 2 (Spanish)", "White 2 (Spanish)",
  "Unknown Version"
};

static const std::vector<std::string>  GameVersionName
  (GameVersionNameArray, GameVersionNameArray + 
                    (sizeof(GameVersionNameArray) / sizeof(const char *)));

static const char *NatureNameArray[] =
{ "Hardy", "Lonely", "Brave", "Adamant", "Naughty", "Bold", "Docile", "Relaxed",
  "Impish", "Lax", "Timid", "Hasty", "Serious", "Jolly", "Naive", "Modest",
  "Mild", "Quiet", "Bashful", "Rash", "Calm", "Gentle", "Sassy", "Careful",
  "Quirky", "Any", "<SYNC>", "<ES>", "<MIXED>" };

static const std::vector<std::string>  NatureName
  (NatureNameArray, NatureNameArray +
                    (sizeof(NatureNameArray) / sizeof(const char *)));

static const char *ElementNameArray[] =
{ "Normal", "Fighting", "Flying", "Poison", "Ground", "Rock", "Bug", "Ghost",
  "Steel", "Fire", "Water", "Grass", "Electric", "Psychic", "Ice", "Dragon",
  "Dark", "Any" };

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
  "Somewhat of a clown", "Quick to flee", "Any" };

static const std::vector<std::string>  CharacteristicName
  (CharacteristicNameArray,
   CharacteristicNameArray +
     (sizeof(CharacteristicNameArray) / sizeof(const char *)));

static const char *EncounterItemNameArray[] =
{ "None",
  "Sun Stone", "Moon Stone", "Fire Stone", "Thunder Stone", "Water Stone",
  "Leaf Stone", "Shiny Stone", "Dusk Stone", "Dawn Stone", "Oval Stone",
  "Fire Gem", "Water Gem", "Electric Gem", "Grass Gem", "Ice Gem",
  "Fighting Gem", "Poison Gem", "Ground Gem", "Flying Gem", "Psychic Gem",
  "Bug Gem", "Rock Gem", "Ghost Gem", "Dragon Gem", "Dark Gem", "Steel Gem",
  "Normal Gem", "Everstone", "King's Rock",
  "Health Wing", "Muscle Wing", "Resist Wing", "Genius Wing", "Clever Wing",
  "Swift Wing", "Pretty Wing" };

static const std::vector<std::string>  EncounterItemName
  (EncounterItemNameArray,
   EncounterItemNameArray +
     (sizeof(EncounterItemNameArray) / sizeof(const char *)));

static std::string MakeBadIVIndexExceptionString(int i)
{
  std::ostringstream  os;
  os << "Bad IV Index: " << i;
  return os.str();
}

}

std::string DS::ToString(DS::Type t)
{
  if ((t >= DS::DSPhat) && (t <= DS::_3DS))
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

Game::Version Game::VersionForColorAndLanguage(Game::Color c, Game::Language l)
{
  switch (c)
  {
  case Emerald:    return EmeraldVersion;
  case Diamond:    return DiamondVersion;
  case Pearl:      return PearlVersion;
  case Platinum:   return PlatinumVersion;
  case HeartGold:  return HeartGoldVersion;
  case SoulSilver: return SoulSilverVersion;
  case Black:
    switch (l)
    {
    case English: return BlackEnglish;
    case French: return BlackFrench;
    case German: return BlackGerman;
    case Italian: return BlackItalian;
    case Japanese: return BlackJapanese;
    case Korean: return BlackKorean;
    case Spanish: return BlackSpanish;
    default: return NoVersion;
    }
  case White:
    switch (l)
    {
    case English: return WhiteEnglish;
    case French: return WhiteFrench;
    case German: return WhiteGerman;
    case Italian: return WhiteItalian;
    case Japanese: return WhiteJapanese;
    case Korean: return WhiteKorean;
    case Spanish: return WhiteSpanish;
    default: return NoVersion;
    }
  case Black2:
    switch (l)
    {
    case English: return Black2English;
    case French: return Black2French;
    case German: return Black2German;
    case Italian: return Black2Italian;
    case Japanese: return Black2Japanese;
    case Korean: return Black2Korean;
    case Spanish: return Black2Spanish;
    default: return NoVersion;
    }
  case White2:
    switch (l)
    {
    case English: return White2English;
    case French: return White2French;
    case German: return White2German;
    case Italian: return White2Italian;
    case Japanese: return White2Japanese;
    case Korean: return White2Korean;
    case Spanish: return White2Spanish;
    default: return NoVersion;
    }
  default: return NoVersion;
  }
}

const std::string& Nature::ToString(Nature::Type t)
{
  if ((t >= HARDY) && (t <= MIXED))
  {
    return NatureName[t];
  }
  else
  {
    return EmptyString;
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
    return NONE;
  }
}

const std::string& Element::ToString(Element::Type t)
{
  if ((t >= NORMAL) && (t <= ANY))
  {
    return ElementName[t];
  }
  else
  {
    return EmptyString;
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
    return NONE;
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

const IVs  IVs::Perfect(31, 31, 31, 31, 31, 31);
const IVs  IVs::PerfectTrick(31, 31, 31, 31, 31, 0);
const IVs  IVs::PhysPerfect(31, 31, 31, 0, 31, 31);
const IVs  IVs::PhysPerfectTrick(31, 31, 31, 0, 31, 0);
const IVs  IVs::SpecPerfect(31, 0, 31, 31, 31, 31);
const IVs  IVs::SpecPerfectTrick(31, 0, 31, 31, 31, 0);
const IVs  IVs::HpPerfectLow(30, 0, 30, 30, 30, 30);
const IVs  IVs::HpPerfectHigh(31, 31, 31, 31, 31, 31);
const IVs  IVs::HpPerfectTrickLow(30, 0, 30, 30, 30, 2);
const IVs  IVs::HpPerfectTrickHigh(31, 31, 31, 31, 31, 3);

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
// on the desired hidden power and types.
uint64_t IVs::AdjustExpectedResultsForHiddenPower
  (uint64_t numResults, IndividualValues minIVs, IndividualValues maxIVs,
   uint32_t typeMask, uint32_t minPower)
      throw (ImpossibleHiddenTypeException, ImpossibleMinHiddenPowerException)
{
  static const Type  IVOrdering[] = { HP, AT, DF, SP, SA, SD };
  static const uint32_t  AllHiddenTypes = (0x1 << 16) - 1;
  
  if ((typeMask == AllHiddenTypes) || (typeMask == 0))
    return numResults;
  
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
  
  if (typeMask != AllHiddenTypes)
  {
    uint64_t  typeMultiplier = 0;
    uint32_t  typeIdx = 1;
    
    while (typeMask != 0)
    {
      if ((typeMask & 0x1) != 0)
      {
        Element::Type  type = Element::Type(typeIdx);
        
        std::map<Element::Type, uint32_t>::const_iterator  it;
        it = typeCountMap.find(type);
        
        if (it == typeCountMap.end())
          throw ImpossibleHiddenTypeException(minIVs, maxIVs, type);
        
        typeMultiplier += it->second;
      }
      
      typeMask >>= 1;
      ++typeIdx;
    }
    
    hpMultiplier *= typeMultiplier;
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

IVPattern::Type IVPattern::Get(const IVs &min, const IVs &max,
                               bool considerHiddenPower,
                               uint32_t minHiddenPower)
{
  if ((min == IVs::Perfect) && (max == IVs::Perfect))
  {
    return IVPattern::HEX_FLAWLESS;
  }
  else if ((min == IVs::PerfectTrick) && (max == IVs::PerfectTrick))
  {
    return IVPattern::HEX_FLAWLESS_TRICK;
  }
  else if (((min.word & IVs::PhysPerfect.word) == IVs::PhysPerfect.word) &&
           ((max.word & IVs::PhysPerfect.word) == IVs::PhysPerfect.word))
  {
    return IVPattern::PHYSICAL_FLAWLESS;
  }
  else if (((min.word & IVs::PhysPerfect.word) == IVs::PhysPerfectTrick.word) &&
           ((max.word & IVs::PhysPerfect.word) == IVs::PhysPerfectTrick.word))
  {
    return IVPattern::PHYSICAL_FLAWLESS_TRICK;
  }
  else if (((min.word & IVs::SpecPerfect.word) == IVs::SpecPerfect.word) &&
           ((max.word & IVs::SpecPerfect.word) == IVs::SpecPerfect.word))
  {
    return IVPattern::SPECIAL_FLAWLESS;
  }
  else if (((min.word & IVs::SpecPerfect.word) == IVs::SpecPerfectTrick.word) &&
           ((max.word & IVs::SpecPerfect.word) == IVs::SpecPerfectTrick.word))
  {
    return IVPattern::SPECIAL_FLAWLESS_TRICK;
  }
  else if (considerHiddenPower && (minHiddenPower == 70))
  {
    if (min.betterThanOrEqual(IVs::HpPerfectLow) &&
        max.worseThanOrEqual(IVs::HpPerfectHigh))
    {
      return IVPattern::SPECIAL_HIDDEN_POWER_FLAWLESS;
    }
    else if (min.betterThanOrEqual(IVs::HpPerfectTrickLow) &&
             max.worseThanOrEqual(IVs::HpPerfectTrickHigh))
    {
      return IVPattern::SPECIAL_HIDDEN_POWER_FLAWLESS_TRICK;
    }
  }
  
  return IVPattern::CUSTOM;
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
  
  return NONE;
}

const std::string& Characteristic::ToString(Characteristic::Type c)
{
  if ((c >= LOVES_TO_EAT) && (c < NUM_CHARACTERISTICS))
  {
    return CharacteristicName[c];
  }
  else
  {
    return EmptyString;
  }
}

namespace
{

typedef std::map<EncounterSlot::Value, std::string>  ESVNameMap;

static ESVNameMap::value_type ESVNameData[] =
{
  ESVNameMap::value_type(ESV::LAND_0, "Land 0"),
  ESVNameMap::value_type(ESV::LAND_1, "Land 1"),
  ESVNameMap::value_type(ESV::LAND_2, "Land 2"),
  ESVNameMap::value_type(ESV::LAND_3, "Land 3"),
  ESVNameMap::value_type(ESV::LAND_4, "Land 4"),
  ESVNameMap::value_type(ESV::LAND_5, "Land 5"),
  ESVNameMap::value_type(ESV::LAND_6, "Land 6"),
  ESVNameMap::value_type(ESV::LAND_7, "Land 7"),
  ESVNameMap::value_type(ESV::LAND_8, "Land 8"),
  ESVNameMap::value_type(ESV::LAND_9, "Land 9"),
  ESVNameMap::value_type(ESV::LAND_10, "Land 10"),
  ESVNameMap::value_type(ESV::LAND_11, "Land 11"),
  ESVNameMap::value_type(ESV::SWARM, "Swarm"),
  ESVNameMap::value_type(ESV::SURF_0, "Surf 0"),
  ESVNameMap::value_type(ESV::SURF_1, "Surf 1"),
  ESVNameMap::value_type(ESV::SURF_2, "Surf 2"),
  ESVNameMap::value_type(ESV::SURF_3, "Surf 3"),
  ESVNameMap::value_type(ESV::SURF_4, "Surf 4"),
  ESVNameMap::value_type(ESV::OLD_ROD_0, "Old Rod 0"),
  ESVNameMap::value_type(ESV::OLD_ROD_1, "Old Rod 1"),
  ESVNameMap::value_type(ESV::OLD_ROD_2, "Old Rod 2"),
  ESVNameMap::value_type(ESV::OLD_ROD_3, "Old Rod 3"),
  ESVNameMap::value_type(ESV::OLD_ROD_4, "Old Rod 4"),
  ESVNameMap::value_type(ESV::GOOD_ROD_0, "Good Rod 0"),
  ESVNameMap::value_type(ESV::GOOD_ROD_1, "Good Rod 1"),
  ESVNameMap::value_type(ESV::GOOD_ROD_2, "Good Rod 2"),
  ESVNameMap::value_type(ESV::GOOD_ROD_3, "Good Rod 3"),
  ESVNameMap::value_type(ESV::GOOD_ROD_4, "Good Rod 4"),
  ESVNameMap::value_type(ESV::SUPER_ROD_0, "Super Rod 0"),
  ESVNameMap::value_type(ESV::SUPER_ROD_1, "Super Rod 1"),
  ESVNameMap::value_type(ESV::SUPER_ROD_2, "Super Rod 2"),
  ESVNameMap::value_type(ESV::SUPER_ROD_3, "Super Rod 3"),
  ESVNameMap::value_type(ESV::SUPER_ROD_4, "Super Rod 4")
};

static const ESVNameMap ESVName
  (ESVNameData, ESVNameData +
                    (sizeof(ESVNameData) / sizeof(ESVNameMap::value_type)));

// J, K, Gen 5 Land ESV
static const uint32_t LandESVThreshold[] =
{ 19, 39, 49, 59, 69, 79, 84, 89, 93, 97, 98, 99 };

// J, K, Gen5 Surf ESV
static const uint32_t SurfESVThreshold[] =
{ 59, 89, 94, 98, 99 };

// J Good/Super Fishing, Gen 5 Fishing (?)
static const uint32_t GoodSuperFishingESVThresholdJ[] =
{ 39, 79, 94, 98, 99 };

// K Good/Super Fishing, Gen 5 Water Spot (?)
static const uint32_t GoodSuperFishingESVThresholdK[] =
{ 39, 69, 84, 94, 99 };

static const uint32_t BugContestESVThreshold[] =
{ 19, 39, 49, 59, 69, 79, 84, 89, 94, 99 };

static const uint32_t SafariZoneESVThreshold[] =
{ 9, 19, 29, 39, 49, 59, 69, 79, 89, 99 };

static uint32_t GetESV(uint32_t value, const uint32_t esvThreshold[])
{
  uint32_t  slot = 0;
  while (value > esvThreshold[slot])
    ++slot;
  
  return slot;
}

}


std::string EncounterSlot::ToString(EncounterSlot::Value v)
{
  ESVNameMap::const_iterator  i = ESVName.find(v);
  if (i != ESVName.end())
    return i->second;
  
  return "Unknown Slot";
}


EncounterSlot::Value EncounterSlot::Gen4Land(uint32_t percent)
{
  return Value(GetESV(percent, LandESVThreshold) | LAND_TYPE);
}

EncounterSlot::Value EncounterSlot::Gen4Surfing(uint32_t percent)
{
  return Value(GetESV(percent, SurfESVThreshold) | SURF_TYPE);
}

EncounterSlot::Value EncounterSlot::Gen4OldRod(uint32_t percent)
{
  return Value(GetESV(percent, SurfESVThreshold) | OLD_ROD_TYPE);
}

EncounterSlot::Value EncounterSlot::Gen4GoodRodJ(uint32_t percent)
{
  return Value(GetESV(percent, GoodSuperFishingESVThresholdJ) | GOOD_ROD_TYPE);
}

EncounterSlot::Value EncounterSlot::Gen4GoodRodK(uint32_t percent)
{
  return Value(GetESV(percent, GoodSuperFishingESVThresholdK) | GOOD_ROD_TYPE);
}

EncounterSlot::Value EncounterSlot::Gen4SuperRodJ(uint32_t percent)
{
  return Value(GetESV(percent, GoodSuperFishingESVThresholdJ) | SUPER_ROD_TYPE);
}

EncounterSlot::Value EncounterSlot::Gen4SuperRodK(uint32_t percent)
{
  return Value(GetESV(percent, GoodSuperFishingESVThresholdK) | SUPER_ROD_TYPE);
}


EncounterSlot::Value EncounterSlot::Gen5Land(uint32_t percent)
{
  return Value(GetESV(percent, LandESVThreshold) | LAND_TYPE);
}

EncounterSlot::Value EncounterSlot::Gen5Surfing(uint32_t percent)
{
  return Value(GetESV(percent, SurfESVThreshold) | SURF_TYPE);
}

EncounterSlot::Value EncounterSlot::Gen5Fishing(uint32_t percent)
{
  return Value(GetESV(percent, GoodSuperFishingESVThresholdJ) | GOOD_ROD_TYPE);
}

EncounterSlot::Value EncounterSlot::Gen5WaterSpot(uint32_t percent)
{
  return Value(GetESV(percent, GoodSuperFishingESVThresholdK) | GOOD_ROD_TYPE);
}


std::string EncounterItem::ToString(EncounterItem::Type t)
{
  if ((t >= EncounterItem::NONE) && (t <= EncounterItem::PRETTY_WING))
  {
    return EncounterItemName[t];
  }
  else
  {
    return "Unknown Item";
  }
}


CoinFlips::CoinFlips(uint32_t seed, uint32_t numFlips)
  : word(0)
{
  MTRNG     rng(seed);
  
  for (uint32_t i = 0; i < numFlips; ++i)
    word = word | (CalcResult(rng.Next()) << i);
  
  word = word | (numFlips << FLIP_COUNT_SHIFT);
}


ProfElmResponses::ProfElmResponses(uint32_t seed, uint32_t numResponses)
  : word(0)
{
  LCRNG34  rng(seed);
  
  for (uint32_t i = 0; i < numResponses; ++i)
    word = word | (uint64_t(CalcResponse(rng.Next())) << (i << 1));
  
  word = word | (uint64_t(numResponses) << RESPONSE_COUNT_SHIFT);
}

uint32_t GetJohtoRoamerLocation(LCRNG34 &rng)
{
  uint32_t  result = (rng.Next() >> 16) % 16;
  return (result < 11) ? (result + 29) : (result + 31);
}

uint32_t GetKantoRoamerLocation(LCRNG34 &rng)
{
  uint32_t  result = (rng.Next() >> 16) % 25;
  return (result < 22) ? (result + 1) : (24 + ((result - 22) * 2));
}

HGSSRoamers::HGSSRoamers(uint32_t seed, uint32_t raikouLocation,
                         uint32_t enteiLocation, uint32_t latiLocation)
  : word(0)
{
  LCRNG34   rng(seed);
  uint32_t  result = 0;
  uint32_t  consumed = 0;
  
  if (raikouLocation > 0)
  {
    uint32_t  newLocation;
    do
    {
      newLocation = GetJohtoRoamerLocation(rng);
      ++consumed;
    }
    while (newLocation == raikouLocation);
    
    result = newLocation << (LOCATION_BITS * RAIKOU);
  }
  
  if (enteiLocation > 0)
  {
    uint32_t  newLocation;
    do
    {
      newLocation = GetJohtoRoamerLocation(rng);
      ++consumed;
    }
    while (newLocation == enteiLocation);
    
    result = result | (newLocation << (LOCATION_BITS * ENTEI));
  }
  
  if (latiLocation > 0)
  {
    uint32_t  newLocation;
    do
    {
      newLocation = GetKantoRoamerLocation(rng);
      ++consumed;
    }
    while (newLocation == latiLocation);
    
    result = result | (newLocation << (LOCATION_BITS * LATI));
  }
  
  word = result | (consumed << CONSUMED_FRAMES_SHIFT);
}


namespace
{

uint64_t GenerateSpinnerPositions(LCRNG5 &rng, uint32_t numSpins)
{
  uint64_t  word = 0;
  
  for (uint32_t i = 0; i < numSpins; ++i)
  {
    word = word |
      (uint64_t(SpinnerPositions::CalcPosition(rng.Next())) << (i * 3));
    rng.Next();
  }
  
  word = word | (uint64_t(numSpins) << SpinnerPositions::SPIN_COUNT_SHIFT);
  
  return word;
}

}

SpinnerPositions::SpinnerPositions(uint64_t seed, uint32_t numSpins)
  : word(0)
{
  LCRNG5  rng(seed);
  
  word = GenerateSpinnerPositions(rng, numSpins);
}


SpinnerPositions::SpinnerPositions(const HashedSeed &seed, bool memoryLinkUsed,
                                   uint32_t numSpins)
  : word(0)
{
  LCRNG5  rng(0);
  
  seed.SeedAndSkipPIDFrames(rng, memoryLinkUsed);
  
  if (!memoryLinkUsed)
    rng.Next();
  
  word = GenerateSpinnerPositions(rng, numSpins);
}

}
