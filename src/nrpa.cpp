#include <float.h>
#include "rollout.hpp" 
#include "nrpa.hpp"

#include <vector>
#include <iostream>
#include <fstream>




// class MoveStat {
// public:
//   int code;
//   double sumScores;
//   int nbPlayouts;
// };

// class PolicyAMAF {
// public:
//   vector<MoveStat> table [SizeTablePolicy + 1];
//   double maxScore, minScore;

//   PolicyAMAF () {
//     maxScore = 0.0;
//     minScore = DBL_MAX;
//   }

//   void add (int code, double score) {
//     if (score > maxScore)
//       maxScore = score;
//     if (score < minScore)
//       minScore = score;
//     int index = code & SizeTablePolicy;
//     for (int i = 0; i < table [index].size (); i++) {
//       if (table [index] [i].code == code) {
// 	table [index] [i].sumScores += score;
// 	//if (score > table [index] [i].sumScores)
// 	//table [index] [i].sumScores = score;
// 	table [index] [i].nbPlayouts++;
// 	return;
//       }
//     }
//     MoveStat p;
//     p.code = code;
//     p.sumScores = score;
//     p.nbPlayouts = 1;
//     table [index].push_back (p);
//   }

//   double get (int code) {
//     int index = code & SizeTablePolicy;
//     for (int i = 0; i < table [index].size (); i++)
//       if (table [index] [i].code == code) {
// 	return (table [index] [i].sumScores / table [index] [i].nbPlayouts - minScore) / (maxScore - minScore);
// 	//return table [index] [i].sumScores / maxScore;
//       }
//     return 0.0;
//   }

// };

// PolicyAMAF policyAMAF;

// class ProbabilityCode {
//   friend ostream &operator<<(ostream &, ProbabilityCode); 
// public:
//   int code;
//   double proba;
// };

// ostream &operator<<(ostream &os, ProbabilityCode p){
//   os<<"Code: "<<p.code<<" Proba : "<<p.proba; 
//   return os; 
// }


// // void adapt (int length, Move rollout [MaxPlayoutLength], double pol [MaxMoveNumber]) {
// //   for (int i = 0; i < MaxMoveNumber; i++)
// //     polpAdapt [i] = pol [i];
// //   int nbMoves;
// //   Move moves [MaxLegalMoves];
// //   Board board;
// //   for (int i = 0; i < length; i++) {
// //     nbMoves = board.legalMoves (moves);
// //     polpAdapt [board.code (rollout [i])] += ALPHA;
// //     double z = 0.0;
// //     for (int j = 0; j < nbMoves; j++)
// //       z += exp (pol [board.code (moves [j])]); 
// //     for  (int j = 0; j < nbMoves; j++)
// //       polpAdapt [board.code (moves [j])] -= ALPHA * exp (pol [board.code (moves [j])]) / z; 
// //     board.play (rollout [i]);
// //   }
// //   for (int i = 0; i < MaxMoveNumber; i++)
// //     pol [i] = polpAdapt [i];
// // }

// // /*
// // void adaptLevel (int length, int level, double pol [MaxMoveNumber]) {
// //   memcpy (polpAdapt, pol, MaxMoveNumber * sizeof (double));
// //   //for (int i = 0; i < MaxMoveNumber; i++)
// //   //polpAdapt [i] = pol [i];
// //   for (int i = 0; i < length; i++) {
// //     polpAdapt [bestCodeBestRollout [level] [i]] += ALPHA;
// //     double z = 0.0;
// //     for (int j = 0; j < nbMovesBestRollout [level] [i]; j++)
// //       z += exp (pol [codeBestRollout [level] [i] [j]]); 
// //     for  (int j = 0; j < nbMovesBestRollout [level] [i]; j++)
// //       polpAdapt [codeBestRollout [level] [i] [j]] -= ALPHA * exp (pol [codeBestRollout [level] [i] [j]]) / z; 
// //   }
// //   //for (int i = 0; i < MaxMoveNumber; i++)
// //   //pol [i] = polpAdapt [i];
// //   memcpy (pol, polpAdapt, MaxMoveNumber * sizeof (double));
// // }
// // /**/

// // Policy polAdapt;


// // void adaptLevel (int length, int level, Policy & pol) {
// //     if(level == 4) {
// //         Rollout r(bestCodeBestRollout[level], lengthBestRollout [level], level, scoreBestRollout[level]);
// // 	r.compareAndSwap("blah", "lock"); 
// // 	// copy(r.data(), r.data() + r.length(), bestCodeBestRollout[level]);
// // 	// lengthBestRollout[level] = r.length();
// // 	// scoreBestRollout[level] = r.score(); 
// // 	/* Code table */
// // 	for (int i = 0; i < length; i++) {
// // 	  cout<<"Code table for acion at action nmbr " <<i<<endl;
// // 	  for(int j = 0; j < MaxLegalMoves; j++){
// // 	    cout<<"Code : "<<j<<" -> "<<codeBestRollout [level] [i] [j]<<endl;
// // 	  }
// // 	}
// //     }

// //     /* Copy pol into pol adapt (assign same probs to the same move taking into account code change) */
// //   for (int i = 0; i < length; i++) {
// //     for (int j = 0; j < nbMovesBestRollout [level] [i]; j++)
// //       polAdapt.set (codeBestRollout [level] [i] [j], pol.get (codeBestRollout [level] [i] [j]));
// //   }


// //   for (int i = 0; i < length; i++) {
    
// //     /* For each move in bestrollout, increase the prob of the corresponding move by 1 */ 
// //     polAdapt.set (bestCodeBestRollout [level] [i], polAdapt.get (bestCodeBestRollout [level] [i]) + ALPHA);
// //     double z = 0.0;

// //     /* Adjust probs */

// //     /* sum probls for each move */ 
// //     for (int j = 0; j < nbMovesBestRollout [level] [i]; j++)
// //       z += exp (pol.get (codeBestRollout [level] [i] [j])); 

// //     /* substract exp(policy(code(m))) see paper */
// //     for  (int j = 0; j < nbMovesBestRollout [level] [i]; j++)
// //       polAdapt.set (codeBestRollout [level] [i] [j], polAdapt.get (codeBestRollout [level] [i] [j]) - ALPHA * exp (pol.get (codeBestRollout [level] [i] [j])) / z); 
// //   }

// //   /* Copy result into pol */ 
// //   for (int i = 0; i < length; i++) {
// //     for (int j = 0; j < nbMovesBestRollout [level] [i]; j++)
// //       pol.set (codeBestRollout [level] [i] [j], polAdapt.get (codeBestRollout [level] [i] [j]));
// //   }
// // }
// // /**/

// // static double polpLevel [10] [MaxMoveNumber];
// // static Policy polLevel [10];

// // double alpha = 1.0;

// void writeValues (int nbThreads) {
//   char s [1000];
//   //  sprintf (s, "NRPA.time.nbThreads=%d.nbSearches=%d.k=%2.3f.epsilon=%2.4f.plot", nbThreads, nbSearchesNRPA, k, epsilon);
//   sprintf (s, "NRPA.time.nbThreads=%d.nbSearches=%d.k=%2.3f", nbThreads, nbSearchesNRPA, k);
//   FILE * fp = fopen (s, "w");
//   if (fp != NULL) {
//     fprintf (fp, "# %d searches\n", indexSearch + 1);
//     double t = firstTimeNRPA;
//     for (int j = 0; j <= nbTimesNRPA; j++) {
//       float sum = 0.0;
//       int nbValues = 0;
//       for (int l = 0; l < indexSearch + 1; l = l + nbThreads)
// 	if (l + nbThreads - 1 < indexSearch + 1) {
// 	  double maxi = -DBL_MAX;
// 	  for (int m = 0; m < nbThreads; m++)
// 	    if (valueAfterTimeNRPA [l + m] [j] > maxi)
// 	      maxi = valueAfterTimeNRPA [l + m] [j];
// 	  sum += maxi;
// 	  nbValues++;
// 	}
//       fprintf (fp, "%f %f\n", t, sum / nbValues);
//       t *= 2;
//     }
//   }
//   fclose (fp); 
// }

// void testTimeNRPA (int level) {

//   char s [1000];
//   for (int i = 0; i < 100; i++) {
//     for (int j = 0; j < 10000; j++)
//       valueAfterTimeNRPA [j] [i] = 0.0;
//     sumValueAfterTimeNRPA [i] = 0.0;
//     nbSearchTimeNRPA [i] = 0;
//   }
//   stopOnTime = true;
//   for (indexSearch = 0; indexSearch < nbSearchesNRPA; indexSearch++) {
//     //    bestScoreNRPA = -DBL_MAX;
//     //    nextTimeNRPA = firstTimeNRPA;
//     //    indexTimeNRPA = 0;
//     //    startClockNRPA = clock ();

//     while (true) {
//       Policy pol;
//       Nrpa nrpa(level);
      
//       for (int i = 0; i <= level; i++)
// 	polLevel [i] = pol;

//       nrpa.nrpa (level, pol);
//       for (int i = 0; i <= level; i++){
// 	nrpa.printPolicy(i); 
//       }

//       if (indexTimeNRPA > nbTimesNRPA)
// 	break;
//     }

//     // double t = firstTimeNRPA;
//     // fprintf (stderr, "level %d, iteration %d\n", level, indexSearch + 1);
//     // for (int j = 0; j <= nbTimesNRPA; j++) {
//     //   if (nbSearchTimeNRPA [j] >= (7 * (indexSearch + 1)) / 10)
//     // 	fprintf (stderr, "%f %f\n", t, sumValueAfterTimeNRPA [j] / nbSearchTimeNRPA [j]);
//     //   t *= 2;
//     // }
//     // writeValues (1);
//     // writeValues (2);
//     // writeValues (4);
//     // writeValues (8);
//     // writeValues (16);
//   }

// }
