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

using namespace std;

const int MaxMoveNumber = 2;
const int MaxLegalMoves = 2;
const int MaxPlayoutLength = 101;

typedef int Move;

class Board {
 public:
  Move rollout [MaxPlayoutLength];
  int length;
  int nbMovesLeft;
  int maxi;

  Board (int m = 100) {
    length = 0;
    maxi = m;
    nbMovesLeft = 0;
  }

  int code (Move m) {
    return m + length * MaxMoveNumber;
    return m;
  }
  
  int legalMoves (Move moves [MaxLegalMoves]) {
    moves [0] = 0;
    moves [1] = 1;
    return 2;
  }

  void play (Move m) {
    if (m == 0)
      nbMovesLeft++;
    rollout [length] = m;
    length++;
  }

  bool terminal () {
    return length == maxi;
  }
  
  double score () {
    return nbMovesLeft;
  }

  void print (FILE *fp) {
    fprintf (fp, "nbMovesLeft = %d, length = %d\n", nbMovesLeft, length);
  }
};

//#include "nestedSH.c"
//#include "nested.c"
//#include "nestedSimple.c"
#include "nrpa.cpp"
//#include "beamnrpa.c"

int main(int argc, char *argv []) {
  //testTimeNested (3);
  testTimeNRPA (4);
  exit (0);
  while (true) {
    double pol [MaxMoveNumber];
    for (int i = 0 ; i < MaxMoveNumber ; i++)
      pol [i] = 0;
    Board b;
    //testTimeNested (3);
    break;
    //nested (b, 4);
    //nrpa (4, pol);
    fprintf (stderr, "score final : ");
    bestBoard.print (stderr);
    fprintf (stderr, "\n");
  }
}
