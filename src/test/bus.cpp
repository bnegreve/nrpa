
#include <unistd.h>
#include <math.h>
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
#include <sys/time.h>
#include <sys/resource.h> 
#include <unistd.h>

#include "nrpa.hpp"

const int MaxStations = 100;
const int MaxBus = 50;
const int MinTempsTrajet = 2;
const int MaxTempsTrajet = 15;
const int MaxAttente = 5;
const int MaxPassagers = 10000;
const int MaxNouveauxPassagersEnAttente = 5;
const int MaxMinutes = 1000;

const int MaxPlayoutLength = MaxMinutes * MaxBus;
const int MaxMoveNumber = MaxMinutes * MaxBus * MaxStations * MaxAttente;
const int MaxLegalMoves = MaxAttente;

int debugLevel = 1;

int tempsTrajetStation [MaxMinutes] [MaxStations + 1];
int tempsIncidentStation [MaxMinutes] [MaxStations + 1];
int nouveauxPassagersStation [MaxMinutes] [MaxStations + 1];

int deltaAttente = 5, deltaStations = 5;
int minutesAuDebut = 200, fermeture = 300;

bool recallBestSequence = true;
bool optimal = false, meta = false, alea = false, UCB = false, UCBA = false;

//int constante = 30000;

class Move {
 public:
  int minute, bus, station, attente;
};

class Board {
 public:
  // la periode est le temps entre chaque depart du terminus
  int nbStations, nbBus, periode;
  int minutes;

  int station [MaxBus];
  int tempsAttenteStation [MaxBus];
  int tempsAvantProchaineStation [MaxBus];
  bool actif [MaxBus];
  
  int nbBusStation [MaxStations];
  int nbPassagersEnAttente [MaxStations];
  int tempsTrajetActuel [MaxStations];

  bool randomized;
  bool printProgress;
  unsigned long long sommeAttentes;

  Board ();

  Board (int s, int b, int p) {
    nbStations = s;
    nbBus = b;
    periode = p;
    minutes = 0;
    sommeAttentes = 0;
    randomized = false;
    printProgress = true;
    for (int i = 0 ; i < nbStations; i++) {
      nbBusStation [i] = 0;
      nbPassagersEnAttente [i] = 0;
      tempsTrajetActuel [i] = 0;
    }
    for (int i = 0 ; i < nbBus; i++) {
      actif [i] = false;
      tempsAttenteStation [i] = 0;
      tempsAvantProchaineStation [i] = 0;
      if (i < nbBus / 2) {
	station [i] = 0;
	nbBusStation [0]++;
      }
      else {
	station [i] = nbStations / 2;
	nbBusStation [nbStations / 2]++;
      }
    }
    length = 0;
    currentBus = 0;
  }

  int tempsIncident (int station) {
    if (randomized) {
      float proba = rand () / (RAND_MAX + 1.0);
      if (proba > 0.99)
	return 5;
      return 0;
    }
    return tempsIncidentStation [minutes] [station];
  }

  int tempsTrajet (int station, int tempsActuel) {
    if (randomized) {
      float proba = rand () / (RAND_MAX + 1.0);
      int nouveauTemps = tempsActuel;
      if (proba > 0.6)
	nouveauTemps++;
      else if (proba < 0.4)
	nouveauTemps--;
      if (nouveauTemps > MaxTempsTrajet)
	nouveauTemps = MaxTempsTrajet;
      if (nouveauTemps < MinTempsTrajet)
	nouveauTemps = MinTempsTrajet;
      return nouveauTemps;
    }
    return tempsTrajetStation [minutes] [station];
  }

  int nouveauxPassagers (int station) {
    if (randomized)
      return (int) (MaxNouveauxPassagersEnAttente * (rand () / (RAND_MAX + 1.0)));
    return nouveauxPassagersStation [minutes] [station];
  }

  void precalcul (int pas) {
    randomized = true;
    for (int i = 0; i < pas; i++)
      for (int s = 0; s < nbStations + 1; s++) {
	nouveauxPassagersStation [i] [s] = nouveauxPassagers (s);
	if (i > 0)
	  tempsTrajetStation [i] [s] = tempsTrajet (s, tempsTrajetStation [i - 1] [s]);
	else
	  tempsTrajetStation [i] [s] = tempsTrajet (s, MinTempsTrajet);
	tempsIncidentStation [i] [s] = tempsIncident (s);
      }
    randomized = false;
  }

  void miseAJour () {
    for (int i = 1; i < nbStations + 1; i++) {
      tempsTrajetActuel [i] = tempsTrajet (i, tempsTrajetActuel [i]);
      nbPassagersEnAttente [i] += nouveauxPassagers (i);
    }
  }
  
  void comptePassagersEnAttente () {
    for (int i = 1 ; i < nbStations; i++) 
      if (i != nbStations / 2) {
	sommeAttentes += nbPassagersEnAttente [i];
      }
  }

  bool departBus (int station) {
    if (nbBusStation [station] > 0)
      if (minutes % periode == 0)
	return true;
    return false;
  }
  
  void departNouveauxBus () {
    if (departBus (0)) {
      for (int i = 0; i < nbBus; i++)
	if (station [i] == 0) {
	  actif [i] = true;
	  tempsAvantProchaineStation [i] = tempsTrajetActuel [1];
	  tempsAvantProchaineStation [i] += tempsIncident (1);
	  break;
	}
    }
    if (departBus (nbStations / 2)) {
      for (int i = 0; i < nbBus; i++)
	if (station [i] == nbStations / 2) {
	  actif [i] = true;
	  tempsAvantProchaineStation [i] = tempsTrajetActuel [nbStations / 2 + 1];
	  tempsAvantProchaineStation [i] += tempsIncident (nbStations / 2 + 1);
	  break;
	}
    }
  }

  int stationBusSuivant (int bus) {
    for (int i = 0; i < nbBus; i++)
      if (i < bus)
	if (station [i] == station [bus])
	  return station [i];
    int best = 0;
    for (int i = 0; i < nbBus; i++)
      if (station [i] < station [bus])
	if (station [i] > best)
	  best = station [i];
    return best;
  }

  int minutesBusSuivant (int bus) {
    for (int i = 0; i < nbBus; i++)
      if (station [i] == station [bus])
	return tempsAvantProchaineStation [i];
    int best = 0;
    for (int i = 0; i < nbBus; i++)
      if (station [i] < station [bus])
	if (station [i] > best)
	  best = station [i];
    int temps = tempsAvantProchaineStation [best];
    for (int i = station [best] + 1; i < station [bus] ; i++) 
      if (minutes + temps < MaxMinutes) {
	int t = tempsTrajetStation [minutes + temps] [i];
	t += tempsIncidentStation [minutes + temps] [i];
	temps += t;
      }
    return temps;
  }

  int deltaBusSuivant (int bus) {
    for (int i = 0; i < nbBus; i++)
      if (i != bus)
	if (station [i] == station [bus])
	  return 0;
    int best = 0;
    for (int i = 0; i < nbBus; i++)
      if (station [i] < station [bus])
	if (station [i] > best)
	  best = station [i];
    return station [bus] - best;
  }

  int stationBusPrecedent (int bus) {
    for (int i = 0; i < nbBus; i++)
      if (i > bus)
	if (station [i] == station [bus])
	  return station [i];
    int best = nbStations + 1;
    for (int i = 0; i < nbBus; i++)
      if (station [i] > station [bus])
	if (station [i] < best)
	  best = station [i];
    return best;
  }

  void avanceBus (int i) {
    if (tempsAttenteStation [i] > 0) {
      tempsAttenteStation [i]--;
      nbPassagersEnAttente [station [i]] = 0;
      if (tempsAttenteStation [i] <= 0) {
	tempsAvantProchaineStation [i] = tempsTrajetActuel [station [i] + 1];
	tempsAvantProchaineStation [i] += tempsIncident (station [i] + 1);
      }
    }
    else {
      tempsAvantProchaineStation [i]--;
      if (tempsAvantProchaineStation [i] <= 0) {
	nbBusStation [station [i]]--;
	station [i]++;
	nbBusStation [station [i]]++;
	if (station [i] == nbStations / 2) {
	  actif [i] = false;
	}
	else if (station [i] == nbStations) {
	  station [i] = 0;
	  nbBusStation [nbStations]--;
	  nbBusStation [0]++;
	  actif [i] = false;
	}
	else {
	  nbPassagersEnAttente [station [i]] = 0;
	}
      }
    }
  }
  
  int currentBus;
  int length;
  Move rollout [MaxPlayoutLength];

  int legalMoves (Move moves [MaxLegalMoves]) {
    int nb = 0;
    for (int i = 1; i < MaxAttente; i++) {
      moves [nb].minute = minutes;
      moves [nb].bus = currentBus;
      moves [nb].station = station [currentBus];
      moves [nb].attente = i;
      nb++;
    }
    int t = deltaBusSuivant (currentBus);
    //if (t < 7)
    //return 1;
    //if (t < 6)
    //return 3;
    return nb;
  }

  void init () {
    while (minutes < fermeture) {
      departNouveauxBus ();
      for (int i = 0; i < nbBus; i++)
	if (actif [i]) {
	  avanceBus (i);
	  if (actif [i])
	    if (tempsAvantProchaineStation [i] == 0) {
	      currentBus = i;
	      return;
	    }
	}
      comptePassagersEnAttente ();
      miseAJour ();
      minutes++;
    }
  }

  void play (Move & m) {
    rollout [length] = m;
    length++;
    tempsAttenteStation [m.bus] = m.attente;
    for (int i = m.bus + 1; i < nbBus; i++)
      if (actif [i]) {
	avanceBus (i);
	if (actif [i]) 
	  if (tempsAvantProchaineStation [i] == 0) {
	    currentBus = i;
	    return;
	  }
      }
    comptePassagersEnAttente ();
    miseAJour ();
    minutes++;
    while (minutes < fermeture) {
      departNouveauxBus ();
      for (int i = 0; i < nbBus; i++)
	if (actif [i]) {
	  avanceBus (i);
	  if (actif [i])
	    if (tempsAvantProchaineStation [i] == 0) {
	      currentBus = i;
	      return;
	    }
	}
      comptePassagersEnAttente ();
      miseAJour ();
      minutes++;
    }
  }

  int code (Move & m) {
    return m.minute + MaxMinutes * m.bus + MaxMinutes * MaxBus * m.attente;
    /**/
    int t = deltaBusSuivant (m.bus);
    if (t > MaxStations - 1)
      t = MaxStations - 1;
    return m.minute + MaxMinutes * m.bus + MaxMinutes * MaxBus * t + MaxMinutes * MaxBus * MaxStations * m.attente;
    /**/
    //return m.minute + MaxMinutes * m.station + MaxMinutes * MaxStations * m.attente;
    //return m.minute + MaxMinutes * 0 + MaxMinutes * MaxBus * m.station + MaxMinutes * MaxBus * MaxStations * m.attente;
    return m.minute + MaxMinutes * m.bus + MaxMinutes * MaxBus * m.station + MaxMinutes * MaxBus * MaxStations * m.attente;
  }

  void avanceUneMinute () {
    departNouveauxBus ();
    for (int i = 0; i < nbBus; i++)
      if (actif [i])
	avanceBus (i);
    comptePassagersEnAttente ();
    miseAJour ();
    minutes++;
  }

  void avance (int pas) {
    for (int i = 0; i < pas; i++) {
      if (printProgress && (debugLevel > 0)) {
	print (stderr);
      }
      avanceUneMinute ();
    }
  }

  bool terminal () {
    if (minutes == fermeture)
      return true;
    return false;
  }

  double score () {
    return -((double)sommeAttentes - 200000) / 1000;
  }

  void print (FILE * fp) {
    if (false) {
      for (int i = 0 ; i < nbStations; i++) 
	fprintf (stderr, "%d ", nbBusStation [i]);
      fprintf (stderr, "\n");
      for (int i = 0 ; i < nbBus; i++) 
	fprintf (stderr, "%d ", station [i]);
      fprintf (stderr, ", sommeAttentes (%d) = %llu ", minutes, sommeAttentes);
      fprintf (stderr, "\n");
    }
  }
};

Board board (70, 20, 20);

Board::Board () {
  *this = board;
}
 
double scoreSansAttente () {
  Board ligneLocale = board;
  Move moves [MaxLegalMoves];
  int nb;

  while (!ligneLocale.terminal ()) {
    nb = ligneLocale.legalMoves (moves);
    ligneLocale.play (moves [0]);
  }
  fprintf (stderr, "ligneLocale.sommeAttentes = %llu\n", ligneLocale.sommeAttentes);

  return ligneLocale.score ();
}

//#include "nestedSH.c"
//#include "nested.c"
//#include "nrpa.c"
//#include "beamnrpa.c"

int main(int argc, char *argv[])  {
  board.precalcul (MaxMinutes);
  board.printProgress = false;  
  board.avance (minutesAuDebut);
  board.init ();
  board.sommeAttentes = 0;

  fermeture = 500;

  fprintf (stderr, "scoreSansAttente = %lf\n", scoreSansAttente ());

  SizeBeam [1] = 1;
  SizeBeam [2] = 1;
  SizeBeam [3] = 1;
  SizeBeam [4] = 1;
  startLearning = 1;
  //testTimeNested (1);
  Nrpa<Board, Move, 5, MaxPlayoutLength, MaxLegalMoves>::test(Options::parse(argc, argv));


  // testTimeNRPA (5);
  // exit (0);
  // while (true) {
  //   Board b = board;
  //   //nested (b, 3);
  //   //Policy pol;
  //   //nrpa (2, pol);
  //   fprintf (stderr, "best score %lf\n", bestBoard.score ());
  //   break;
  // }
  // return 0;
}
