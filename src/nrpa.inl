// nrpa.inl
// Made by Benjamin Negrevergne 
// Started on <2017-03-08 Wed>
#include <limits>
#include <algorithm>
#include <sstream> 
#include <mutex>
#include "threadpool.hpp"


template <typename B,typename  M, int L, int PL, int LM>
Nrpa<B,M,L,PL,LM>::Nrpa(int maxThreads, int parLevel, bool threadStats){
  assert(maxThreads < MAX_THREADS); 

  if(maxThreads == 1){
    _nbThreads = 1;
    _parLevel = 0; 
    /* Only one thread is nedded, we don't need to create the threadpool */ 
  }
  else{
    if( ! _threadPool.initialized() ){
      if(maxThreads == 0){
	_threadPool.init(thread::hardware_concurrency() - 1, threadStats); 
	_nbThreads = _threadPool.nbThreads() + 1; // main thread included
      }
      else{
	_threadPool.init(maxThreads - 1, threadStats);
	_nbThreads = maxThreads; 
      }
      _parLevel = parLevel; 
    }
  }
}

template <typename B,typename  M, int L, int PL, int LM>
double Nrpa<B,M,L,PL,LM>::run(int level, int nbIter, int timeout){
  assert(level < L); 

  _startLevel = level; 
  _nbIter = nbIter; 

  //  setTimers(timeout, true); 

  Policy policy; 
  double score =  run(&_nrpa[level], level, policy);  

  cout<<"Bestscore: "<<score<<endl;
  
  return score; 
}

template <typename B,typename  M, int L, int PL, int LM>
double Nrpa<B,M,L,PL,LM>::test(const Options &o){

  int nbRun = o.numRun;
  int nbIter = o.numIter;
  int timeout = o.timeout;
  int nbThreads = o.numThread;
  int level = o.numLevel;
  int parLevel = o.parallelLevel; 

  _parStrat = o.parStrat; 
  if(o.seed >= 0)
    if(o.seed == 0)
      srand(clock() * getpid());
    else
      srand(o.seed); 

  errorif(level >= L, "level should be lower than L template argument."); 
  
  double avgscore = 0;
  double maxscore = numeric_limits<double>::lowest(); 

  if(o.iterStats)  _stats.initIterStats();
  if(o.timerStats) _stats.initTimerStats(); 

  for(int i = 0; i < o.numRun; i++){
    Nrpa<B,M,L,PL,LM> nrpa(nbThreads, parLevel, o.threadStats); 
    _stats.startRun(&nrpa, o.timeout); 

    double score = nrpa.run(level, nbIter, timeout);
    avgscore += score;
    maxscore = max(maxscore,  score); 

    _stats.finishRun(); 
  }
  
  _stats.writeStats("dat/nrpa_stats", o); 

  cout<<"Avgscore: "<< avgscore / nbRun<<endl; 
  cout<<"Bestscore-overall: "<< maxscore <<endl; 
  return avgscore / nbRun; 
}

template <typename B,typename  M, int L, int PL, int LM>
double Nrpa<B,M,L,PL,LM>::test(int nbRun, int level, int nbIter, int timeout, int nbThreads){
  Options o;
  o.numRun = nbRun;
  o.numLevel = level;
  o.numIter = nbIter;
  o.timeout = timeout;
  o.numThread = nbThreads;
  test(o); 
}

template <typename B,typename  M, int L, int PL, int LM>
double Nrpa<B,M,L,PL,LM>::run(NrpaLevel *nl, int level, const Policy &policy){
  using namespace std; 
  assert(level < L); 

  double score; 

  if (level == 0) {
    score = nl->playout(policy); 
  }
  else if(_nbThreads > 1 && level == _parLevel){

    /* Parallel call */ 
    switch(_parStrat){
    case 1:
      score = runpar(nl, level, policy);
      break;
    case 2:
      score = runpar2(nl, level, policy);
      break;
    case 3: 
      score = runpar3(nl, level, policy); 
      break;
    default:
      errorif(true, "Unknown parallelization strategy"); 
    }

  }
  else{

    /* Sequential call */ 
    score = runseq(nl, level, policy); 
  }

  return score; 

}

template <typename B,typename  M, int L, int PL, int LM>
double Nrpa<B,M,L,PL,LM>::runseq(NrpaLevel *nl, int level, const Policy &policy){
  using namespace std; 
  assert(level < L); 
  assert(level != 0); // level 0 should be a call to rollout 

  nl->bestRollout.reset(); 
  nl->levelPolicy = policy; 

  /* sequential call */ 
  NrpaLevel *sub; 
  if(level > _parLevel)
    sub = &_nrpa[level -1];
  else
    sub = new NrpaLevel;


  for(int i = 0; i < _nbIter; i++){
    double score = run(sub, level - 1, nl->levelPolicy); 
    if (score >= nl->bestRollout.score()) {
      int length = sub->bestRollout.length(); 
      nl->bestRollout = sub->bestRollout; // TODO only copy at the end of the loop
      nl->legalMoveCodes = sub->legalMoveCodes; 
	
      if (level > L - 3) {
	for (int t = 0; t < level - 1; t++)
	  fprintf (stdout, "\t");
	fprintf(stdout,"Level : %d, N:%d, score : %f\n", level, i, nl->bestRollout.score());
      }
    }

    if(i != _nbIter - 1)
      nl->updatePolicy(); 

    if(level == _startLevel) _stats.recordIterStats(i, *nl); 

    if(_stats.timeout()) break;

  }
  
  /* Cleanup */ 
  if(level == _startLevel)
    _stats.resetTimeout(); 
  if( level <= _parLevel) delete sub; 

  return nl->bestRollout.score();

}


template <typename B,typename  M, int L, int PL, int LM>
double Nrpa<B,M,L,PL,LM>::runpar(NrpaLevel *nl, int level, const Policy &policy){
  using namespace std; 
  assert(level < L); 
  assert(level != 0); // level 0 should be a call to rollout

  nl->bestRollout.reset(); 
  nl->levelPolicy = policy; 
    
  for(int i = 0; i < _nbIter; i+= _nbThreads){
    /* Run n threads */ 
    for(int j = 0; j < _nbThreads; j++){
      if(j != _nbThreads - 1){ // push task to threadpool!
	_subs[j].result = _threadPool.submit([ this, nl, level, j ]() -> int {
	    NrpaLevel *sub = &_subs[j];
	    run(sub, level - 1, nl->levelPolicy); return 1; });
      }
      else{ // last iter is handled by this thread
	NrpaLevel *sub = &_subs[j];
	run(sub, level - 1, nl->levelPolicy);
      }
    }

    /* fetch the best rollout among all parallel runs (if any better than before) */ 
    int best = -1; 
    double bestScore = nl->bestRollout.score();// numeric_limits<double>::lowest(); 
    for(int j = 0; j < _nbThreads; j++){
      if(j != _nbThreads - 1) 
	_subs[j].result.wait(); 

      if(_subs[j].bestRollout.score() >= bestScore){
	bestScore = _subs[j].bestRollout.score(); 
	best = j;
      }
    }
    if(best >= 0){
      nl->bestRollout = _subs[best].bestRollout; // TODO is this copy necessary
      nl->legalMoveCodes = _subs[best].legalMoveCodes;
    }

    nl->updatePolicy( ALPHA * _nbThreads );

    if(_stats.timeout()) break; 

  }

  if(level == _startLevel)
    _stats.resetTimeout(); 
  return nl->bestRollout.score();

}


template <typename B,typename  M, int L, int PL, int LM>
int Nrpa<B,M,L,PL,LM>::doTask2(NrpaLevel *nl, int level, int tid, double localBest, mutex *m){
  NrpaLevel *sub = &_subs[tid];
  for(int i = 0; i < _nbIter; i+= _nbThreads){
    double score = run(sub, level - 1, nl->levelPolicy);
    if(score >= localBest){ 
      m->lock(); 
      if(score >= nl->bestRollout.score()){
	localBest = score;
	nl->bestRollout = sub->bestRollout; // TODO only copy at the end of the loop
	nl->legalMoveCodes = sub->legalMoveCodes;
      }
      nl->updatePolicy();  // TODO: OUTCHH!! don't update inside the mutex
      m->unlock();  
    }

    if(_stats.timeout()) break;

  }
  return 1;  
}


template <typename B,typename  M, int L, int PL, int LM>
double Nrpa<B,M,L,PL,LM>::runpar2(NrpaLevel *nl, int level, const Policy &policy){
  using namespace std; 
  assert(level < L); 
  assert(level != 0); // level 0 should be a call to rollout

  nl->bestRollout.reset(); 
  nl->levelPolicy = policy; 

  double best = nl->bestRollout.score(); 
  mutex m; 
  for(int j = 0; j < _nbThreads; j++){
    if(j != _nbThreads - 1){
      _subs[j].result = _threadPool.submit([ this, nl, level, j, best, &m ]() -> int {
	  return doTask2(nl, level, j, best, &m); 
	});
    }
    else{
      doTask2(nl, level, j, best, &m);
    }
  }

  for(int j = 0; j < _nbThreads; j++){
    if(j != _nbThreads - 1) _subs[j].result.wait();
  }

  return nl->bestRollout.score();

}

template <typename B,typename  M, int L, int PL, int LM>
int Nrpa<B,M,L,PL,LM>::doTask3(NrpaLevel *nl, NrpaLevel *localnl, int level, int tid, mutex *m){
  // nl is the parent nrpa level
  // localnl is the threadlocal copy of the parent nrpa level // may be removed ? 
  // sub is the child nrpalevel 
  NrpaLevel *sub = &_subs[tid];
  double localBest = localnl->bestRollout.score(); 

  m->lock(); 
  *localnl = *nl; 
  m->unlock(); 

  for(int i = 0; i < _nbIter; i+= _nbThreads){
    double score = run(sub, level - 1, localnl->levelPolicy);
    if(score >= localnl->bestRollout.score()){
      localnl->bestRollout = sub->bestRollout; 
      localnl->legalMoveCodes = sub->legalMoveCodes;
    }

    m->lock();
    if(nl->bestRollout.score() <= localnl->bestRollout.score()){
      nl->bestRollout = sub->bestRollout; 
      nl->legalMoveCodes = sub->legalMoveCodes;
    }
    else { // nl is better than local nl, 
      localnl->bestRollout = nl->bestRollout; 
      localnl->legalMoveCodes = nl->legalMoveCodes;
    }
    m->unlock();  

    localnl->updatePolicy(); 

    if(_stats.timeout()) break;

  }
  return localnl->bestRollout.score();
}


template <typename B,typename  M, int L, int PL, int LM>
double Nrpa<B,M,L,PL,LM>::runpar3(NrpaLevel *nl, int level, const Policy &policy){
  using namespace std; 
  assert(level < L); 
  assert(level != 0); // level 0 should be a call to rollout

  nl->bestRollout.reset(); 
  nl->levelPolicy = policy; 

  mutex m; 
  double bestScore; 
  int best; 
  static NrpaLevel localNrpaLevels[MAX_THREADS];

  for(int j = 0; j < _nbThreads - 1; j++){ 
    _subs[j].result = _threadPool.submit([ this, nl, level, j, &m, &best ]() -> int {
	doTask3(nl, &localNrpaLevels[j], level, j, &m); 
      }); 
  }

  /* Do last task in this thread */ 
  bestScore = doTask3(nl, &localNrpaLevels[_nbThreads - 1], level, _nbThreads - 1, &m); 
  best = _nbThreads - 1; 

  for(int j = 0; j < _nbThreads - 1; j++){
    double score = _subs[j].result.get(); 
    if(score > bestScore){
      best = j;
      bestScore = score; 
    }
  }

  *nl = localNrpaLevels[best];

  return nl->bestRollout.score();

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

  Policy newPol;
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

template <typename B,typename M, int L, int PL, int LM>
void Nrpa<B,M,L,PL,LM>::errorif(bool cond, const std::string &msg){
  if(cond){
    cerr<<"Error : "<<msg<<endl;
    exit(1);
  }
}



/* Instanciation of static NRPA structures */ 
template <typename B,typename M, int L, int PL, int LM>
typename Nrpa<B,M,L,PL,LM>::NrpaLevel Nrpa<B,M,L,PL,LM>::_nrpa[L]; 

template <typename B, typename M, int L, int PL, int LM>
int Nrpa<B,M,L,PL,LM>::_nbThreads; 

template <typename B, typename M, int L, int PL, int LM>
int Nrpa<B,M,L,PL,LM>::_parLevel; 

template <typename B, typename M, int L, int PL, int LM>
int Nrpa<B,M,L,PL,LM>::_parStrat; 

template <typename B, typename M, int L, int PL, int LM>
ThreadPool Nrpa<B,M,L,PL,LM>::_threadPool; 

template <typename B, typename M, int L, int PL, int LM>
typename Nrpa<B,M,L,PL,LM>::NrpaLevel Nrpa<B,M,L,PL,LM>::_subs[MAX_THREADS]; 

template <typename B, typename M, int L, int PL, int LM>
Stats<Nrpa<B,M,L,PL,LM>> Nrpa<B,M,L,PL,LM>::_stats; 

