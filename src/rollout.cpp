// Rollout.cpp
// Made by Benjamin Negrevergne 
// Started on <2017-02-22 Wed>

#include <sys/file.h>// lock
#include <cassert>
#include <unistd.h>
#include <iostream>
#include <fstream>

#include "rollout.hpp"

using namespace std; 

Rollout::Rollout():_level(0), _score(0){}

/* Create a rollout object from rollout data */ 
Rollout::Rollout(int *rolloutData, int length, int level, double score)
  :std::vector<int>(length), _level(level), _score(score){
  copy(rolloutData, rolloutData + length, this->begin()); 
}

/* store rollout into filename */
void Rollout::store(const string &filename, const string &lockfile) const{
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
void Rollout::load(const string &filename, const string &lockfile){
  int fd = open(lockfile.c_str(), O_CREAT); 
  flock(fd, LOCK_SH); 
  std::fstream fs;
  fs.open (filename, std::fstream::in);
  fs>>*this; 
  fs.close();
  flock(fd, LOCK_UN); 
  close(fd); 
}

void Rollout::compareAndSwap(const string &filename, const string &lockfile){
  Rollout best;
  best.load(filename, lockfile);
  if(_score > best._score || best.size() == 0){
    //	    cout<<best.size() << " " <<best._level <<" " <<_level<<endl; 
    assert(best.size() == 0 || best._level == _level);
    this->store(filename, lockfile); 
  }
  else
    *this = best;
}


ostream &operator<<(ostream &os, const Rollout &r){
  os<<r.length()<<" "; 
  os<<r._level<<" "; 
  os<<r._score<<" "; 
  for(auto it = r.begin(); it != r.end() - 1; it++)
    os<<*it<<" "; 
  if(r.length() > 0) os<<r.back();
  return os; 
}

istream &operator>>(istream &is, Rollout &r){
  int length;
  is>>std::skipws; 
  is>>length;
  is>>r._level; 
  is>>r._score; 
  r.clear();
  r.resize(length); 
  for(int i = 0; i < length; i++){
    is>>r[i];
  }
  if(is.eof()){
    cerr<<"Warning, cannot read Rollout from file"<<endl;
    r = Rollout(); 
  }
}

