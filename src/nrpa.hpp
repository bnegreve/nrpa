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




template <typename B, typename M,
	  typename H = std::hash<M>,
	  typename EQ = std::equal_to<M>>
class Nrpa{

public:

  static const int N = 10; 
  static constexpr double ALPHA = 1.0; 

  int _level; 
  int _maxLegalMoves;
  int _maxPlayoutLength; 
  double _bestScore; 
  Policy _policy; 
  Rollout _bestRollout; 
  vector<M> _bestRolloutMoves;  
  vector<vector<int>> _legalMoveCodes; // codes of every legal moves at step i
  //B _bestBoard; 
  


public:

  Nrpa(int level, int maxLegalMoves, int maxPlayoutLength);
  double run(); 
  double playout ();
  void updatePolicy(const Rollout &rollout); 
  inline double bestScore(){ return _bestScore; }


  /* Swap all data between parent and child Nrpm object (saves the
   * trouble of copying all the data Beware, this is not a real "swap"
   * after the call, the child object may be inconsistant state */
  inline void swap(Nrpa *sub){
    assert(_maxPlayoutLength == sub->_maxPlayoutLength); 
    assert(_maxLegalMoves == sub->_maxLegalMoves); 
    _bestScore = sub->_bestScore;
    _bestRollout.swap(&sub->_bestRollout); 
    _bestRolloutMoves.swap(sub->_bestRolloutMoves); 
    _legalMoveCodes.swap(sub->_legalMoveCodes); 
  }

  inline void reset(){
    _bestScore = std::numeric_limits<double>::lowest(); 
    _policy.reset(); 
    _bestRollout.clear();
    _bestRolloutMoves.clear(); 
    for(auto lm = _legalMoveCodes.begin(); lm != _legalMoveCodes.end(); ++lm)
      lm->clear(); 
    //    _bestBoard = B(); 

    
  }

  inline void printPolicy(std::ostream &os) const{
    _policy.print(os); 
  }
  
}; 

template <typename B,typename  M,typename  H,typename EQ>
Nrpa<B,M,H,EQ>::Nrpa(int level, int maxLegalMoves, int maxPlayoutLength):
  _level(level),
  _maxLegalMoves(maxLegalMoves),
  _maxPlayoutLength(maxPlayoutLength), 
  _bestScore(std::numeric_limits<double>::lowest()){
  
  _legalMoveCodes.resize(maxPlayoutLength, std::vector<int>(0));
  for(auto lm = _legalMoveCodes.begin(); lm != _legalMoveCodes.end(); ++lm)
    lm->reserve(maxLegalMoves); 
  }

template <typename B,typename  M,typename  H,typename EQ>
double Nrpa<B,M,H,EQ>::run(){
  using namespace std; 

  if (_level == 0) {
    return playout(); 
  }
  else {

    int last = 0;

    Nrpa sub(_level - 1, _maxLegalMoves, _maxPlayoutLength); 
    for (int i = 0; i < N; i++) {
      sub.reset(); 
      sub._policy = _policy; //TODO Pass as an argument to run
      sub.run();

      if (sub.bestScore() >= _bestScore) {
	last = i;
	this->swap(&sub); // move data of sub nrpa to parent nrpa (saves the copy)
	
	if (_level > 2) {
	  for (int t = 0; t < _level - 1; t++)
	    fprintf (stderr, "\t");
	  fprintf(stderr,"Level : %d, N:%d, score : %f\n", _level, i, _bestRollout.score());
	}

      }

      /* Update policy only a new best sequence is found. */ 
      updatePolicy(_bestRollout); 

    }
    return _bestScore;
  }
}

template <typename B,typename  M,typename  H,typename EQ>
void Nrpa<B,M,H,EQ>::updatePolicy(const Rollout &rollout){

  //  assert(rollout.length() <= _legalMoveCodes.size()); 

  Policy newPol = _policy; //TODO remove this useless copy!!

  for(int step = 0; step < rollout.length(); step++){
    int move = rollout.move(step); 
    newPol.updateProb(move, ALPHA); 

    double z = 0.; 
    const std::vector<int> &legalMoves = _legalMoveCodes[step]; 
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
    int step = board.length; 

    if (board.terminal ()) {

      double score = board.score(); 

      _bestRollout.setScore(score);
      _bestRollout.moves()->resize(board.length); // free up overallocated space
      _bestScore = board.score(); 


      // if( score > _bestScoreNRPA ) {
      // 	_bestScoreNRPA = score; 
      // 	_bestBoard = board;
      // 	board.print (stderr);
      // }
	
      return score; 
    }

    /* board is at a non terminal step ... */


    /* Get all legal moves for this step */ 
    vector<M> legalMoves(board.maxLegalMoves()); 
    int nbMoves = board.legalMoves(&legalMoves.front());
    legalMoves.resize(nbMoves); 

    /* store legal move codes */
    _legalMoveCodes[step].resize(nbMoves); 
    for(int i = 0; i < legalMoves.size(); i++){
            static H hasher; 
	    _legalMoveCodes[step][i] = hasher(legalMoves[i]);
	    // #ifdef CHECK_COLLISION
	    // 	    /* If each move has a unique code, this will never happen,
	    // 	       but for games with a very large number of moves this
	    // 	       may happen, in doubt, define CHECK_COLLISION to find
	    // 	       out. */
	    // 	    sort( _legalMoveCodes[i].begin(), _legalMoveCodes[i].end());
	    // 	    assert(adjacent_find(_legalMoveCodes[i].begin(),
	    // 				 _legalMoveCodes[i].end()) == _legalMoveCodes[i].end());
	    // #endif 
    }

    /* Compute probs for each move (code) */ 
    vector<double> moveProbs(nbMoves); 
    double sum = 0; 
    for (int i = 0; i < nbMoves; i++) {
      double prob = exp(_policy.prob( _legalMoveCodes[step][i] )); // TODO save exp computation
      moveProbs[i] = prob;
      sum += prob; 
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
    _bestRolloutMoves.push_back(legalMoves[j]); // this copy is required because move.play is non const
    _bestRollout.addMove(_legalMoveCodes[step][j]); 
    board.play(legalMoves[j]); 
  }
  return 0.0;  
}


#endif 
