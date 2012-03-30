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
#include "Frame.h"
#include "FrameGenerator.h"
#include "LinearCongruentialRNG.h"

namespace pprng
{

namespace
{

template <class Method>
static void UpdateESVs(uint32_t percentage1, uint32_t percentage2,
                       uint32_t percentage3, uint32_t frameNumber,
                       Gen4Frame::EncounterData::FrameType frameType,
                       Gen4Frame::EncounterData &encData)
{
  ESV::Value  value = ESV::Gen4Land(percentage1);
  encData.landESVs |= 0x1 << ESV::Slot(value);
  encData.esvFrames[value].number[frameType] = frameNumber;
  
  value = ESV::Gen4Surfing(percentage2);
  encData.surfESVs |= 0x1 << ESV::Slot(value);
  encData.esvFrames[value].number[frameType] = frameNumber;
  
  if (percentage3 < Method::OldRodThreshold)
  {
    value = Method::OldRodESV(percentage2);
    encData.oldRodESVs |= ESV::Slot(value);
    encData.esvFrames[value].number[frameType] = frameNumber;
  }
  
  if (percentage3 < Method::GoodRodThreshold)
  {
    value = Method::GoodRodESV(percentage2);
    encData.goodRodESVs |= ESV::Slot(value);
    encData.esvFrames[value].number[frameType] = frameNumber;
  }
  
  if (percentage3 < Method::SuperRodThreshold)
  {
    value = Method::SuperRodESV(percentage2);
    encData.superRodESVs |= ESV::Slot(value);
    encData.esvFrames[value].number[frameType] = frameNumber;
  }
}

template <class Method>
static void UpdateEncounterData(Gen4Frame::EncounterData &encData,
                                Nature::Type nature,  uint32_t frameNumber,
                                uint32_t randomValue1, uint32_t randomValue2,
                                uint32_t randomValue3, uint32_t randomValue4,
                                uint32_t randomValue5)
{
  // check non-sync Method frame
  if (Method::DetermineNature(randomValue1) == nature)
  {
    encData.lowestFrames.number[Gen4Frame::EncounterData::NoSync] = frameNumber;
    uint32_t  percentage2 = Method::CalculatePercentage(randomValue2);
    uint32_t  percentage3 = Method::CalculatePercentage(randomValue3);
    uint32_t  percentage4 = Method::CalculatePercentage(randomValue4);
    
    UpdateESVs<Method>(percentage2, percentage3, percentage4,
                       frameNumber, Gen4Frame::EncounterData::NoSync,
                       encData);
    
    // check failed synchronize
    if (!Method::DetermineSync(randomValue2))
    {
      encData.lowestFrames.number[Gen4Frame::EncounterData::FailedSync] =
        frameNumber - 1;
      uint32_t  percentage5 = Method::CalculatePercentage(randomValue5);
      
      UpdateESVs<Method>(percentage3, percentage4, percentage5,
                         frameNumber - 1, Gen4Frame::EncounterData::FailedSync,
                         encData);
    }
    
    // avoid percentage recalc by checking synchronize here
    if (Method::DetermineSync(randomValue1))
    {
      encData.lowestFrames.number[Gen4Frame::EncounterData::Sync] = frameNumber;
      UpdateESVs<Method>(percentage2, percentage3, percentage4,
                         frameNumber, Gen4Frame::EncounterData::Sync, encData);
    }
  }
  // check Method synchronize
  else if (Method::DetermineSync(randomValue1))
  {
    encData.lowestFrames.number[Gen4Frame::EncounterData::Sync] = frameNumber;
    uint32_t  percentage2 = Method::CalculatePercentage(randomValue2);
    uint32_t  percentage3 = Method::CalculatePercentage(randomValue3);
    uint32_t  percentage4 = Method::CalculatePercentage(randomValue4);
    
    UpdateESVs<Method>(percentage2, percentage3, percentage4,
                       frameNumber, Gen4Frame::EncounterData::Sync, encData);
  }
}

}

Gen4Frame::Gen4Frame(const Gen34Frame &baseFrame)
  : seed(baseFrame.seed), number(baseFrame.number),
    pid(baseFrame.pid), ivs(baseFrame.ivs),
    methodJ(), methodK()
{
  Nature::Type  nature = baseFrame.pid.Gen34Nature();
  LCRNG34_R     rng(baseFrame.frameSeed);
  int32_t       candidateFrameNumber = baseFrame.number - 1;
  uint32_t      randomValue3 = rng.Next();
  uint32_t      randomValue4 = rng.Next();
  uint32_t      randomValue5 = rng.Next();
  
  while (candidateFrameNumber > 0)
  {
    uint32_t  randomValue1 = randomValue3;
    uint32_t  randomValue2 = randomValue4;
    
    randomValue3 = randomValue5;
    randomValue4 = rng.Next();
    randomValue5 = rng.Next();
    
    UpdateEncounterData<MethodJ>(methodJ, nature, candidateFrameNumber,
                                 randomValue1, randomValue2, randomValue3,
                                 randomValue4, randomValue5);
    
    UpdateEncounterData<MethodK>(methodK, nature, candidateFrameNumber,
                                 randomValue1, randomValue2, randomValue3,
                                 randomValue4, randomValue5);
    
    PID  randomPID((randomValue1 & 0xffff0000) | (randomValue2 >> 16));
    
    if ((randomPID.Gen34Nature() == nature) || (candidateFrameNumber == 1))
    {
      break;
    }
    
    candidateFrameNumber -= 2;
  }
}


namespace
{

static OptionalIVs GetEggIVs(const Gen5BreedingFrame &frame, IVs baseIVs,
                             const OptionalIVs &parentXIVs,
                             const OptionalIVs &parentYIVs)
{
  OptionalIVs  ivs;
  
  for (uint32_t i = 0; i < 6; ++i)
  {
    switch (frame.inheritance[i])
    {
    default:
    case Gen5BreedingFrame::NotInherited:
      ivs.setIV(i, baseIVs.iv(i));
      break;
    case Gen5BreedingFrame::ParentX:
      if (parentXIVs.isSet(i))
        ivs.setIV(i, parentXIVs.iv(i));
      break;
    case Gen5BreedingFrame::ParentY:
      if (parentYIVs.isSet(i))
        ivs.setIV(i, parentYIVs.iv(i));
      break;
    }
  }
  
  return ivs;
}

}

Gen5EggFrame::Gen5EggFrame(const Gen5BreedingFrame &f,
                           uint32_t ivFrame, IVs baseIVs,
                           const OptionalIVs &parentXIVs,
                           const OptionalIVs &parentYIVs)
  : Gen5BreedingFrame(f),
    ivFrameNumber(ivFrame), ivs(GetEggIVs(f, baseIVs, parentXIVs, parentYIVs)),
    characteristic(ivs.allSet() ?
                    Characteristic::Get(f.pid, ivs.values) :
                    Characteristic::NONE)
{
  //for (uint32_t i = 0; i < 6; ++i)
    //inheritance[i] = f.inheritance[i];
}

}
