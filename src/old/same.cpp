
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>
#include<cassert>
#include<iostream>
#include<algorithm>

const int MaxSize = 15;
const int MaxProblem = 20;
const int MaxColor = 5;

const int MaxPlayoutLength = MaxSize * MaxSize / 2;
//const int MaxMoveNumber = MaxColor * MaxSize * MaxSize;
//const int MaxMoveNumber = MaxColor * MaxSize * MaxSize * MaxSize * MaxSize;
const int MaxMoveNumber = 262144;
const int MaxLegalMoves = MaxSize * MaxSize / 2;

int HashArray [MaxSize * MaxSize] [MaxColor];
int HashTime [MaxSize * MaxSize];

double epsilon = 0.0;
double p = 0.0;

int pb = 0, highScorepb = 0;
const bool simpleMoves = false;
void printHighScore (int i);
bool saveHighScore = true;

class Move {
 public:
  int nbLocations;
  int locations [MaxSize * MaxSize];
  int color;
  int hash;
  double penalty;

  Move () {
    nbLocations = 0;
    hash = 0;
    penalty = 0.0;
  }

  void add (int loc) {
    locations [nbLocations] = loc;
    nbLocations++;
    hash ^= HashArray [loc] [color];
  }

  void load (FILE *fp) {
    int r = fscanf (fp, "%d", &nbLocations);
    for (int i = 0; i < nbLocations; i++)
      r = fscanf (fp, "%d", &locations [i]);
  }

  void print (FILE *fp) {
    fprintf (fp, "%d ", nbLocations);
    for (int i = 0; i < nbLocations; i++)
      fprintf (fp, "%d ", locations [i]);
    fprintf (fp, "\n");
    if (nbLocations == 1)
      fprintf (stderr, "bug,"); 
  }

  void printCoord (FILE *fp) {
    fprintf (fp, "(%d,%d) ", locations [0] % MaxSize, locations [0] / MaxSize);
  }

  void sort () {
    //print (stderr);
    std::sort (locations, locations + nbLocations);
    //print (stderr);
  }
};

class Seen {
 public:
  char seen [MaxSize * MaxSize];

  void init () {
    memset (seen, 0, MaxSize * MaxSize * sizeof (char));
  }

  bool test (int loc) {
    return (seen [loc] == 0);
  }

  void set (int loc) {
    seen [loc] = 1;
  }
};

static Move moves [MaxSize * MaxSize];

int currentColor [MaxSize * MaxSize];
int tabu, secondBest;

class Board {
 public:
  int color [MaxSize * MaxSize];
  int nbMoves;
  int currentScore;
  int length;
  Move rollout [MaxPlayoutLength];
  int nbCellsColor [MaxColor], MaxCellsColor [MaxColor];
  unsigned long long hash;

  Board () {
    for (int i = 0; i < MaxSize * MaxSize; i++)
      color [i] = currentColor [i];
    for (int i = 0; i < MaxColor; i++) {
      MaxCellsColor [i] = 0;
      nbCellsColor [i] = 0;
    }
    MaxCellsColor [tabu] = 0;
    currentScore = 0;
    length = 0;
    hash = 0;
  }

  void load (FILE *fp) {
    for (int i = 0; i < MaxSize; i++)
      for (int j = 0; j < MaxSize; j++)
	int r = fscanf (fp, "%d", &color [MaxSize * i + j]);
    currentScore = 0;
    length = 0;
  }
  
  void print (FILE *fp) {
    if (false) {
      for (int i = 0; i < length; i++) {
	rollout [i].printCoord (fp);
      }
      fprintf (fp, "currentScore = %d\n", currentScore);
    }
    if (saveHighScore)
      if (currentScore > highScorepb) {
	highScorepb = currentScore;
	char s [1000];
	sprintf (s, "same.high.%d", pb);
	FILE * fp = fopen (s, "w");
	if (fp != NULL) {
	  printScore (fp);
	  fclose (fp);
	}
	for (int i = 0; i < length; i++)
	  rollout [i].printCoord (fp);
	fprintf (stderr, "new highscore pb %d = %d\n", pb + 1, highScorepb);
      }
  }

  void printBoard (FILE *fp) {
    for (int i = 0; i < MaxSize; i++) {
      for (int j = 0; j < MaxSize; j++)
	fprintf (fp, "%d ", color [MaxSize * i + j]);
      fprintf (fp, "\n");
    }
    fprintf (fp, "currentScore = %d\n", currentScore);
  }

  void loadScore (FILE *fp) {
    int r = fscanf (fp, "%d", &currentScore);
    r = fscanf (fp, "%d", &length);
    for (int i = 0; i < length; i++)
      rollout [i].load (fp);
  }

  void printScore (FILE *fp) {
    fprintf (fp, "%d\n", currentScore);
    fprintf (fp, "%d\n", length);
    for (int i = 0; i < length; i++)
      rollout [i].print (fp);
  }

  int code (Move m) {
    //return m.locations [0];
    //return color [m.locations [0]] * MaxSize * MaxSize + m.locations [0];
    //return color [m.locations [0]] * MaxSize * MaxSize * MaxSize * MaxSize + m.nbLocations * MaxSize * MaxSize + m.locations [0];
    //return m.hash & (MaxMoveNumber - 1);
    //if (length > 0)
    //return m.hash ^ rollout [length - 1].hash;
    return m.hash;
  }
  
  double penalty (Move m) {
    if (m.color == tabu)
      return -1000.0;
    //if ((m.color == secondBest) && (m.nbLocations == 2))
    //return -1.0;
    return 0.0;
  }
  
  void buildMove (int loc, Seen & seen, Move & move) {
    int c = color [loc];
    seen.set (loc);
    move.color = c;
    move.nbLocations = 0;
    move.hash = 0;
    //if (length > 30)
    //move.hash ^= HashTime [length / 20];
    move.add (loc);
    int stack [MaxSize * MaxSize];
    stack [0] = 1;
    stack [1] = loc;
    while (stack [0] > 0) {
      int l = stack [stack [0]], neigh;
      stack [0]--;
      if (l >= MaxSize) {
	neigh = l - MaxSize;
	if (color [neigh] == c)
	  if (seen.test (neigh)) {
	    seen.set (neigh);
	    move.add (neigh);
	    stack [0]++;
	    stack [stack [0]] = neigh;
	  }
      }
      if (l < MaxSize * MaxSize - MaxSize) {
	neigh = l + MaxSize;
	if (color [neigh] == c)
	  if (seen.test (neigh)) {
	    seen.set (neigh);
	    move.add (neigh);
	    stack [0]++;
	    stack [stack [0]] = neigh;
	  }
      }
      if ((l % MaxSize) != 0) {
	neigh = l - 1;
	if (color [neigh] == c)
	  if (seen.test (neigh)) {
	    seen.set (neigh);
	    move.add (neigh);
	    stack [0]++;
	    stack [stack [0]] = neigh;
	  }
      }
      if ((l % MaxSize) != MaxSize - 1) {
	neigh = l + 1;
	if (color [neigh] == c)
	  if (seen.test (neigh)) {
	    seen.set (neigh);
	    move.add (neigh);
	    stack [0]++;
	    stack [stack [0]] = neigh;
	  }
      }
    }
  }

  bool possibleMove (int l) {
    int c = color [l], neigh;
    if (l >= MaxSize) {
      neigh = l - MaxSize;
      if (color [neigh] == c)
	return true;
    }
    if (l < MaxSize * MaxSize - MaxSize) {
      neigh = l + MaxSize;
      if (color [neigh] == c)
	return true;
    }
    if ((l % MaxSize) != 0) {
      neigh = l - 1;
      if (color [neigh] == c)
	return true;
    }
    if ((l % MaxSize) != MaxSize - 1) {
      neigh = l + 1;
      if (color [neigh] == c)
	return true;
    }
    return false;
  }

  bool moreThanOneMove (int c) {
    Seen seen;
    Move mv;
    int nb = 0;
    seen.init ();
    for (int i = 0; i < MaxSize * MaxSize; i++) 
      if (color [i] == c)
	if (seen.test (i)) {
	  if (nb > 0)
	    return true;
	  buildMove (i, seen, mv);
	  nb++;
	}
    return false;
  }
  
  void findMoves (int tabu = 9, int secondBest = 9) {
    Seen seen;
    nbMoves = 0;
    seen.init ();

    if (!moreThanOneMove (tabu))
      tabu = 9;
    /*
    if ((!moreThanOneMove (secondBest)) || ((rand () % 10) > 9))
      secondBest = 9;
    for (int i = 0; i < MaxSize * MaxSize; i++) 
      if ((color [i] != 9) && (color [i] != tabu) && (color [i] != secondBest))
	if (seen.test (i)) {
	  buildMove (i, seen, moves [nbMoves]);
	  if (moves [nbMoves].nbLocations > 1)
	    nbMoves++;
	}
    /**/
    if (nbMoves == 0) {
      for (int i = 0; i < MaxSize * MaxSize; i++) 
	if ((color [i] != 9) && (color [i] != tabu))
	  if (seen.test (i)) {
	    buildMove (i, seen, moves [nbMoves]);
	    if (moves [nbMoves].nbLocations > 1)
	      nbMoves++;
	  } 
    }

    if (nbMoves == 0) {
      for (int i = 0; i < MaxSize * MaxSize; i++) 
	if (color [i] != 9)
	  if (seen.test (i)) {
	    buildMove (i, seen, moves [nbMoves]);
	    if (moves [nbMoves].nbLocations > 1)
	      nbMoves++;
	  } 
    }
/*     for (int i = 0; i < nbMoves; i++) */
/*       moves [i].print (stderr); */
  }
  
  bool allMoves;

  void findMoves (Move moves [MaxLegalMoves], int tabu = 9, int secondBest = 9, bool sizeTwo = false) {
    Seen seen;
    bool noMoves = true;
    nbMoves = 0;
    seen.init ();
    if (!moreThanOneMove (tabu))
      tabu = 9;
    /**/
    for (int i = 0; i < MaxSize * MaxSize; i++) 
      if (color [i] != 9)
	if (seen.test (i)) {
	  buildMove (i, seen, moves [nbMoves]);
	  if (moves [nbMoves].nbLocations > 1) {
	    if (color [i] == tabu) {
	      //if ((moves [nbMoves].nbLocations == 2) && 
	      //  ((rand () % 10000) >= 9900))
	      //if (nbCellsColor [tabu] + moves [nbMoves].nbLocations <= MaxCellsColor [tabu])
	      if ((moves [nbMoves].nbLocations <= 2) && (length > 10)) {
		moves [nbMoves].penalty = 0.0;//-1.0;
		nbMoves++;
		noMoves = false;
	      }
	      //if (sizeTwo)
	      //if (moves [nbMoves].nbLocations <= 2)
	      //nbMoves++;
	      //if (allMoves)
	      //else {
	      //moves [nbMoves].penalty = -1000.0;
	      //nbMoves++;
	      //}
	    }
	    else {
	      moves [nbMoves].penalty = 0.0;
	      nbMoves++;
	      noMoves = false;
	    }
	    //else if (color [i] == secondBest) {
	    //  if ((rand () % 100) >= 5)
	    //nbMoves++;
	    //}
	    //else if (color [i] != secondBest)
	    //  nbMoves++;
	    //else if ((color [i] == secondBest) && (moves [nbMoves].nbLocations > 2))
	    //  nbMoves++;
	    //else if ((rand () % 100) >= 80)
	    //  nbMoves++;
	  }
	}
    /*
    if (noMoves) {
      nbMoves = 0;
      for (int i = 0; i < MaxSize * MaxSize; i++) 
	if ((color [i] != 9) && (color [i] != tabu))
	  if (seen.test (i)) {
	    buildMove (i, seen, moves [nbMoves]);
	    if (moves [nbMoves].nbLocations > 1) {
	      moves [nbMoves].penalty = 0.0;
	      nbMoves++;
	    }
	  } 
    }
    */
      
    if (noMoves) {
      nbMoves = 0;
      seen.init ();
      for (int i = 0; i < MaxSize * MaxSize; i++) 
	if (color [i] != 9)
	  if (seen.test (i)) {
	    buildMove (i, seen, moves [nbMoves]);
	    if (moves [nbMoves].nbLocations > 1) {
	      moves [nbMoves].penalty = 0.0;
	      nbMoves++;
	    }
	  } 
    }
    /*     for (int i = 0; i < nbMoves; i++) */
    /*       moves [i].print (stderr); */
  }
  
  void findMovesSimple (Move moves [MaxLegalMoves], int tabu = 9) {
    nbMoves = 0;
    if (!moreThanOneMove (tabu))
      tabu = 9;
    for (int i = 0; i < MaxSize * MaxSize; i++) 
      if ((color [i] != 9) && (color [i] != tabu))
	if (possibleMove (i)) {
	  moves [nbMoves].locations [0] = i;
	  moves [nbMoves].nbLocations = 1;
	  nbMoves++;
	}
    if (nbMoves == 0)
      for (int i = 0; i < MaxSize * MaxSize; i++) 
	if (color [i] != 9)
	  if (possibleMove (i)) {
	    moves [nbMoves].locations [0] = i;
	    moves [nbMoves].nbLocations = 1;
	    nbMoves++;
	  } 
  }
  
  int legalMoves (Move mvs [MaxLegalMoves]) {
    /*
    if (length == 0) {
      allMoves = false;
      double proba = (double)(rand () / (RAND_MAX + 1.0));
      if (proba < epsilon) {
	allMoves = true;
	//findMoves (mvs, 9, 9, true);
	//return nbMoves;
      }
    }
    allMoves = false;
    */
    //if (simpleMoves)
    //findMovesSimple (mvs, tabu);
    //else
    findMoves (mvs, tabu, secondBest, allMoves);
    //for (int i = 0; i < nbMoves; i++) 
    //if (mvs [i].nbLocations == 1)
    //	fprintf (stderr, "bug,"); 
    return nbMoves;
  }

  void play (Move & move) {
    if (simpleMoves) {
      Move m;
      Seen seen;
      seen.init ();
      buildMove (move.locations [0], seen, m);
      move = m;
    }
    /**/
    // it is necessary to sort in order to remove
    // location on top before
    move.sort ();
    for (int i = 0; i < move.nbLocations; i++) {
      remove (move.locations [i]);
    }

    int column = 0;
    for (int i = 0; i < MaxSize; i++) {
      if (color [MaxSize * MaxSize - MaxSize + column] == 9)
	removeColumn (column);
      else
	column++;
    }

    currentScore += (move.nbLocations - 2) * (move.nbLocations - 2);

    if (color [MaxSize * MaxSize - MaxSize] == 9)
      currentScore += 1000;
    
    nbCellsColor [move.color] += move.nbLocations;
    
    rollout [length] = move;
    length++;
    hash = hash ^ move.hash;
  }

  bool terminal () {
    for (int i = 0; i < MaxSize * MaxSize; i++) 
      if (color [i] != 9)
	if (possibleMove (i)) 
	  return false;
    return true;
  }

  double score () {
    return (double)currentScore;
  }

  void findMovesColor (int c) {
    Seen seen;
    nbMoves = 0;
    seen.init ();
    for (int i = 0; i < MaxSize * MaxSize; i++) 
      if (color [i] == c)
	if (seen.test (i)) {
	  buildMove (i, seen, moves [nbMoves]);
	  if (moves [nbMoves].nbLocations > 1)
	    nbMoves++;
	}

    if (nbMoves == 0) {
      for (int i = 0; i < MaxSize * MaxSize; i++) 
	if (color [i] != 9)
	  if (seen.test (i)) {
	    buildMove (i, seen, moves [nbMoves]);
	    if (moves [nbMoves].nbLocations > 1)
	      nbMoves++;
	  } 
    }
  }
  
  void remove (int loc) {
    while (loc > MaxSize - 1) {
      color [loc] = color [loc - MaxSize];
      loc = loc - MaxSize;
    }
    color [loc] = 9;
  }

  void removeColumn (int column) {
    for (int row = 0; row < MaxSize; row++) {
      for (int i = column; i < MaxSize - 1; i++) {
	color [row * MaxSize + i] = color [row * MaxSize + i + 1];
      }
      color [row * MaxSize + MaxSize - 1] = 9;
    }
  }

  int bestColor (int & secondBest) {
    int nbColors [10];
    for (int i = 0; i < 10; i++)
      nbColors [i] = 0;
    for (int i = 0; i < MaxSize * MaxSize; i++) 
      if (color [i] != 9)
	nbColors [color [i]]++;
    int best = 0, bestScore = 0, secondBestScore = 0;
    for (int i = 0; i < 10; i++)
      if (nbColors [i] > bestScore) {
	secondBestScore = bestScore;
	secondBest = best;
	bestScore = nbColors [i];
	best = i;
      }
      else if (nbColors [i] > secondBestScore) {
	secondBestScore = nbColors [i];
	secondBest = i;
      }
    return best;
  }

  void playout () {
    secondBest = 9;
    tabu = bestColor (secondBest);
    findMoves (tabu, secondBest);
    while (nbMoves > 0) {
      int index = nbMoves * (rand () / (RAND_MAX + 1.0));
      play (moves [index]);
      findMoves (tabu, secondBest);
    }
  }

  int evaluation () {
    findMoves ();
    int score = 0;
    for (int i = 0; i < nbMoves; i++) {
      score += (moves [i].nbLocations - 2) * (moves [i].nbLocations - 2);
    }
    return score;
  }


  int chooseColor (int tabu = 9) {
    int nbColors [10], maxColor = 5, total = 0;
    for (int i = 0; i < 10; i++)
      nbColors [i] = 0;
    for (int i = 0; i < MaxSize * MaxSize; i++) 
      if (color [i] != 9) {
	nbColors [color [i]]++;
	total++;
/* 	if (color [i] > maxColor) */
/* 	  maxColor = color [i]; */
      }
    float beta = 4.0;
    float alpha = 1.0 + beta * total / 225.0;
    float theta = 0.0;
    int best = 0, minScore = nbColors [0];
    for (int i = 1; i < maxColor; i++)
      if (nbColors [i] < minScore) {
	minScore = nbColors [i];
	best = i;
      }
    theta = minScore / 2.0;
    float term [10], sumTerms = 0.0;
    for (int i = 0; i < maxColor; i++) {
      term [i] = pow (nbColors [i] - theta, alpha);
      if (i == tabu)
	term [i] = 0.0;
      sumTerms += term [i];
    }
    float proba [10], sumProbas = 0.0;
    for (int i = 0; i < maxColor; i++) {
      proba [i] = (1.0 - (term [i] / sumTerms)) / (maxColor - 1);
      if (i == tabu)
	proba [i] = 0.0;
      sumProbas += proba [i];
    }
    float r = sumProbas * (rand () / (RAND_MAX + 1.0));
    float s = 0.0;
    for (int i = 0; i < maxColor; i++) {
      s += proba [i];
      if (s > r)
	return i;
    }
    return tabu;
  }

  void playoutColored () {
    secondBest = 9;
    tabu = bestColor (secondBest);
    int color = chooseColor (tabu);
    findMovesColor (color);
    while (nbMoves > 0) {
      int index = nbMoves * (rand () / (RAND_MAX + 1.0));
      play (moves [index]);
      color = chooseColor (tabu);
      findMovesColor (color);
    }
  }

  bool playRandomMove (int tabu) {
    int loc [MaxSize * MaxSize], nbLocs = 0;
    for (int i = 0; i < MaxSize * MaxSize; i++) 
      if ((color [i] != 9) && (color [i] != tabu)) {
	loc [nbLocs] = i;
	nbLocs++;
      }
    Seen seen;
    Move m;
    while (nbLocs > 0) {
      int index = nbLocs * (rand () / (RAND_MAX + 1.0));
      buildMove (loc [index], seen, m);
      if (m.nbLocations > 1)
	break;
      loc [index] = loc [nbLocs - 1];
      nbLocs--;
    }
    if (m.nbLocations > 1)
      play (m);
    else {
      nbLocs = 0;
      for (int i = 0; i < MaxSize * MaxSize; i++) 
	if ((color [i] == tabu)) {
	  loc [nbLocs] = i;
	  nbLocs++;
	}
      while (nbLocs > 0) {
	int index = nbLocs * (rand () / (RAND_MAX + 1.0));
	buildMove (loc [index], seen, m);
	if (m.nbLocations > 1)
	  break;
	loc [index] = loc [nbLocs - 1];
	nbLocs--;
      }
      if (m.nbLocations > 1)
	play (m);
      else
	return false;
    }
    return true;
  }

  void playoutOptimized () {
    while (playRandomMove (tabu))
      ;
  }
};

Board problem [MaxProblem];
Board highScore [MaxProblem];
int seed = 1;

void load (int nb, const char *name) {
  FILE * fp = fopen (name, "r");
  if (fp != NULL) {
    for (int i = 0; i < nb; i++)
      problem [i].load (fp);
    fclose (fp);
  }
  else 
    fprintf (stderr, "pb ouverture %s\n", name);
}

void loadHighScores (int nb) {
  int sum = 0;
  for (int i = 0; i < nb; i++) {
    highScore [i].currentScore = 0;
    highScore [i].length = 0;
    char s [1000];
    sprintf (s, "same.high.%d", i);
    FILE * fp = fopen (s, "r");
    if (fp != NULL) {
      highScore [i].loadScore (fp);
      fclose (fp);
    }
    sum += highScore [i].currentScore;
  }
  fprintf (stderr, "sum = %d\n", sum);
}

void printHighScore (int i) {
  char s [1000];
  sprintf (s, "same.high.%d", i);
  FILE * fp = fopen (s, "w");
  if (fp != NULL) {
    highScore [i].printScore (fp);
    fclose (fp);
  }
}

//#include "nestedSH.c"
//#include "nested.c"
#include "nrpa.cpp"
//#include "nrpauct.c"
//#include "beamnrpa.c"
//#include "beamuctnrpa.c"

int main(int argc, char *argv []) {
  constante = 1000.0;
  if (argc > 1) 
    pb = atoi (argv [1]);
  if (argc > 2)  {
    k = atof (argv [2]);
    fprintf (stderr, "k = %f\n", k);
    //seed = atoi (argv [2]);
    //srand (seed);
  }
  if (argc > 3)  {
    //kAMAF = atof (argv [3]);
    //fprintf (stderr, "kAMAF = %f\n", kAMAF);
    minNorm = atof (argv [3]);
    fprintf (stderr, "minNorm = %f\n", minNorm);
    //stepCopyPolicy = atoi (argv [3]);
    //fprintf (stderr, "stepCopyPolicy = %d\n", stepCopyPolicy);
    //p = atof (argv [3]);
    //fprintf (stderr, "p = %f\n", p);
    //epsilon = atof (argv [3]);
    //fprintf (stderr, "epsilon = %f\n", epsilon);
    //alpha = atof (argv [3]);
    //fprintf (stderr, "alpha = %f\n", alpha);
    //SizeBeam [1] = atoi (argv [3]);
  }
  if (argc > 4)  {
    maxNorm = atof (argv [4]);
    fprintf (stderr, "maxNorm = %f\n", maxNorm);
    //N = atoi (argv [4]);
    //fprintf (stderr, "N = %d\n", N);
    //SizeBeam [2] = atoi (argv [4]);
  }
  if (argc > 5)  {
    kAMAF = atof (argv [3]);
    fprintf (stderr, "kAMAF = %f\n", kAMAF);
    //constante = atof (argv [5]);
    //fprintf (stderr, "constante = %f\n", constante);
    SizeBeam [3] = atoi (argv [5]);
  }
  if (argc > 6)  {
    SizeBeam [4] = atoi (argv [6]);
  }
  if (argc > 7)  {
    SizeBeam [5] = atoi (argv [7]);
  }

  startLearning = 0;//10;
  if (argc > 8)  {
    startLearning = atoi (argv [8]);
  }

  if (argc > 9)  {
    epsilon = atof (argv [9]);
    fprintf (stderr, "epsilon = %f\n", epsilon);
  }

  if (argc > 10)  {
    constante = atof (argv [10]);
    fprintf (stderr, "constante = %f\n", constante);
  }

  for (int i = 0; i < MaxSize * MaxSize; i++) {
    HashTime [i] = rand ();
    for (int c = 0; c < MaxColor; c++)
      HashArray [i] [c] = rand ();
  }
  fprintf (stderr, "RAND_MAX = %d\n", RAND_MAX);

  load (20, "../problems.txt");
  for (int i = 0; i < MaxSize * MaxSize; i++)
    currentColor [i] = problem [pb].color [i];
  secondBest = 9;
  tabu = problem [pb].bestColor (secondBest);
  loadHighScores (20);
  for (int i = 0; i < 20; i++)
    fprintf (stderr, "%d -> %d\n", i+1, highScore [i].currentScore);
  highScorepb = highScore [pb].currentScore;
  //for (int i = 0; i < highScore [1].length; i++)
  //highScore [1].rollout [i].printCoord (stderr);
  fprintf (stderr, "\n");
  //testTimeNested (2);
  saveHighScore = false;
  //testTimeNRPA (5);
  levelPrint = 4;
  saveHighScore = true;
  Policy pol;
  double s = nrpa (4, pol);
  exit (0);
  /**/
  int nb = 0;
  double sum = 0.0;
  while (true) {
    Policy pol;
    Board b;
    /*
    int nbMoves = b.legalMoves (moves);
    for (int i = 0; i < MaxSize * MaxSize - MaxSize; i++) {
      int c = HashArray [i] [secondBest] ^ HashArray [i + MaxSize] [secondBest];
      pol [c & (MaxMoveNumber - 1)] = -1.0;
    }
    /**/
    //nested (b, 3);
    levelPrint = 4;
    double s = nrpa (5, pol);
    sum += s;
    fprintf (stderr, "nb = %d\nbest score (pb %d) = %2.0lf\n", nb, pb, bestBoard.score ());
    nb++;
    fprintf (stderr, "mean score %2.0lf\n", sum / nb);
    if (nb == 1)
      break;
  }
  /**/
}

