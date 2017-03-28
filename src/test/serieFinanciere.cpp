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

#include <iostream>
#include <fstream>

#include <nrpa.hpp>

using namespace std;

const int MaxChildren = 3;
const int MaxFunctor = 100;
const int MaxExpressions = 1000;
const int MaxNodes = 2000;
const int MaxVariables = 50;
const int FixedFunctors = 67;
const int MaxScore = 10000;

const int MaxPlayoutLength = 200;
//const int MaxMoveNumber = MaxFunctor * MaxPlayoutLength;
const int MaxMoveNumber = MaxFunctor * MaxPlayoutLength + MaxFunctor * 1024;
const int MaxLegalMoves = 100;

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

double _N(double arg) 
{
    double b1 =  0.31938153; 
    double b2 = -0.356563782; 
    double b3 =  1.781477937;
    double b4 = -1.821255978;
    double b5 =  1.330274429; 
    double p  =  0.2316419; 
    double c2 =  0.3989423; 

    if (arg >  6.0) { return 1.0; }; // this guards against overflow 
    if (arg < -6.0) { return 0.0; };
    double a=fabs(arg); 
    double t = 1.0/(1.0+a*p); 
    double b = c2*exp((-arg)*(arg/2.0)); 
    double n = ((((b5*t+b4)*t+b3)*t+b2)*t+b1)*t; 
    n = 1.0-b*n; 
    if ( arg < 0.0 ) n = 1.0 - n; 
    return n; 
}

//Fonction de lecture des données

int nbVariables;
vector< vector<double> > inputs;
vector<double> targets;

void load_training_sample(const char * nom_sample){
  double x;
  ifstream fs(nom_sample);
  if (!fs){
    cout<<"fichier echantillon apprentissage non trouve"<<endl;
    exit(0);  
  }
  fs>>nbVariables;
  //cout<<"nbre var "<<nbVariables<<endl;
  //inputs.clear();
  while (! fs.eof()){
    vector<double> v;
    for (int i=0; i<nbVariables; i++){
      fs>>x;
      v.push_back(x);
    }
    if (! fs.eof())
      inputs.push_back(v);
    
  }
  fs.close();
}

// fonction de lecture de l'échantillon target séparée
void load_target_sample(const char * nom_target){
  double x;
  ifstream ft(nom_target);
  if (!ft){
    cout<<"fichier non trouve"<<endl;
    exit(0);  
  }
  //targets.clear();
  ft>>x;
  while (! ft.eof()){		
    targets.push_back(x);
    ft>>x;
  }
  ft.close();  
}

int nbVariablesTest;
vector< vector<double> > inputsTest;
vector<double> targetsTest;

void load_training_sampleTest(const char * nom_sample){
  double x;
  ifstream fs(nom_sample);
  if (!fs){
    cout<<"fichier echantillon apprentissage non trouve"<<endl;
    exit(0);  
  }
  fs>>nbVariablesTest;
  //cout<<"nbre var "<<nbVariables<<endl;
  //inputsTest.clear();
  while (! fs.eof()){
    vector<double> v;
    for (int i=0; i<nbVariablesTest; i++){
      fs>>x;
      v.push_back(x);
    }
    if (! fs.eof())
      inputsTest.push_back(v);
    
  }
  fs.close();
}

// fonction de lecture de l'échantillon target séparée
void load_target_sampleTest(const char * nom_target){
  double x;
  ifstream ft(nom_target);
  if (!ft){
    cout<<"fichier non trouve"<<endl;
    exit(0);  
  }
  //targetsTest.clear();
  ft>>x;
  while (! ft.eof()){		
    targetsTest.push_back(x);
    ft>>x;
  }
  ft.close();  
}



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
    maxi = 30;
    numberNode = 1;
  }

  int code (Move m) {
    //return m;
    return length * MaxFunctor + m;
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
    fprintf (stderr, "score Test = %f\n", scoreTest ());
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
    case 65 : result = _N (evalChildren [0]); break;
    case 66 : result = sin (evalChildren [0]); break;
    case 67 : result = cos (evalChildren [0]); break;
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
    //int depth = 0;
    //numberNode = numberLeaf (depth, 1);
  }

  bool terminal () {
    return feuillesOuvertes == 0;
  }
  
  double score (int level = 2) {
    double output;
    double target;
								
    float x=0;
    double MSE=0;
    int nbTests = inputs.size ();
    if (level == 1)
      nbTests = 100;
    for(int i=0; i<nbTests; i++){
      for (int j=0; j<nbVariables; j++)
        variableGP[j]=inputs[i][j];
      int depth = 0;
      double eval = evalStack (depth);
      target=targets[i];
      MSE += powf((target - eval), 2);
      //MSE += fabs(target - output);
      //fprintf (stderr, "eval = %f, target = %f, MSE = %f\n", eval, target, MSE);
    }
    MSE/=nbTests;
    //fprintf (stderr, "MSE = %f\n", MSE);

    return -MSE;
  }

  double scoreTest () {
    double output;
    double target;
								
    float x=0;
    double MSE=0;
    for(int i=0; i<inputsTest.size(); i++){
      for (int j=0; j<nbVariablesTest; j++)
        variableGP[j]=inputsTest[i][j];
      int depth = 0;
      double eval = evalStack (depth);
      target=targetsTest[i];
      MSE += powf((target - eval), 2);
      //MSE += fabs(target - output);
      //fprintf (stderr, "eval = %f, target = %f, MSE = %f\n", eval, target, MSE);
    }
    MSE/=inputsTest.size();
    //fprintf (stderr, "MSE = %f\n", MSE);

    return -MSE;
  }

  double writeScoreTest (char *s) {
    FILE *fp = fopen (s, "w");
    if (fp != NULL) {
      double output;
      double target;
      
      float x=0;
      double MSE=0;
      for(int i=0; i<inputsTest.size(); i++){
        for (int j=0; j<nbVariablesTest; j++)
          variableGP[j]=inputsTest[i][j];
        int depth = 0;
        double eval = evalStack (depth);
        target=targetsTest[i];
        MSE += powf((target - eval), 2);
        //MSE += fabs(target - output);
        fprintf (fp, "%f %f\n", eval, powf((target - eval), 2));
      }
      fclose (fp);
      MSE/=inputsTest.size();
      //fprintf (stderr, "MSE = %f\n", MSE);
      
      return -MSE;
    }
    return 0.0;
  }
};

char nameVariables [100] [100];

void initVariables () {
  for (int i = 0; i < 3; i++) {
    sprintf (nameVariables [i], "x%d", i + 1); 
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
  tableFunctor.add ("+", 2, true);
  tableFunctor.add ("-", 2, true);
  tableFunctor.add ("*", 2, true);
  tableFunctor.add ("/", 2, true);
  tableFunctor.add ("-unaire", 1, true);
  tableFunctor.add (">", 2, false);
  tableFunctor.add ("<", 2, false);
  tableFunctor.add ("=", 2, false);
  tableFunctor.add ("sqrt", 1, false);
  tableFunctor.add ("log", 1, false);
  tableFunctor.add ("exp", 1, false);
  tableFunctor.add ("pow", 2, false);
  tableFunctor.add ("and", 2, false);
  tableFunctor.add ("or", 2, false);
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
  tableFunctor.add ("nand", 2, false);
  tableFunctor.add ("nor", 2, false);
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
  tableFunctor.add ("N", 1, true);
  tableFunctor.add ("sin", 1, true);
  tableFunctor.add ("cos", 1, true);
  initVariables ();
}

//#include "nested.c"
//#include "nrpa.c"

int main(int argc, char *argv []) {
  bool echantillon [100];
  int e = 1;

  for (int i = 0; i < 100; i++)
    echantillon [i] = false;

  for (int i = 1; i < argc; i++) {
    e = atoi (argv [i]);
    echantillon [e] = true;
    fprintf (stderr, "echantillon %d\n", e);
  }

  //srand (time(NULL));
  initFunctors ();

  inputs.clear();
  targets.clear();
  if (echantillon [1]) {
    load_training_sample("data/serieFinanciere/trainbs1.txt");
    load_target_sample("data/serieFinanciere/targetbs1.txt");
  }
  if (echantillon [2]) {
    load_training_sample("data/serieFinanciere/trainbs2.txt");
    load_target_sample("data/serieFinanciere/targetbs2.txt");
  }
  if (echantillon [3]) {
    load_training_sample("data/serieFinanciere/trainbs3.txt");
    load_target_sample("data/serieFinanciere/targetbs3.txt");
  }
  if (echantillon [4]) {
    load_training_sample("data/serieFinanciere/trainbs4.txt");
    load_target_sample("data/serieFinanciere/targetbs4.txt");
  }
  if (echantillon [5]) {
    load_training_sample("data/serieFinanciere/trainbs5.txt");
    load_target_sample("data/serieFinanciere/targetbs5.txt");
  }
  if (echantillon [6]) {
    load_training_sample("data/serieFinanciere/trainbs6.txt");
    load_target_sample("data/serieFinanciere/targetbs6.txt");
  }
  if (echantillon [7]) {
    load_training_sample("data/serieFinanciere/trainbs7.txt");
    load_target_sample("data/serieFinanciere/targetbs7.txt");
  }
  if (echantillon [8]) {
    load_training_sample("data/serieFinanciere/trainbs8.txt");
    load_target_sample("data/serieFinanciere/targetbs8.txt");
  }
  if (echantillon [9]) {
    load_training_sample("data/serieFinanciere/trainbs9.txt");
    load_target_sample("data/serieFinanciere/targetbs9.txt");
  }

  for (int i = 0; i < 100; i++)
    echantillon [i] = false;
  inputsTest.clear();
  targetsTest.clear();
  if (echantillon [1] == false) {
    load_training_sampleTest("data/serieFinanciere/trainbs1.txt");
    load_target_sampleTest("data/serieFinanciere/targetbs1.txt");
  }
  if (echantillon [2] == false) {
    load_training_sampleTest("data/serieFinanciere/trainbs2.txt");
    load_target_sampleTest("data/serieFinanciere/targetbs2.txt");
  }
  if (echantillon [3] == false) {
    load_training_sampleTest("data/serieFinanciere/trainbs3.txt");
    load_target_sampleTest("data/serieFinanciere/targetbs3.txt");
  }
  if (echantillon [4] == false) {
    load_training_sampleTest("data/serieFinanciere/trainbs4.txt");
    load_target_sampleTest("data/serieFinanciere/targetbs4.txt");
  }
  if (echantillon [5] == false) {
    load_training_sampleTest("data/serieFinanciere/trainbs5.txt");
    load_target_sampleTest("data/serieFinanciere/targetbs5.txt");
  }
  if (echantillon [6] == false) {
    load_training_sampleTest("data/serieFinanciere/trainbs6.txt");
    load_target_sampleTest("data/serieFinanciere/targetbs6.txt");
  }
  if (echantillon [7] == false) {
    load_training_sampleTest("data/serieFinanciere/trainbs7.txt");
    load_target_sampleTest("data/serieFinanciere/targetbs7.txt");
  }
  if (echantillon [8] == false) {
    load_training_sampleTest("data/serieFinanciere/trainbs8.txt");
    load_target_sampleTest("data/serieFinanciere/targetbs8.txt");
  }
  if (echantillon [9] == false) {
    load_training_sampleTest("data/serieFinanciere/trainbs9.txt");
    load_target_sampleTest("data/serieFinanciere/targetbs9.txt");
  }

  Nrpa<Board, Move, 5, MaxPlayoutLength, MaxLegalMoves>::test(Options::parse(argc, argv));

  // //testTimeNested (3);
  // //testTimeNRPA (4);
  // //exit (0);
  // //while (true) {
  // Board best;
  // double bestScoreTest = -DBL_MAX;
  // for (int j = 0; j < 11; j++) {
  //   double pol [MaxMoveNumber];
  //   for (int i = 0 ; i < MaxMoveNumber ; i++)
  //     pol [i] = 0;
  //   Board b;
  //   stopOnTime = true;
  //   bestScoreNested = -DBL_MAX;
  //   nextTimeNested = firstTimeNested;
  //   indexTimeNested = 0;
  //   nbTimesNested = 20; //17;
  //   startClockNested = clock ();
  //   nested (b, 3);
  //   double s = bestBoard.scoreTest ();
  //   if (s > bestScoreTest) {
  //     bestScoreTest = s;
  //     best = bestBoard;
  //     fprintf (stderr, "\n\nbest score test = %f\n", bestScoreTest);
  //     best.print (stderr);
  //     fprintf (stderr, "\n\n");
  //   }
  //   //break;
  //   //nrpa (3, pol);
  // }
  // fprintf (stderr, "best score test %d = %f\n", e, bestScoreTest);
  // char s [1000];
  // sprintf (s, "fich%d.txt", e);
  // double score = best.writeScoreTest (s);
  // fprintf (stderr, "best %d  = %f\n", e, score);
  // best.print (stderr);
}
