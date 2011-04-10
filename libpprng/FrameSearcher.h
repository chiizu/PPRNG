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

#ifndef FRAME_SEARCHER_H
#define FRAME_SEARCHER_H

#include "BasicTypes.h"

namespace pprng
{

template <class FrameGenerator>
class FrameSearcher
{
public:
  typedef typename FrameGenerator::Frame  Frame;
  
  struct FrameRange
  {
    FrameRange(uint32_t mi, uint32_t ma) : min(mi), max(ma) {}
    
    uint32_t  min;
    uint32_t  max;
  };
  
  FrameSearcher(FrameGenerator &frameGenerator)
    : m_FrameGenerator(frameGenerator), m_frameNum(0)
  {}
  
  uint32_t FrameNum() const { return m_frameNum; }
  
  template <class FrameChecker, class ResultCallback>
  bool Search(const FrameRange &frameRange,
              FrameChecker checker, ResultCallback callback)
  {
    uint32_t  frameNum = m_frameNum;
    uint32_t  limitFrame;
    bool      found = false;
    
    limitFrame = frameRange.min - 1;
    while (frameNum < limitFrame)
    {
      m_FrameGenerator.AdvanceFrame();
      ++frameNum;
    }
    
    limitFrame = frameRange.max;
    while ((frameNum < limitFrame) && !found)
    {
      m_FrameGenerator.AdvanceFrame();
      ++frameNum;
      
      found = checker(m_FrameGenerator.CurrentFrame());
    }
    
    m_frameNum = frameNum;
    
    if (found)
    {
      callback(m_FrameGenerator.CurrentFrame());
      return true;
    }
    
    return false;
  }
  
private:
  FrameGenerator  &m_FrameGenerator;
  uint32_t        m_frameNum;
};

}

#endif
