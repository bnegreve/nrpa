#include <float.h>
#include "rollout.hpp" 

double ALPHA = 1.0;
int N = 10;
//int N [10] = {20, 20, 20, 20, 20, 20, 20, 20, 50, 50};

double k = 0.0;

const int MaxLevel = 6;

double scoreBestRollout [10];
int lengthBestRollout [10];
Move bestRollout [10] [MaxPlayoutLength];
int bestCodeBestRollout [10] [MaxPlayoutLength];
int nbMovesBestRollout [10] [MaxPlayoutLength];
int codeBestRollout [10] [MaxPlayoutLength] [MaxLegalMoves];

Board bestBoard;

clock_t startClockNRPA, stopClockNRPA;
double bestScoreNRPA = -DBL_MAX;
double nextTimeNRPA = 0.01;
int indexTimeNRPA;
int indexSearch;
float valueAfterTimeNRPA [10000] [100];
float sumValueAfterTimeNRPA [100];
int nbSearchTimeNRPA [100];

int nbTimesNRPA = 12;
bool stopOnTime = false;
float firstTimeNRPA = 0.01;
int nbSearchesNRPA = 200;

int SizeBeam [MaxLevel] = {1, 1, 1, 1, 1, 1};
int startLearning = 0;
int levelPrint = 3;
float constante = 0.4;
float kAMAF = 1.0;
double minNorm = -0.1, maxNorm = 2.0;

#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

const int SizeTablePolicy = 65535;

class MoveStat {
 public:
  int code;
  double sumScores;
  int nbPlayouts;
};

class PolicyAMAF {
 public:
  vector<MoveStat> table [SizeTablePolicy + 1];
  double maxScore, minScore;

  PolicyAMAF () {
    maxScore = 0.0;
    minScore = DBL_MAX;
  }

  void add (int code, double score) {
    if (score > maxScore)
      maxScore = score;
    if (score < minScore)
      minScore = score;
    int index = code & SizeTablePolicy;
    for (int i = 0; i < table [index].size (); i++) {
      if (table [index] [i].code == code) {
	table [index] [i].sumScores += score;
	//if (score > table [index] [i].sumScores)
	//table [index] [i].sumScores = score;
	table [index] [i].nbPlayouts++;
	return;
      }
    }
    MoveStat p;
    p.code = code;
    p.sumScores = score;
    p.nbPlayouts = 1;
    table [index].push_back (p);
  }

  double get (int code) {
    int index = code & SizeTablePolicy;
    for (int i = 0; i < table [index].size (); i++)
      if (table [index] [i].code == code) {
	return (table [index] [i].sumScores / table [index] [i].nbPlayouts - minScore) / (maxScore - minScore);
	//return table [index] [i].sumScores / maxScore;
      }
    return 0.0;
  }

};

PolicyAMAF policyAMAF;

class ProbabilityCode {
    friend ostream &operator<<(ostream &, ProbabilityCode); 
 public:
  int code;
  double proba;
};

ostream &operator<<(ostream &os, ProbabilityCode p){
    os<<"Code: "<<p.code<<" Proba : "<<p.proba; 
    return os; 
}

/* This is more or less a priority queue implemented with a hashtable internally */ 
class Policy {

public:

    void print() const{
	cout<<"Policy "<<this<<endl; 
	for(int i = 0; i < SizeTablePolicy; i++){
	    for(int j = 0; j < table[i].size(); j++){
		cout<<table[i][j]<<endl;
	    }
	}
	cout<<"End of policy"<<endl;
    }
  

 public:
  vector<ProbabilityCode> table [SizeTablePolicy + 1]; // hashtable! 

  void set (int code, double proba) {
    int index = code & SizeTablePolicy;
    for (int i = 0; i < table [index].size (); i++) {
      if (table [index] [i].code == code) {
	table [index] [i].proba = proba;
	return;
      }
    }
    ProbabilityCode p;
    p.code = code;
    p.proba = proba;
    table [index].push_back (p);
  }

  double get (int code) {
    int index = code & SizeTablePolicy;
    for (int i = 0; i < table [index].size (); i++)
      if (table [index] [i].code == code) {
	return table [index] [i].proba;
      }
    //return minNorm + (maxNorm - minNorm) * policyAMAF.get (code);
    return 0.0;
  }

  bool element (int code) {
    int index = code & SizeTablePolicy;
    for (int i = 0; i < table [index].size (); i++)
      if (table [index] [i].code == code) {
	return true;
      }
    return false;
  }

  void add (Policy & pol) {
    for (int index = 0; index < SizeTablePolicy + 1; index++)
      for (int i = 0; i < pol.table [index].size (); i++) {
	int code = pol.table [index] [i].code;
	bool present = false;
	for (int j = 0; j < table [index].size (); j++) {
	  if (table [index] [j].code == code) {
	    present = true;
	    table [index] [j].proba = pol.table [index] [i].proba;
	    break;
	  }
	}
	if (!present)
	  table [index].push_back (pol.table [index] [i]);
      }
  }
  
  double maximum () {
    double m = -DBL_MAX;
    bool empty = true;
    for (int index = 0; index < SizeTablePolicy + 1; index++)
      for (int i = 0; i < table [index].size (); i++)
	if (table [index] [i].proba > m) {
	  empty = false;
	  m = table [index] [i].proba;
	}
    if (empty)
      return 0.0;
    return m;
  }
  
  double minimum () {
    double m = DBL_MAX;
    bool empty = true;
    for (int index = 0; index < SizeTablePolicy + 1; index++)
      for (int i = 0; i < table [index].size (); i++)
	if (table [index] [i].proba < m) {
	  empty = false;
	  m = table [index] [i].proba;
	}
    if (empty)
      return 0.0;
    return m;
  }
  
  void normalize (double mi, double ma) {
    double maxi = maximum (), mini = minimum ();
    for (int index = 0; index < SizeTablePolicy + 1; index++)
      for (int i = 0; i < table [index].size (); i++)
	table [index] [i].proba =
	  mi + (ma - mi) * (table [index] [i].proba - mini) / (maxi - mini);
  }
};


double playoutNRPA (Policy & pol) {
  int nbMoves = 0;
  Move moves [MaxLegalMoves];
  double probaMove [MaxLegalMoves];
  Board board;
 
  while (true) {
    if (board.terminal ()) {
      double score = board.score ();
      scoreBestRollout [0] = score;
      lengthBestRollout [0] = board.length;
      for (int k = 0; k < board.length; k++)
	bestRollout [0] [k] = board.rollout [k];
      //for (int k = 0; k < board.length; k++)
      //policyAMAF.add (board.code (board.rollout [k]), score);
      if (score > bestScoreNRPA) {
	bestScoreNRPA = score;
	bestBoard = board;
	board.print (stderr);
      }
      stopClockNRPA = clock ();
      double time = ((double)(stopClockNRPA - startClockNRPA)) / CLOCKS_PER_SEC;
      if (time > nextTimeNRPA) {
	while (time > 2 * nextTimeNRPA) {
	  indexTimeNRPA++;
	  nextTimeNRPA *= 2;
	}
	valueAfterTimeNRPA [indexSearch] [indexTimeNRPA] = bestScoreNRPA;
	sumValueAfterTimeNRPA [indexTimeNRPA] += bestScoreNRPA;
	nbSearchTimeNRPA [indexTimeNRPA]++;
	indexTimeNRPA++;
	nextTimeNRPA *= 2;
      }
      return score;
    }
    nbMoves = board.legalMoves (moves);
    nbMovesBestRollout [0] [board.length] = nbMoves;
    for (int i = 0; i < nbMoves; i++) {
      int c = board.code (moves [i]);
      /**/
      //double p = board.penalty (moves [i]);
      //probaMove [i] = exp (pol.get (c) + moves [i].penalty);
      //if (moves [i].penalty == -1000.0)
      //fprintf (stderr, "tabu move, probaMove [%d/%d] = %lf, ", i, nbMoves, probaMove [i]);
      //if (!pol.element (c))
      //probaMove [i] = exp (-p);
      //else
      //probaMove [i] = exp (pol.get (c));
      /**/
      probaMove [i] = exp (pol.get (c));
      codeBestRollout [0] [board.length] [i] = c;
    }
	
    double sum = probaMove [0];
    for (int i = 1; i < nbMoves; i++) 
      sum += probaMove [i];
    double r = (rand () / (RAND_MAX + 1.0)) * sum;
    int j = 0;
    double s = probaMove [0];
    while (s < r) { 
      j++;
      s += probaMove [j];
    }
    bestCodeBestRollout [0] [board.length] = codeBestRollout [0] [board.length] [j];
    board.play (moves [j]);
  }
  return 0.0;  
}

static double polpAdapt [MaxMoveNumber];

void adapt (int length, Move rollout [MaxPlayoutLength], double pol [MaxMoveNumber]) {
  for (int i = 0; i < MaxMoveNumber; i++)
    polpAdapt [i] = pol [i];
  int nbMoves;
  Move moves [MaxLegalMoves];
  Board board;
  for (int i = 0; i < length; i++) {
    nbMoves = board.legalMoves (moves);
    polpAdapt [board.code (rollout [i])] += ALPHA;
    double z = 0.0;
    for (int j = 0; j < nbMoves; j++)
      z += exp (pol [board.code (moves [j])]); 
    for  (int j = 0; j < nbMoves; j++)
      polpAdapt [board.code (moves [j])] -= ALPHA * exp (pol [board.code (moves [j])]) / z; 
    board.play (rollout [i]);
  }
  for (int i = 0; i < MaxMoveNumber; i++)
    pol [i] = polpAdapt [i];
}

/*
void adaptLevel (int length, int level, double pol [MaxMoveNumber]) {
  memcpy (polpAdapt, pol, MaxMoveNumber * sizeof (double));
  //for (int i = 0; i < MaxMoveNumber; i++)
  //polpAdapt [i] = pol [i];
  for (int i = 0; i < length; i++) {
    polpAdapt [bestCodeBestRollout [level] [i]] += ALPHA;
    double z = 0.0;
    for (int j = 0; j < nbMovesBestRollout [level] [i]; j++)
      z += exp (pol [codeBestRollout [level] [i] [j]]); 
    for  (int j = 0; j < nbMovesBestRollout [level] [i]; j++)
      polpAdapt [codeBestRollout [level] [i] [j]] -= ALPHA * exp (pol [codeBestRollout [level] [i] [j]]) / z; 
  }
  //for (int i = 0; i < MaxMoveNumber; i++)
  //pol [i] = polpAdapt [i];
  memcpy (pol, polpAdapt, MaxMoveNumber * sizeof (double));
}
/**/

Policy polAdapt;


void adaptLevel (int length, int level, Policy & pol) {
    if(level == 4) {
        Rollout r(bestCodeBestRollout[level], lengthBestRollout [level], level, scoreBestRollout[level]);
	r.compareAndSwap("blah", "lock"); 
	// copy(r.data(), r.data() + r.length(), bestCodeBestRollout[level]);
	// lengthBestRollout[level] = r.length();
	// scoreBestRollout[level] = r.score(); 
    }

  for (int i = 0; i < length; i++) {
    for (int j = 0; j < nbMovesBestRollout [level] [i]; j++)
      polAdapt.set (codeBestRollout [level] [i] [j], pol.get (codeBestRollout [level] [i] [j]));
  }
  for (int i = 0; i < length; i++) {
    polAdapt.set (bestCodeBestRollout [level] [i], polAdapt.get (bestCodeBestRollout [level] [i]) + ALPHA);
    double z = 0.0;
    for (int j = 0; j < nbMovesBestRollout [level] [i]; j++)
      z += exp (pol.get (codeBestRollout [level] [i] [j])); 
    for  (int j = 0; j < nbMovesBestRollout [level] [i]; j++)
      polAdapt.set (codeBestRollout [level] [i] [j], polAdapt.get (codeBestRollout [level] [i] [j]) - ALPHA * exp (pol.get (codeBestRollout [level] [i] [j])) / z); 
  }
  for (int i = 0; i < length; i++) {
    for (int j = 0; j < nbMovesBestRollout [level] [i]; j++)
      pol.set (codeBestRollout [level] [i] [j], polAdapt.get (codeBestRollout [level] [i] [j]));
  }
}
/**/

static double polpLevel [10] [MaxMoveNumber];
static Policy polLevel [10];

double alpha = 1.0;

double getAlpha (int last, int current) {
  // return 1.0; // nrpa
  // if (last == current) // edelkamp
  // return 1.0;
  // else
  // return 0.0;
  return alpha * exp (-k * (current - last));
}

int stepCopyPolicy = 0;

double nrpa(int level, Policy & pol) {
  scoreBestRollout [level] = -DBL_MAX;
  if (level == 0) {
    //polLevel [level].add (pol);
    //return playoutNRPA (polLevel [level]);
    return playoutNRPA (pol);
  }
  else {
    polLevel [level] = pol;
    //polLevel [level].normalize (pol.minimum (), pol.maximum ());
    //polLevel [level].normalize (pol.minimum () / minNorm, pol.maximum () / minNorm);
    //polLevel [level].normalize (polLevel [level].minimum () / minNorm, polLevel [level].maximum () / minNorm);
    
    //polLevel [level].normalize (minNorm, maxNorm);
    //polLevel [level].add (pol);
    int last = 0;
    for (int i = 0; i < N; i++) {
      double score = nrpa (level - 1, polLevel [level]);
      //if ((i == stepCopyPolicy) && (level == 3))
      //polLevel [level] = polLevel [level - 1];
      if (score >= scoreBestRollout [level]) {
	//if (score > scoreBestRollout [level]) {
	  last = i;
	  //if (level > 1)
	  //polLevel [level] = polLevel [level - 1];
	  //if (level == 3)
	  //polLevel [level].add (polLevel [level - 1]);
	  //}
	//if (score > scoreBestRollout [level]) 
	//alpha = 1.0;
	scoreBestRollout [level] = score;
	lengthBestRollout [level] = lengthBestRollout [level - 1];
	/* Cpy best rollout from previous level */ 
	for (int k = 0; k < lengthBestRollout [level]; k++)
	  bestRollout [level] [k] = bestRollout [level - 1] [k];

	/* Cpy best rellout for previous level into the current level */ 
	for (int k = 0; k < lengthBestRollout [level]; k++) {
	  bestCodeBestRollout [level] [k] = bestCodeBestRollout [level - 1] [k];
	  nbMovesBestRollout [level] [k] = nbMovesBestRollout [level - 1] [k];
	  for (int l = 0; l < nbMovesBestRollout [level - 1] [k]; l++)
	    codeBestRollout [level] [k] [l] = codeBestRollout [level - 1] [k] [l];
	}
	if (level > 2) {
	  for (int t = 0; t < level - 1; t++)
	    fprintf (stderr, "\t");
	  fprintf(stderr,"Level : %d, N:%d, score : %f\n", level, i, score);
	}
	//adapt (lengthBestRollout [level], bestRollout [level], polp);
	//adaptLevel (lengthBestRollout [level], level, polp);
	//ALPHA = 2.0;
	//adaptLevel (lengthBestRollout [level], level, polLevel [level]);
      }
      //adapt (lengthBestRollout [level], bestRollout [level], polp);
      ALPHA = alpha;
      ALPHA = getAlpha (last, i);
      adaptLevel (lengthBestRollout [level], level, polLevel [level]);
      //if (level > 2)
      //polLevel [level].add (polLevel [level - 1]);
	//polLevel [level] = polLevel [level - 1];
      //alpha *= 0.998;
      if (stopOnTime && (indexTimeNRPA > nbTimesNRPA))
	return scoreBestRollout [level];
    }
    return scoreBestRollout [level];
  }
}

void writeValues (int nbThreads) {
  char s [1000];
  //  sprintf (s, "NRPA.time.nbThreads=%d.nbSearches=%d.k=%2.3f.epsilon=%2.4f.plot", nbThreads, nbSearchesNRPA, k, epsilon);
  sprintf (s, "NRPA.time.nbThreads=%d.nbSearches=%d.k=%2.3f", nbThreads, nbSearchesNRPA, k);
  FILE * fp = fopen (s, "w");
  if (fp != NULL) {
    fprintf (fp, "# %d searches\n", indexSearch + 1);
    double t = firstTimeNRPA;
    for (int j = 0; j <= nbTimesNRPA; j++) {
      float sum = 0.0;
      int nbValues = 0;
      for (int l = 0; l < indexSearch + 1; l = l + nbThreads)
	if (l + nbThreads - 1 < indexSearch + 1) {
	  double maxi = -DBL_MAX;
	  for (int m = 0; m < nbThreads; m++)
	    if (valueAfterTimeNRPA [l + m] [j] > maxi)
	      maxi = valueAfterTimeNRPA [l + m] [j];
	  sum += maxi;
	  nbValues++;
	}
      fprintf (fp, "%f %f\n", t, sum / nbValues);
      t *= 2;
    }
  }
  fclose (fp); 
}

void testTimeNRPA (int level) {
  char s [1000];
  for (int i = 0; i < 100; i++) {
    for (int j = 0; j < 10000; j++)
      valueAfterTimeNRPA [j] [i] = 0.0;
    sumValueAfterTimeNRPA [i] = 0.0;
    nbSearchTimeNRPA [i] = 0;
  }
  stopOnTime = true;
  for (indexSearch = 0; indexSearch < nbSearchesNRPA; indexSearch++) {
    bestScoreNRPA = -DBL_MAX;
    nextTimeNRPA = firstTimeNRPA;
    indexTimeNRPA = 0;
    startClockNRPA = clock ();
    while (true) {
      Policy pol;
      for (int i = 0; i <= level; i++)
	polLevel [i] = pol;

      nrpa (level, pol);
      for (int i = 0; i <= level; i++){
	polLevel[i].print();
      }

      if (indexTimeNRPA > nbTimesNRPA)
	break;
    }
    double t = firstTimeNRPA;
    fprintf (stderr, "level %d, iteration %d\n", level, indexSearch + 1);
    for (int j = 0; j <= nbTimesNRPA; j++) {
      if (nbSearchTimeNRPA [j] >= (7 * (indexSearch + 1)) / 10)
	fprintf (stderr, "%f %f\n", t, sumValueAfterTimeNRPA [j] / nbSearchTimeNRPA [j]);
      t *= 2;
    }
    writeValues (1);
    writeValues (2);
    writeValues (4);
    writeValues (8);
    writeValues (16);
  }
  /*
  sprintf (s, "NRPA.time.Level=%d.nbSearches=%d.k=%2.3f.alpha=%2.2f.N=%d.plot", level, nbSearchesNRPA, k, alpha, N);
  FILE * fp = fopen (s, "w");
  if (fp != NULL) {
    fprintf (fp, "# %d searches\n", nbSearchesNRPA);
    double t = firstTimeNRPA;
    for (int j = 0; j <= nbTimesNRPA; j++) {
      if (nbSearchTimeNRPA [j] >= (7 * nbSearchesNRPA) / 10)
	fprintf (fp, "%f %f\n", t, sumValueAfterTimeNRPA [j] / nbSearchTimeNRPA [j]);
      t *= 2;
    }
  }
  fclose (fp);
  */
}
