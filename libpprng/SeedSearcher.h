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

#ifndef SEED_SEARCHER_H
#define SEED_SEARCHER_H

#include "PPRNGTypes.h"
#include "SearchCriteria.h"
#include "FrameSearcher.h"
#include <boost/function.hpp>
#include <sstream>

namespace pprng
{

template <class FrameGeneratorFactory>
class SeedFrameSearcher
{
public:
  typedef typename FrameGeneratorFactory::FrameGenerator  FrameGenerator;
  typedef typename FrameGenerator::Seed                   Seed;
  typedef typename FrameGenerator::Frame                  Frame;
  typedef Frame                                           ResultType;
  
  typedef boost::function<void (const Frame&)> ResultCallback;
  
  SeedFrameSearcher(const FrameGeneratorFactory &frameGeneratorFactory,
                    const SearchCriteria::FrameRange &frameRange)
    : m_frameGeneratorFactory(frameGeneratorFactory), m_frameRange(frameRange)
  {}
  
  template <class FrameChecker>
  void Search(const Seed &seed, const FrameChecker &frameChecker,
              const ResultCallback &resultHandler)
  {
    FrameGenerator     frameGenerator = m_frameGeneratorFactory(seed);
    FrameSearcher<FrameGenerator>  frameSearcher(frameGenerator);
    
    while(frameSearcher.Search(m_frameRange, frameChecker, resultHandler))
      /* search all frames, not just first */;
  }
  
private:
  const FrameGeneratorFactory       &m_frameGeneratorFactory;
  const SearchCriteria::FrameRange  &m_frameRange;
};

}

#endif
