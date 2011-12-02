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


#if 1

#define K0  0x5A827999
#define K1  0x6ED9EBA1
#define K2  0x8F1BBCDC
#define K3  0xCA62C1D6

#define H0  0x67452301
#define H1  0xEFCDAB89
#define H2  0x98BADCFE
#define H3  0x10325476
#define H4  0xC3D2E1F0

Digest SHA1(const Message &message)
{
  uint32_t  w[80];
  
  uint32_t  a = H0;
  uint32_t  b = H1;
  uint32_t  c = H2;
  uint32_t  d = H3;
  uint32_t  e = H4;
  
  ::memcpy(w, message.w, sizeof(message.w));
  
  uint32_t i, temp;
  
  // extend to 80 32-bit words
  for (i = 16; i < 80; ++i)
  {
    temp = w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16];
    w[i] = (temp << 1) | (temp >> 31);
  }
  
  uint32_t  f;
  for (i = 0; i < 20; ++i)
  {
    f = (b & c) | (~b & d);  //f = d ^ (b & (c ^ d));
    
    temp = ((a << 5) | (a >> 27)) + f + e + K0 + w[i];
    
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;
  }
  
  for (i = 20; i < 40; ++i)
  {
    f = b ^ c ^ d;
    
    temp = ((a << 5) | (a >> 27)) + f + e + K1 + w[i];
    
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;
  }

  for(i = 40; i < 60; ++i)
  {
    f = (b & c) | (b & d) | (c & d);  //f = (b & c) | (d & (b | c));
    
    temp = ((a << 5) | (a >> 27)) + f + e + K2 + w[i];
    
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;
  }

  for(i = 60; i < 80; ++i)
  {
    f = b ^ c ^ d;
    
    temp = ((a << 5) | (a >> 27)) + f + e + K3 + w[i];
    
    e = d;
    d = c;
    c = (b << 30) | (b >> 2);
    b = a;
    a = temp;
  }
  
  Digest  result;
  
  result.h[0] = H0 + a;
  result.h[1] = H1 + b;
  result.h[2] = H2 + c;
  result.h[3] = H3 + d;
  result.h[4] = H4 + e;
  
  return result;
}

#else

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
      //f = d ^ (b & (c ^ d));
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
      //f = (b & c) | (d & (b | c));
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

#endif


uint32_t SwapEndianess(uint32_t value)
{
  value = ((value << 8) & 0xFF00FF00) | ((value >> 8) & 0xFF00FF);
  return (value << 16) | (value >> 16);
}

uint32_t ToBCD(uint32_t value)
{
  return (((value / 1000) % 10) << 12) |
         (((value / 100) % 10) << 8) |
         (((value / 10) % 10) << 4) |
         (value % 10);
}


enum
{
  FirstNazoOffset = 0xFC,
  SecondNazoOffset = FirstNazoOffset + 0x4C,
  ButtonMask = 0x2FFF
};

uint64_t MakeSeed(const HashedSeed::Parameters &parameters)
{
  Message  m;
  
  HashedSeed::Nazo  nazo =
    HashedSeed::NazoForVersionAndDS(parameters.version, parameters.dsType);
  
  m.w[0] = SwapEndianess(nazo);
  m.w[1] = m.w[2] = SwapEndianess(nazo + FirstNazoOffset);
  m.w[3] = m.w[4] = SwapEndianess(nazo + SecondNazoOffset);
  
  m.w[5] = SwapEndianess((parameters.vcount << 16) | parameters.timer0);
  
  m.w[6] = parameters.macAddress.low & 0xffff;
  
  m.w[7] = (((parameters.macAddress.low >> 16) & 0xff) |
             (parameters.macAddress.high << 8)) ^
           SwapEndianess(parameters.gxStat ^ parameters.vframe);
  
  m.w[8] = ((ToBCD(parameters.date.year()) & 0xff) << 24) |
           ((ToBCD(parameters.date.month()) & 0xff) << 16) |
           ((ToBCD(parameters.date.day()) & 0xff) << 8) |
           (parameters.date.day_of_week() & 0xff);
  
  m.w[9] = (((ToBCD(parameters.hour) +
              (((parameters.hour >= 12) && (parameters.dsType != DS::_3DS)) ?
               0x40 : 0)) & 0xff) << 24) |
           ((ToBCD(parameters.minute) & 0xff) << 16) |
           ((ToBCD(parameters.second) & 0xff) << 8);
  
  m.w[10] = 0;
  m.w[11] = 0;
  
  m.w[12] = SwapEndianess(parameters.heldButtons ^ ButtonMask);
  
  m.w[13] = 0x80000000;
  m.w[14] = 0x00000000;
  m.w[15] = 0x000001A0; // 416
  
  Digest d = SHA1(m);
  
  uint64_t  preSeed = SwapEndianess(d.h[1]);
  preSeed = (preSeed << 32) | SwapEndianess(d.h[0]);
  
  return LCRNG5(preSeed).Next();
}



enum { NumTableRows = 6, NumTableColumns = 5, NumLoops = 5 };

static uint32_t  PercentageTable[NumTableRows][NumTableColumns] =
{
  {  50,100,100,100,100 },
  {  50, 50,100,100,100 },
  {  30, 50,100,100,100 },
  {  25, 30, 50,100,100 },
  {  20, 25, 33, 50,100 },
  { 100,100,100,100,100 }
};

static uint32_t CalculateConsumedPIDRNGFrames(uint64_t rawSeed)
{
  LCRNG5    rng(rawSeed);
  uint32_t  count = 0;
  
  for (uint32_t i = 0; i < NumLoops; ++i)
  {
    for (uint32_t j = 0; j < NumTableRows; ++j)
    {
      for (uint32_t k = 0; k < NumTableColumns; ++k)
      {
        uint32_t  percent = PercentageTable[j][k];
        if (percent == 100)
          break;
        
        ++count;
        
        uint32_t d101 = ((rng.Next() >> 32) * 101) >> 32;
        if (d101 <= percent)
          break;
      }
    }
  }
  
  return count;
}

}

HashedSeed::HashedSeed(const HashedSeed::Parameters &parameters)
  : version(parameters.version), dsType(parameters.dsType),
    macAddress(parameters.macAddress), gxStat(parameters.gxStat),
    vcount(parameters.vcount), vframe(parameters.vframe),
    timer0(parameters.timer0),
    date(parameters.date), hour(parameters.hour),
    minute(parameters.minute), second(parameters.second),
    heldButtons(parameters.heldButtons),
    rawSeed(MakeSeed(parameters)),
    m_skippedPIDFramesCalculated(false),
    m_skippedPIDFrames(0)
{}

HashedSeed::HashedSeed(const Parameters &parameters, uint64_t rawSeed_)
  : version(parameters.version), dsType(parameters.dsType),
    macAddress(parameters.macAddress), gxStat(parameters.gxStat),
    vcount(parameters.vcount), vframe(parameters.vframe),
    timer0(parameters.timer0),
    date(parameters.date), hour(parameters.hour),
    minute(parameters.minute), second(parameters.second),
    heldButtons(parameters.heldButtons),
    rawSeed(rawSeed_),
    m_skippedPIDFramesCalculated(false),
    m_skippedPIDFrames(0)
{}

HashedSeed::Nazo HashedSeed::NazoForVersionAndDS(Game::Version version,
                                                 DS::Type dsType)
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
      
    default:
      return static_cast<Nazo>(0);
      break;
  }
}

uint32_t HashedSeed::GetSkippedPIDFrames() const
{
  if (!m_skippedPIDFramesCalculated)
  {
    m_skippedPIDFrames = CalculateConsumedPIDRNGFrames(rawSeed);
    
    m_skippedPIDFramesCalculated = true;
  }
  
  return m_skippedPIDFrames;
}

}
