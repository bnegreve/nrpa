// nrpa.hpp
// Made by Benjamin Negrevergne 
// Started on <2017-02-22 Wed>

#ifndef NRPA_HPP
#define NRPA_HPP

#include <unordered_map>
#include <cmath>
#include <limits>
#include <atomic>

#include "rollout.hpp"
#include "policy.hpp"
#include "threadpool.hpp"
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

  static constexpr double ALPHA = 1.0; 
  static const int MAX_THREADS = 128; 


  Nrpa(int maxThreads = 0, int parLevel = 2);
  double run(int level = L - 1, int nbIter = 10, int timeout = -1); 
  void setTimeout(int sec);

  static double test(int nbRun = 5, int level = L - 1, int nbIter = 10, int timeout = -1, int nbThreads = 0); 

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

  }; 

  double run(NrpaLevel *nl, int level, const Policy &policy);
  double runseq(NrpaLevel *nl, int level, const Policy &policy);     
  double runpar(NrpaLevel *nl, int level, const Policy &policy);     
  bool checkTimeout(); 

  int _nbIter; 
  atomic_bool _timeout; 

  /* Data structures for simple, recursive calls */
  static NrpaLevel _nrpa[L];

  /* parallel calls */
  static int _nbThreads; 
  static int _parLevel; 
  static ThreadPool _threadPool; 

  /* Data structures for parallel calls */ 
  static NrpaLevel _subs[MAX_THREADS]; 
  
}; 

#include "nrpa.inl"

#endif 
