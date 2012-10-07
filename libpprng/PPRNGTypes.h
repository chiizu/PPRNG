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

#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

#include <stdint.h>
#include <string>
#include <vector>
#include <stdexcept>

namespace pprng
{

// base exception class for the library
class Exception : public std::runtime_error
{
public:
  Exception(const std::string &what)
    : std::runtime_error(what)
  {}
};

struct DS
{
  enum Type
  {
    None = -1,
    DSPhat = 0,
    DSLite,
    DSi,
    DSi_XL,
    _3DS,
    
    UnknownDSType
  };
  
  static std::string ToString(Type v);
};


struct MACAddress
{
  MACAddress() : high(0), low(0) {}
  MACAddress(uint32_t h, uint32_t l) : high(h), low(l) {}
  
  uint32_t  high, low; // high 3 bytes and low 3 bytes
};


struct Button
{
  enum Binary
  {
    NO_BUTTON = 0,
    A_BUTTON = 0x1,
    B_BUTTON = 0x2,
    SELECT_BUTTON = 0x4,
    START_BUTTON = 0x8,
    R_BUTTON = 0x100,
    L_BUTTON = 0x200,
    X_BUTTON = 0x400,
    Y_BUTTON = 0x800,
    
    SINGLE_BUTTON_MASK = 0xF0F,
    
    NUM_BINARY_BUTTONS = 8
  };
  
  // treat as one button
  enum DPad
  {
    RIGHT_BUTTON = 0x10,
    LEFT_BUTTON = 0x20,
    UP_BUTTON = 0x40,
    DOWN_BUTTON = 0x80,
    UP_RIGHT = 0x50,
    UP_LEFT = 0x60,
    DOWN_RIGHT = 0x90,
    DOWN_LEFT = 0xA0,
    
    DPAD_MASK = 0xF0,
    DPAD_SHIFT = 4,
    
    NUM_DPAD_DIRECTIONS = 8
  };
  
  typedef std::vector<uint32_t>  List;
  
  static std::string ToString(uint32_t buttons);
  
  static const List& SingleButtons();
  static const List& TwoButtonCombos();
  static const List& ThreeButtonCombos();
};

struct Game
{
  enum Version
  {
    None = -1,
    
    ThirdGenStart = 0,
    
    Emerald = ThirdGenStart,
    
    ThirdGenEnd = Emerald + 1,
    
    
    FourthGenStart = ThirdGenEnd,
    
    Diamond = FourthGenStart,
    Pearl,
    Platinum,
    
    HeartGold,
    SoulSilver,
    
    FourthGenEnd = SoulSilver + 1,
    
    
    FifthGenStart = FourthGenEnd,
    
    BlackWhiteStart = FifthGenStart,
    
    BlackEnglish = BlackWhiteStart,
    BlackFrench,
    BlackGerman,
    BlackItalian,
    BlackJapanese,
    BlackSpanish,
    
    WhiteEnglish,
    WhiteFrench,
    WhiteGerman,
    WhiteItalian,
    WhiteJapanese,
    WhiteSpanish,
    
    // add after the fact...
    BlackKorean,
    WhiteKorean,
    
    BlackWhiteEnd,
    
    Black2White2Start = BlackWhiteEnd,
    Black2Japanese = Black2White2Start,
    White2Japanese,
    Black2English,
    White2English,
    Black2French,
    White2French,
    Black2German,
    White2German,
    Black2Italian,
    White2Italian,
    Black2Korean,
    White2Korean,
    Black2Spanish,
    White2Spanish,
    Black2White2End,
    
    FifthGenEnd = Black2White2End,
    
    UnknownVersion = FifthGenEnd,
    
    NumVersions = FifthGenEnd
  };
  
  static bool IsBlackWhite(Version v)
  { return (v >= Game::BlackWhiteStart) && (v < Game::BlackWhiteEnd); }
  
  static bool IsBlack2White2(Version v)
  { return (v >= Game::Black2White2Start) && (v < Game::Black2White2End); }
  
  static std::string ToString(Version v);
};


struct Nature
{
  enum Type
  {
    NONE = -1,
    
    HARDY = 0,
    LONELY,
    BRAVE,
    ADAMANT,
    NAUGHTY,
    BOLD,
    DOCILE,
    RELAXED,
    IMPISH,
    LAX,
    TIMID,
    HASTY,
    SERIOUS,
    JOLLY,
    NAIVE,
    MODEST,
    MILD,
    QUIET,
    BASHFUL,
    RASH,
    CALM,
    GENTLE,
    SASSY,
    CAREFUL,
    QUIRKY,
    
    NUM_TYPES,
    
    // used when setting search criteria
    ANY = NUM_TYPES,
    
    // used to indicate synchronize/everstone was activated
    SYNCHRONIZE,
    EVERSTONE,
    MIXED
  };
  
  static const std::string& ToString(Type t);
  static Type FromString(const std::string &name);
};


struct Ability
{
  enum Type
  {
    NONE = -1,
    ZERO = 0,
    ONE,
    HIDDEN,
    
    ANY
  };
};


struct Gender
{
  enum Type
  {
    NONE = -1,
    FEMALE = 0,
    MALE,
    GENDERLESS,
    
    ANY
  };
  
  enum Ratio
  {
    NO_RATIO = -1,
    ONE_EIGHTH_FEMALE = 0,
    ONE_FOURTH_FEMALE,
    ONE_HALF_FEMALE,
    THREE_FOURTHS_FEMALE,
    FEMALE_ONLY,
    MALE_ONLY,
    
    ANY_RATIO
  };
  
  enum Threshold
  {
    MALE_ONLY_THRESHOLD = 0,
    ONE_EIGHTH_FEMALE_THRESHOLD = 31,
    ONE_FOURTH_FEMALE_THRESHOLD = 63,
    ONE_HALF_FEMALE_THRESHOLD = 127,
    THREE_FOURTHS_FEMALE_THRESHOLD = 191,
    SEVEN_EIGHTHS_FEMALE_THRESHOLD = 223,
    FEMALE_ONLY_THRESHOLD = 254,
    GENDERLESS_THRESHOLD = 255
  };
  
  static Threshold GetThreshold(Ratio r)
  {
    switch (r)
    {
    case MALE_ONLY:
      return MALE_ONLY_THRESHOLD;
    case ONE_EIGHTH_FEMALE:
      return ONE_EIGHTH_FEMALE_THRESHOLD;
    case ONE_FOURTH_FEMALE:
      return ONE_FOURTH_FEMALE_THRESHOLD;
    case ONE_HALF_FEMALE:
      return ONE_HALF_FEMALE_THRESHOLD;
    case THREE_FOURTHS_FEMALE:
      return THREE_FOURTHS_FEMALE_THRESHOLD;
    case FEMALE_ONLY:
      return FEMALE_ONLY_THRESHOLD;
    case NO_RATIO:
    default:
      return GENDERLESS_THRESHOLD;
    }
  }
  
  static bool GenderValueMatches(uint32_t value, Type t, Ratio r)
  {
    if ((t == ANY) || (r == ANY_RATIO))
      return true;
    
    uint32_t  threshold = GetThreshold(r);
    
    return (t == FEMALE) ? (value < threshold) : (value >= threshold);
  }
  
  static uint32_t MakeGenderValue(Type t, Ratio r, uint64_t randomValue)
  {
    switch (r)
    {
    case FEMALE_ONLY:
      return ((randomValue * 0x8) >> 32) + 1;
      
    case MALE_ONLY:
      return ((randomValue * 0xF6) >> 32) + 8;
      
    default:
      Threshold  th = GetThreshold(r);
      
      switch (t)
      {
      case FEMALE:
        return ((randomValue * (th - 1)) >> 32) + 1;
        
      case MALE:
        return ((randomValue * (0xFE - th)) >> 32) + th;
        
      case GENDERLESS:
      default:
        return randomValue;
      }
    }
  }
};


struct PersonalityValue
{
  PersonalityValue() : word(0) {}
  
  PersonalityValue(const PersonalityValue &pid) : word(pid.word) {}
  explicit PersonalityValue(uint32_t pidWord) : word(pidWord) {}
  
  PersonalityValue& operator=(const PersonalityValue &pid)
  {
    word = pid.word;
    return *this;
  }
  
  PersonalityValue& operator=(uint32_t pidWord)
  {
    word = pidWord;
    return *this;
  }
  
  uint32_t GenderValue() const { return word & 0xff; }
  
  Ability::Type Gen34Ability() const
  { return Ability::Type(word & 0x1); }
  
  Ability::Type Gen5Ability() const
  { return Ability::Type((word >> 16) & 0x1); }
  
  Nature::Type Gen34Nature() const
  { return Nature::Type(word % 25); }
  
  bool IsShiny(uint32_t tid, uint32_t sid) const
  {
    uint32_t  e = tid ^ sid;
    uint32_t  f = (word >> 16) ^ (word & 0xffff);
    return (e ^ f) < 8;
  }
  
  uint32_t  word;
};

typedef PersonalityValue  PID;


struct Element
{
  enum Type
  {
    NONE = -1,
    
    NORMAL = 0,
    FIGHTING,
    FLYING,
    POISON,
    GROUND,
    ROCK,
    BUG,
    GHOST,
    STEEL,
    FIRE,
    WATER,
    GRASS,
    ELECTRIC,
    PSYCHIC,
    ICE,
    DRAGON,
    DARK,
    
    NUM_TYPES,
    
    // used when setting search criteria
    ANY = NUM_TYPES
  };
  
  static const std::string& ToString(Type t);
  static Type FromString(const std::string &name);
};


struct OptionalIVs;

// structure holding the 6 individual values
struct IndividualValues
{
  enum Type
  {
    HP = 0,
    AT,
    DF,
    SA,
    SD,
    SP,
    
    NUM_IVS
  };
  
  // bit shift amounts for packing / unpacking ivs from 32 bit word
  // in Gen 3 / 4 format (empty-bit, sd, sa, sp, empty-bit, df, at, hp)
  enum Shift
  {
    HP_SHIFT = 0,
    AT_SHIFT = 5,
    DF_SHIFT = 10,
    SA_SHIFT = 21,
    SD_SHIFT = 26,
    SP_SHIFT = 16
  };
  
  static const Shift  IVShift[NUM_IVS];
  
  enum Mask
  {
    IV_MASK = 0x1f,
    
    HP_MASK = IV_MASK << HP_SHIFT,
    AT_MASK = IV_MASK << AT_SHIFT,
    DF_MASK = IV_MASK << DF_SHIFT,
    SA_MASK = IV_MASK << SA_SHIFT,
    SD_MASK = IV_MASK << SD_SHIFT,
    SP_MASK = IV_MASK << SP_SHIFT,
    
    ALL_IVS_MASK = HP_MASK | AT_MASK | DF_MASK | SA_MASK | SD_MASK | SP_MASK
  };
  
  static const Mask  IVMask[NUM_IVS];
  
  // commonly used IVs
  static const IndividualValues  Perfect;
  static const IndividualValues  PerfectTrick;
  static const IndividualValues  PhysPerfect;
  static const IndividualValues  PhysPerfectTrick;
  static const IndividualValues  SpecPerfect;
  static const IndividualValues  SpecPerfectTrick;
  static const IndividualValues  HpPerfectLow;
  static const IndividualValues  HpPerfectHigh;
  static const IndividualValues  HpPerfectTrickLow;
  static const IndividualValues  HpPerfectTrickHigh;
  
  
  IndividualValues() : word(0) {}
  
  // this does not check value ranges - assumes user passes valid IV values
  IndividualValues(uint32_t hp, uint32_t at, uint32_t df,
                   uint32_t sa, uint32_t sd, uint32_t sp)
    : word((hp << HP_SHIFT) | (at << AT_SHIFT) | (df << DF_SHIFT) |
           (sa << SA_SHIFT) | (sd << SD_SHIFT) | (sp << SP_SHIFT))
  {}
  
  IndividualValues(const IndividualValues &ivs) : word(ivs.word) {}
  explicit IndividualValues(uint32_t ivWord) : word(ivWord & ALL_IVS_MASK) {}
  
  IndividualValues& operator=(const IndividualValues &ivs)
  {
    word = ivs.word;
    return *this;
  }
  
  IndividualValues& operator=(uint32_t ivWord)
  {
    word = ivWord & ALL_IVS_MASK;
    return *this;
  }
  
  // don't use comparison operators because they will break the rules
  // like if operator< is false, mathematically operator>= should be true
  bool betterThan(const IndividualValues &ivs) const;
  bool betterThanOrEqual(const IndividualValues &ivs) const;
  bool worseThan(const IndividualValues &ivs) const
  { return ivs.betterThan(*this); }
  bool worseThanOrEqual(const IndividualValues &ivs) const
  { return ivs.betterThanOrEqual(*this); }
  
  // and for optional IVs
  bool betterThan(const OptionalIVs &oivs) const;
  bool betterThanOrEqual(const OptionalIVs &oivs) const;
  bool worseThan(const OptionalIVs &oivs) const;
  bool worseThanOrEqual(const OptionalIVs &oivs) const;
  
  uint32_t hp() const { return (word & HP_MASK) >> HP_SHIFT; }
  void hp(uint32_t iv)
  { word = (word & ~HP_MASK) | ((iv << HP_SHIFT) & HP_MASK); }
  
  uint32_t at() const { return (word & AT_MASK) >> AT_SHIFT; }
  void at(uint32_t iv)
  { word = (word & ~AT_MASK) | ((iv << AT_SHIFT) & AT_MASK); }
  
  uint32_t df() const { return (word & DF_MASK) >> DF_SHIFT; }
  void df(uint32_t iv)
  { word = (word & ~DF_MASK) | ((iv << DF_SHIFT) & DF_MASK); }
  
  uint32_t sa() const { return (word & SA_MASK) >> SA_SHIFT; }
  void sa(uint32_t iv)
  { word = (word & ~SA_MASK) | ((iv << SA_SHIFT) & SA_MASK); }
  
  uint32_t sd() const { return (word & SD_MASK) >> SD_SHIFT; }
  void sd(uint32_t iv)
  { word = (word & ~SD_MASK) | ((iv << SD_SHIFT) & SD_MASK); }
  
  uint32_t sp() const { return (word & SP_MASK) >> SP_SHIFT; }
  void sp(uint32_t iv)
  { word = (word & ~SP_MASK) | ((iv << SP_SHIFT) & SP_MASK); }
  
  
  class BadIVIndexException : public Exception
  {
  public:
    BadIVIndexException(int i);
  };
  
  uint32_t iv(int i) const throw (BadIVIndexException)
  {
    if ((i < HP) || (i > SP)) throw BadIVIndexException(i);
    
    return (word >> IVShift[i]) & IV_MASK;
  }
  
  void setIV(int i, uint32_t iv) throw (BadIVIndexException)
  {
    if ((i < HP) || (i > SP)) throw BadIVIndexException(i);
    
    word = (word & ~(IV_MASK << IVShift[i])) | ((iv & IV_MASK) << IVShift[i]);
  }
  
  uint32_t Sum() const { return hp() + at() + df() + sa() + sd() + sp(); }
  
  void ShiftDownNormal(uint32_t iv)
  {
    word = ((word & (AT_MASK | DF_MASK)) >> (AT_SHIFT - HP_SHIFT)) |
           ((word & SA_MASK) >> (SA_SHIFT - DF_SHIFT)) |
           ((word & SD_MASK) >> (SD_SHIFT - SA_SHIFT)) |
           ((word & SP_MASK) << (SD_SHIFT - SP_SHIFT)) |
           ((iv & IV_MASK) << SP_SHIFT);
  }
  
  void ShiftUpNormal(uint32_t iv)
  {
    word = ((iv & IV_MASK) << HP_SHIFT) |
           ((word & (HP_MASK | AT_MASK)) << (AT_SHIFT - HP_SHIFT)) |
           ((word & DF_MASK) << (SA_SHIFT - DF_SHIFT)) |
           ((word & SA_MASK) << (SD_SHIFT - SA_SHIFT)) |
           ((word & SD_MASK) >> (SD_SHIFT - SP_SHIFT));
  }
  
  void ShiftDownRoamer(uint32_t iv)
  {
    word = ((word & (AT_MASK | DF_MASK)) >> (AT_SHIFT - HP_SHIFT)) |
           ((word & SD_MASK) >> (SD_SHIFT - DF_SHIFT)) |
           ((word & SP_MASK) << (SD_SHIFT - SP_SHIFT)) |
           ((word & SA_MASK) >> (SA_SHIFT - SP_SHIFT)) |
           ((iv & IV_MASK) << SA_SHIFT);
  }
  
  void ShiftUpRoamer(uint32_t iv)
  {
    word = ((iv & IV_MASK) << HP_SHIFT) |
           ((word & (HP_MASK | AT_MASK)) << (AT_SHIFT - HP_SHIFT)) |
           ((word & DF_MASK) << (SD_SHIFT - DF_SHIFT)) |
           ((word & SD_MASK) >> (SD_SHIFT - SP_SHIFT)) |
           ((word & SP_MASK) << (SA_SHIFT - SP_SHIFT));
  }
  
  Element::Type HiddenType() const;
  uint32_t      HiddenPower() const;
  
  uint32_t  word;
  
  class ImpossibleMinMaxIVRangeException : public Exception
  {
  public:
    ImpossibleMinMaxIVRangeException
      (IndividualValues minIVs, IndividualValues maxIVs);
  };
  
  static uint32_t CalculateNumberOfCombinations
    (IndividualValues minIVs, IndividualValues maxIVs)
    throw (ImpossibleMinMaxIVRangeException)
  {
    if (!minIVs.worseThanOrEqual(maxIVs))
      throw ImpossibleMinMaxIVRangeException(minIVs, maxIVs);
  
    return (maxIVs.hp() - minIVs.hp() + 1) *
           (maxIVs.at() - minIVs.at() + 1) *
           (maxIVs.df() - minIVs.df() + 1) *
           (maxIVs.sa() - minIVs.sa() + 1) *
           (maxIVs.sd() - minIVs.sd() + 1) *
           (maxIVs.sp() - minIVs.sp() + 1);
  }
  
  class ImpossibleHiddenTypeException : public Exception
  {
  public:
    ImpossibleHiddenTypeException
      (IndividualValues minIVs, IndividualValues maxIVs, Element::Type t);
  };
  
  class ImpossibleMinHiddenPowerException : public Exception
  {
  public:
    ImpossibleMinHiddenPowerException
      (IndividualValues minIVs, IndividualValues maxIVs, uint32_t minPower);
  };
  
  static uint64_t AdjustExpectedResultsForHiddenPower
    (uint64_t numResults, IndividualValues minIVs, IndividualValues maxIVs,
     Element::Type type, uint32_t minPower)
    throw (ImpossibleHiddenTypeException, ImpossibleMinHiddenPowerException);
};

typedef IndividualValues  IVs;


inline bool operator==(const IVs &ivs1, const IVs &ivs2)
{ return ivs1.word == ivs2.word; }
inline bool operator!=(const IVs &ivs1, const IVs &ivs2)
{ return ivs1.word != ivs2.word; }


// for breeding, when not all of a parent's IVs are set
struct OptionalIVs
{
  OptionalIVs() : values(), setIVs(0) {}
  
  // this does not check value ranges - assumes user passes valid IV values
  OptionalIVs(uint32_t hp, uint32_t at, uint32_t df,
              uint32_t sa, uint32_t sd, uint32_t sp)
    : values(hp, at, df, sa, sd, sp), setIVs(0x3f)
  {}
  
  OptionalIVs(const OptionalIVs &ivs)
    : values(ivs.values), setIVs(ivs.setIVs)
  {}
  
  OptionalIVs(const IVs &ivs) : values(ivs), setIVs(0x3f) {}
  
  OptionalIVs& operator=(const OptionalIVs &ivs)
  {
    values = ivs.values;
    setIVs = ivs.setIVs;
    return *this;
  }
  
  uint32_t hp() const { return values.hp(); }
  uint32_t at() const { return values.at(); }
  uint32_t df() const { return values.df(); }
  uint32_t sa() const { return values.sa(); }
  uint32_t sd() const { return values.sd(); }
  uint32_t sp() const { return values.sp(); }
  
  uint32_t iv(int i) const throw (IVs::BadIVIndexException)
  { return values.iv(i); }
  
  void hp(uint32_t iv)
  { values.hp(iv); setIVs |= 0x1 << IVs::HP; }
  
  void at(uint32_t iv)
  { values.at(iv); setIVs |= 0x1 << IVs::AT; }
  
  void df(uint32_t iv)
  { values.df(iv); setIVs |= 0x1 << IVs::DF; }
  
  void sa(uint32_t iv)
  { values.sa(iv); setIVs |= 0x1 << IVs::SA; }
  
  void sd(uint32_t iv)
  { values.sd(iv); setIVs |= 0x1 << IVs::SD; }
  
  void sp(uint32_t iv)
  { values.sp(iv); setIVs |= 0x1 << IVs::SP; }
  
  void setIV(int i, uint32_t iv) throw (IVs::BadIVIndexException)
  {
    values.setIV(i, iv);
    setIVs |= 0x1 << i;
  }
  
  void clearIV(int i) throw (IVs::BadIVIndexException)
  {
    values.setIV(i, 0);
    setIVs &= ~(0x1 << i);
  }
  
  bool isSet(int i) const
  { return (setIVs & (0x1 << i)) != 0; }
  
  bool allSet() const
  { return setIVs == 0x3f; }
  
  bool betterThan(const OptionalIVs &oivs) const
  {
    return allSet() && oivs.allSet() && values.betterThan(oivs.values);
  }
  
  bool betterThanOrEqual(const OptionalIVs &oivs) const
  {
    return allSet() && oivs.allSet() && values.betterThanOrEqual(oivs.values);
  }
  
  bool worseThan(const OptionalIVs &oivs) const
  {
    return allSet() && oivs.allSet() && values.worseThan(oivs.values);
  }
  
  bool worseThanOrEqual(const OptionalIVs &oivs) const
  {
    return allSet() && oivs.allSet() && values.worseThanOrEqual(oivs.values);
  }
  
  // to guarantee absolute better than or worse than, all IVs must be set
  bool betterThan(const IVs &ivs) const
  { return allSet() && values.betterThan(ivs); }
  
  bool worseThan(const IVs &ivs) const
  { return allSet() && values.worseThan(ivs); }
  
  // if equality is also allowed, unset IVs satisfy if the IV being compared
  // agaist is min / max value
  bool betterThanOrEqual(const IVs &ivs) const
  {
    if (allSet())
      return values.betterThanOrEqual(ivs);
    
    // unset IVs ok if other IV is 0 (guaranteed better than or equal)
    bool  result = true;
    for (uint32_t iv = 0; iv < 6; ++iv)
    {
      uint32_t  otherIV = ivs.iv(iv);
      
      if (isSet(iv) ? (values.iv(iv) < otherIV) : (otherIV > 0))
      {
        result = false;
        break;
      }
    }
    
    return result;
  }
  
  bool worseThanOrEqual(const IVs &ivs) const
  {
    if (allSet())
      return values.worseThanOrEqual(ivs);
    
    // unset IVs ok if other IV is 31 (guaranteed worse than or equal)
    bool  result = true;
    for (uint32_t iv = 0; iv < 6; ++iv)
    {
      uint32_t  otherIV = ivs.iv(iv);
      
      if (isSet(iv) ? (values.iv(iv) > otherIV) : (otherIV < 31))
      {
        result = false;
        break;
      }
    }
    
    return result;
  }
  
  IVs      values;
  uint8_t  setIVs;
};

// define comparisons declared in IVs
inline bool IVs::betterThan(const OptionalIVs &oivs) const
{ return oivs.worseThan(*this); }

inline bool IVs::betterThanOrEqual(const OptionalIVs &oivs) const
{ return oivs.worseThanOrEqual(*this); }

inline bool IVs::worseThan(const OptionalIVs &oivs) const
{ return oivs.betterThan(*this); }

inline bool IVs::worseThanOrEqual(const OptionalIVs &oivs) const
{ return oivs.betterThanOrEqual(*this); }


struct IVPattern
{
  enum Type
  {
    CUSTOM = 0,
    HEX_FLAWLESS,
    PHYSICAL_FLAWLESS,
    SPECIAL_FLAWLESS,
    SPECIAL_HIDDEN_POWER_FLAWLESS,
    HEX_FLAWLESS_TRICK,
    PHYSICAL_FLAWLESS_TRICK,
    SPECIAL_FLAWLESS_TRICK,
    SPECIAL_HIDDEN_POWER_FLAWLESS_TRICK
  };
  
  static Type Get(const IVs &min, const IVs &max,
                  bool considerHiddenPower, uint32_t minHiddenPower);
};


struct Characteristic
{
  enum Type
  {
    NONE = -1,
    
    // HP
    LOVES_TO_EAT = 0,        // 0 or 5
    OFTEN_DOZES_OFF,         // 1 or 6
    OFTEN_SCATTERS_THINGS,   // 2 or 7
    SCATTERS_THINGS_OFTEN,   // 3 or 8
    LIKES_TO_RELAX,          // 4 or 9
    
    // ATK
    PROUD_OF_ITS_POWER,      // 0 or 5
    LIKES_TO_THRASH_ABOUT,   // 1 or 6
    A_LITTLE_QUICK_TEMPERED, // 2 or 7
    LIKES_TO_FIGHT,          // 3 or 8
    QUICK_TEMPERED,          // 4 or 9
    
    // DEF
    STURDY_BODY,             // 0 or 5
    CAPABLE_OF_TAKING_HITS,  // 1 or 6
    HIGHLY_PERSISTENT,       // 2 or 7
    GOOD_ENDURANCE,          // 3 or 8
    GOOD_PERSEVERANCE,       // 4 or 9
    
    // SPA
    HIGHLY_CURIOUS,          // 0 or 5
    MISCHIEVOUS,             // 1 or 6
    THOROUGHLY_CUNNING,      // 2 or 7
    OFTEN_LOST_IN_THOUGHT,   // 3 or 8
    VERY_FINICKY,            // 4 or 9
    
    // SPD
    STRONG_WILLED,           // 0 or 5
    SOMEWHAT_VAIN,           // 1 or 6
    STRONGLY_DEFIANT,        // 2 or 7
    HATES_TO_LOSE,           // 3 or 8
    SOMEWHAT_STUBBORN,       // 4 or 9
    
    // SPE
    LIKES_TO_RUN,            // 0 or 5
    ALERT_TO_SOUNDS,         // 1 or 6
    IMPETUOUS_AND_SILLY,     // 2 or 7
    SOMEWHAT_OF_A_CLOWN,     // 3 or 8
    QUICK_TO_FLEE,           // 4 or 9
    
    NUM_CHARACTERISTICS,
    
    ANY = NUM_CHARACTERISTICS
  };
  
  static Type Get(PID pid, IVs ivs);
  static const std::string& ToString(Type c);
};


struct EncounterLead
{
  enum Ability
  {
    NONE = -1,
    
    OTHER = 0,
    SYNCHRONIZE,
    CUTE_CHARM,
    COMPOUND_EYES,
    SUCTION_CUPS,
    
    NUM_ABILITIES,
    
    ANY = NUM_ABILITIES
  };
};


struct EncounterSlot
{
  enum Mask
  {
    TYPE_MASK = 0xf00,
    SLOT_MASK = 0x00f,
    SLOT2_MASK = 0x0f0
  };
  
  enum Shift
  {
    SLOT2_SHIFT = 4
  };
  
  enum Type
  {
    LAND_TYPE = 0x000,
    SURF_TYPE = 0x100,
    OLD_ROD_TYPE = 0x200,
    GOOD_ROD_TYPE = 0x300,
    SUPER_ROD_TYPE = 0x400,
    DOUBLES_GRASS_SINGLE_TYPE = 0x500,
    DOUBLES_GRASS_DOUBLE_TYPE = 0x600,
    NO_TYPE = 0xF00
  };
  
  enum Value
  {
    LAND_0 = LAND_TYPE,
    LAND_1,
    LAND_2,
    LAND_3,
    LAND_4,
    LAND_5,
    LAND_6,
    LAND_7,
    LAND_8,
    LAND_9,
    LAND_10,
    LAND_11,
    SWARM,
    
    SURF_0 = SURF_TYPE,
    SURF_1,
    SURF_2,
    SURF_3,
    SURF_4,
    
    OLD_ROD_0 = OLD_ROD_TYPE,
    OLD_ROD_1,
    OLD_ROD_2,
    OLD_ROD_3,
    OLD_ROD_4,
    
    GOOD_ROD_0 = GOOD_ROD_TYPE,
    GOOD_ROD_1,
    GOOD_ROD_2,
    GOOD_ROD_3,
    GOOD_ROD_4,
    
    SUPER_ROD_0 = SUPER_ROD_TYPE,
    SUPER_ROD_1,
    SUPER_ROD_2,
    SUPER_ROD_3,
    SUPER_ROD_4,
    
    NO_SLOT = NO_TYPE
  };
  
  static Value MakeESV(Type type, uint32_t slot)
  { return Value(type | slot); }
  
  static Value MakeDoublesESV(Type type, uint32_t rightSlot, uint32_t leftSlot)
  { return Value(type | rightSlot | (leftSlot << SLOT2_SHIFT)); }
  
  static Type SlotType(Value esv)
  { return Type(esv & TYPE_MASK); }
  
  static uint32_t Slot(Value esv)
  { return esv & SLOT_MASK; }
  
  static uint32_t Slot2(Value esv)
  { return (esv & SLOT2_MASK) >> SLOT2_SHIFT; }
  
  static std::string ToString(Value v);
  
  static Value Gen4Land(uint32_t percent);
  static Value Gen4Surfing(uint32_t percent);
  static Value Gen4OldRod(uint32_t percent);
  static Value Gen4GoodRodJ(uint32_t percent);
  static Value Gen4GoodRodK(uint32_t percent);
  static Value Gen4SuperRodJ(uint32_t percent);
  static Value Gen4SuperRodK(uint32_t percent);
  
  static Value Gen5Land(uint32_t percent);
  static Value Gen5Surfing(uint32_t percent);
  static Value Gen5Fishing(uint32_t percent);
  static Value Gen5WaterSpot(uint32_t percent);
};

typedef EncounterSlot  ESV;


// type of item found in swirling dust or bridge shadows
struct EncounterItem
{
  enum Type
  {
    NONE = 0,
    
    SWIRLING_DUST_ITEM_START,
    
    SWIRLING_DUST_STONE_START = SWIRLING_DUST_ITEM_START,
    SUN_STONE = SWIRLING_DUST_STONE_START,
    MOON_STONE,
    FIRE_STONE,
    THUNDER_STONE,
    WATER_STONE,
    LEAF_STONE,
    SHINY_STONE,
    DUSK_STONE,
    DAWN_STONE,
    OVAL_STONE,
    
    SWIRLING_DUST_GEM_START,
    FIRE_GEM = SWIRLING_DUST_GEM_START,
    WATER_GEM,
    ELECTRIC_GEM,
    GRASS_GEM,
    ICE_GEM,
    FIGHTING_GEM,
    POISON_GEM,
    GROUND_GEM,
    FLYING_GEM,
    PSYCHIC_GEM,
    BUG_GEM,
    ROCK_GEM,
    GHOST_GEM,
    DRAGON_GEM,
    DARK_GEM,
    STEEL_GEM,
    NORMAL_GEM,
    
    SWIRLING_DUST_RARE_ITEM_START,
    EVERSTONE = SWIRLING_DUST_RARE_ITEM_START,
    KINGS_ROCK,
    
    NUM_DUST_ITEMS,
    
    
    BRIDGE_SHADOW_ITEM_START = NUM_DUST_ITEMS,
    
    HEALTH_WING = BRIDGE_SHADOW_ITEM_START,
    MUSCLE_WING,
    RESIST_WING,
    GENIUS_WING,
    CLEVER_WING,
    SWIFT_WING,
    PRETTY_WING,
    
    NUM_BRIDGE_SHADOW_ITEMS = PRETTY_WING - BRIDGE_SHADOW_ITEM_START + 1
  };
  
  static std::string ToString(Type t);
};

// type of item a wild pokemon might hold
struct HeldItem
{
  enum Type
  {
    NO_ITEM = 0,
    FIFTY_PERCENT_ITEM,
    FIVE_PERCENT_ITEM,
    ONE_PERCENT_ITEM
  };
};


// special parents when breeding
struct FemaleParent
{
  enum Type
  {
    OTHER = -1,
    NIDORAN_FEMALE = 0,
    ILLUMISE
  };
};

// special eggs when breeding with special mothers (see above)
struct EggSpecies
{
  enum Type
  {
    OTHER = -1,
    
    NIDORAN_F,
    NIDORAN_M,
    VOLBEAT,
    ILLUMISE,
    
    ANY
  };
};


struct WonderCardShininess
{
  enum Type
  {
    NEVER_SHINY = 0,
    MAY_BE_SHINY,
    ALWAYS_SHINY
  };
};


struct CoinFlips
{
  uint32_t  word;
  
  enum Result
  {
    HEADS = 1,
    TAILS = 0
  };
  
  static Result CalcResult(uint32_t rawRNG)
  { return Result(rawRNG & 0x1); }
  
  enum
  {
    FLIP_COUNT_SHIFT = 28,
    FLIP_BITS_MASK = 0x0FFFFFFF
  };
  
  CoinFlips() : word(0) {}
  CoinFlips(uint32_t seed, uint32_t numFlips);
  explicit CoinFlips(uint32_t w) : word(w) {}
  
  uint32_t NumFlips() const
  { return (word >> FLIP_COUNT_SHIFT); }
  
  Result FlipResult(uint32_t flipNum) const
  { return Result((word >> flipNum) & 0x1); }
  
  void AddFlipResult(Result r)
  {
    word = ((NumFlips() + 1) << FLIP_COUNT_SHIFT) |
           (word & FLIP_BITS_MASK) | (r << NumFlips());
  }
  
  void RemoveFlipResult()
  {
    word = ((NumFlips() - 1) << FLIP_COUNT_SHIFT) |
           (word & ((0x1 << (NumFlips() - 1)) - 1));
  }
  
  bool Contains(const CoinFlips &subFlips) const
  {
    uint32_t  searchMask = (0x1 << subFlips.NumFlips()) - 1;
    uint32_t  searchValue = subFlips.word & searchMask;
    
    return (subFlips.NumFlips() <= NumFlips()) &&
      ((word & searchMask) == searchValue);
  }
};

struct ProfElmResponses
{
  uint64_t  word;
  
  enum Response
  {
    EVOLUTION = 0,
    KANTO = 1,
    POKERUS = 2
  };
  
  static Response CalcResponse(uint32_t rawRNG)
  { return Response((rawRNG >> 16) % 3); }
  
  enum
  {
    RESPONSE_COUNT_SHIFT = 56,
    RESPONSE_BITS_MASK = 0x00FFFFFFFFFFFFFFULL
  };
  
  ProfElmResponses() : word(0) {}
  ProfElmResponses(uint32_t seed, uint32_t numResponses);
  explicit ProfElmResponses(uint64_t w) : word(w) {}
  
  uint32_t NumResponses() const
  { return (word >> RESPONSE_COUNT_SHIFT); }
  
  Response GetResponse(uint32_t responseNum) const
  { return Response((word >> (responseNum << 1)) & 0x3); }
  
  void AddResponse(Response r)
  {
    uint64_t  numResponses = NumResponses();
    
    word = ((numResponses + 1) << RESPONSE_COUNT_SHIFT) |
           (word & RESPONSE_BITS_MASK) | (uint64_t(r) << (numResponses << 1));
  }
  
  void RemoveResponse()
  {
    uint64_t  numResponses = NumResponses();
    word = ((numResponses - 1) << RESPONSE_COUNT_SHIFT) |
           (word & ((0x1ULL << ((numResponses - 1) << 1)) - 1));
  }
  
  bool Contains(const ProfElmResponses &subResponses) const
  {
    uint64_t  numResponses = subResponses.NumResponses();
    uint64_t  searchMask = (0x1ULL << (numResponses << 1)) - 1;
    uint64_t  searchValue = subResponses.word & searchMask;
    
    return (numResponses <= NumResponses()) &&
      ((word & searchMask) == searchValue);
  }
};


struct HGSSRoamers
{
  uint32_t  word;
  
  enum Roamer
  {
    RAIKOU = 0,
    ENTEI,
    LATI
  };
  
  enum
  {
    LOCATION_BITS = 6,
    LOCATION_MASK = (0x1 << LOCATION_BITS) - 1,
    CONSUMED_FRAMES_SHIFT = LOCATION_BITS * 3
  };
  
  HGSSRoamers() : word(0) {}
  HGSSRoamers(uint32_t seed, uint32_t raikouLocation, uint32_t enteiLocation,
              uint32_t latiLocation);
  explicit HGSSRoamers(uint32_t w) : word(w) {}
  
  uint32_t ConsumedFrames() const
  { return word >> CONSUMED_FRAMES_SHIFT; }
  
  uint32_t Location(Roamer r) const
  { return (word >> (LOCATION_BITS * r)) & LOCATION_MASK; }
};


struct Chatot
{

static uint32_t Gen4Pitch(uint32_t rawRNGValue)
{
  return (((rawRNGValue >> 16) & 0x1FFF) * 100) >> 13;
}

static uint32_t Gen5Pitch(uint64_t rawRNGValue)
{
  return (((rawRNGValue >> 32) * 0x1FFF) >> 32) / 82;
}

};


class HashedSeed;

struct SpinnerPositions
{
  uint64_t  word;
  
  enum Position
  {
    UP = 0,
    UP_RIGHT,
    RIGHT,
    DOWN_RIGHT,
    DOWN,
    DOWN_LEFT,
    LEFT,
    UP_LEFT
  };
  
  static Position CalcPosition(uint64_t rawRNG)
  { return Position(rawRNG >> 61); }
  
  enum
  {
    SPIN_COUNT_SHIFT = 57,
    SPIN_BITS_MASK = 0x01FFFFFFFFFFFFFFULL,
    MAX_SPINS = 19
  };
  
  SpinnerPositions() : word(0) {}
  SpinnerPositions(uint64_t seed, uint32_t numSpins);
  explicit SpinnerPositions(uint64_t w) : word(w) {}
  SpinnerPositions(const HashedSeed &seed, bool memoryLinkUsed,
                   uint32_t numSpins = MAX_SPINS);
  
  uint32_t NumSpins() const
  { return (word >> SPIN_COUNT_SHIFT); }
  
  Position GetPosition(uint32_t positionNum) const
  { return Position((word >> (positionNum * 3)) & 0x7); }
  
  void AddSpin(Position p)
  {
    uint64_t  numSpins = NumSpins();
    
    word = ((numSpins + 1) << SPIN_COUNT_SHIFT) |
           (word & SPIN_BITS_MASK) | (uint64_t(p) << (numSpins * 3));
  }
  
  void RemoveSpin()
  {
    uint64_t  numSpins = NumSpins();
    word = ((numSpins - 1) << SPIN_COUNT_SHIFT) |
           (word & ((0x1ULL << ((numSpins - 1) * 3)) - 1));
  }
  
  bool Contains(const SpinnerPositions &subSpins) const
  {
    uint64_t  numSpins = subSpins.NumSpins();
    uint64_t  searchMask = (0x1ULL << (numSpins * 3)) - 1;
    uint64_t  searchValue = subSpins.word & searchMask;
    
    return (numSpins <= NumSpins()) && ((word & searchMask) == searchValue);
  }
};


// design blatantly stolen from RNG Reporter, with adjustment
class CGearFrameTime
{
public:
  CGearFrameTime(uint32_t startOffset)
    : m_state(StartState), m_totalTicks(startOffset), m_currentTicks(0)
  {}
  
  uint32_t GetTicks() { return m_currentTicks; }
  
  void AdvanceFrame(uint64_t rawRNGValue)
  {
    switch (m_state)
    {
    case StartState:
      if (m_totalTicks > 0)
      {
        --m_totalTicks;
      }
      else
      {
        m_currentTicks = m_totalTicks = 21;
        m_state = SkippedState;
      }
      break;
      
    case SkippedState:
    default:
      m_currentTicks = 0;
      m_state = LongState;
      break;
      
    case LongState:
      m_currentTicks = m_totalTicks += (((rawRNGValue >> 32) * 152) >> 32) + 60;
      m_state = ShortState;
      break;
      
    case ShortState:
      m_currentTicks = m_totalTicks += (((rawRNGValue >> 32) * 40) >> 32) + 60;
      m_state = SkippedState;
      break;
    }
  }
  
private:
  enum State
  {
    StartState = 0,
    SkippedState,
    LongState,
    ShortState
  };
  
  State     m_state;
  uint32_t  m_totalTicks;
  uint32_t  m_currentTicks;
};


}

#endif
