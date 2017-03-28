#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cfloat>
#include <cmath>
#include <cassert>
#include <string>
#include <nrpa.hpp>
using namespace std;

const int MaxVilles = 50;
const int MaxTime = 100;

const int MaxPlayoutLength = MaxVilles;
//const int MaxMoveNumber = MaxVilles;
//const int MaxMoveNumber = 3 * MaxVilles * MaxVilles;
const int MaxMoveNumber = MaxTime * MaxVilles * MaxVilles;
//const int MaxMoveNumber = MaxVilles * MaxVilles * MaxVilles;
const int MaxLegalMoves = MaxVilles;

class Move {
 public:
  int d, a;
};

class Board {
 public:
  static int n; 
  static vector<double> window_start;
  static vector<double> window_end;
  static vector<vector<double> > distance;
  static string instance;  

  vector<int> permutation;
  Move rollout [MaxPlayoutLength];
  int length;
  bool seen [MaxVilles];

  int _constraint_violations;
  double _makespan;
  double _tourcost; // Sum of the traversal cost along the tour.
  
  Board () : permutation (1,0), // Start at the depot.
	_constraint_violations (0),
	_makespan (0),
	_tourcost (0) {
    permutation.reserve (n+1);
    for (int i = 1; i < n; i++) 
      seen [i] = false;
    length = 0;
  }

  static void LoadInstance (string filename) {
    ifstream indata;
    double rtime;
    double ddate;
    
    instance = filename;
    indata.open (instance.c_str());
    if (!indata) { // file couldn't be opened
      cout << "error: file " << instance.c_str() << " could not be opened"
	   << endl;
      exit (1);
    }
    
    // Customer 0 is the depot.
    indata >> n;
    if (!indata || n <= 0) {
      cout << "error: invalid number of customers" << endl;
      exit (1);
    }
    
    window_start.reserve(n);
    window_end.reserve(n);
    distance.reserve(n);
    
    for (int i = 0 ; i < n; i++) {
      distance.push_back(vector<double>(n,0));
      for (int j = 0; j < n; j++) {
	indata >> distance[i][j];
      }
    }
    
    if (!indata) {
      cout << "error: invalid time windows" << endl;
      exit (1);
    }
    
    for (int i = 0; i < n; i++) {
      indata >> rtime >> ddate;
      window_start.push_back (rtime);
      window_end.push_back(ddate);
    }
    
    if (!indata) {
      cout << "error: invalid distance matrix" << endl;
      exit (1);
    }
    
    indata.close();
  }
  
  double makespan() const {
    return _makespan;
  }

  bool terminal () {
    if (int(permutation.size() - 1) < n) 
      return false;
    return true;
  }

  void evaluate() {
    _makespan = 0;
    _tourcost = 0;
    int prev = 0; // starts at the depot
    int cviols = 0;
    
    if (int(permutation.size() - 1) != n) {
      printf ("invalid: (permutation.size() == %d) != (n == %d)\n",
	      int(permutation.size() - 1), n);
      print (stderr);
      exit (1);
    }
    
    for (int i = 1; i < n; i++) {
      int node = permutation[i];
      
      _tourcost += distance[prev][node];
      _makespan = max (_makespan + distance[prev][node], window_start[node]);
      
      if (_makespan > window_end[node]) {
	cviols++;
      }
      prev = node;
    }
    
    // finish at the depot
    _tourcost += distance[prev][0];
    
    _makespan = max (_makespan + distance[prev][0], window_start[0]);
    
    if (_makespan > window_end[0])
      cviols++;
    
    _constraint_violations = cviols;
  }
  
  double score () {
    evaluate ();
    return -(1000000 *_constraint_violations + _tourcost);
    //return _constraint_violations + _makespan;
  }

  void play (Move m) {
    permutation.push_back (m.a);
    seen [m.a] = true;
    rollout [length] = m;
    length++;
    _tourcost += distance[m.d][m.a];
    _makespan = max (_makespan + distance[m.d][m.a], window_start[m.a]);
  }

  int code (Move m) {
    return m.d * MaxVilles + m.a;
    //return permutation.size () * MaxVilles * MaxVilles + m.d * MaxVilles + m.a;
    //return permutation.size () * MaxVilles + m.a;
    //return m.a;
    //return (_makespan / 200) * MaxVilles * MaxVilles + m.d * MaxVilles + m.a;
    if (_makespan + distance [m.d] [m.a] < window_start [m.a])
      return (1 + (window_start [m.a] - (_makespan + distance [m.d] [m.a])) / 10) * MaxVilles * MaxVilles + m.d * MaxVilles + m.a;
    return m.d * MaxVilles + m.a;
    // arrive dans la fenetre
    if (_makespan + distance [m.d] [m.a] > window_end [m.a])
      return 2 * MaxVilles * MaxVilles + m.d * MaxVilles + m.a;
    else if (_makespan + distance [m.d] [m.a] < window_start [m.a])
      return MaxVilles * MaxVilles + m.d * MaxVilles + m.a;
    else
      return m.d * MaxVilles + m.a;
  }

  int legalMoves (Move moves [MaxLegalMoves]) {
    _makespan = 0;
    _tourcost = 0;
    int prev = 0; // starts at the depot
    int cviols = 0;
    
    int maxMoves = n - permutation.size ();
    
    int nb = 0;
    /*
      for (int i = 1; i < n; i++) 
      if (!seen [i]) 
      {
      moves [nb] = i; 
      nb++; 
      } 
    */
    
    /**/
    for (int i = 1; i < permutation.size (); i++) {
      int node = permutation [i];
      
      _tourcost += distance[prev][node];
      _makespan = max (_makespan + distance [prev] [node], window_start [node]);
      
      if (_makespan > window_end[node]) {
	cviols++;
      }
      prev = node;
    }
    
    if (true) {
      if (permutation.size () > 0)
	prev = permutation [permutation.size () - 1];
      for (int i = 1; i < n; i++)
	if (!seen [i]) {
	  moves [nb].d = prev;
	  moves [nb].a = i;
	  if (_makespan + distance [prev] [i] > window_end [i])
	    nb++;
	}
    }
    
    if (true)
      if (nb == 0) {
	if (permutation.size () > 0)
	  prev = permutation [permutation.size () - 1];
	for (int i = 1; i < n; i++)
	  if (!seen [i]) {
	    moves [nb].d = prev;
	    moves [nb].a = i;
	    bool tropTard = false;
	    for (int j = 1; j < n; j++)
	      if (j != i)
		if (!seen [j])
		  if ((_makespan <= window_end [j]) &&
		      (_makespan + distance [prev] [j] <= window_end [j]) &&
		      (max (_makespan + distance [prev] [i], window_start [i]) > window_end [j]))
		    tropTard = true;
	    if (!tropTard)
	      nb++;
	  }
      }
    
    if (nb == 0)
      for (int i = 1; i < n; i++)
	if (!seen [i]) {
	  moves [nb].d = prev;
	  moves [nb].a = i;
	  nb++;
	}
    /**/
    return nb;
  }

  void print (FILE *stream) const {
    fprintf (stream, "makespan = %.2f\ttourcost = %.2f\tconstraint violations = %d\tpermutation =",
	     double (makespan()), double(_tourcost), _constraint_violations);
    
    // Customers 0 and n+1 are always the depot (permutation[0] == 0,
    // permutation[n] == 0), so do not print them.
    for (int i = 1; i < int(permutation.size()) - 1; i++) {
      fprintf (stream, " %d", permutation[i]);
    }
    fprintf (stream, "\n");
  }
};

string Board::instance;
int Board::n = 0;
vector<double> Board::window_start;
vector<double> Board::window_end;
vector<vector<double> > Board::distance;

//#include "nestedSH.c"
//#include "nested.c"
//#include "nrpa.c"

int main (int argc, char *argv []) {
  const char * output_instance = "SolomonPotvinBengio/rc_204.1.sol"; 
  string input_instance =        "SolomonPotvinBengio/rc_204.1.txt"; 
  //string input_instance =        "SolomonPotvinBengio/rc_206.3.txt"; 
  //const char * output_instance = "SolomonPesant/rc203.0.sol";
  //string input_instance =        "SolomonPesant/rc203.0";
  /*   const char * output_instance = "OhlmannThomas/n200w140.001.txt.sol"; */
  /*   string input_instance =        "OhlmannThomas/n200w140.001.txt"; */
  /*   const char * output_instance = "SolomonPesant/rc204.2.sol"; */
  /*   string input_instance =        "SolomonPesant/rc204.2"; */
  /*   const char * output_instance = "AFG/rbg010a.tw.sol"; */
  /*   string input_instance =        "AFG/rbg010a.tw"; */
  
  if (argc > 1) 
    input_instance = argv [1];
  
  Board::LoadInstance (input_instance);
  //srand (time(NULL));
  //testTimeNested (2);
  //testTimeNRPA (3);
  //exit (0);
  /**/
  Nrpa<Board, Move, 5, MaxPlayoutLength, MaxLegalMoves>::test(Options::parse(argc, argv));

  // while (true) {
  //   Board b;
  //   //nested (b, 3);
  //   Policy pol;
  //   nrpa (4, pol);
  //   break;
  // }
  // /**/
}

