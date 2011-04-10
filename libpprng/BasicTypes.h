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
    Phat,
    Lite,
    i,
    i_XL,
    _3DS,
    
    UnknownDSType
  };
  
  static std::string ToString(Type v);
};


struct Button
{
  enum Binary
  {
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
  
  static std::string ToString(uint32_t keys);
  
  static const List& SingleButtons();
  static const List& TwoButtonCombos();
  static const List& ThreeButtonCombos();
};

struct Game
{
  enum Version
  {
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
    
    BlackStart = FifthGenStart,
    
    BlackEnglish = BlackStart,
    BlackFrench,
    BlackGerman,
    BlackItalian,
    BlackJapanese,
    BlackSpanish,
    
    BlackEnd = BlackSpanish + 1,
    
    WhiteStart = BlackEnd,
    
    WhiteEnglish = WhiteStart,
    WhiteFrench,
    WhiteGerman,
    WhiteItalian,
    WhiteJapanese,
    WhiteSpanish,
    
    WhiteEnd = WhiteSpanish + 1,
    
    FifthGenEnd = WhiteEnd,
    
    UnknownVersion = FifthGenEnd,
    
    NumVersions = FifthGenEnd
  };
  
  static std::string ToString(Version v);
};


struct Nature
{
  enum Type
  {
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
    
    // returned from FromString if the string given is not a nature
    UNKNOWN
  };
  
  static const std::string& ToString(Type t);
  static Type FromString(const std::string &name);
};


struct PersonalityValue
{
  PersonalityValue() : word(0) {}
  
  PersonalityValue(const PersonalityValue &pid) : word(pid.word) {}
  PersonalityValue(uint32_t pidWord) : word(pidWord) {}
  
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
  std::string GenderString() const;
  
  uint32_t Gen34Ability() const { return word & 0x1; }
  uint32_t Gen5Ability() const { return (word >> 16) & 0x1; }
  
  Nature::Type Gen34Nature() const
  { return static_cast<Nature::Type>(word % 25); }
  
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
    ANY = NUM_TYPES,
    
    // returned from FromString if the string given is not an elemental type
    UNKNOWN
  };
  
  static const std::string& ToString(Type t);
  static Type FromString(const std::string &name);
};


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
    HP_MASK = 0x1f << HP_SHIFT,
    AT_MASK = 0x1f << AT_SHIFT,
    DF_MASK = 0x1f << DF_SHIFT,
    SA_MASK = 0x1f << SA_SHIFT,
    SD_MASK = 0x1f << SD_SHIFT,
    SP_MASK = 0x1f << SP_SHIFT
  };
  
  static const Mask  IVMask[NUM_IVS];
  
  IndividualValues() : word(0) {}
  
  IndividualValues(const IndividualValues &ivs) : word(ivs.word) {}
  IndividualValues(uint32_t ivWord) : word(ivWord & 0x7fff7fff) {}
  
  IndividualValues& operator=(const IndividualValues &ivs)
  {
    word = ivs.word;
    return *this;
  }
  
  IndividualValues& operator=(uint32_t ivWord)
  {
    word = ivWord & 0x7fff7fff;
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
  
  uint32_t hp() const { return (word >> HP_SHIFT) & 0x1f; }
  void hp(uint32_t iv)
  { word = (word & ~HP_MASK) | ((iv & 0x1f) << HP_SHIFT); }
  
  uint32_t at() const { return (word >> AT_SHIFT) & 0x1f; }
  void at(uint32_t iv)
  { word = (word & ~AT_MASK) | ((iv & 0x1f) << AT_SHIFT); }
  
  uint32_t df() const { return (word >> DF_SHIFT) & 0x1f; }
  void df(uint32_t iv)
  { word = (word & ~DF_MASK) | ((iv & 0x1f) << DF_SHIFT); }
  
  uint32_t sa() const { return (word >> SA_SHIFT) & 0x1f; }
  void sa(uint32_t iv)
  { word = (word & ~SA_MASK) | ((iv & 0x1f) << SA_SHIFT); }
  
  uint32_t sd() const { return (word >> SD_SHIFT) & 0x1f; }
  void sd(uint32_t iv)
  { word = (word & ~SD_MASK) | ((iv & 0x1f) << SD_SHIFT); }
  
  uint32_t sp() const { return (word >> SP_SHIFT) & 0x1f; }
  void sp(uint32_t iv)
  { word = (word & ~SP_MASK) | ((iv & 0x1f) << SP_SHIFT); }
  
  
  class BadIVIndexException : public Exception
  {
  public:
    BadIVIndexException(int i);
  };
  
  uint32_t iv(int i) const throw (BadIVIndexException)
  {
    if ((i < HP) || (i > SP)) throw BadIVIndexException(i);
    
    return (word >> IVShift[i]) & 0x1f;
  }
  
  void setIV(int i, uint32_t iv) throw (BadIVIndexException)
  {
    if ((i < HP) || (i > SP)) throw BadIVIndexException(i);
    
    word = (word & ~(0x1f << IVShift[i])) | ((iv & 0x1f) << IVShift[i]);
  }
  
  uint32_t Sum() const { return hp() + at() + df() + sa() + sd() + sp(); }
  
  void ShiftDown(uint32_t iv)
  {
    word = ((word & AT_MASK) >> (AT_SHIFT - HP_SHIFT)) |
           ((word & DF_MASK) >> (DF_SHIFT - AT_SHIFT)) |
           ((word & SA_MASK) >> (SA_SHIFT - DF_SHIFT)) |
           ((word & SD_MASK) >> (SD_SHIFT - SA_SHIFT)) |
           ((word & SP_MASK) << (SD_SHIFT - SP_SHIFT)) |
           ((iv & 0x1f) << SP_SHIFT);
  }
  
  void ShiftUp(uint32_t iv)
  {
    word = ((iv & 0x1f) << HP_SHIFT) |
           ((word & HP_MASK) << (AT_SHIFT - HP_SHIFT)) |
           ((word & AT_MASK) << (DF_SHIFT - AT_SHIFT)) |
           ((word & DF_MASK) << (SA_SHIFT - DF_SHIFT)) |
           ((word & SA_MASK) << (SD_SHIFT - SA_SHIFT)) |
           ((word & SD_MASK) >> (SD_SHIFT - SP_SHIFT));
  }
  
  Element::Type HiddenType() const;
  uint32_t      HiddenPower() const;
  
  uint32_t  word;
};

typedef IndividualValues  IVs;


inline bool operator==(const IVs &ivs1, const IVs &ivs2)
{ return ivs1.word == ivs2.word; }
inline bool operator!=(const IVs &ivs1, const IVs &ivs2)
{ return ivs1.word != ivs2.word; }


}

#endif
