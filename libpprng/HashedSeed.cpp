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

enum { NumTableRows = 5, NumTableColumns = 4, NumLoops = 5 };

static uint32_t  PercentageTable[NumTableRows][NumTableColumns] =
{
  {  50,100,100,100 },
  {  50, 50,100,100 },
  {  30, 50,100,100 },
  {  25, 30, 50,100 },
  {  20, 25, 33, 50 }
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
