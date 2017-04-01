// stats.hpp
// Made by Benjamin Negrevergne 
// Started on <2017-04-01 Sat>

#ifndef STATS_HPP
#define STATS_HPP
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <sstream>

template <typename NRPA> 
class Stats{

  static const int MAX_RUNS = 16; 
  static const int MAX_ITER = 128;  // maximum number of iteration (used for iteration-based stat collection) 
  static const int MAX_TIME_EVENTS = 16;  // maximum number of timer events (used for time-based stat collection)

  
public: 

  struct NrpaStats{
    NrpaStats(); 
    void prettyPrint(); 

    float date; 
    int iter; 
    int eventIdx; 
    double bestScore;

  }; 
  
  Stats();

  void initIterStats();
  void initTimerStats(); 

  void startRun(NRPA *nrpa, int timeout = 0); 
  void finishRun(); 

  void recordIterStats(int iter, const typename NRPA::NrpaLevel &nl); 
  void recordTimerStats(const typename NRPA::NrpaLevel &nl); 

  void writeStats(const std::string &prefix = "nrpa_stats",
		  const std::string &tag = "") const; 

  float getTime() const; 

  bool timeout() const;
  void resetTimeout();

private:

  void setTimers(); 

  std::chrono::system_clock::time_point _startTime; 

  bool _iterStatsOn; 
  bool _timerStatsOn;

  int _runId; 

  int _runNbIter[MAX_RUNS]; 
  int _runNbEvents[MAX_RUNS]; 

  NrpaStats _iterStats[MAX_ITER][MAX_RUNS]; // stastics collected at each top level iteration 
  NrpaStats _timerStats[MAX_TIME_EVENTS][MAX_RUNS]; // stastics collected on time events

  atomic_bool _done; 
  std::mutex _doneMutex; 
  std::condition_variable _doneCond; 

  int _nbTimerEvents; 
  int _lastEventIdx; 

  int _timeout; 

  NRPA *_nrpa; 
  thread *_thread; 
}; 


template <typename NRPA>
Stats<NRPA>::Stats():
  _iterStatsOn(false),
  _timerStatsOn(false){
}

template <typename NRPA>
void Stats<NRPA>::initIterStats(){
  _iterStatsOn = true; 
}

template <typename NRPA>
void Stats<NRPA>::initTimerStats(){
  _timerStatsOn = true; 
}

template <typename NRPA>
void Stats<NRPA>::startRun(NRPA *nrpa, int timeout){
  using namespace std::chrono; 
  _nrpa = nrpa; 
  _timeout = timeout; 
  _done = false; 

  if(_iterStatsOn){
    fill_n(_iterStats[_runId], MAX_ITER, NrpaStats()); // reset stats
    _runNbIter[_runId] = 0;
  }

  if(_timerStatsOn){
    fill_n(_timerStats[_runId], MAX_TIME_EVENTS, NrpaStats()); // reset stats  
    _runNbEvents[_runId] = 0;
  }

  if(_timerStatsOn || _timeout > 0)
    setTimers(); 
}

template <typename NRPA>
float Stats<NRPA>::getTime() const{
  using namespace std::chrono; 
  return duration_cast<milliseconds>(system_clock::now() - _startTime).count() / 1000.;   
}


template <typename NRPA>
bool Stats<NRPA>::timeout() const{
  return _done; 
}


template <typename NRPA>
void Stats<NRPA>::resetTimeout(){
  _done = true;
  _doneCond.notify_one();
}


template <typename NRPA>
void Stats<NRPA>::finishRun(){
  _runId++; 
  if(_timerStatsOn || _timeout > 0){
    _thread->join();
    delete _thread;
  }
}

template <typename NRPA>
void Stats<NRPA>::recordIterStats(int iter, const typename NRPA::NrpaLevel &nl){
  if(_iterStatsOn){
    assert(iter == _runNbIter[_runId]); 

    float date = getTime(); 
    _iterStats[_runId][iter].date =  date; 
    _iterStats[_runId][iter].bestScore = nl.bestRollout.score(); 

    _runNbIter[_runId]++;
  }
}

template <typename NRPA>
void Stats<NRPA>::recordTimerStats(const typename NRPA::NrpaLevel &nl){
  if(_timerStatsOn){
    int eventIdx = _runNbEvents[_runId]; 

    _timerStats[_runId][eventIdx].date = getTime(); 
    _timerStats[_runId][eventIdx].bestScore = nl.bestRollout.score(); 
    _timerStats[_runId][eventIdx].eventIdx = eventIdx;

    _runNbEvents[_runId]++;
  }
}


template <typename NRPA>
void Stats<NRPA>::writeStats(const string &prefix, const std::string &tag) const{
  if( ! ( _iterStatsOn || _timerStatsOn ) ) return ; 

  fstream fs;
  ostringstream filename;

  filename<<prefix;
  filename<<"_nbRun."<<_runId;
  filename<<"_level."<<_nrpa->_startLevel;
  filename<<"_nbIter."<<_nrpa->_nbIter;
  filename<<"_timeout."<<_timeout;
  filename<<"_nbThreads."<<_nrpa->_nbThreads;

  if(_iterStatsOn){
    ostringstream iterfilename; 
    iterfilename<<filename.str()<<".iter.dat"; 
    if(tag != "")
      iterfilename<<"."<<tag;
    
    fs.open(iterfilename.str(), fstream::out);
    fs<<"#<RunId> <iterId> <timestamp> <currentbestscore>"<<"\n";
    //fs<<"# nbRun "<<nbRun<<" level "<<level<<" nbIter "<<nbIter<<" timeout "<<timeout<<" nbThreads "<<nbThreads<<endl;
    for(int i = 0; i < _runId; i++){
      for(int j = 0; j < _runNbIter[i]; j++){
	const NrpaStats &s = _iterStats[i][j]; 
	fs<<i<<" "<<j<<" "<<s.date<<" "<<s.bestScore<<" "<<"\n";
      }
    }
    fs.close(); 
  }

  if(_timerStatsOn){
    ostringstream timerfilename;
    timerfilename<<filename.str()<<".timer.dat"; 
    if(tag != "")
      timerfilename<<"."<<tag;

    fs.open(timerfilename.str(), fstream::out);
    //fs<<"# nbRun "<<nbRun<<" level "<<level<<" nbIter "<<nbIter<<" timeout "<<timeout<<" nbThreads "<<nbThreads<<endl;
    fs<<"#<RunId> <timereventid> <timestamp> <currentbestscore>"<<"\n";
    for(int i = 0; i < _runId; i++){
      for(int j = 0; j < _runNbEvents[i]; j++){
	const NrpaStats &s = _timerStats[i][j]; 
	fs<<i<<" "<<j<<" "<<s.date<<" "<<s.bestScore<<" "<<"\n";
      }
    }
    fs.close();
  }
}

template <typename NRPA>
void Stats<NRPA>::setTimers(){
  using namespace chrono;

  int timerEvents[MAX_TIME_EVENTS];
  int i = 0; 
  if(_timerStatsOn){ /* prepare timer events for time stats and timeout */
    for(i = 0; i < MAX_TIME_EVENTS; i++){
      timerEvents[i] = 1<<i; 
      if(_timeout > 0 && timerEvents[i] >= _timeout){
	timerEvents[i] = _timeout;
	break; 
      }
    }
  }
  else{ 
    timerEvents[i] = _timeout; 
  }
  _lastEventIdx = i; 

  _startTime = system_clock::now();
  _thread = new thread([this, timerEvents] {
      unique_lock<mutex> lk(_doneMutex);
      for(int i = 0; !_done && i <= _lastEventIdx; i++){
  	_doneCond.wait_until(lk, _startTime + seconds(timerEvents[i]));
	float d =  duration_cast<seconds>(system_clock::now() - _startTime).count(); 
	//	cout<<"TIMER EVENT at "<<d<<" last idx "<<_lastEventIdx<<" i "<<i<<endl;
	recordTimerStats(_nrpa->_nrpa[_nrpa->_startLevel]);
	if(i == _lastEventIdx) {
	  _done = true; 
	  cout<<"Timeout! "<<endl; 
	}
      }
    });
}



template <typename NRPA>
Stats<NRPA>::NrpaStats::NrpaStats(): bestScore(numeric_limits<double>::lowest()),
				     iter(0), 
				     date(0){}


#endif //STATS_HPP
