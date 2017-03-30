#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <functional>
#include <thread>
#include <future>
#include <queue>
#include <iostream>
#include <cassert> 
#include <time.h>
#include <chrono>

//#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <sched.h>


using namespace std; 
class ThreadPool{
  static const int MAX_THREADS = 128; 
  typedef function<int()> FunctionType; 
  typedef pair< promise<int> *, FunctionType > TaskHandler; 

  atomic_bool _done; 
  queue< TaskHandler > _tasks; 
  int _nbThreads; 
  mutex _mutex;
  thread *_threads[MAX_THREADS]; 
  int _numTasks[MAX_THREADS] = {0}; 

  /* clock */ 
  typedef chrono::high_resolution_clock clock; 
  clock::time_point _startTime;
  clock::duration _workTime[MAX_THREADS]; 

  bool _threadStats; 


  inline void workerThread(int id) {
    //    bindThread(id); 

    while(!_done) {         
      FunctionType f; 
      promise<int> *p; 
      bool gotSomething = false; 
      
      _mutex.lock();
      if(! _tasks.empty() ){

	p = _tasks.front().first;
	f = _tasks.front().second;
	_tasks.pop();
	gotSomething = true; 
      }
      _mutex.unlock(); 

      if(gotSomething){
	clock::time_point start; 
	if(_threadStats){ _numTasks[id]++; start = clock::now(); }

	int ret = f();
	p->set_value( ret ) ; 
	
	if(_threadStats) _workTime[id] += clock::now() - start; 

	delete p; 
      }
      else {
	yield(); 
      }
    }
  }


  inline void yield(){
    this_thread::yield();
  }

public:
  /* Done set to true initially, must call init() */ 
  inline ThreadPool(): _done(true), _nbThreads(-1){}

  inline ~ThreadPool(){
    end();
    printStats(); 
  }


  inline future<int> submit(FunctionType f) {
    promise<int> *p = new promise<int>; 
    future<int> res = p->get_future(); 
    _mutex.lock(); 
    _tasks.push(make_pair(p, f));
    _mutex.unlock(); 
    return res;  
  }

  inline void init(int nbThreads = thread::hardware_concurrency(), bool threadStats = false){
    assert(nbThreads <= MAX_THREADS); 
    _nbThreads = nbThreads; 
    _threadStats = threadStats; 
    _startTime = clock::now(); 
    fill_n(_workTime, _nbThreads, clock::duration::zero()); 

    cout<<"Initializing thread pool with "<<_nbThreads<<" thread(s)."<<endl; 
    _done = false; 
 
    for(int i = 0; i < _nbThreads; i++){
      _threads[i] = new thread( [this, i] { this->workerThread(i); } );

    }

  }

  inline bool initialized() const{
    return _nbThreads != -1; 
  }

  inline void end(){
    if(!_done) {
      _done = true;
      for(int i = 0; i < _nbThreads; i++){
	_threads[i]->join(); 
	delete _threads[i]; 
      }
    }
  }

  inline void printStats(){
    clock::duration totalDuration = clock::now() - _startTime; 
    if(_threadStats){
      for(int i = 0; i < _nbThreads; i++){
	cout<<"Thread "<<i<<" has completed "<<_numTasks[i]<<" task(s) and worked for "
	    <<chrono::duration_cast<chrono::milliseconds>(_workTime[i]).count() / 1000.f<<"sec. ("
	    <<(static_cast<float>(_workTime[i].count()) / totalDuration.count()) * 100<<"% of the time.)\n"; 
      }
    }
    cout<<"Total duration: "<<
      chrono::duration_cast<chrono::milliseconds>(totalDuration).count() / 1000.f<<"s"<<endl;
  }

  inline int nbThreads() const { assert(_nbThreads != -1);  return _nbThreads; }

  inline void bindThread(int cpuId){
    cpu_set_t set;
    CPU_ZERO(&set); 
    CPU_SET(cpuId, &set);  
    sched_setaffinity(0, sizeof(cpu_set_t), &set);
  }

};

#if 0
int main(){
  ThreadPool t;
  future<int> v1 = t.submit( []() -> int { cout<<"poulet1"<<endl; } ); 
  future<int> v2 = t.submit( []() -> int { cout<<"poulet2"<<endl; } ); 
  future<int> v3 = t.submit( []() -> int { cout<<"poulet3"<<endl; return 3; } ); 
  t.init(); 
  cout<<"V3: "<<v3.get()<<endl;

  t.end(); 
  

  cout<<"done!"<<endl;
	    
}
#endif 

#endif //THREADPOOL
