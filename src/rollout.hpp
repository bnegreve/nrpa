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

template <int PL>
class Rollout{

  template <int PL_>
  friend std::ostream &operator<<(std::ostream &, const Rollout<PL_> &);

  template <int PL_>
  friend std::istream &operator>>(std::istream &, Rollout<PL_> &); 

public: 

  Rollout(int level = 0); 

    /* Create a rollout object from rollout data */ 
  Rollout(int *rolloutData, int length, int level, double score);

    /* store rollout into filename */
  void store(const string &filename, const string &lockfile) const; 

    /* load rollout from file*/
  void load(const string &filename, const string &lockfile);

  void compareAndSwap(const string &filename, const string &lockfile); 

  //  inline const int *data() const { return &front(); }
  inline int length() const { return _length;  }
  inline double score() const { return _score; }
  inline int level() const { return _level; }
  inline int move(int step) const { assert(step < length()); return _moves[step]; }

  inline void setScore(int score) { _score = score; }

  inline void addMove(int code){ _moves[_length++] = code; }

  inline std::vector<int> *moves() { assert(false);return 0; }

  inline void addAllMoves(int *moves, int length){ 
    std::copy(moves, moves + length, _moves);
    _length = length; 
  }

  inline void reset(){
    _score = std::numeric_limits<double>::lowest();
    _length = 0; 
  }

private:
  int _moves[PL]; 
  int _length; 
  int _level; 
  double _score; 
}; 


template <int PL>
std::ostream &operator<<(std::ostream &, const Rollout<PL> &);

template <int PL>
std::istream &operator>>(std::istream &is, Rollout<PL> &r); 

template <int PL, int LM>
class LegalMoves{
public:


  inline void setNbSteps(int step){
    _nbSteps = step; 
  }

  inline void addStep(){
    _nbSteps++;
  }


  inline void setNbMoves(int step, int size){
    assert(step < _nbSteps); 
    _nbMoves[step] = size; 
  }

  inline void addMove(int step, int move){
    assert(step < _nbSteps);
    int idx = _nbMoves[step]++;
    _moves[step][idx] = move; 
  }

  inline void setMove(int step, int idx, int move){
    assert(step < _nbSteps);
    assert(idx < _nbMoves[step]); 
    _moves[step][idx] = move; 
  }


  inline int nbMoves(int step) const{
    assert(step < _nbSteps); 
    return _nbMoves[step]; 
  }


  inline int move(int step, int idx) const{
    assert(step < _nbSteps && idx < _nbMoves[step]);
    return _moves[step][idx]; 
  }

  inline void copy(const LegalMoves &lm){
    _nbSteps = lm._nbSteps; 
    std::copy(lm._nbMoves, lm._nbMoves + _nbSteps, _nbMoves); 
    for(int i = 0; i < _nbSteps; i++)
      std::copy(lm._moves[i], lm._moves[i] + lm._nbMoves[i], _moves[i]);
  }

  inline void operator=(const LegalMoves &lm){
    this->copy(lm); 
  }

  inline void resetStep(){
    _nbSteps = 0; 
  }

  inline void reset(){
    for(int i = 0; i < _nbSteps; i++)
      _nbMoves[i] = 0; 
    _nbSteps = 0; 
  }

private:
  int _nbSteps; 
  int _nbMoves[PL]; 
  int _moves[PL][LM];

};


#include "rollout.inl"


#endif
