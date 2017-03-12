// nrpa.inl
// Made by Benjamin Negrevergne 
// Started on <2017-03-08 Wed>

#include "threadpool.hpp"

template <typename B,typename  M, int L, int PL, int LM>
double Nrpa<B,M,L,PL,LM>::run(int level){
  assert(level < L); 
  Policy policy; 
  double score =  _nrpa[level].run(level, policy);
  cout<<"Maxscore: "<<score<<endl;
}

template <typename B,typename  M, int L, int PL, int LM>
double Nrpa<B,M,L,PL,LM>::NrpaLevel::run(int level, const Policy &policy){
  using namespace std; 
  assert(level < L); 

  if (level == 0) {
    return playout(policy); 
  }

  bestRollout.reset(); 
  levelPolicy = policy; 

  int par = (level == 1); // TODO: cannot be anything but 1 at this time, 
  if(par){
    
    static ThreadPool t; 
    static const int nbThreads = t.nbThreads(); 
    static NrpaLevel *subs = new NrpaLevel[nbThreads]; 
    static future<int> *res = new future<int>[nbThreads]; 

    for(int i = 0; i < N; i+= nbThreads){
      /* Run n thraeds */ 
      for(int j = 0; j < nbThreads; j++){
	res[j] = t.submit([ this, level, j ]() -> int {
	    NrpaLevel *sub = &subs[j]; 
	    sub->run(level - 1, this->levelPolicy); return 1; });
      }


      /* fetch the best rollout among all parallel runs (if any better than before) */ 
      int best = -1; 
      double bestScore = bestRollout.score();// numeric_limits<double>::lowest(); 
      for(int j = 0; j < nbThreads; j++){
      	res[j].wait();
      	if(subs[j].bestRollout.score() >= bestScore){
      	  bestScore = subs[j].bestRollout.score(); 
      	  best = j;
      	}
      }
      if(best >= 0){
	bestRollout = subs[best].bestRollout; // TODO is this copy necessary
	legalMoveCodes = subs[best].legalMoveCodes;
      }
      updatePolicy( ALPHA * nbThreads );
    }
  }
  else{

    NrpaLevel *sub = &_nrpa[level -1]; 

    for(int i = 0; i < N; i++){
      double score = sub->run(level - 1, levelPolicy); 
      if (score >= bestRollout.score()) {
	int length = sub->bestRollout.length(); 
	bestRollout = sub->bestRollout;
	legalMoveCodes = sub->legalMoveCodes; 
	
	if (level > 2) {
	  for (int t = 0; t < level - 1; t++)
	    fprintf (stderr, "\t");
	  fprintf(stderr,"Level : %d, N:%d, score : %f\n", level, i, bestRollout.score());
	}
      }

      if(i != N - 1)
	updatePolicy(); 
    }
  }
  return bestRollout.score();
}


template <typename B,typename  M, int L, int PL, int LM>
double Nrpa<B,M,L,PL,LM>::NrpaLevel::playout (const Policy &policy) {
  using namespace std; 
  
  B board; 

  bestRollout.reset(); 
  legalMoveCodes.setNbSteps(0); 

  while(! board.terminal ()) {

    /* board is at a non terminal step, make a new move ... */
    int step = board.length; 

    /* Get all legal moves for this step */ 
    M moves [LM];
    int nbMoves = board.legalMoves (moves);

    double moveProbs [LM];

    legalMoveCodes.setNbSteps(step + 1); 
    legalMoveCodes.setNbMoves(step, nbMoves); 
    for (int i = 0; i < nbMoves; i++) {
      int c = board.code (moves [i]);
      moveProbs [i] = exp (policy.prob(c));
      legalMoveCodes.setMove(step, i, c); 
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
    assert(step == bestRollout.length()); 
    bestRollout.addMove(legalMoveCodes.move(step, j)); 
    board.play(moves[j]); 
  }

  /* Board is terminal */ 

  double score = board.score(); 
  bestRollout.setScore(score);
  return score; 
  
}


template <typename B,typename M, int L, int PL, int LM>
void Nrpa<B,M,L,PL,LM>::NrpaLevel::updatePolicy( double alpha ){

  //  assert(rollout.length() <= _nrpa[level].legalMoveCodes.size()); 
  using namespace std; 

  static Policy newPol;
  int length = bestRollout.length(); 

  //  newPol = *levelPolicy; //complete copy is not necessary

  /* Copy data for the legal moves only into a new policy */ 
  for (int step = 0; step < length; step++) 
    for (int i = 0; i < legalMoveCodes.nbMoves(step);  i++){
      newPol.setProb (legalMoveCodes.move(step, i), levelPolicy.prob (legalMoveCodes.move(step,i)));
    }

  for(int step = 0; step < length; step++){
    int code = bestRollout.move(step); 
    newPol.setProb(code, newPol.prob(code) + alpha);

    double z = 0.; 
    for(int i = 0; i < legalMoveCodes.nbMoves(step); i++)
      z += exp (levelPolicy.prob( legalMoveCodes.move(step,i) ));

    for(int i = 0; i < legalMoveCodes.nbMoves(step); i++){
      int move = legalMoveCodes.move(step, i); 
      newPol.updateProb(move, - alpha * exp (levelPolicy.prob(move)) / z );
    }

  }

  /* Copy updated data back into the policy */ 
  for (int step = 0; step < length; step++) 
    for (int j = 0; j < legalMoveCodes.nbMoves(step); j++)
      levelPolicy.setProb (legalMoveCodes.move(step, j), newPol.prob(legalMoveCodes.move(step,j) ));

  //  *levelPolicy = newPol; 

}

/* Instanciation of static NRPA structures */ 
template <typename B,typename M, int L, int PL, int LM>
typename Nrpa<B,M,L,PL,LM>::NrpaLevel Nrpa<B,M,L,PL,LM>::_nrpa[L]; 
