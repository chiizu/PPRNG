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
  "White (Japanese)", "White (Spanish)",
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
        result += ", ";
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
          result += ", " + ButtonName[i];
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

std::string PersonalityValue::GenderString() const
{
  uint32_t  genderValue = GenderValue();
  
  if (genderValue < 31)
  {
    return "F/F/F/F";
  }
  else if (genderValue < 63)
  {
    return "M/F/F/F";
  }
  else if (genderValue < 127)
  {
    return "M/M/F/F";
  }
  else if (genderValue < 191)
  {
    return "M/M/M/F";
  }
  else
  {
    return "M/M/M/M";
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
  
  return static_cast<Element::Type>(((typeSum * 15) / 63) + 1);
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

}
