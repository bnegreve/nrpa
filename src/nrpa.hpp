// nrpa.cpp
// Made by Benjamin Negrevergne 
// Started on <2017-02-22 Wed>

#include <map>
#include <limits>
#include <rollout.hpp>


class Nrpa{

public:

  static const int N = 10; 

  vector<Policy> policies; // one for each level 
  vector<Rollout> bestRollouts; // one for each level (necessary?)

  double _bestScoreNRPA = numeric_limits<double>::lowest(); 

  clock_t startClockNRPA, stopClockNRPA;
  double nextTimeNRPA = 0.01;
  int indexTimeNRPA;
  int indexSearch;
  float valueAfterTimeNRPA [10000] [100];
  float sumValueAfterTimeNRPA [100];
  int nbSearchTimeNRPA [100];


  double nrpa(int level, Policy & pol) {
    //    scoreBestRollout [level] = -DBL_MAX;
    if (level == 0) {
      return playoutNRPA (pol); // TODO !
    }
    else {
      policies [level] = pol;
      
      Policy &policy = policy[level]; 
      Rollout &bestRollout = bestRollouts[level]; 

      int last = 0;
      for (int i = 0; i < N; i++) {
	double score = nrpa (level - 1, policy);
	if (score >= bestRollout.score()) {

	  last = i;

	  bestRollout = bestRollout[level - 1]; 

	  if (level > 2) {
	    for (int t = 0; t < level - 1; t++)
	      fprintf (stderr, "\t");
	    fprintf(stderr,"Level : %d, N:%d, score : %f\n", level, i, bestRollout.score());
	  }
	}

	ALPHA = alpha;
	ALPHA = getAlpha (last, i);

	adaptLevel (bestRollouts [level], level, policies [level]);

	if (stopOnTime && (indexTimeNRPA > nbTimesNRPA))
	  return bestRollout.score();
      }
      return bestRollout.score();
    }
  }

  void updatePolicy(const Rollout &rollout){
    //TODO 

  }

  double playout (Policy & pol) {
    int nbMoves = 0;
    Move moves [MaxLegalMoves];
    double probaMove [MaxLegalMoves];
    Board board;
 
    cout<<"Playout start from step "<<board.length<<endl;

    while (true) {

      if (board.terminal ()) {

	double score = board.score(); 
	Rollout r; // create rollout for level 0

	r.setScore(score); 
	r.addAllMoves(board.rollout, board.length); 

	if( score > _bestScoreNRPA ) {
	  _bestScoreNRPA = score; 
	  bestBoard = board;
	  board.print (stderr);
	}

	/* Timining .. I guess */ 
	stopClockNRPA = clock ();
	double time = ((double)(stopClockNRPA - startClockNRPA)) / CLOCKS_PER_SEC;
	if (time > nextTimeNRPA) {
	  while (time > 2 * nextTimeNRPA) {
	    indexTimeNRPA++;
	    nextTimeNRPA *= 2;
	  }
	  valueAfterTimeNRPA [indexSearch] [indexTimeNRPA] = _bestScoreNRPA;
	  sumValueAfterTimeNRPA [indexTimeNRPA] += _bestScoreNRPA;
	  nbSearchTimeNRPA [indexTimeNRPA]++;
	  indexTimeNRPA++;
	  nextTimeNRPA *= 2;
	}
	/* .. */ 
	
	cout<<"Found terminal state at "<<board.length<<" score = "<<score<<endl;
	return score; 
      }

      /* board is at a non terminal step */
      int step = board.length; 
      
      /* TODO Fix useless copy */ 
      nbMoves = board.legalMoves (moves);

      vector<int> moveCodes(nbMoves); 
      vector<double> moveProbs(nbMoves); 
      double sum = 0; 

      for (int i = 0; i < nbMoves; i++) {
	moveCodes[i] = board.code(*move);
	moveProbs[i] = math::exp(pol.prob(moveCodes[i]));
	sum += moveProbs[i]; 
      }

      /* Pick a move randomly */
      double r = (rand () / (RAND_MAX + 1.0)) * sum;
      int j = 0;
      double s = moveProbs[0];
      while (s < r) { 
	j++;
	s += moveProbs[j];
      }

      bestRollout[0].addMove(moveCodes[j]);
      assert(bestRollout[0].size() == step); // SURE? 
      board.play (moves [j]);
    }
    return 0.0;  
  }
  
}; 



class Policy{

public:


  inline double prob(int code) const {
    auto hit = _probs.find(code);
    assert(hit != _probs.end()); 
    return hit->second; 
  }

  inline void setProb(int code, double prob){
    _probs[code] = prob; 
  }

  inline void updateProb(int code, double delta){
    auto hit = _probs.find(code);
    assert(hit != _probs.end()); 
    hit->second += delta; 
  }

  inline void print(std::ostream &os) const{
    os<<"Policy: "<<endl; 
    for(auto it = _probs.begin(); it != _probs.end(); ++it){
      os<<"\tCode : "<<it->first<<" prob: "<<it->second<<endl;
    }
    os<<"End of Policy"<<endl; 
  }

private: 
  std::Map<int, double> _probs; 
};

