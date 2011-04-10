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
#include "Frame.h"
#include "LinearCongruentialRNG.h"

namespace pprng
{

Gen4Frame::Gen4Frame(const Gen34Frame &baseFrame)
  : seed(baseFrame.seed), number(baseFrame.number),
    pid(baseFrame.pid), ivs(baseFrame.ivs),
    methodJNumber(0), methodJSyncNumber(0), methodJFailedSyncNumber(0),
    methodKNumber(0), methodKSyncNumber(0), methodKFailedSyncNumber(0)
{
  Nature::Type  nature = baseFrame.pid.Gen34Nature();
  LCRNG34_R     rng(baseFrame.frameSeed);
  uint32_t      candidateFrameNumber = baseFrame.number - 1;
  
  while (candidateFrameNumber > 0)
  {
    uint32_t  randomValue1 = rng.Next();
    uint32_t  randomValue2 = rng.Next();
    PID       randomPID((randomValue1 & 0xffff0000) | (randomValue2 >> 16));
    
    // check normal Method J frame
    if (((randomValue1 >> 16) / 0xa3e) == nature)
    {
      methodJNumber = candidateFrameNumber;
      
      // check failed synchronize
      if ((randomValue2 & 0x80000000) == 0)
      {
        methodJFailedSyncNumber = candidateFrameNumber - 1;
      }
    }
    
    // check Method J synchronize
    if ((randomValue1 & 0x80000000) == 0)
    {
      methodJSyncNumber = candidateFrameNumber;
    }
    
    // check normal Method K frame
    if (((randomValue1 >> 16) % 25) == nature)
    {
      methodKNumber = candidateFrameNumber;
      
      // check failed synchronize
      if ((randomValue2 & 0x00010000) == 0)
      {
        methodKFailedSyncNumber = candidateFrameNumber - 1;
      }
    }
    
    // check Method K synchronize
    if ((randomValue1 & 0x00010000) == 0)
    {
      methodKSyncNumber = candidateFrameNumber;
    }
    
    if ((randomPID.Gen34Nature() == nature) || (candidateFrameNumber == 1))
    {
      break;
    }
    
    candidateFrameNumber -= 2;
  }
}

}
