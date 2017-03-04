// movemap.hpp
// Made by Benjamin Negrevergne 
// Started on <2017-02-24 Fri>

#ifndef MOVEMAP_HPP
#define MOVEMAP_HPP

#include <unordered_map>
#include <vector>
#include <iostream>
#include <cassert>


/* Maps move to code and conversly */
/* See movemap.cpp for usage examples */
template <typename M, typename H = std::hash<M>, typename EQ = std::equal_to<M>>
class MoveMap{


  template <typename M_, typename H_, typename EQ_>
  friend std::ostream &operator<< (std::ostream &, const MoveMap<M_,H_,EQ_> &);

public:

  MoveMap();

  ~MoveMap();
  
  /* get move from code */ 
  const M &move(int code) const; 

  /* get code from move, fail in DEBUG mode if move has not been registered */ 
  int code(const M &move) const;

  /* get codes for many moves, fail in DEBUG mode if any move has not been registered */ 
  void codes(const std::vector<const M> &moves, std::vector<int> *moveCodes) const; 

  /* same as before, but for a set move provided as a C array */ 
  void codes(const M *moves, int nbMoves, std::vector<int> *moveCodes) const; 


  /* Add a new move */ 
  int registerMove(const M &move); 

private:

  struct PtrHasher {
    inline size_t operator()(const M *move) const{
      static H hasher; 
      return hasher(*move); 
    }
  };

  struct PtrEqOp{
    inline bool operator()(const M *m1, const M *m2) const{
      static EQ eq; 
      return eq(*m1, *m2); 
    }
  };

  std::vector<const M *> _moves;  // maps codes to moves
  std::unordered_map<const M *, int, PtrHasher, PtrEqOp> _codes; // maps moves to codes
};


/* Template definitions */
template <typename M, typename H, typename EQ>
MoveMap<M,H,EQ>::MoveMap():_moves(), _codes(){
  // _moves.reserve(1024*1024);
  // _codes.max_load_factor(0.5); 
  
}

template <typename M, typename H, typename EQ>
MoveMap<M,H,EQ>::~MoveMap(){
  for(auto m = _moves.begin(); m != _moves.end(); ++m){
    delete *m; 
  }
}

template <typename M, typename H, typename EQ>
const M &MoveMap<M,H,EQ>::move(int code) const {
    assert(code < _moves.size()); 
    return *_moves[ code ]; 
  }

template <typename M, typename H, typename EQ>
int MoveMap<M,H,EQ>::code(const M &move) const{
  auto hit = _codes.find(&move);
  assert(hit != _codes.end()); 
  return hit->second; 
}

template <typename M, typename H, typename EQ>
void MoveMap<M,H,EQ>::codes(const std::vector<const M> &moves, std::vector<int> *moveCodes) const{
  assert(moveCodes->empty()); 
  moveCodes->reserve(moves.size()); 
  for(auto move = moves.begin(); move != moves.end(); ++move)
    moveCodes->push_back(code(*move)); 
}

template <typename M, typename H, typename EQ>
void MoveMap<M,H,EQ>::codes(const M *moves, int nbMoves, std::vector<int> *moveCodes) const{ 
  assert(moveCodes->empty()); 
  moveCodes->reserve(nbMoves); 
  const M *end = moves + nbMoves;
  for(const M *move = moves; move != end; move++)
    moveCodes->push_back(code(*move)); 
}



template <typename M, typename H, typename EQ>
int MoveMap<M,H,EQ>::registerMove(const M &move){
  // if(_codes.size() % 1000 == 0) 
  //   std::cout<<"MAP SIZE "<<_codes.size() / 1E3<<"K "
  // 	     <<" "<< sizeof(M) * _codes.size() / 1E6
  // 	     <<" load factor "<<_codes.load_factor()
  // 	     <<std::endl;
  auto hit = _codes.find(&move);

  if(hit == _codes.end()){
    /* this is a new move */ 
    int newCode = _moves.size(); 
    _moves.push_back(new M(move)); 
    _codes[ _moves.back() ] = newCode; 
    return newCode; 
  }
  else{
    return hit->second; 
  }
}

template <typename M, typename H, typename EQ>
std::ostream &operator<<(std::ostream &os, const MoveMap<M, H, EQ> &m){
  for(int i = 0; i < m._moves.size(); i++){
    os<<"Code : "<<i<<" "<<" Move: "<<(*m._moves[i])<<std::endl;
  }
  return os; 
}

#endif 
