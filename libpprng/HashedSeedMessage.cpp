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


#include "HashedSeedMessage.h"
#include "LinearCongruentialRNG.h"


using namespace boost::gregorian;

namespace pprng
{

namespace
{

struct Digest
{
  uint32_t  h[5];
};


#define K0  0x5A827999
#define K1  0x6ED9EBA1
#define K2  0x8F1BBCDC
#define K3  0xCA62C1D6

#define H0  0x67452301
#define H1  0xEFCDAB89
#define H2  0x98BADCFE
#define H3  0x10325476
#define H4  0xC3D2E1F0

#define CalcW(I) \
  temp = w[(I - 3) & 0xf] ^ w[(I - 8) & 0xf] ^ w[(I - 14) & 0xf] ^ w[(I - 16) & 0xf]; \
  w[I & 0xf] = temp = (temp << 1) | (temp >> 31)


#define Section1Calc() \
  temp = ((a << 5) | (a >> 27)) + ((b & c) | (~b & d)) + e + K0 + temp

#define Section2Calc() \
  temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + K1 + temp

#define Section3Calc() \
  temp = ((a << 5) | (a >> 27)) + ((b & c) | ((b | c) & d)) + e + K2 + temp

#define Section4Calc() \
  temp = ((a << 5) | (a >> 27)) + (b ^ c ^ d) + e + K3 + temp

#define UpdateVars() \
  e = d; \
  d = c; \
  c = (b << 30) | (b >> 2); \
  b = a; \
  a = temp

Digest SHA1(const uint32_t *message)
{
  uint32_t  w[16], temp;
  
  uint32_t  a = H0;
  uint32_t  b = H1;
  uint32_t  c = H2;
  uint32_t  d = H3;
  uint32_t  e = H4;
  
  // Section 1: 0-19
  w[0] = temp = message[0]; Section1Calc(); UpdateVars();
  w[1] = temp = message[1]; Section1Calc(); UpdateVars();
  w[2] = temp = message[2]; Section1Calc(); UpdateVars();
  w[3] = temp = message[3]; Section1Calc(); UpdateVars();
  w[4] = temp = message[4]; Section1Calc(); UpdateVars();
  w[5] = temp = message[5]; Section1Calc(); UpdateVars();
  w[6] = temp = message[6]; Section1Calc(); UpdateVars();
  w[7] = temp = message[7]; Section1Calc(); UpdateVars();
  w[8] = temp = message[8]; Section1Calc(); UpdateVars();
  w[9] = temp = message[9]; Section1Calc(); UpdateVars();
  w[10] = temp = message[10]; Section1Calc(); UpdateVars();
  w[11] = temp = message[11]; Section1Calc(); UpdateVars();
  w[12] = temp = message[12]; Section1Calc(); UpdateVars();
  w[13] = temp = message[13]; Section1Calc(); UpdateVars();
  w[14] = temp = message[14]; Section1Calc(); UpdateVars();
  w[15] = temp = message[15]; Section1Calc(); UpdateVars();
  
  CalcW(16); Section1Calc(); UpdateVars();
  CalcW(17); Section1Calc(); UpdateVars();
  CalcW(18); Section1Calc(); UpdateVars();
  CalcW(19); Section1Calc(); UpdateVars();
  
  // Section 2: 20 - 39
  CalcW(20); Section2Calc(); UpdateVars();
  CalcW(21); Section2Calc(); UpdateVars();
  CalcW(22); Section2Calc(); UpdateVars();
  CalcW(23); Section2Calc(); UpdateVars();
  CalcW(24); Section2Calc(); UpdateVars();
  CalcW(25); Section2Calc(); UpdateVars();
  CalcW(26); Section2Calc(); UpdateVars();
  CalcW(27); Section2Calc(); UpdateVars();
  CalcW(28); Section2Calc(); UpdateVars();
  CalcW(29); Section2Calc(); UpdateVars();
  CalcW(30); Section2Calc(); UpdateVars();
  CalcW(31); Section2Calc(); UpdateVars();
  CalcW(32); Section2Calc(); UpdateVars();
  CalcW(33); Section2Calc(); UpdateVars();
  CalcW(34); Section2Calc(); UpdateVars();
  CalcW(35); Section2Calc(); UpdateVars();
  CalcW(36); Section2Calc(); UpdateVars();
  CalcW(37); Section2Calc(); UpdateVars();
  CalcW(38); Section2Calc(); UpdateVars();
  CalcW(39); Section2Calc(); UpdateVars();
  
  // Section 3: 40 - 59
  CalcW(40); Section3Calc(); UpdateVars();
  CalcW(41); Section3Calc(); UpdateVars();
  CalcW(42); Section3Calc(); UpdateVars();
  CalcW(43); Section3Calc(); UpdateVars();
  CalcW(44); Section3Calc(); UpdateVars();
  CalcW(45); Section3Calc(); UpdateVars();
  CalcW(46); Section3Calc(); UpdateVars();
  CalcW(47); Section3Calc(); UpdateVars();
  CalcW(48); Section3Calc(); UpdateVars();
  CalcW(49); Section3Calc(); UpdateVars();
  CalcW(50); Section3Calc(); UpdateVars();
  CalcW(51); Section3Calc(); UpdateVars();
  CalcW(52); Section3Calc(); UpdateVars();
  CalcW(53); Section3Calc(); UpdateVars();
  CalcW(54); Section3Calc(); UpdateVars();
  CalcW(55); Section3Calc(); UpdateVars();
  CalcW(56); Section3Calc(); UpdateVars();
  CalcW(57); Section3Calc(); UpdateVars();
  CalcW(58); Section3Calc(); UpdateVars();
  CalcW(59); Section3Calc(); UpdateVars();
  
  // Section 3: 60 - 79
  CalcW(60); Section4Calc(); UpdateVars();
  CalcW(61); Section4Calc(); UpdateVars();
  CalcW(62); Section4Calc(); UpdateVars();
  CalcW(63); Section4Calc(); UpdateVars();
  CalcW(64); Section4Calc(); UpdateVars();
  CalcW(65); Section4Calc(); UpdateVars();
  CalcW(66); Section4Calc(); UpdateVars();
  CalcW(67); Section4Calc(); UpdateVars();
  CalcW(68); Section4Calc(); UpdateVars();
  CalcW(69); Section4Calc(); UpdateVars();
  CalcW(70); Section4Calc(); UpdateVars();
  CalcW(71); Section4Calc(); UpdateVars();
  CalcW(72); Section4Calc(); UpdateVars();
  CalcW(73); Section4Calc(); UpdateVars();
  CalcW(74); Section4Calc(); UpdateVars();
  CalcW(75); Section4Calc(); UpdateVars();
  CalcW(76); Section4Calc(); UpdateVars();
  CalcW(77); Section4Calc(); UpdateVars();
  CalcW(78); Section4Calc(); UpdateVars();
  CalcW(79); Section4Calc(); UpdateVars();
  
  Digest  result;
  
  result.h[0] = H0 + a;
  result.h[1] = H1 + b;
  result.h[2] = H2 + c;
  result.h[3] = H3 + d;
  result.h[4] = H4 + e;
  
  return result;
}


uint32_t SwapEndianess(uint32_t value)
{
  value = ((value << 8) & 0xFF00FF00) | ((value >> 8) & 0xFF00FF);
  return (value << 16) | (value >> 16);
}

uint32_t ToBCD(uint32_t value)
{
  uint32_t  thousands = value / 1000;
  uint32_t  allHundreds = value / 100;
  uint32_t  allTens = value / 10;
  
  uint32_t  hundreds = allHundreds - (thousands * 10);
  uint32_t  tens = allTens - (allHundreds * 10);
  
  return (thousands << 12) |
         (hundreds << 8) |
         (tens << 4) |
         (value - (allTens * 10));
}


enum Nazo
{
  // Black / White
  JPBlackNazo = 0x02215f10,
  JPWhiteNazo = 0x02215f30,
  JPBlackDSiNazo = 0x02761150,
  JPWhiteDSiNazo = 0x02761150,
  
  ENBlackNazo = 0x022160B0,
  ENWhiteNazo = 0x022160D0,
  ENBlackDSiNazo = 0x02760190,
  ENWhiteDSiNazo = 0x027601B0,
  
  SPBlackNazo = 0x02216050,
  SPWhiteNazo = 0x02216070,
  SPBlackDSiNazo = 0x027601f0,
  SPWhiteDSiNazo = 0x027601f0,
  
  FRBlackNazo = 0x02216030,
  FRWhiteNazo = 0x02216050,
  FRBlackDSiNazo = 0x02760230,
  FRWhiteDSiNazo = 0x02760250,
  
  DEBlackNazo = 0x02215FF0,
  DEWhiteNazo = 0x02216010,
  DEBlackDSiNazo = 0x027602f0,
  DEWhiteDSiNazo = 0x027602f0,
  
  ITBlackNazo = 0x02215FB0,
  ITWhiteNazo = 0x02215FD0,
  ITBlackDSiNazo = 0x027601d0,
  ITWhiteDSiNazo = 0x027601d0,
  
  KRBlackNazo = 0x022167B0,
  KRWhiteNazo = 0x022167B0,
  KRBlackDSiNazo = 0x02761150,
  KRWhiteDSiNazo = 0x02761150,
  
  
  // Black2 / White2
  JPBlack2Nazo0 = 0x0209A8DC,
  JPBlack2Nazo1 = 0x02039AC9,
  JPBlack2Nazo2DS = 0x021FF9B0,
  JPBlack2Nazo2DSi = 0x027AA730,
  
  JPWhite2Nazo0 = 0x0209A8FC,
  JPWhite2Nazo1 = 0x02039AF5,
  JPWhite2Nazo2DS = 0x021FF9D0,
  JPWhite2Nazo2DSi = 0x027AA5F0,
  
  ENBlack2Nazo0 = 0x0209AEE8,
  ENBlack2Nazo1 = 0x02039DE9,
  ENBlack2Nazo2DS = 0x02200010,
  ENBlack2Nazo2DSi = 0x027A5F70,
  
  ENWhite2Nazo0 = 0x0209AF28,
  ENWhite2Nazo1 = 0x02039E15,
  ENWhite2Nazo2DS = 0x02200050,
  ENWhite2Nazo2DSi = 0x027A5E90
};

static Nazo NazoForVersionAndDS(Game::Version version, DS::Type dsType)
{
  bool isPlainDS = (dsType == DS::DSPhat) || (dsType == DS::DSLite);
  
  switch (version)
  {
    case Game::BlackJapanese:
      return isPlainDS ? JPBlackNazo : JPBlackDSiNazo;
      break;
    
    case Game::WhiteJapanese:
      return isPlainDS ? JPWhiteNazo : JPWhiteDSiNazo;
      break;
    
    case Game::BlackEnglish:
      return isPlainDS ? ENBlackNazo : ENBlackDSiNazo;
      break;
    
    case Game::WhiteEnglish:
      return isPlainDS ? ENWhiteNazo : ENWhiteDSiNazo;
      break;
    
    case Game::BlackSpanish:
      return isPlainDS ? SPBlackNazo : SPBlackDSiNazo;
      break;
    
    case Game::WhiteSpanish:
      return isPlainDS ? SPWhiteNazo : SPWhiteDSiNazo;
      break;
    
    case Game::BlackFrench:
      return isPlainDS ? FRBlackNazo : FRBlackDSiNazo;
      break;
      
    case Game::WhiteFrench:
      return isPlainDS ? FRWhiteNazo : FRWhiteDSiNazo;
      break;
      
    case Game::BlackItalian:
      return isPlainDS ? ITBlackNazo : ITBlackDSiNazo;
      break;
      
    case Game::WhiteItalian:
      return isPlainDS ? ITWhiteNazo : ITWhiteDSiNazo;
      break;
      
    case Game::BlackGerman:
      return isPlainDS ? DEBlackNazo : DEBlackDSiNazo;
      break;
      
    case Game::WhiteGerman:
      return isPlainDS ? DEWhiteNazo : DEWhiteDSiNazo;
      break;
      
    case Game::BlackKorean:
      return isPlainDS ? KRBlackNazo : KRBlackDSiNazo;
      break;
      
    case Game::WhiteKorean:
      return isPlainDS ? KRWhiteNazo : KRWhiteDSiNazo;
      break;
      
    case Game::Black2Japanese:
      return isPlainDS ? JPBlack2Nazo2DS : JPBlack2Nazo2DSi;
      break;
    
    case Game::White2Japanese:
      return isPlainDS ? JPWhite2Nazo2DS : JPWhite2Nazo2DSi;
      break;
      
    case Game::Black2English:
      return isPlainDS ? ENBlack2Nazo2DS : ENBlack2Nazo2DSi;
      break;
    
    case Game::White2English:
      return isPlainDS ? ENWhite2Nazo2DS : ENWhite2Nazo2DSi;
      break;
      
    default:
      return static_cast<Nazo>(0);
      break;
  }
}

static void SetBlack2White2FirstNazos(uint32_t message[], Game::Version version)
{
  switch (version)
  {
    case Game::Black2Japanese:
      message[0] = SwapEndianess(JPBlack2Nazo0);
      message[1] = SwapEndianess(JPBlack2Nazo1);
      break;
    
    case Game::White2Japanese:
      message[0] = SwapEndianess(JPWhite2Nazo0);
      message[1] = SwapEndianess(JPWhite2Nazo1);
      break;
      
    case Game::Black2English:
      message[0] = SwapEndianess(ENBlack2Nazo0);
      message[1] = SwapEndianess(ENBlack2Nazo1);
      break;
    
    case Game::White2English:
      message[0] = SwapEndianess(ENWhite2Nazo0);
      message[1] = SwapEndianess(ENWhite2Nazo1);
      break;
      
    default:
      message[0] = 0;
      message[1] = 0;
      break;
  }
}

enum
{
  BWFirstNazoOffset = 0xFC,
  BWSecondNazoOffset = BWFirstNazoOffset + 0x4C,
  
  BW2NazoOffset = 0x54,
  
  ButtonMask = 0x2FFF
};

static void SetNazos(uint32_t message[], Game::Version version, DS::Type dsType)
{
  Nazo  nazo = NazoForVersionAndDS(version, dsType);
  
  if (Game::IsBlack2White2(version))
  {
    SetBlack2White2FirstNazos(message, version);
    message[2] = SwapEndianess(nazo);
    message[3] = message[4] = SwapEndianess(nazo + BW2NazoOffset);
  }
  else
  {
    message[0] = SwapEndianess(nazo);
    message[1] = message[2] = SwapEndianess(nazo + BWFirstNazoOffset);
    message[3] = message[4] = SwapEndianess(nazo + BWSecondNazoOffset);
  }
}

void MakeMessage(uint32_t message[], const HashedSeed::Parameters &parameters)
{
  SetNazos(message, parameters.version, parameters.dsType);
  
  message[5] = SwapEndianess((parameters.vcount << 16) | parameters.timer0);
  
  message[6] = parameters.macAddress.low & 0xffff;
  
  message[7] = (((parameters.macAddress.low >> 16) & 0xff) |
                (parameters.macAddress.high << 8)) ^
               SwapEndianess(parameters.gxStat ^ parameters.vframe);
  
  message[8] = ((ToBCD(parameters.date.year()) & 0xff) << 24) |
               ((ToBCD(parameters.date.month()) & 0xff) << 16) |
               ((ToBCD(parameters.date.day()) & 0xff) << 8) |
               (parameters.date.day_of_week() & 0xff);
  
  message[9] = (((ToBCD(parameters.hour) +
                (((parameters.hour >= 12) && (parameters.dsType != DS::_3DS)) ?
                 0x40 : 0)) & 0xff) << 24) |
               ((ToBCD(parameters.minute) & 0xff) << 16) |
               ((ToBCD(parameters.second) & 0xff) << 8);
  
  message[10] = 0;
  message[11] = 0;
  
  message[12] = SwapEndianess(parameters.heldButtons ^ ButtonMask);
  
  message[13] = 0x80000000;
  message[14] = 0x00000000;
  message[15] = 0x000001A0; // 416
}

uint64_t CalcRawSeed(const uint32_t *message)
{
  Digest d = SHA1(message);
  
  uint64_t  preSeed = SwapEndianess(d.h[1]);
  preSeed = (preSeed << 32) | SwapEndianess(d.h[0]);
  
  return LCRNG5(preSeed).Next();
}

}


HashedSeedMessage::HashedSeedMessage(const HashedSeed::Parameters &parameters)
  : m_parameters(parameters), m_message(),
    m_monthDays(parameters.date.end_of_month().day()),
    m_rawSeedCalculated(false)
{
  MakeMessage(m_message, parameters);
}

HashedSeed HashedSeedMessage::AsHashedSeed() const
{
  return HashedSeed(m_parameters, GetRawSeed());
}

uint64_t HashedSeedMessage::GetRawSeed() const
{
  if (!m_rawSeedCalculated)
  {
    m_rawSeed = CalcRawSeed(m_message);
    m_rawSeedCalculated = true;
  }
  
  return m_rawSeed;
}

void HashedSeedMessage::SetMACAddress(const MACAddress &macAddress)
{
  m_message[6] = macAddress.low & 0xffff;
  m_message[7] = (m_message[7] ^
                  (((m_parameters.macAddress.low >> 16) & 0xff) |
                   (m_parameters.macAddress.high << 8))) ^
                 (((macAddress.low >> 16) & 0xff) | (macAddress.high << 8));
  
  m_parameters.macAddress = macAddress;
  m_rawSeedCalculated = false;
}

void HashedSeedMessage::SetGxStat(HashedSeed::GxStat gxStat)
{
  m_message[7] = m_message[7] ^
                 SwapEndianess(m_parameters.gxStat) ^ SwapEndianess(gxStat);
  
  m_parameters.gxStat = gxStat;
  m_rawSeedCalculated = false;
}

void HashedSeedMessage::SetVCount(uint32_t vcount)
{
  m_message[5] = (m_message[5] & 0xffff0000) |
                 ((vcount & 0xff) << 8) | (vcount >> 8);
  
  m_parameters.vcount = vcount;
  m_rawSeedCalculated = false;
}

void HashedSeedMessage::SetVFrame(uint32_t vframe)
{
  m_message[7] = (m_message[7] ^ (m_parameters.vframe << 24)) ^ (vframe << 24);
  
  m_parameters.vframe = vframe;
  m_rawSeedCalculated = false;
}

void HashedSeedMessage::SetTimer0(uint32_t timer0)
{
  m_message[5] = (m_message[5] & 0xffff) |
                 ((timer0 & 0xff) << 24) | ((timer0 & 0xff00) << 8);
  
  m_parameters.timer0 = timer0;
  m_rawSeedCalculated = false;
}

void HashedSeedMessage::SetDate(boost::gregorian::date d)
{
  m_message[8] = ((ToBCD(d.year()) & 0xff) << 24) |
                 ((ToBCD(d.month()) & 0xff) << 16) |
                 ((ToBCD(d.day()) & 0xff) << 8) |
                 (d.day_of_week() & 0xff);
  
  m_monthDays = d.end_of_month().day();
  m_parameters.date = d;
  m_rawSeedCalculated = false;
}

void HashedSeedMessage::NextDay()
{
  m_parameters.date = m_parameters.date + date_duration(1);
  
  uint32_t  dayInfo = m_message[8] & 0xffff;
  uint32_t  dayOnesDigit = (dayInfo >> 8) & 0xf;
  uint32_t  dayTensDigit = dayInfo >> 12;
  uint32_t  dayInt = (dayTensDigit * 10) + dayOnesDigit;
  uint32_t  dow = dayInfo & 0xff;
  
  if (++dow > 6)
    dow = 0;
  
  if (dayInt == m_monthDays)
  {
    dayInfo = 0x0100 | dow;
    m_monthDays = m_parameters.date.end_of_month().day();
    
    uint32_t  monthInfo = (m_message[8] >> 16) & 0xff;
    
    if (monthInfo == 0x12)
    {
      monthInfo = 0x01;
      
      uint32_t  yearInfo = (m_message[8] >> 24) & 0xff;
      
      if ((++yearInfo & 0xf) > 9)
      {
        yearInfo = (yearInfo & 0xf0) + 0x10;
        if (yearInfo >= 0xa0)
          yearInfo = 0x00;
      }
      
      m_message[8] = (yearInfo << 24) | (monthInfo << 16) | dayInfo;
    }
    else
    {
      if (++monthInfo == 0xA)
      {
        monthInfo = 0x10;
      }
      
      m_message[8] = (m_message[8] & 0xff000000) | (monthInfo << 16) | dayInfo;
    }
  }
  else if (++dayOnesDigit > 9)
  {
    dayOnesDigit = 0;
    ++dayTensDigit;
    m_message[8] = (m_message[8] & 0xffff0000) |
      (dayTensDigit << 12) | (dayOnesDigit << 8) | dow;
  }
  else
  {
    m_message[8] = (m_message[8] & 0xfffff000) | (dayOnesDigit << 8) | dow;
  }
  
  m_rawSeedCalculated = false;
}

void HashedSeedMessage::SetHour(uint32_t hour)
{
  m_message[9] = (m_message[9] & 0x00ffffff) |
    ((ToBCD(hour) +
      (((hour >= 12) && (m_parameters.dsType != DS::_3DS)) ? 0x40 : 0)) << 24);
  
  m_parameters.hour = hour;
  m_rawSeedCalculated = false;
}

void HashedSeedMessage::NextHour()
{
  if (m_parameters.hour == 23)
  {
    m_parameters.hour = 0;
    m_message[9] = m_message[9] & 0x00ffffff;
    NextDay();
  }
  else if (++m_parameters.hour == 12)
  {
    m_message[9] = (m_message[9] & 0x00ffffff) | 0x12000000 |
      ((m_parameters.dsType != DS::_3DS) ? 0x40000000 : 0);
  }
  else
  {
    uint32_t  hour = (m_message[9] & 0xff000000) >> 24;
    uint32_t  onesDigit = hour & 0x0f;
    
    if (++onesDigit > 9)
    {
      onesDigit = 0;
      
      uint32_t  tensDigit = (hour & 0xF0) + 0x10;
      
      m_message[9] = (m_message[9] & 0x00ffffff) |
                     ((tensDigit | onesDigit) << 24);
    }
    else
    {
      m_message[9] = (m_message[9] & 0xf0ffffff) | (onesDigit << 24);
    }
  }
  
  m_rawSeedCalculated = false;
}

void HashedSeedMessage::SetMinute(uint32_t minute)
{
  m_message[9] = (m_message[9] & 0xff00ffff) | (ToBCD(minute) << 16);
  
  m_parameters.minute = minute;
  m_rawSeedCalculated = false;
}

void HashedSeedMessage::NextMinute()
{
  if (m_parameters.minute == 59)
  {
    m_parameters.minute = 0;
    m_message[9] = m_message[9] & 0xff00ffff;
    NextHour();
  }
  else
  {
    ++m_parameters.minute;
    uint32_t  minute = (m_message[9] & 0xff0000) >> 16;
    uint32_t  onesDigit = minute & 0x0f;
    
    if (++onesDigit > 9)
    {
      onesDigit = 0;
      
      uint32_t  tensDigit = (minute & 0xf0) + 0x10;
      
      m_message[9] = (m_message[9] & 0xff00ffff) |
                     ((tensDigit | onesDigit) << 16);
    }
    else
    {
      m_message[9] = (m_message[9] & 0xfff0ffff) | (onesDigit << 16);
    }
  }
  
  m_rawSeedCalculated = false;
}

void HashedSeedMessage::SetSecond(uint32_t second)
{
  m_message[9] = (m_message[9] & 0xffff00ff) | (ToBCD(second) << 8);
  
  m_parameters.second = second;
  m_rawSeedCalculated = false;
}

void HashedSeedMessage::NextSecond()
{
  if (m_parameters.second == 59)
  {
    m_parameters.second = 0;
    m_message[9] = m_message[9] & 0xffff00ff;
    NextMinute();
  }
  else
  {
    ++m_parameters.second;
    uint32_t  second = (m_message[9] & 0xff00) >> 8;
    uint32_t  onesDigit = second & 0x0f;
    
    if (++onesDigit > 9)
    {
      onesDigit = 0;
      
      uint32_t  tensDigit = (second & 0xf0) + 0x10;
      
      m_message[9] = (m_message[9] & 0xffff00ff) |
                     ((tensDigit | onesDigit) << 8);
    }
    else
    {
      m_message[9] = (m_message[9] & 0xfffff0ff) | (onesDigit << 8);
    }
  }
  
  m_rawSeedCalculated = false;
}

void HashedSeedMessage::SetHeldButtons(uint32_t heldButtons)
{
  m_parameters.heldButtons = heldButtons;
  
  heldButtons = heldButtons ^ ButtonMask;
  m_message[12] = ((heldButtons & 0xff) << 24) | ((heldButtons & 0xff00) << 8);
  
  m_rawSeedCalculated = false;
}

}
