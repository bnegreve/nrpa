// nrpa.hpp
// Made by Benjamin Negrevergne 
// Started on <2017-02-22 Wed>

#ifndef NRPA_HPP
#define NRPA_HPP

#include <map>
#include <cmath>
#include <limits>
#include "rollout.hpp"
#include "movemap.hpp"

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



class Policy{

public:

  inline double prob(int code) const {
    auto hit = _probs.find(code);
    //    assert(hit != _probs.end()); 
    if( hit != _probs.end() )
      return hit->second;
    else
      return 0.;
  }

  inline void setProb(int code, double prob){
    _probs[code] = prob; 
  }

  inline double updateProb(int code, double delta){
    auto hit = _probs.find(code);
    assert(hit != _probs.end()); 
    hit->second += delta; 
    return hit->second; 
  }

  inline void print(std::ostream &os) const{
    os<<"Policy: "<<std::endl; 
    for(auto it = _probs.begin(); it != _probs.end(); ++it){
      os<<"\tCode : "<<it->first<<" prob: "<<it->second<<std::endl;
    }
    os<<"End of Policy"<<std::endl; 
  }

private: 
  std::map<int, double> _probs; 
};

template <typename B, typename M,
	  typename H = std::hash<M>,
	  typename EQ = std::equal_to<M>>
class Nrpa{

public:

  static const int N = 10; 

  vector<Policy> policies; // one for each level 
  vector<Rollout> bestRollouts; // one for each level TODO: not used any more. 

  int _startLevel; 
  double _bestScoreNRPA = std::numeric_limits<double>::lowest(); 

  clock_t startClockNRPA, stopClockNRPA;
  double nextTimeNRPA = 0.01;
  int indexTimeNRPA;
  int indexSearch;
  //  float valueAfterTimeNRPA [10000] [100];
  // float sumValueAfterTimeNRPA [100];
  // int nbSearchTimeNRPA [100];
  MoveMap<M, H, EQ> _movemap; 
  B _bestBoard; 

  Nrpa(int startLevel);

  double nrpa(); 

  double nrpa(int level, Policy & pol, Rollout *bestRollout); 
  double playout (const Policy & pol, Rollout *bestRollout);

  void updatePolicy(); 


  inline void printPolicy(std::ostream &os, int level) const{
    assert (level <= _startLevel);
    policies[level].print(os); 
  }
  
}; 

template <typename B,typename  M,typename  H,typename EQ>
Nrpa<B,M,H,EQ>::Nrpa(int startLevel): policies(startLevel + 1),
				      bestRollouts(startLevel+1),
				      _startLevel(startLevel),
				      _movemap(){
  
}

template <typename B,typename  M,typename  H,typename EQ>
double Nrpa<B,M,H,EQ>::nrpa(){
  Policy p;
  Rollout r; 
  double score = nrpa(_startLevel, p, &r);
  std::cout<<r<<std::endl;
  return score; 
  
}

template <typename B,typename  M,typename  H,typename EQ>
double Nrpa<B,M,H,EQ>::nrpa(int level, Policy &pol, Rollout *bestRollout){
  using namespace std; 
  //    scoreBestRollout [level] = -DBL_MAX;

  if (level == 0) {
    return playout (pol, bestRollout); 
  }
  else {
    policies [level] = pol;
    Policy &policy = policies[level]; 

    int last = 0;
    for (int i = 0; i < N; i++) {
      //std::cout<<"New ITER "<<std::endl;
      Rollout rollout; 
      nrpa (level - 1, policy, &rollout);
      if (rollout.score() >= bestRollout->score()) {
	last = i;
	(*bestRollout) = rollout; 
	
	if (level > 2) {
	  double score = bestRollout->score();
	  for (int t = 0; t < level - 1; t++)
	    fprintf (stderr, "\t");
	  fprintf(stderr,"Level : %d, N:%d, score : %f\n", level, i, score);
	}
      }

      //	adaptLevel (bestRollouts [level], level, policies [level]);
      updatePolicy(); 

      // TODO : fix 
      // if (stopOnTime && (indexTimeNRPA > nbTimesNRPA))
      //   return bestRollout.score();
    }
    return bestRollout->score();
  }
}

template <typename B,typename  M,typename  H,typename EQ>
void Nrpa<B,M,H,EQ>::updatePolicy(){
  //TODO 
  
}

template <typename B,typename  M,typename  H,typename EQ>
double Nrpa<B, M, H, EQ>::playout (const Policy & pol, Rollout *rollout) {
  using namespace std; 
  assert(rollout->length() == 0); 
  
  B board; 
 
  while (true) {

    if (board.terminal ()) {

      double score = board.score(); 

      rollout->setScore(score);
      std::vector<int> codes; 

      _movemap.codes(board.rollout, board.length, &codes); 
      // TODO lots of useless copies, at least this one can be avoided easily
      rollout->addAllMoves(codes);  

      if( score > _bestScoreNRPA ) {
	_bestScoreNRPA = score; 
	_bestBoard = board;
	board.print (stderr);
      }
	
      return score; 
    }

    /* board is at a non terminal step */
    int step = board.length; 
      

    /* Get all legal moves for this stage, and register them a code if needed */ 

    /* Compatibility code, a bit redudant but required to work with standard Board class */ 
    std::vector<M> legalMoves(board.maxLegalMoves());
    int nbMoves = board.legalMoves(&legalMoves.front());
    legalMoves.resize(nbMoves); 

    /* Fetch move codes from the movemap */ 
    std::vector<int> moveCodes(nbMoves);
    for(int i = 0; i < nbMoves; i++){
      int code = _movemap.registerMove(legalMoves[i]);
      moveCodes[i] = code; 
    }

    /* Compute probs for each move (code) */ 
    vector<double> moveProbs(nbMoves); 
    double sum = 0; 

    for (int i = 0; i < nbMoves; i++) {
      double prob = exp(pol.prob(moveCodes[i])); 
      moveProbs[i] = prob;
      sum += prob; 
    }

    /* Pick a move randomly */
    double r = (rand () / (RAND_MAX + 1.0)) * sum;
    int j = 0;
    double s = moveProbs[0];
    while (s < r) { 
      j++;
      s += moveProbs[j];
    }

    int newMoveCode = moveCodes[j];
    M newMove = _movemap.move(newMoveCode); // this copy is required because
                                            // move.play is non const
   
    rollout->addMove(newMoveCode); 
    board.play(newMove);
  }
  return 0.0;  
}

#endif 
