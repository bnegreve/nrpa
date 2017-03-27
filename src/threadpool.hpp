#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <functional>
#include <thread>
#include <future>
#include <queue>
#include <iostream>
#include <cassert> 

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

  inline void workerThread(int id) {

    bindThread(id); 

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
	int ret = f();
	p->set_value( ret ) ; 
	delete p; 
      }
      else {
	this_thread::yield();
      }
    }
  }


public:
  /* Done set to true initially, must call init() */ 
  inline ThreadPool(): _done(true), _nbThreads(-1){}

  inline ~ThreadPool(){
    end();
  }


  inline future<int> submit(FunctionType f) {
    promise<int> *p = new promise<int>; 
    future<int> res = p->get_future(); 
    _mutex.lock(); 
    _tasks.push(make_pair(p, f));
    _mutex.unlock(); 
    return res;  
  }

  inline void init(int nbThreads = thread::hardware_concurrency()){
    assert(nbThreads <= MAX_THREADS); 
    _nbThreads = nbThreads; 
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
