// rollout.inl
// Made by Benjamin Negrevergne 
// Started on <2017-02-22 Wed>

#include <sys/file.h>// lock
#include <cassert>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <limits>

using namespace std; //TODO remove 

template <int PL>
Rollout<PL>::Rollout(int level):_level(level), _score(numeric_limits<double>::lowest()){}

/* Create a rollout object from rollout data */ 
template <int PL>
Rollout<PL>::Rollout(int *rolloutData, int length, int level, double score): _length(length), _level(level), _score(score){
addAllMoves(rolloutData, length); 
}

/* store rollout into filename */
template <int PL>
void Rollout<PL>::store(const string &filename, const string &lockfile) const{
  int fd = open(lockfile.c_str(), O_CREAT); 
  flock(fd, LOCK_SH); 
  std::fstream fs;
  fs.open (filename, std::fstream::out);
  fs<<*this<<"\n"; 
  fs.close();
  flock(fd, LOCK_UN); 
  close(fd); 
}    

/* load rollout from file*/
template <int PL>
void Rollout<PL>::load(const string &filename, const string &lockfile){
  int fd = open(lockfile.c_str(), O_CREAT); 
  flock(fd, LOCK_SH); 
  std::fstream fs;
  fs.open (filename, std::fstream::in);
  fs>>*this; 
  fs.close();
  flock(fd, LOCK_UN); 
  close(fd); 
}

template <int PL>
void Rollout<PL>::compareAndSwap(const string &filename, const string &lockfile){
  // Rollout<PL>best;
  // best.load(filename, lockfile);
  // if(_score > best._score || best.size() == 0){
  //   //	    cout<<best.size() << " " <<best._level <<" " <<_level<<endl; 
  //   assert(best.size() == 0 || best._level == _level);
  //   this->store(filename, lockfile); 
  // }
  // else
  //   *this = best;
}

template <int PL>
ostream &operator<<(ostream &os, const Rollout<PL> &r){
  os<<r.length()<<" "; 
  os<<r._level<<" "; 
  os<<r._score<<" "; 
  for(int i = 0; i < r._length; i++)
    os<<r._moves[i]<<" "; 
  //  if(r.length() > 0) os<<r.back();
  return os; 
}

template <int PL>
istream &operator>>(istream &is, Rollout<PL> &r){
  int length;
  is>>std::skipws; 
  is>>length;
  is>>r._level; 
  is>>r._score; 
  r.reset(); 
  for(int i = 0; i < length; i++){
    is>>r._moves[i];
  }
  if(is.eof()){
    cerr<<"Warning, cannot read Rollout from file"<<endl;
    r = Rollout<PL>(); 
  }
}

