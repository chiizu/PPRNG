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


#include "HashedSeed.h"
#include "LinearCongruentialRNG.h"
#include <iostream>
#include <iomanip>

namespace pprng
{

namespace
{

struct Message
{
  uint32_t  w[16];
};

struct Digest
{
  uint32_t  h[5];
};

Digest SHA1(const Message &message)
{
  uint32_t  w[80];
  
  uint32_t  h0 = 0x67452301;
  uint32_t  h1 = 0xEFCDAB89;
  uint32_t  h2 = 0x98BADCFE;
  uint32_t  h3 = 0x10325476;
  uint32_t  h4 = 0xC3D2E1F0;
  
  uint32_t  a = h0;
  uint32_t  b = h1;
  uint32_t  c = h2;
  uint32_t  d = h3;
  uint32_t  e = h4;
  
  ::memcpy(w, message.w, sizeof(message.w));
  
  uint32_t i;
  
  // extend to 80 32-bit words
  for (i = 16; i < 80; ++i)
  {
    uint32_t  temp = w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16];
    w[i] = (temp << 1) | (temp >> 31);
  }
  
  uint32_t  f, k;
  for (i = 0; i < 80; ++i)
  {
    if (i < 20)
    {
      f = (b & c) | (~b & d);
      k = 0x5A827999;
    }
    else if (i < 40)
    {
      f = b ^ c ^ d;
      k = 0x6ED9EBA1;
    }
    else if (i < 60)
    {
      f = (b & c) | (b & d) | (c & d);
      k = 0x8F1BBCDC;
    }
    else
    {
      f = b ^ c ^ d;
      k = 0xCA62C1D6;
    }
    
    uint32_t  temp = ((a << 5) | (a >> 27)) + f + e + k + w[i];
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;
  }
  
  Digest  result;
  
  result.h[0] = h0 + a;
  result.h[1] = h1 + b;
  result.h[2] = h2 + c;
  result.h[3] = h3 + d;
  result.h[4] = h4 + e;
  
  return result;
}

uint32_t SwapEndianess(uint32_t value)
{
  return ((value >> 24) & 0xff) | ((value >> 8) & 0xff00) |
    ((value << 8) & 0xff0000) | ((value << 24) & 0xff000000);
}

uint32_t ToBCD(uint32_t value)
{
  return (((value / 1000) % 10) << 12) |
         (((value / 100) % 10) << 8) |
         (((value / 10) % 10) << 4) |
         (value % 10);
}

uint64_t MakeSeed(uint32_t year, uint32_t month, uint32_t day,
                  uint32_t dayOfWeek,
                  uint32_t hour, uint32_t minute, uint32_t second,
                  uint32_t macAddressLow, uint32_t macAddressHigh,
                  uint32_t nazo, uint32_t n0, uint32_t n1, uint32_t n2,
                  uint32_t vcount, uint32_t timer, uint32_t gxStat,
                  uint32_t vframe, uint32_t keyInput)
{
  Message  m;
  
  m.w[0] = SwapEndianess(nazo);
  m.w[1] = m.w[2] = SwapEndianess(nazo + 0xfc);
  m.w[3] = m.w[4] = SwapEndianess(nazo + 0xfc + 0x4c);
  
  m.w[5] = SwapEndianess((vcount << 16) | timer);
  
  m.w[6] = (macAddressLow & 0xffff) | n2;
  
  m.w[7] = (((macAddressLow >> 16) & 0xff) | (macAddressHigh << 8)) ^
           SwapEndianess(gxStat ^ vframe);
  
  m.w[8] = ((ToBCD(year) & 0xff) << 24) | ((ToBCD(month) & 0xff) << 16) |
           ((ToBCD(day) & 0xff) << 8) | (dayOfWeek & 0xff);
  
  m.w[9] = ((((hour >= 12) ? ToBCD(hour) + 0x40 : ToBCD(hour)) & 0xff) << 24) |
           ((ToBCD(minute) & 0xff) << 16) | ((ToBCD(second) & 0xff) << 8);
  
  m.w[10] = n0;
  m.w[11] = n1;
  
  m.w[12] = SwapEndianess(keyInput);
  
  m.w[13] = 0x80000000;
  m.w[14] = 0x00000000;
  m.w[15] = 0x000001A0; // 416
  
#if 0
  for (uint32_t i = 0; i < 16; ++i)
  {
    std::cout << std::setw(8) << m.w[i] << (((i % 4) == 3) ? '\n' : ' ');
  }
  std::cout << std::endl;
#endif
  
  Digest d = SHA1(m);
  
#if 0
  std::cout
    << std::setw(8) << d.h[0] << ' '
    << std::setw(8) << d.h[1] << ' '
    << std::setw(8) << d.h[2] << ' '
    << std::setw(8) << d.h[3] << ' '
    << std::setw(8) << d.h[4] << std::endl;
  
  std::cout
    << std::setw(8) << SwapEndianess(d.h[0]) << ' '
    << std::setw(8) << SwapEndianess(d.h[1]) << ' '
    << std::setw(8) << SwapEndianess(d.h[2]) << ' '
    << std::setw(8) << SwapEndianess(d.h[3]) << ' '
    << std::setw(8) << SwapEndianess(d.h[4]) << std::endl;
#endif
  
  uint64_t  preSeed = SwapEndianess(d.h[1]);
  preSeed = (preSeed << 32) | SwapEndianess(d.h[0]);
  
  return LCRNG5(preSeed).Next();
}

}

HashedSeed::HashedSeed(uint32_t year, uint32_t month, uint32_t day,
                           uint32_t dayOfWeek,
                           uint32_t hour, uint32_t minute, uint32_t second,
                           uint32_t macAddressLow, uint32_t macAddressHigh,
                           uint32_t nazo, uint32_t n0, uint32_t n1, uint32_t n2,
                           uint32_t vcount, uint32_t timer, uint32_t gxStat,
                           uint32_t vframe, uint32_t keyInput)
  : m_year(year), m_month(month), m_day(day),
    m_hour(hour), m_minute(minute), m_second(second),
    m_macAddressLow(macAddressLow), m_macAddressHigh(macAddressHigh),
    m_nazo(nazo), m_vcount(vcount), m_timer0(timer), m_GxStat(gxStat),
    m_vframe(vframe), m_keyInput(keyInput),
    m_rawSeed(MakeSeed(year, month, day, dayOfWeek, hour, minute, second,
                       macAddressLow, macAddressHigh, nazo, n0, n1, n2,
                       vcount, timer,
                       gxStat, vframe, 0x2FFF ^ keyInput))
{}

HashedSeed::Nazo HashedSeed::NazoForVersion(Game::Version version)
{
  switch (version)
  {
    case Game::BlackJapanese:
      return JPBlackNazo;
      break;
    
    case Game::WhiteJapanese:
      return JPWhiteNazo;
      break;
    
    case Game::BlackEnglish:
      return ENBlackNazo;
      break;
    
    case Game::WhiteEnglish:
      return ENWhiteNazo;
      break;
    
    case Game::BlackSpanish:
      return SPBlackNazo;
      break;
    
    case Game::WhiteSpanish:
      return SPWhiteNazo;
      break;
    
    case Game::BlackFrench:
      return FRBlackNazo;
      break;
      
    case Game::WhiteFrench:
      return FRWhiteNazo;
      break;
      
    case Game::BlackItalian:
      return ITBlackNazo;
      break;
      
    case Game::WhiteItalian:
      return ITWhiteNazo;
      break;
      
    case Game::BlackGerman:
      return DEBlackNazo;
      break;
      
    case Game::WhiteGerman:
      return DEWhiteNazo;
      break;
      
    default:
      return static_cast<Nazo>(0);
      break;
  }
}

}
