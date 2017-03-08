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


template <typename B, typename M, int L, int PL, int LM>
class Nrpa {

public:

  static const int N = 10; 
  static constexpr double ALPHA = 1.0; 

  /* Data structures preallocated for each  Nrpa recursive call (i.e.) one per level. */ 
  struct NrpaData{

    double bestScore; 
    Policy policy; 
    Rollout<PL> bestRollout; 
    vector<M> bestRolloutMoves;  
    LegalMoves<PL, LM> legalMoveCodes;

  }; 

  NrpaData _nrpa[L]; 

public:

  double run(int level = 4); 
  double run(int level, const Policy &policy); 
  double playout (const Policy &policy);
  void updatePolicy(int level, Policy *policy); 
  
}; 

template <typename B,typename  M, int L, int PL, int LM>
double Nrpa<B,M,L,PL,LM>::run(int level){
  Policy policy; 
  return run(level, policy); 
}

template <typename B,typename  M, int L, int PL, int LM>
double Nrpa<B,M,L,PL,LM>::run(int level, const Policy &policy){
  using namespace std; 
  assert(level < L); 

  if (level == 0) {
    return playout(policy); 
  }
  else {

    _nrpa[level].bestRollout.reset(); 
    _nrpa[level].policy = policy; 

    for (int i = 0; i < N; i++) {
      double score = run(level - 1, _nrpa[level].policy); 

      if (score >= _nrpa[level].bestRollout.score()) {
	int length = _nrpa[level - 1].bestRollout.length(); 
	_nrpa[level].bestRollout = _nrpa[level - 1].bestRollout;
	_nrpa[level].legalMoveCodes = _nrpa[level - 1].legalMoveCodes; 
	
	if (level > 2) {
	  for (int t = 0; t < level - 1; t++)
	    fprintf (stderr, "\t");
	  fprintf(stderr,"Level : %d, N:%d, score : %f\n", level, i, _nrpa[level].bestRollout.score());
	}
      }

      /* Update policy only a new best sequence is found. */ 
      updatePolicy(level, &_nrpa[level].policy); 

    }
    return _nrpa[level].bestRollout.score(); 
  }
}

template <typename B,typename  M, int L, int PL, int LM>
double Nrpa<B,M,L,PL,LM>::playout (const Policy &policy) {
  using namespace std; 
  
  B board; 

  _nrpa[0].bestRollout.reset(); 
  _nrpa[0].legalMoveCodes.setNbSteps(0); 

  while(! board.terminal ()) {

    /* board is at a non terminal step, make a new move ... */
    int step = board.length; 

    /* Get all legal moves for this step */ 
    M moves [LM];
    int nbMoves = board.legalMoves (moves);

    double moveProbs [LM];

    _nrpa[0].legalMoveCodes.setNbSteps(step + 1); 
    _nrpa[0].legalMoveCodes.setNbMoves(step, nbMoves); 
    for (int i = 0; i < nbMoves; i++) {
      int c = board.code (moves [i]);
      moveProbs [i] = exp (policy.prob(c));
      _nrpa[0].legalMoveCodes.setMove(step, i, c); 
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
    assert(step == _nrpa[0].bestRollout.length()); 
    _nrpa[0].bestRollout.addMove(_nrpa[0].legalMoveCodes.move(step, j)); 
    board.play(moves[j]); 
  }

  /* Board is terminal */ 

  double score = board.score(); 
  _nrpa[0].bestRollout.setScore(score);
  return score; 
  
}


template <typename B,typename M, int L, int PL, int LM>
void Nrpa<B,M,L,PL,LM>::updatePolicy(int level, Policy *policy){

  //  assert(rollout.length() <= _nrpa[level].legalMoveCodes.size()); 
  using namespace std; 

  static Policy newPol;
  int length = _nrpa[level].bestRollout.length(); 

  //  newPol = *policy; //complete copy is not necessary

  /* Copy data for the legal moves only into a new policy */ 
  for (int step = 0; step < length; step++) 
    for (int i = 0; i < _nrpa[level].legalMoveCodes.nbMoves(step);  i++){
      newPol.setProb (_nrpa[level].legalMoveCodes.move(step, i), policy->prob (_nrpa[level].legalMoveCodes.move(step,i)));
    }

  for(int step = 0; step < length; step++){
    int code = _nrpa[level].bestRollout.move(step); 
    newPol.setProb(code, newPol.prob(code) + ALPHA);

    double z = 0.; 
    for(int i = 0; i < _nrpa[level].legalMoveCodes.nbMoves(step); i++)
      z += exp (policy->prob( _nrpa[level].legalMoveCodes.move(step,i) ));

    for(int i = 0; i < _nrpa[level].legalMoveCodes.nbMoves(step); i++){
      int move = _nrpa[level].legalMoveCodes.move(step, i); 
      newPol.updateProb(move, - ALPHA * exp (policy->prob(move)) / z );
    }

  }

  /* Copy updated data back into the policy */ 
  for (int step = 0; step < length; step++) 
    for (int j = 0; j < _nrpa[level].legalMoveCodes.nbMoves(step); j++)
      policy->setProb (_nrpa[level].legalMoveCodes.move(step, j), newPol.prob(_nrpa[level].legalMoveCodes.move(step,j) ));

  //  *policy = newPol; 

}


#endif 
