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
    if(hit != _probs.end()){
      hit->second += delta;
      return hit->second;
    }
    else{
      _probs[code] = delta;
      return delta; 
    }
  }

  inline void print(std::ostream &os) const{
    os<<"Policy: "<<std::endl; 
    for(auto it = _probs.begin(); it != _probs.end(); ++it){
      os<<"\tCode : "<<it->first<<" prob: "<<it->second<<std::endl;
    }
    os<<"End of Policy"<<std::endl; 
  }

  template <typename X, typename Y, typename Z>
  inline void print(std::ostream &os, const MoveMap<X,Y,Z> &movemap) const{
    os<<"Policy: "<<std::endl; 
    for(auto it = _probs.begin(); it != _probs.end(); ++it){
      os<<"\tCode : "<<movemap.move(it->first).hash<<" prob: "<<it->second<<std::endl;
    }
    os<<"End of Policy"<<std::endl; 
  }

private: 
  std::unordered_map<int, double> _probs; 
};

template <typename B, typename M,
	  typename H = std::hash<M>,
	  typename EQ = std::equal_to<M>>
class Nrpa{

public:

  static const int N = 10; 
  static constexpr double ALPHA = 1.0; 


  Policy _policy; 
  Rollout _bestRollout; 
  
  // vector<M> _bestRolloutMoves;
  // vector<int> _bestRollout; // the codes of the moves of the best rollout
  // vector<vector<int>> _legalMoves; // _legalMoves[i][j] is the code of
				   // the legal move j at step i.


  int _startLevel; 
  double _bestScoreNRPA = std::numeric_limits<double>::lowest(); 

  clock_t startClockNRPA, stopClockNRPA;
  // double nextTimeNRPA = 0.01;
  // int indexTimeNRPA;
  // int indexSearch;
  //  float valueAfterTimeNRPA [10000] [100];
  // float sumValueAfterTimeNRPA [100];
  // int nbSearchTimeNRPA [100];
  MoveMap<M, H, EQ> *_movemap; 
  B _bestBoard; 

  Nrpa(int startLevel);

  double run(); 

  double run(int level); 
  double playout ();

  void updatePolicy(const Rollout &rollout); 


  inline void printPolicy(std::ostream &os, int level) const{
    assert (level <= _startLevel);
    _policy.print(os); 
  }
  
}; 

template <typename B,typename  M,typename  H,typename EQ>
Nrpa<B,M,H,EQ>::Nrpa(int startLevel): _startLevel(startLevel){
  _movemap = new MoveMap<M,H,EQ>(); 
}

template <typename B,typename  M,typename  H,typename EQ>
double Nrpa<B,M,H,EQ>::run(){
  Policy p;
  Rollout r; 
  double score = run(_startLevel);
  std::cout<<r<<std::endl;
  return score; 
  
}

template <typename B,typename  M,typename  H,typename EQ>
double Nrpa<B,M,H,EQ>::run(int level){
  using namespace std; 
  //    scoreBestRollout [level] = -DBL_MAX;

  if (level == 0) {
    return playout(); 
  }
  else {

    int last = 0;

    for (int i = 0; i < N; i++) {
      Nrpa sub(level); 
      sub._policy = _policy;
      sub._movemap = _movemap; //TODO FIX
      sub.run (level - 1);

      if (sub._bestRollout.score() >= _bestRollout.score()) {
	last = i;
	_bestRollout = sub._bestRollout; 
	
	if (level > 2) {
	  for (int t = 0; t < level - 1; t++)
	    fprintf (stderr, "\t");
	  fprintf(stderr,"Level : %d, N:%d, score : %f\n", level, i, _bestRollout.score());
	}

      }

      /* Update policy only a new best sequence is found. */ 
      updatePolicy(_bestRollout); 

      // TODO : fix 
      // if (stopOnTime && (indexTimeNRPA > nbTimesNRPA))
      //   return bestRollout.score();
    }
    return _bestRollout.score();
  }
}

template <typename B,typename  M,typename  H,typename EQ>
void Nrpa<B,M,H,EQ>::updatePolicy(const Rollout &rollout){

  Policy newPol = _policy; //TODO remove this useless copy!!

  for(int step = 0; step < rollout.length(); step++){
    int move = rollout.move(step); 
    newPol.updateProb(move, ALPHA); 

    double z = 0.; 
    const std::vector<int> &legalMoves = rollout.legalMoves(step); 
    for(auto move = legalMoves.begin(); move != legalMoves.end(); ++move)
      z += exp (_policy.prob( *move ));
  
    for(auto move = legalMoves.begin(); move != legalMoves.end(); ++move){
      newPol.updateProb( *move, - ALPHA * (exp (_policy.prob( *move )) / z ));
    }
  }

  _policy = newPol; 
}

  
/* This code contains number of copies which are required to work with
 * standard Board class, so beware before optimizing.  */ 

template <typename B,typename  M,typename  H,typename EQ>
double Nrpa<B, M, H, EQ>::playout () {

  using namespace std; 
  assert(_bestRollout.length() == 0); 
  
  B board; 
 
  while (true) {

    if (board.terminal ()) {

      double score = board.score(); 

      _bestRollout.setScore(score);
      std::vector<int> codes; 

      // TODO copy inside addAllMoves can be avoided by passing the poitner 
      _movemap->codes(board.rollout, board.length, &codes);       
      _bestRollout.setMoves(codes);  

      // if( score > _bestScoreNRPA ) {
      // 	_bestScoreNRPA = score; 
      // 	_bestBoard = board;
      // 	board.print (stderr);
      // }
	
      return score; 
    }

    /* board is at a non terminal step ... */

    int step = board.length; 
      

    /* Get all legal moves for this stage */ 

    vector<M> legalMoves(board.maxLegalMoves()); 
    int nbMoves = board.legalMoves(&legalMoves.front());
    legalMoves.resize(nbMoves); 
    
    /* Fetch the code for each legal moves, and store all legal move
     * codes directly in the Rollout object */ 

    vector<int> *legalMoveCodes = _bestRollout.legalMoveStorage(step, nbMoves); 
    for(int i = 0; i < nbMoves; i++){
      int code = _movemap->registerMove(legalMoves[i]);
      (*legalMoveCodes)[i] = code; 
    }

    /* Compute probs for each move (code) */ 

    vector<double> moveProbs(nbMoves); 
    double sum = 0; 

    for (int i = 0; i < nbMoves; i++) {
      double prob = exp(_policy.prob( (*legalMoveCodes)[i] )); 
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

    int newMoveCode = (*legalMoveCodes)[j];
    M newMove = _movemap->move(newMoveCode); // this copy is required because
                                            // move.play is non const
    board.play(newMove);
  }
  return 0.0;  
}


#endif 
