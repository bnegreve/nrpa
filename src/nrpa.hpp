// nrpa.hpp
// Made by Benjamin Negrevergne 
// Started on <2017-02-22 Wed>

#ifndef NRPA_HPP
#define NRPA_HPP

#include <unordered_map>
#include <cmath>
#include <limits>
#include <atomic>
#include <time.h>

#include "rollout.hpp"
#include "policy.hpp"
#include "threadpool.hpp"
#include "cli.hpp"
#include "stats.hpp"

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

/* 
 * Main class for the Nrpa algorith. 
 *
 * Warning: You may create multiple instance of this class (with
 * different parameters), but only one instance can run at a time.
 * (i.e., no parallel calls to Nrpa::run() or Nrpa::test() is allowed)
 * 
 * Here is how to run a Nrpa with all the defaults: 
 * Nrpa<Board, Move, 5, MaxPlayoutLength, MaxLegalMoves>::test(Options::parse(argc, argv));
 *
 * Template arguments: 
 * B = Board class
 * M = Move class 
 * L = Max level 
 * PL = Playout maximum length (in nb moves) 
 * LM = Maximum number of legal moves for each turn 
 */ 
template <typename B, typename M, int L, int PL, int LM>
class Nrpa {
  friend class Stats<Nrpa<B,M,L,PL,LM>>; 

public: 

  static constexpr double ALPHA = 1.0; 
  static const int MAX_THREADS = 128; 
 
  Nrpa(int maxThreads = 0, int parLevel = 1, bool threadStats = false);

  /* One nrpa run */
  double run(int level = L - 1, int nbIter = 10, int timeout = -1); 

  /* Nrpa testing methods, (make multiple runs, collect statstics etc */
  /* Options is described in cli.hpp */ 
  static double test(const Options &options); 
  static double test(int nbRun = 5, int level = L - 1, int nbIter = 10,
		     int timeout = -1, int nbThreads = 0); 

private:
  
  /* Data structures preallocated for each  Nrpa recursive call (i.e.) one per level. */ 
  struct NrpaLevel{

    double bestScore; 
    Policy levelPolicy; 
    Rollout<PL> bestRollout; 
    vector<M> bestRolloutMoves;  
    LegalMoves<PL, LM> legalMoveCodes;
    future<int> result; // only used for parallel calls. 

    void updatePolicy(double alpha = ALPHA); 
    double playout (const Policy &policy);
    inline NrpaLevel &operator=(const NrpaLevel &o){ //TODO move elsewhere
      bestScore = o.bestScore;
      levelPolicy = o.levelPolicy;
      bestRollout = o.bestRollout;
      legalMoveCodes = o.legalMoveCodes; 
      return *this; 
    }

  }; 

  double run(NrpaLevel *nl, int level, const Policy &policy);
  double runseq(NrpaLevel *nl, int level, const Policy &policy);     
  double runpar(NrpaLevel *nl, int level, const Policy &policy);     
  double runpar2(NrpaLevel *nl, int level, const Policy &policy);     
  double runpar3(NrpaLevel *nl, int level, const Policy &policy);     

  int doTask2(NrpaLevel *nl, int level, int tid, double localBest, mutex *mutex); 
  int doTask3(NrpaLevel *nl, int level, int tid, double localBest, mutex *mutex); 
  int doTask3(NrpaLevel *nl, NrpaLevel *localnl, int level, int tid, mutex *m); 


  static void errorif(bool cond, const std::string &msg = "unknown."); 
  int _startLevel; 
  int _nbIter; 

  /* Data structures for simple, recursive calls */
  static NrpaLevel _nrpa[L];

  /* parallel calls */
  static int _nbThreads; 
  static int _parLevel; 
  static ThreadPool _threadPool; 

  /* Data structures for parallel calls */ 
  static NrpaLevel _subs[MAX_THREADS]; 

  static Stats<Nrpa<B,M,L,PL,LM>> _stats; 

  static int _parStrat;
  
}; 

#include "nrpa.inl"

#endif 
