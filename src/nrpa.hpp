// nrpa.hpp
// Made by Benjamin Negrevergne 
// Started on <2017-02-22 Wed>

#ifndef NRPA_HPP
#define NRPA_HPP

#include <unordered_map>
#include <cmath>
#include <limits>
#include "rollout.hpp"
#include "movemap.hpp"
#include "policy.hpp"

/* Old constants kepts for compatibility with old game file. Their
 * values are set to old defaults, they have no effect on the nrpa
 * algorithm, but they might have an impact on the old game code. 
 */
extern const int MaxLevel; 
extern double k;
extern float constante;
extern float kAMAF;
extern double minNorm, maxNorm;
extern int SizeBeam [];
extern int levelPrint; 
extern int startLearning;



/* Pool (for NRPA objects) */
template <typename T>
struct Pool{
  inline T *get(){
    if(available.size() > 0){
      T *back = available.back();
      available.pop_back();
      return back; 
    }
    else
      return new T(); 
  }

  inline void release(T *t){
    available.push_back(t); 
  }
  std::vector<T *> available;
  int numactive = 0; 
};

template <typename B, typename M, int L, int PL, int LM>
class Nrpa {

public:

  static const int N = 10; 
  static constexpr double ALPHA = 1.0; 

  double _bestScore; 
  Policy _policy[L]; 
  Rollout _bestRollout[L]; 
  vector<M> _bestRolloutMoves[L];  
  int _legalMoveCodes[L][PL][LM]; // codes of every legal moves at step i
  int _legalMoveCodeLen[L][PL]; 
  //B _bestBoard; 
  


public:

  double run(int level = 4); 
  double run(int level, Policy *policy); 
  double playout (const Policy &policy);
  void updatePolicy(int level, Policy *policy); 
  inline double bestScore(){ return _bestScore; }
  
}; 

template <typename B,typename  M, int L, int PL, int LM>
double Nrpa<B,M,L,PL,LM>::run(int level){
  Policy policy; 
  return run(level, &policy); 
}

template <typename B,typename  M, int L, int PL, int LM>
double Nrpa<B,M,L,PL,LM>::run(int level, Policy *policy){
  using namespace std; 
  assert(level < L); 

  if (level == 0) {
    return playout(*policy); 
  }
  else {

    _bestRollout[level].reset(); 
    _policy[level] = *policy; 

    for (int i = 0; i < N; i++) {
      double score = run(level - 1, &_policy[level]); 

      if (score >= _bestRollout[level].score()) {
	int length = _bestRollout[level - 1].length(); 
	_bestRollout[level] = _bestRollout[level - 1];
	copy(_legalMoveCodeLen[level - 1], _legalMoveCodeLen[level - 1] + length, _legalMoveCodeLen[level]);

	for(int step = 0; step < length; step++){

	  copy(_legalMoveCodes[level - 1][step],
	       _legalMoveCodes[level - 1][step] + _legalMoveCodeLen[level - 1][step],
	       _legalMoveCodes[level][step]);
	}

      
	
	if (level > 2) {
	  for (int t = 0; t < level - 1; t++)
	    fprintf (stderr, "\t");
	  fprintf(stderr,"Level : %d, N:%d, score : %f\n", level, i, _bestRollout[level].score());
	}
      }

      /* Update policy only a new best sequence is found. */ 
      updatePolicy(level, &_policy[level]); 

    }
    return _bestRollout[level].score(); 
  }
}

template <typename B,typename M, int L, int PL, int LM>
void Nrpa<B,M,L,PL,LM>::updatePolicy(int level, Policy *policy){

  //  assert(rollout.length() <= _legalMoveCodes.size()); 
  using namespace std; 

  static Policy newPol;
  newPol = *policy; //TODO remove this useless copy!!


  int length = _bestRollout[level].length(); 

  for(int step = 0; step < length; step++){
    int code = _bestRollout[level].move(step); 
    newPol.setProb(code, newPol.prob(code) + ALPHA);

    double z = 0.; 
    for(int i = 0; i < _legalMoveCodeLen[level][step]; i++)
      z += exp (policy->prob( _legalMoveCodes[level][step][i] ));
    // cout<<"LEGAL MOVE LEN "<<_legalMoveCodeLen[level][step]<<" "<<level<<endl;
    // for(int i = 0; i < _legalMoveCodeLen[level][step]; i++){
    //   cout<<_legalMoveCodes[level][step][i]<<" "; 
    // }
    // cout<<endl;

    for(int i = 0; i < _legalMoveCodeLen[level][step]; i++){
      int move = _legalMoveCodes[level][step][i]; 
      newPol.setProb(move, newPol.prob(move) - ALPHA * exp (policy->prob(move)) / z );
    }
    
    //    newPol.print(std::cout);
    //    exit(1); 



  }
  // for(int i = 0; i < _bestRollout.length(); i++)
  //   for(int j = 0; j < _legalMoveCodes[i].size(); j++)
  //     _policy.setProb(_legalMoveCodes[i][j], newPol.prob(_legalMoveCodes[i][j])); 

  *policy = newPol; 

  // cout<<"POLICY "<<endl;
  // policy->print(cout);
  //  exit(1); 


}

  
/* This code contains number of copies which are required to work with
 * standard Board class, so beware before optimizing.  */ 

template <typename B,typename  M, int L, int PL, int LM>
double Nrpa<B,M,L,PL,LM>::playout (const Policy &policy) {

  _bestRollout[0].reset(); 

  using namespace std; 

  B board; 

  while (true) {

    if (board.terminal ()) {
      double score = board.score(); 
      _bestRollout[0].setScore(score);
      return score; 
    }

    /* board is at a non terminal step, make a new move ... */
    int step = board.length; 

    /* Get all legal moves for this step */ 
    M moves [LM];
    int nbMoves = board.legalMoves (moves);
    _legalMoveCodeLen[0][step] = nbMoves; 

    double moveProbs [LM];

    for (int i = 0; i < nbMoves; i++) {
      int c = board.code (moves [i]);
      moveProbs [i] = exp (policy.prob(c));
      _legalMoveCodes[0][step][i] = c;
    }

    double sum = moveProbs[0]; 
    for (int i = 1; i < nbMoves; i++) {
      sum += moveProbs[i]; 
    }


    /* Pick a move randomly according to the policy distribution */
    double r = (rand () / (RAND_MAX + 1.0)) * sum;
    int j = 0;
    double s = moveProbs[0];
    while (s < r) { 
      j++;
      s += moveProbs[j];
    }

    /* Store move, movecode, and actually play the move */
    assert(step == _bestRollout[0].length()); 
    _bestRollout[0].addMove(_legalMoveCodes[0][step][j]); 
    board.play(moves[j]); 
  }
  return 0.0;  
}


#endif 
