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

#include "BasicTypes.h"
#include "FrameSearcher.h"
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <functional>
#include <sstream>

namespace pprng
{

struct SeedSearchCriteria
{
  class ImpossibleMinMaxFrameRangeException : public Exception
  {
  private:
    static std::string MakeExceptionText
      (uint32_t minFrame, uint32_t maxFrame, const std::string &frameType)
    {
      std::ostringstream  os;
      os << "Minimum " << frameType << " frame " << minFrame
         << " is not less than or equal to maximum " << frameType << " frame "
         << maxFrame;
      return os.str();
    }
  public:
    ImpossibleMinMaxFrameRangeException
      (uint32_t minFrame, uint32_t maxFrame, const std::string &frameType)
      : Exception(MakeExceptionText(minFrame, maxFrame, frameType))
    {}
  };
  
  virtual uint64_t ExpectedNumberOfResults() const = 0;
};

template <class FrameGenerator>
class SeedSearcher
{
public:
  typedef typename FrameGenerator::Frame          Frame;
  typedef FrameSearcher<FrameGenerator>           SeedFrameSearcher;
  typedef typename SeedFrameSearcher::FrameRange  FrameRange;
  
  typedef boost::function<void (const Frame&)> ResultCallback;
  
  typedef boost::function<bool (double percent)>  ProgressCallback;
  
  template <class SeedGenerator, class FrameGeneratorFactory, class FrameChecker>
  void Search(SeedGenerator &seedGenerator,
              const FrameGeneratorFactory &frameGeneratorFactory,
              const FrameRange &frameRange,
              FrameChecker &frameChecker,
              const ResultCallback &resultHandler,
              const ProgressCallback &progressHandler,
              uint32_t numSplits = 1)
  {
    typename SeedGenerator::SeedCountType  numSeeds = seedGenerator.NumSeeds();
    
    double  seedPercent = double(SeedGenerator::SeedsPerChunk) /
                            (numSeeds * numSplits);
    
    if (seedPercent > 0.002)
    {
      seedPercent = 0.002;
    }
    
    typename SeedGenerator::SeedCountType  stepPercentSeeds =
      (seedPercent * (numSeeds * numSplits)) + 1;
    
    const double stepPercent = seedPercent * 100.0;
    
    typename SeedGenerator::SeedCountType  threshold = stepPercentSeeds;
    
    for (typename SeedGenerator::SeedCountType i = 0;
         (i < numSeeds) && progressHandler(stepPercent);
         /* empty */)
    {
      for (/* empty */; i < threshold; ++i)
      {
        typename SeedGenerator::SeedType  seed = seedGenerator.Next();
        
        FrameGenerator     frameGenerator = frameGeneratorFactory(seed);
        SeedFrameSearcher  frameSearcher(frameGenerator);
        
        while(frameSearcher.Search(frameRange, frameChecker, resultHandler))
          /* search all frames, not just first */;
      }
      
      threshold += stepPercentSeeds;
      if (threshold > numSeeds)
      {
        threshold = numSeeds;
      }
    }
  }
  
  template <class SeedGenerator, class FrameGeneratorFactory, class FrameChecker>
  void SearchThreaded(SeedGenerator &seedGenerator,
                      const FrameGeneratorFactory &frameGeneratorFactory,
                      const FrameRange &frameRange,
                      FrameChecker &frameChecker,
                      const ResultCallback &resultHandler,
                      const ProgressCallback &progressHandler)
  {
    boost::condition_variable  progressUpdate;
    boost::mutex               progressMutex,  resultMutex;
    std::deque<double>         progressQueue;
    std::deque<Frame>          resultQueue;
    bool                       shouldContinue = true;
    
    uint32_t  numProcs = boost::thread::hardware_concurrency();
    
    std::list<SeedGenerator>  generators = seedGenerator.Split(numProcs);
    
    typedef std::list<boost::shared_ptr<boost::thread> >  ThreadList;
    ThreadList  threadList;
    
    ThreadResultHandler    threadResultHandler(resultMutex, resultQueue);
    ThreadProgressHandler  threadProgressHandler(progressUpdate, progressMutex,
                                                 progressQueue, shouldContinue,
                                                 numProcs);
    
    typename std::list<SeedGenerator>::iterator  sg = generators.begin();
    for (uint32_t i = 0; i < numProcs; ++i)
    {
      SearchFunctor<SeedGenerator, FrameGeneratorFactory, FrameChecker>
        searchFunctor(*this, *sg++, frameGeneratorFactory, frameRange,
                      frameChecker, threadResultHandler, threadProgressHandler,
                      numProcs);
      
      boost::shared_ptr<boost::thread>  t(new boost::thread(searchFunctor));
      
      threadList.push_back(t);
    }
    
    while (threadProgressHandler.m_numActiveThreads > 0)
    {
      // look for new results
      {
        boost::lock_guard<boost::mutex>  lock(resultMutex);
        while (!resultQueue.empty())
        {
          resultHandler(resultQueue.front());
          resultQueue.pop_front();
        }
      }
      
      // update progress display
      {
        boost::unique_lock<boost::mutex>  lock(progressMutex);
        if (progressQueue.empty())
        {
          progressUpdate.wait(lock);
        }
        while (!progressQueue.empty())
        {
          shouldContinue = shouldContinue &&
                           progressHandler(progressQueue.front());
          progressQueue.pop_front();
        }
      }
    }
    
    ThreadList::iterator  it;
    for (it = threadList.begin(); it != threadList.end(); ++it)
    {
      (*it)->join();
    }
  }
  
private:
  struct ThreadResultHandler
  {
    ThreadResultHandler(boost::mutex &mut, std::deque<Frame> &queue)
      : m_mut(mut), m_queue(queue)
    {}
    
    void operator()(const Frame &frame)
    {
      boost::lock_guard<boost::mutex>  lock(m_mut);
      
      m_queue.push_back(frame);
    }
    
    boost::mutex               &m_mut;
    std::deque<Frame>          &m_queue;
  };
  
  struct ThreadProgressHandler
  {
    ThreadProgressHandler(boost::condition_variable &condVar, boost::mutex &mut,
                          std::deque<double> &queue, bool &shouldContinue,
                          uint32_t numActiveThreads)
      : m_condVar(condVar), m_mut(mut), m_queue(queue),
        m_shouldContinue(shouldContinue), m_numActiveThreads(numActiveThreads)
    {}
    
    bool operator()(double progress)
    {
      {
        boost::unique_lock<boost::mutex>  lock(m_mut);
        
        m_queue.push_back(progress);
      }
      m_condVar.notify_one();
      
      return m_shouldContinue;
    }
    
    void ThreadFinished()
    {
      {
        boost::unique_lock<boost::mutex>  lock(m_mut);
        --m_numActiveThreads;
      }
      m_condVar.notify_one();
    }
    
    boost::condition_variable  &m_condVar;
    boost::mutex               &m_mut;
    std::deque<double>         &m_queue;
    bool                       &m_shouldContinue;
    uint32_t                   m_numActiveThreads;
  };
  
  template <class SeedGenerator, class FrameGeneratorFactory, class FrameChecker>
  struct SearchFunctor
  {
    SearchFunctor(SeedSearcher &searcher,
                  SeedGenerator &seedGenerator,
                  const FrameGeneratorFactory &frameGeneratorFactory,
                  const FrameRange &frameRange,
                  FrameChecker &frameChecker,
                  ThreadResultHandler &resultHandler,
                  ThreadProgressHandler &progressHandler,
                  uint32_t numSplits)
      : m_searcher(searcher),
        m_seedGenerator(seedGenerator),
        m_frameGeneratorFactory(frameGeneratorFactory),
        m_frameRange(frameRange),
        m_frameChecker(frameChecker),
        m_resultHandler(resultHandler),
        m_progressHandler(progressHandler),
        m_numSplits(numSplits)
    {}
    
    void operator()()
    {
      m_searcher.Search(m_seedGenerator, m_frameGeneratorFactory, m_frameRange,
                        m_frameChecker, m_resultHandler, m_progressHandler,
                        m_numSplits);
      m_progressHandler.ThreadFinished();
    }
    
    SeedSearcher                &m_searcher;
    SeedGenerator               &m_seedGenerator;
    const FrameGeneratorFactory &m_frameGeneratorFactory;
    const FrameRange            &m_frameRange;
    FrameChecker                &m_frameChecker;
    ThreadResultHandler         &m_resultHandler;
    ThreadProgressHandler       &m_progressHandler;
    const uint32_t              m_numSplits;
  };
};

}

#endif
