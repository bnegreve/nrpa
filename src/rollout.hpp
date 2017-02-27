// rollout.hpp
// Made by Benjamin Negrevergne 
// Started on <2017-02-22 Wed>

#ifndef ROLLOUT_HPP
#define ROLLOUT_HPP

#include <vector>
#include <iostream>
#include <cassert>

using std::string;
using std::vector; 

class Rollout : vector<int> {

  friend std::ostream &operator<<(std::ostream &, const Rollout &);
  friend std::istream &operator>>(std::istream &is, Rollout &r); 

public: 

  Rollout(int level = 0); 

    /* Create a rollout object from rollout data */ 
  Rollout(int *rolloutData, int length, int level, double score);

    /* store rollout into filename */
  void store(const string &filename, const string &lockfile) const; 

    /* load rollout from file*/
  void load(const string &filename, const string &lockfile);

  void compareAndSwap(const string &filename, const string &lockfile); 

  inline const int *data() const { return &front(); }
  inline int length() const { return size(); }
  inline double score() const { return _score; }
  inline int level() const { return _level; }
  inline int move(int step) const { assert(step < length()); return (*this)[step]; }

  inline int setScore(int score) { _score = score; }

  inline void addMove(int code){ this->push_back(code); }
  inline void addAllMoves(const std::vector<int> &rolloutCodes){
    copy(rolloutCodes.begin(), rolloutCodes.end(), this->begin()); }
  inline void addAllMoves(int *rolloutCodes, int length){ this->resize(length); copy(rolloutCodes, rolloutCodes + length, this->begin()); }

  /* Store all legal move codes for step */
  inline void setLegalMoves(int step, const std::vector<int> &legalMoves){
    assert(step == _legalMoves.size()); 
    _legalMoves.resize(step + 1, legalMoves);
  }

  inline void setLegalMoves(int step, int *legalMoves, int nbMoves){
    assert(step == _legalMoves.size()); 
    _legalMoves.resize(step + 1, std::vector<int>(0));
    _legalMoves[step].resize(nbMoves);
    copy(legalMoves, legalMoves + nbMoves, _legalMoves[step].begin());
  }
  
  /* Required to avoid one useless copy */ 
  inline std::vector<int> *legalMoveStorage(int step, int maxMoves){
    assert(step == _legalMoves.size()); 
    _legalMoves.resize(step + 1, std::vector<int>(0));
    _legalMoves[step].resize(maxMoves); 
    return &_legalMoves[step]; 
  }

  inline const std::vector<int> &legalMoves(int step) const{
    assert(step < _legalMoves.size());
    return _legalMoves[step]; 
  }
  
private:
    int _level; 
    double _score; 
    std::vector<std::vector<int>> _legalMoves;  // possible move codes for stage i
}; 


std::ostream &operator<<(std::ostream &, const Rollout &);
std::istream &operator>>(std::istream &is, Rollout &r); 



#endif
