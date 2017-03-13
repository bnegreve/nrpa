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

  double run(int level = L - 1, int nbIter = 10, int timeout = -1); 
  double test(int nbRun = 5, int level = L - 1, int nbIter = 10, int timeout = -1); 

  static void setTimeout(int sec); 

private:
  
  /* Data structures preallocated for each  Nrpa recursive call (i.e.) one per level. */ 
  struct NrpaLevel{

    double bestScore; 
    Policy levelPolicy; 
    Rollout<PL> bestRollout; 
    vector<M> bestRolloutMoves;  
    LegalMoves<PL, LM> legalMoveCodes;

    void updatePolicy(double alpha = ALPHA); 
    double playout (const Policy &policy);

  }; 

  double run(NrpaLevel *nl, int level, const Policy &policy);     

  static NrpaLevel _nrpa[L]; 
  static int _nbIter; 
  static atomic_bool _timeout; 

  
}; 

#include "nrpa.inl"

#endif 
