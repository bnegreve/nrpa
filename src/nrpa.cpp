#include "nrpa.hpp"


/* Old constants kepts for compatibility with old game file. Their values are set to old defaults, they have no effect on the nrpa algorithm, but they might have an impact on the old game code. */
const int MaxLevel = 10;
double k = 0.0;
float constante = 0.4;
float kAMAF = 1.0;
double minNorm = -0.1, maxNorm = 2.0;
int SizeBeam [MaxLevel] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
int levelPrint = 3; 
int startLearning = 0;
