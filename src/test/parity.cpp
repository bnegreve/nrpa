#include <stdio.h>
#include <math.h>
#include <vector>
#include <list>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <limits.h>

/* for setpriority */
#include <sys/time.h>
#include <sys/resource.h> 

#include <nrpa.hpp>

using namespace std;

const int MaxChildren = 3;
const int MaxFunctor = 100;
const int MaxExpressions = 1000;
const int MaxNodes = 2000;
const int MaxVariables = 50;
const int FixedFunctors = 64;
const int MaxScore = 10000;

const int MaxPlayoutLength = 200;
//const int MaxMoveNumber = MaxFunctor * MaxPlayoutLength;
const int MaxMoveNumber = MaxFunctor * MaxPlayoutLength + MaxFunctor * 1024;
const int MaxLegalMoves = 100;

int numberParity = 6;

typedef int Move;

class Functor {
 public:
  const char * name;
  int nbChildren;
  bool active;
};

class TableFunctor {
 public:
  int nbFunctors;
  Functor functor [MaxFunctor];

  TableFunctor () {
    nbFunctors = 0;
  }

  void add (const char * name, int nbChildren, bool active) {
    if (nbFunctors < MaxFunctor) {
      functor [nbFunctors].name = name;
      functor [nbFunctors].nbChildren = nbChildren;
      functor [nbFunctors].active = active;
      nbFunctors++;
    }
    else fprintf (stderr, "Pb TableFunctor::add\n");
  }

  int nbChildren (int i) { return functor [i].nbChildren; }
  const char * name (int i) { return functor [i].name; }
  bool active (int i) { return functor [i].active; }
};

TableFunctor tableFunctor;

class Board {
 public:
  Move rollout [MaxPlayoutLength];
  int length;
  int feuillesOuvertes;
  int maxi;
  int numberNode;

  double variableGP [MaxVariables];

  Board () {
    length = 0;
    feuillesOuvertes = 1;
    maxi = 41;
    numberNode = 1;
  }

  int code (Move m) {
    //return m;
    //return length * MaxFunctor + m;
    //int depth = 0;
    //numberNode = numberLeaf (depth, 1);
    int c = MaxFunctor * MaxPlayoutLength + numberNode * MaxFunctor + m;
    if (c >= MaxMoveNumber) {
      //fprintf (stderr, "Pb code %d ", c);
      //return m;
      return MaxFunctor * length + m;
    }
    return c;
  }

  int numberLeaf (int & depth, int numberFather) {
    if (depth == length)
      return numberFather;
    int code = rollout [depth];
    for (int i = 0; i < tableFunctor.nbChildren (code); i++) {
      depth = depth + 1;
      int n = numberLeaf (depth, 2 * numberFather + i);
      if (n > 0)
	return n;
    }
    return 0;
  }

  void printStack (FILE * fp, int & depth, int max = 1000) {
    if (depth >= max)
      return;
    int code = rollout [depth];
    fprintf (fp, "%s ", tableFunctor.name (code));
    if (tableFunctor.nbChildren (code) == 1) {
      depth = depth + 1;
      printStack (fp, depth, max);
    }
    else if (tableFunctor.nbChildren (code) > 1) {
      fprintf (fp, "( ");
      for (int i = 0; i < tableFunctor.nbChildren (code); i++) {
	depth = depth + 1;
	printStack (fp, depth, max);
	if (i < tableFunctor.nbChildren (code) - 1)
	  fprintf (fp, ", ");
      }
      fprintf (fp, ") ");
    }
  }

  void print (FILE * fp) {
    int depth = 0;
    printStack (fp, depth);
  }

  int depthStack (int & depth) {
    int code = rollout [depth];
    if (tableFunctor.nbChildren (code) == 1) {
      depth = depth + 1;
      depthStack (depth);
    }
    else if (tableFunctor.nbChildren (code) > 1) {
      for (int i = 0; i < tableFunctor.nbChildren (code); i++) {
	depth = depth + 1;
	depthStack (depth);
      }
    }
  }
  
  int depthInStack (int & depth, int d, int current) {
    int code = rollout [depth];
    //if (tableFunctor.nbChildren (code) == 0) {
    //  depth = depth + 1;
    // }
    //else 
    if (tableFunctor.nbChildren (code) == 1) {
      depth = depth + 1;
      if (depth == current)
	return d;
      int res = depthInStack (depth, d + 1, current);
      if (res != -1)
	return res;
    }
    else if (tableFunctor.nbChildren (code) > 1) {
      for (int i = 0; i < tableFunctor.nbChildren (code); i++) {
	depth = depth + 1;
	if (depth == current)
	  return d;
	int res = depthInStack (depth, d + 1, current);
	if (res != -1)
	  return res;
      }
    }
    return -1;
  }
  
  float evalStack (int & depth) {
    float evalChildren [MaxChildren], result = 0.0;
    int code = rollout [depth];
    if (code != 34)
      for (int i = 0; i < tableFunctor.nbChildren (code); i++) {
	depth = depth + 1;
	evalChildren [i] = evalStack (depth);
      }
    
    switch (code) { 
    case 0 : result = 1.0; break;
    case 1 : result = 2.0; break;
    case 2 : result = 3.0; break;
    case 3 : result = 4.0; break;
    case 4 : result = 5.0; break;
    case 5 : result = 6.0; break;
    case 6 : result = 7.0; break;
    case 7 : result = 8.0; break;
    case 8 : result = 9.0; break;
    case 9 : result = 10.0; break;
    case 10 : result = evalChildren [0] + evalChildren [1]; break;
    case 11 : result = evalChildren [0] - evalChildren [1]; break;
    case 12 : result = evalChildren [0] * evalChildren [1]; break;
    case 13 : 
      if (evalChildren [1] != 0.0)
	result = evalChildren [0] / evalChildren [1]; 
      else
	result = 0.0; 
      break;
    case 14 : result = - evalChildren [0]; break;
    case 15 : if (evalChildren [0] > evalChildren [1]) result = 1.0; break;
    case 16 : if (evalChildren [0] < evalChildren [1]) result = 1.0; break;
    case 17 : if (evalChildren [0] == evalChildren [1]) result = 1.0; break;
    case 18 : 
      if (evalChildren [0] > 0.0)
	result = sqrt (evalChildren [0]); 
      else
	result = sqrt (-evalChildren [0]); 
      break;
    case 19 : 
      if (evalChildren [0] > 0.0)
	result = log (evalChildren [0]);
      else if (evalChildren [0] == 0.0)
	result = 0; 
      else
	result = log (-evalChildren [0]); 
      break;
    case 20 : result = exp (evalChildren [0]); break;
    case 21 : result = pow (evalChildren [0], evalChildren [1]); break;
    case 22 : 
      if ((evalChildren [0] > 0.0) && (evalChildren [1] > 0.0))
	result = 1.0; 
      else
	result = 0.0;
      break;
    case 23 : 
      if ((evalChildren [0] > 0.0) || (evalChildren [1] > 0.0))
	result = 1.0; 
      else
	result = 0.0;
      break;
    case 24 : 
      if ((evalChildren [0] > 0.0) && (evalChildren [1] <= 0.0))
	result = 1.0; 
      else if ((evalChildren [0] <= 0.0) && (evalChildren [1] > 0.0))
	result = 1.0; 
      else
	result = 0.0;
      break;
    case 25 : 
      if (evalChildren [0] <= 0.0)
	result = 1.0; 
      else
	result = 0.0;
      break;
    case 26 : 
      if (evalChildren [0] != evalChildren [1])
	result = 1.0; 
      else
	result = 0.0;
    case 27 : result = 0.0; break;
    case 28 : result = 20.0; break;
    case 29 : result = 30.0; break;
    case 30 : result = 40.0; break;
    /*
    case 31 : direction = direction - 1; 
      if (direction < 0)
	direction = 3;
      break;
    case 32 : direction = (direction + 1) % 4; break;
    case 33 : move (); break;
    case 34 : 
      if (!ifFood ()) {
	depth = depth + 1;
	evalChildren [0] = evalStack (level, depth);
      }
      break;
    case 35 : break; // prgn2
    case 36 : break; // prgn3
    /**/
    case 37 : 
      if ((((long)evalChildren [1]) > 0) &&
	  (((long)evalChildren [0]) < INT_MAX) &&
	  (((long)evalChildren [1]) < INT_MAX))
	result = ((long)evalChildren [0]) % ((long)evalChildren [1]); 
      else
	result = 0.0; 
      break;
    case 38 : 
      if ((evalChildren [0] > 0.0) && (evalChildren [1] > 0.0))
	result = 0.0; 
      else
	result = 1.0;
      break;
    case 39 : 
      if ((evalChildren [0] > 0.0) || (evalChildren [1] > 0.0))
	result = 0.0; 
      else
	result = 1.0;
      break;
    case 40 : result = 0.0; break;
    /*
    case 41 : 
      result = Algebra [numAlgebra] [(int)evalChildren [0]] [(int)evalChildren [1]];
      break;
    /**/
    case 42 : result = 193.0; break;
    case 43 : 
      result = 1;
      for (int i = 2; i <= (int)evalChildren [0]; i++)
	result *= i;
      break;
    case 44 : result = 11.0; break;
    case 45 : result = 13.0; break;
    case 46 : result = 17.0; break;
    case 47 : result = 19.0; break;
    case 48 : result = 23.0; break;
    case 49 : result = 29.0; break;
    case 50 : result = 31.0; break;
    case 51 : result = 37.0; break;
    case 52 : result = 41.0; break;
    case 53 : result = 43.0; break;
    case 54 : result = 47.0; break;
    case 55 : result = 53.0; break;
    case 56 : result = 59.0; break;
    case 57 : result = 61.0; break;
    case 58 : result = 67.0; break;
    case 59 : result = 71.0; break;
    case 60 : result = 73.0; break;
    case 61 : result = 79.0; break;
    case 62 : result = 83.0; break;
    case 63 : result = 89.0; break;
    case 64 : result = 97.0; break;
    default : break;
    }
    if (code > FixedFunctors)
      return variableGP [code - FixedFunctors - 1];
    return result;
  }
  
  int legalMoves (Move moves [MaxMoveNumber]) {
    int nb = 0;
    for (int i = 0; i < tableFunctor.nbFunctors; i++)
      if (tableFunctor.active (i))
	if (length + feuillesOuvertes + tableFunctor.nbChildren (i) <= maxi) {
	  moves [nb] = i;
	  nb++;
	}
    return nb;
  }

  void play (Move m) {
    rollout [length] = m;
    length++;
    feuillesOuvertes += tableFunctor.nbChildren (m) - 1;
    int depth = 0;
    numberNode = numberLeaf (depth, 1);
  }

  bool terminal () {
    return feuillesOuvertes == 0;
  }
  
  int b [100];

  double score (int n = numberParity) {
    int nb = 0;
    if (n == 0) {
      int sum = 0;
      for (int i = 0; i < numberParity; i++) {
	variableGP [i] = b [i + 1];
	sum += b [i + 1];
      }
      int depth = 0;
      int eval = (int)evalStack (depth);
      if ((eval == 1) && ((sum & 1) == 0))
	nb++;
      if ((eval == 0) && ((sum & 1) == 1))
	nb++;
      return nb;
    }
    for (b [n] = 0; b [n] < 2; b [n]++)
      nb += score (n - 1);
    
    return (double)nb;
  }
};

char nameVariables [100] [100];

void initVariables () {
  for (int i = 0; i < numberParity; i++) {
    sprintf (nameVariables [i], "b%d", i + 1); 
    tableFunctor.add (nameVariables [i], 0, true);
  }
}

void initFunctors () {
  tableFunctor.add ("1", 0, false);
  tableFunctor.add ("2", 0, false);
  tableFunctor.add ("3", 0, false);
  tableFunctor.add ("4", 0, false);
  tableFunctor.add ("5", 0, false);
  tableFunctor.add ("6", 0, false);
  tableFunctor.add ("7", 0, false);
  tableFunctor.add ("8", 0, false);
  tableFunctor.add ("9", 0, false);
  tableFunctor.add ("10", 0, false);
  tableFunctor.add ("+", 2, false);
  tableFunctor.add ("-", 2, false);
  tableFunctor.add ("*", 2, false);
  tableFunctor.add ("/", 2, false);
  tableFunctor.add ("-unaire", 1, false);
  tableFunctor.add (">", 2, false);
  tableFunctor.add ("<", 2, false);
  tableFunctor.add ("=", 2, false);
  tableFunctor.add ("sqrt", 1, false);
  tableFunctor.add ("log", 1, false);
  tableFunctor.add ("exp", 1, false);
  tableFunctor.add ("pow", 2, false);
  tableFunctor.add ("and", 2, true);
  tableFunctor.add ("or", 2, true);
  tableFunctor.add ("xor", 2, false);
  tableFunctor.add ("not", 1, false);
  tableFunctor.add ("!=", 2, false);
  tableFunctor.add ("0", 0, false);
  tableFunctor.add ("20", 0, false);
  tableFunctor.add ("30", 0, false);
  tableFunctor.add ("40", 0, false);
  tableFunctor.add ("left", 0, false);
  tableFunctor.add ("right", 0, false);
  tableFunctor.add ("move", 0, false);
  tableFunctor.add ("ifFood", 1, false);
  tableFunctor.add ("prgn2", 2, false);
  tableFunctor.add ("prgn3", 3, false);
  tableFunctor.add ("%", 2, false);
  tableFunctor.add ("nand", 2, true);
  tableFunctor.add ("nor", 2, true);
  tableFunctor.add ("0", 0, false);
  tableFunctor.add ("*", 2, false);
  tableFunctor.add ("193", 0, false);
  tableFunctor.add ("fact", 1, false);
  tableFunctor.add ("11", 0, false);
  tableFunctor.add ("13", 0, false);
  tableFunctor.add ("17", 0, false);
  tableFunctor.add ("19", 0, false);
  tableFunctor.add ("23", 0, false);
  tableFunctor.add ("29", 0, false);
  tableFunctor.add ("31", 0, false);
  tableFunctor.add ("37", 0, false);
  tableFunctor.add ("41", 0, false);
  tableFunctor.add ("43", 0, false);
  tableFunctor.add ("47", 0, false);
  tableFunctor.add ("53", 0, false);
  tableFunctor.add ("59", 0, false);
  tableFunctor.add ("61", 0, false);
  tableFunctor.add ("67", 0, false);
  tableFunctor.add ("71", 0, false);
  tableFunctor.add ("73", 0, false);
  tableFunctor.add ("79", 0, false);
  tableFunctor.add ("83", 0, false);
  tableFunctor.add ("89", 0, false);
  tableFunctor.add ("97", 0, false);
  initVariables ();
}

//#include "nestedSH.c"
//#include "nested.c"
//#include "nrpa.c"

int main(int argc, char *argv []) {
//srand (time(NULL));
  initFunctors ();
  //  testTimeNested (2);
  //testTimeNRPA (4);
  Nrpa<Board, Move, 5, MaxPlayoutLength, MaxLegalMoves>::test(Options::parse(argc, argv));
  
  exit (0);
  while (true) {
    double pol [MaxMoveNumber];
    for (int i = 0 ; i < MaxMoveNumber ; i++)
      pol [i] = 0;
    Board b;
    //nested (b, 3);
    //nrpa (3, pol);
  }
}
