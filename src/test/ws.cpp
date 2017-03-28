#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>
#include<iostream>
#include<algorithm>

#include <nrpa.hpp>

const int MaxPlayoutLength = 10000;
const int MaxPartition = 9;
const int MaxMoveNumber = MaxPartition * MaxPlayoutLength;
const int MaxLegalMoves = MaxPartition;

double epsilon = 0.0;

class Move {
 public:
  int partition, number;
};


class Board {
 public:
  int partition [MaxPartition] [MaxPlayoutLength];
  int sizePartition [MaxPartition];
  bool possible [MaxPartition] [MaxPlayoutLength];
  int number;
  int length;
  Move rollout [MaxPlayoutLength];
  bool policy;

  Board () {
    for (int i = 0; i < MaxPartition; i++) 
      sizePartition [i] = 0;
    for (int i = 0; i < MaxPartition; i++) 
      for (int j = 0; j < MaxPlayoutLength; j++) 
	possible [i] [j] = true;
    number = 1;
    length = 0;
    policy = true;
  }

  void print (FILE *fp) {
    if (false) {
      for (int i = 0; i < MaxPartition; i++) {
	fprintf (fp, "[");
	for (int j = 0; j < sizePartition [i]; j++)
	  fprintf (fp, "%d, ", partition [i] [j]);
	fprintf (fp, "]\n");
      }
    }
    fprintf (fp, "score = %d\n", length);
  }

  int code (Move m) {
    // better for Schur Numbers s(6)
    //return m.number + m.partition * MaxPlayoutLength;
    //return m.number + m.partition * MaxPlayoutLength + sizePartition [m.partition] * MaxPlayoutLength * MaxPartition;
    int previous = 0;
    if (sizePartition [m.partition] > 0)
      previous = partition [m.partition] [sizePartition [m.partition] - 1];
    return m.number + m.partition * MaxPlayoutLength + previous * MaxPlayoutLength * MaxPartition;
    // number of free numbers in a row above in the stack
    int n = 0;
    for (n = 0; n < 5; n++)
      if (!possible [m.partition] [sizePartition [m.partition] + n])
	break;
    return m.number + m.partition * MaxPlayoutLength + n * MaxPlayoutLength * MaxPartition;
  }

  bool legal (Move m) {
    return possible [m.partition] [number];
    for (int i = 0; i < sizePartition [m.partition]; i++)
      for (int j = 0; j < sizePartition [m.partition]; j++)
	if (i != j)
	  if (partition [m.partition] [i] + partition [m.partition] [j] == number)
	    return false;
    return true;
  }

  int legalMoves (Move mvs [MaxLegalMoves]) {
    int nbMoves = 0;
    Move m;
    if (policy)
      for (int i = 0; i < MaxPartition; i++) 
        if (sizePartition [i] > 0)
          if (partition [i] [sizePartition [i] - 1] == number - 1) {
            m.partition = i;
            m.number = number;
            if (legal (m)) {
              mvs [nbMoves] = m;
              nbMoves++;
            }
          }
    double proba = (double)(rand () / (RAND_MAX + 1.0));
    //fprintf (stderr, "%f,", proba);
    if ((nbMoves == 0) || (proba < epsilon))
      for (int i = 0; i < MaxPartition; i++) {
	m.partition = i;
	m.number = number;
	if (legal (m)) {
	  mvs [nbMoves] = m;
	  nbMoves++;
	}
      }
    return nbMoves;
  }

  void play (Move & m) {
    partition [m.partition] [sizePartition [m.partition]] = number;
    //for (int i = 0; i <= sizePartition [m.partition]; i++) {
    for (int i = 0; i < sizePartition [m.partition]; i++) {
      int j = partition [m.partition] [i] + number;
      if (j >= MaxPlayoutLength)
	break;
      possible [m.partition] [j] = false;
    }
    sizePartition [m.partition]++;
    number++;
    rollout [length] = m;
    length++;
  }

  bool terminal () {
    Move m;
    for (int i = 0; i < MaxPartition; i++) {
      m.partition = i;
      m.number = number;
      if (legal (m)) 
	return false;
    }
    return true;
  }

  double score () {
    return (double)(length);
  }
 };

int seed = 1;

//#include "nestedSH.c"
//#include "nested.c"
//#include "nrpa.c"
//#include "beamnrpa.c"

// - remplir avec les sequences inferieures
// - les coups pour un nombre sont toujours les memes si code = nombre + partition

int main(int argc, char *argv []) {
  if (argc > 1)  {
    seed = atoi (argv [1]);
    srand (seed);
  }
  SizeBeam [1] = 1;
  SizeBeam [2] = 1;//10;
  SizeBeam [3] = 1;//10;
  SizeBeam [4] = 1;//10;
  startLearning = 0;//20;
  if (argc > 2)  {
    SizeBeam [1] = atoi (argv [2]);
  }
  if (argc > 3)  {
    SizeBeam [2] = atoi (argv [3]);
  }
  if (argc > 4)  {
    SizeBeam [3] = atoi (argv [4]);
  }
  if (argc > 5)  {
    SizeBeam [4] = atoi (argv [5]);
  }
  if (argc > 6)  {
    SizeBeam [5] = atoi (argv [6]);
  }

  k = 0.0;
  if (argc > 7)  {
    k = atof (argv [7]);
  }

  epsilon = 0.0;
  if (argc > 8)  {
    epsilon = atof (argv [8]);
  }

  //testTimeNested (2);
  //  testTimeNRPA (5);
  Nrpa<Board, Move, 5, MaxPlayoutLength, MaxLegalMoves>::test(Options::parse(argc, argv));

  exit (0);
  // /**/
  // while (true) {
  //   //Policy pol;
  //   Board b;
  //   //nested (b, 3);
  //   //nrpa (5, pol);
  //   fprintf (stderr, "best score %lf\n", bestBoard.score ());
  //   break;
  // }
  // /**/
}

