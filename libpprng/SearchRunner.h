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

#ifndef SEARCH_RUNNER_H
#define SEARCH_RUNNER_H

#include <deque>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <functional>

namespace pprng
{

class SearchRunner
{
public:
  typedef boost::function<bool (double)>  ProgressCallback;
  
  template <class SeedGenerator, class SeedSearcher, class ResultChecker,
            class ResultCallback>
  void Search(SeedGenerator &seedGenerator,
              SeedSearcher &seedSearcher,
              ResultChecker &resultChecker,
              ResultCallback &resultHandler,
              const ProgressCallback &progressHandler,
              uint32_t numSplits = 1)
  {
    typedef typename SeedGenerator::SeedType               SeedType;
    typedef typename SeedGenerator::SeedCountType          SeedCountType;
    
    SeedCountType  numSeeds = seedGenerator.NumberOfSeeds();
    
    double  seedPercent = double(SeedGenerator::SeedsPerChunk) /
                            (numSeeds * numSplits);
    
    if (seedPercent > 0.002)
      seedPercent = 0.002;
    
    SeedCountType stepPercentSeeds((seedPercent * (numSeeds * numSplits)) + 1);
    
    const double stepPercent = seedPercent * 100.0;
    
    SeedCountType  threshold = stepPercentSeeds;
    
    for (typename SeedGenerator::SeedCountType i = 0;
         (i < numSeeds) && progressHandler(stepPercent);
         /* empty */)
    {
      for (/* empty */; i < threshold; ++i)
      {
        SeedType  seed = seedGenerator.Next();
        
        seedSearcher.Search(seed, resultChecker, resultHandler);
      }
      
      threshold += stepPercentSeeds;
      if (threshold > numSeeds)
      {
        threshold = numSeeds;
      }
    }
  }
  
  template <class SeedGenerator, class SeedSearcher, class ResultChecker,
            class ResultCallback>
  void SearchThreaded(SeedGenerator &seedGenerator,
                      SeedSearcher &seedSearcher,
                      ResultChecker &resultChecker,
                      ResultCallback &resultHandler,
                      const ProgressCallback &progressHandler)
  {
    typedef typename SeedSearcher::ResultType  ResultType;
    
    boost::condition_variable  progressUpdate;
    boost::mutex               progressMutex,  resultMutex;
    std::deque<double>         progressQueue;
    std::deque<ResultType>     resultQueue;
    bool                       shouldContinue = true;
    
    uint32_t  numProcs = boost::thread::hardware_concurrency();
    
    std::list<SeedGenerator>  generators = seedGenerator.Split(numProcs);
    if (generators.size() < numProcs)
      numProcs = generators.size();
    
    typedef std::list<boost::shared_ptr<boost::thread> >  ThreadList;
    ThreadList  threadList;
    
    ThreadResultHandler<ResultType>  threadResultHandler(resultMutex,
                                                         resultQueue);
    ThreadProgressHandler  threadProgressHandler(progressUpdate, progressMutex,
                                                 progressQueue, shouldContinue,
                                                 numProcs);
    
    typename std::list<SeedGenerator>::iterator  sg = generators.begin();
    for (uint32_t i = 0; i < numProcs; ++i)
    {
      SearchFunctor<SeedGenerator, SeedSearcher, ResultType, ResultChecker>
        searchFunctor(*this, *sg++, seedSearcher, resultChecker,
                      threadResultHandler, threadProgressHandler,
                      numProcs);
      
      boost::shared_ptr<boost::thread>  t(new boost::thread(searchFunctor));
      
      threadList.push_back(t);
    }
    
    while (threadProgressHandler.m_numActiveThreads > 0)
    {
      // update progress display
      {
        boost::unique_lock<boost::mutex>  lock(progressMutex);
        
        if (progressQueue.empty() &&
            (threadProgressHandler.m_numActiveThreads > 0))
          progressUpdate.wait(lock);
        
        while (!progressQueue.empty())
        {
          shouldContinue = shouldContinue &&
                           progressHandler(progressQueue.front());
          progressQueue.pop_front();
        }
      }
      
      // look for new results
      {
        boost::lock_guard<boost::mutex>  lock(resultMutex);
        while (!resultQueue.empty())
        {
          resultHandler(resultQueue.front());
          resultQueue.pop_front();
        }
      }
    }
    
    ThreadList::iterator  it;
    for (it = threadList.begin(); it != threadList.end(); ++it)
      (*it)->join();
  }
  
private:
  template <typename ResultType>
  struct ThreadResultHandler
  {
    ThreadResultHandler(boost::mutex &mut, std::deque<ResultType> &queue)
      : m_mut(mut), m_queue(queue)
    {}
    
    void operator()(const ResultType &result)
    {
      boost::lock_guard<boost::mutex>  lock(m_mut);
      
      m_queue.push_back(result);
    }
    
    boost::mutex            &m_mut;
    std::deque<ResultType>  &m_queue;
  };
  
  struct ThreadProgressHandler
  {
    ThreadProgressHandler(boost::condition_variable &condVar, boost::mutex &mut,
                          std::deque<double> &queue, bool &shouldContinue,
                          uint32_t numActiveThreads)
      : m_condVar(condVar), m_mut(mut), m_queue(queue),
        m_shouldContinue(shouldContinue), m_numActiveThreads(numActiveThreads)
    {}
    
    bool operator()(double progress) const
    {
      {
        boost::unique_lock<boost::mutex>  lock(m_mut);
        
        m_queue.push_back(progress);
        
        m_condVar.notify_one();
      }
      
      return m_shouldContinue;
    }
    
    void ThreadFinished()
    {
      {
        boost::unique_lock<boost::mutex>  lock(m_mut);
        
        --m_numActiveThreads;
        
        m_condVar.notify_one();
      }
    }
    
    boost::condition_variable  &m_condVar;
    boost::mutex               &m_mut;
    std::deque<double>         &m_queue;
    bool                       &m_shouldContinue;
    uint32_t                   m_numActiveThreads;
  };
  
  template <class SeedGenerator, class SeedSearcher,
            class ResultType, class ResultChecker>
  struct SearchFunctor
  {
    SearchFunctor(SearchRunner &searcher,
                  SeedGenerator &seedGenerator,
                  SeedSearcher &seedSearcher,
                  ResultChecker &resultChecker,
                  ThreadResultHandler<ResultType> &resultHandler,
                  ThreadProgressHandler &progressHandler,
                  uint32_t numSplits)
      : m_searcher(searcher),
        m_seedGenerator(seedGenerator),
        m_seedSearcher(seedSearcher),
        m_resultChecker(resultChecker),
        m_resultHandler(resultHandler),
        m_progressHandler(progressHandler),
        m_numSplits(numSplits)
    {}
    
    void operator()()
    {
      m_searcher.Search(m_seedGenerator, m_seedSearcher, m_resultChecker,
                        m_resultHandler, m_progressHandler,
                        m_numSplits);
      m_progressHandler.ThreadFinished();
    }
    
    SearchRunner                     &m_searcher;
    SeedGenerator                    &m_seedGenerator;
    SeedSearcher                     &m_seedSearcher;
    ResultChecker                    &m_resultChecker;
    ThreadResultHandler<ResultType>  &m_resultHandler;
    ThreadProgressHandler            &m_progressHandler;
    const uint32_t                   m_numSplits;
  };
};

}

#endif
