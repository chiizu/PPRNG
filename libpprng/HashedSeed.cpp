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


#include "HashedSeed.h"
#include "HashedSeedMessage.h"
#include "LinearCongruentialRNG.h"

namespace pprng
{

namespace
{

enum { NumTableRows = 5, NumTableColumns = 4 };

static uint32_t  PercentageTable[NumTableRows][NumTableColumns] =
{
  {  50,100,100,100 },
  {  50, 50,100,100 },
  {  30, 50,100,100 },
  {  25, 30, 50,100 },
  {  20, 25, 33, 50 }
};

static uint32_t ProbabilityTableLoop(LCRNG5 &rng)
{
  uint32_t  count = 0;
  
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
  
  return count;
}

static uint32_t CalculateConsumedPIDRNGFrames(uint64_t rawSeed,
                          Game::Version version)
{
  LCRNG5    rng(rawSeed);
  uint32_t  count = 0;
  
  if (Game::IsBlack2White2(version))
  {
    count += ProbabilityTableLoop(rng);
    
    rng.Next(); // 0
    rng.Next(); // 0xffffffff
    rng.Next(); // 0xffffffff
    
    count += 3;
    
    for (uint32_t i = 0; i < 4; ++i)
      count += ProbabilityTableLoop(rng);
    
    bool      duplicatesFound = true;
    uint32_t  buffer[3];
    for (uint32_t limit = 0; duplicatesFound && (limit < 100); ++limit)
    {
      for (uint32_t i = 0; i < 3; ++i)
        buffer[i] = ((rng.Next() >> 32) * 15) >> 32;
      
      count += 3;
      duplicatesFound = false;
      
      for (uint32_t i = 0; i < 3; ++i)
      {
        for (uint32_t j = 0; j < 3; ++ j)
        {
          if (i != j)
          {
            if (buffer[i] == buffer[j])
              duplicatesFound = true;
          }
        }
      }
    }
  }
  else
  {
    for (uint32_t i = 0; i < 5; ++i)
      count += ProbabilityTableLoop(rng);
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
    rawSeed(HashedSeedMessage(parameters).GetRawSeed()),
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
      
    case Game::Black2Japanese:
      return isPlainDS ? JPBlack2Nazo : JPBlack2Nazo;
      break;
    
    case Game::White2Japanese:
      return isPlainDS ? JPWhite2Nazo : JPWhite2Nazo;
      break;
      
    default:
      return static_cast<Nazo>(0);
      break;
  }
}




HashedSeed::Nazo HashedSeed::Nazo2ForVersionAndDS(Game::Version version,
                                                  DS::Type dsType)
{
  bool isPlainDS = (dsType == DS::DSPhat) || (dsType == DS::DSLite);
  
  switch (version)
  {
    case Game::Black2Japanese:
      return isPlainDS ? JPBlack2Nazo2 : JPBlack2Nazo2;
      break;
    
    case Game::White2Japanese:
      return isPlainDS ? JPWhite2Nazo2 : JPWhite2Nazo2;
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
    m_skippedPIDFrames = CalculateConsumedPIDRNGFrames(rawSeed, version);
    
    m_skippedPIDFramesCalculated = true;
  }
  
  return m_skippedPIDFrames;
}

}
