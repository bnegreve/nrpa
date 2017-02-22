// rollout.hpp
// Made by Benjamin Negrevergne 
// Started on <2017-02-22 Wed>

#ifndef ROLLOUT_HPP
#define ROLLOUT_HPP

#include <vector>
#include <iostream>

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
  inline int score() const { return _score; }
  inline int level() const { return _level; }
  inline int setScore(int score) { _score = score; }
  inline addMove(int code){ this->push_back(code); }
  inline addAllMoves(int *rolloutData, int length){ this->resize(length); copy(rolloutData, rolloutData + length, this->begin()); }
    
private:
    int _level; 
    double _score; 
}; 


std::ostream &operator<<(std::ostream &, const Rollout &);
std::istream &operator>>(std::istream &is, Rollout &r); 


/* Maps arbitrary move code to an id ranging from 1 to n */
class CodeMap{

public:

  /* Add a move into the code map and return its code */
  inline int addMove(int m){
    _moves.push_back(m);
    return _moves.size() - 1; // move id
  }

  /* get the code of a move */ 
  inline int code(int move) const{
    assert(false); 
  }

  /* get the move corresponding to a code */ 
  inline int move(int code){
    assert(code < _moves.size()); 
    return _moves[code]; 
  }

  inline int numMoves() const{
    return _moves.size(); 
  }

  
private:

  vector<int> _moves; 

}; 




#endif
