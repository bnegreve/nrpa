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

  inline int setScore(int score) { _score = score; }

  inline void addMove(int code){ _moves[_length++] = code; }

  // inline void setMoves(std::vector<int> rolloutCodes){
  //   std::vector<int>::swap(rolloutCodes); 
  // }

  inline std::vector<int> *moves() { assert(false);return 0; }

  inline void addAllMoves(int *moves, int length){ 
    std::copy(moves, moves + length, _moves);
    _length = length; 
  }

  // inline void swap(Rollout<PL> *other){
  //   _level = other->_level;
  //   _score = other->_score;
  //   std::vector<int>::swap(*other); 
  // }

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


#include "rollout.inl"


#endif
