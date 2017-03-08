// nrpa.hpp
// Made by Benjamin Negrevergne 
// Started on <2017-02-22 Wed>

#ifndef NRPA_HPP
#define NRPA_HPP

#include <unordered_map>
#include <cmath>
#include <limits>
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

#include "nrpa.inl"

#endif 
