// nrpa.inl
// Made by Benjamin Negrevergne 
// Started on <2017-03-28 Tue>

#ifndef CLI_HPP
#define CLI_HPP
#include <string>
#include <iostream>
#include <unistd.h> //GETOPT
#include <getopt.h>

extern char *optarg;
extern int optind, opterr, optopt;



struct Options{

  int numRun = 4;
  int numLevel = 4;
  int numIter = 10;
  int numThread = 0;
  int timeout = 0;
  int iterStats = 0; 
  int timerStats = 0;
  std::string statfilePrefix = "dat/nrpa_stats"; 
  std::string tag = ""; // name for this run, will be used to generate trace data file 
  int parallelLevel = 1; 
  int parStrat = 1; 
  bool threadStats = false; 
  int seed = -1; 
  
  static void usage(const std::string &binName, std::ostream &os = std::cerr); 
  static Options parse(int &argc, char **&argv, bool exitOnError = true); 
  void print(std::ostream &os = std::cout, const std::string &prefix = "") const; 
  void printAll(std::ostream &os = std::cout, const std::string &prefix = "") const; 

}; 


static const char *yesnostring(int i){
  static const char *yes="yes"; 
  return i!=0?"yes":"no"; 
}


inline void Options::usage(const std::string &binName, std::ostream &os) {
  Options d; 
  os<<"Usage: "<<binName<<" [options]\n"
    << "where option is any of these: \n"
    << "\t--num-run=NUM, -r NUM\n"
    << "\t\tNumber of test runs (default: "<<d.numRun<<").\n"

    << "\t--num-level=NUM, -l NUM\n"
    << "\t\tNrpa depth (default: "<<d.numLevel<<").\n"

    << "\t--num-thread=NUM, -x NUM\n"
    << "\t\tNumber of threads (0 = hardware capacity, default: "<<d.numThread<<").\n"

    << "\t--timeout=NUM, -t NUM\n"
    << "\t\tTimeout in sec. for a single run (0 = no timeout, default: "<<d.timeout<<").\n"

    << "\t--iter-stats, -s\n"
    << "\t\tEnable iteration based statistics (default: "<<yesnostring(d.iterStats)<<").\n"

    << "\t--timer-stats, -S\n"
    << "\t\tEnable timer based statistics (default: "<<yesnostring(d.iterStats)<<").\n"

    << "\t--statfilePrefix=STRING, -f STRING\n"
    << "\t\tPrefix for the stat files (default: "<<d.statfilePrefix<<").\n"

    << "\t--tag=STRING, -T STRING\n"
    << "\t\tSet a tag name for this run, it will be appened to statistics file name (default: None).\n"

    << "\t--parallel-level=NUM, -p NUM\n"
    << "\t\tGo parallel call at level N (default: "<<d.parallelLevel<<").\n"

    << "\t--parallel-strat=NUM, -P NUM\n"
    << "\t\tUse parallelization strategy number N (default: "<<d.parStrat<<").\n"

    << "\t--thread-stats, -q\n"
    << "\t\tEnable thread statistics (default: "<<d.threadStats<<").\n"

    << "\t--seed=NUM, -a\n"
    << "\t\tUse NUM to seed the random generator"
    << "(-1 = no seeding, 0 use clock()*getpid(), otheruse use NUM, default: "<<d.seed<<").\n"

    << "\t--help, -h\n"
    << "\t\tThis help."

    << "\t--parse-option-only, -o\n"
    << "\t\tParse options and exit."<<endl;

}

inline Options Options::parse(int &argc, char **&argv, bool exitOnError){
  using namespace std; 
  Options o; 
  int c;
  bool parseOnly = false; 
     
  while (1)
    {
      static struct option long_options[] =
	{
	  {"num-run",   required_argument,       0, 'r'},
	  {"num-level",   required_argument,       0, 'l'},
	  {"num-iter",   required_argument,       0, 'n'},
	  {"num-thread",    required_argument,       0, 'x'},
	  {"timeout",    required_argument,       0, 't'},
	  {"iter-stats",    no_argument,       0, 's'},
	  {"time-stats",    no_argument,       0, 'S'},
	  {"statfile-prefix",    required_argument,       0, 'f'},
	  {"tag",    required_argument,       0, 'T'},
	  {"parallel-level", required_argument, 0, 'p'}, 
	  {"parallel-strat", required_argument, 0, 'P'}, 
	  {"thread-stats", no_argument, 0, 'q'}, 
	  {"seed", required_argument, 0, 'a'}, 
	  {"help", no_argument, 0, 'h'}, 
	  {"parse-options-only", no_argument, 0, 'o'}, 
	  {0, 0, 0, 0}
	};

      int option_index = 0;
      c = getopt_long (argc, argv, "r:l:n:x:t:sSf:T:p:qP:a:ho",
		       long_options, &option_index);
     
      /* Detect the end of the options. */
      if (c == -1)
	break;

      switch (c)
	{
	case 0:
	  /* If this option set a flag, do nothing else now. */
	  if (long_options[option_index].flag != 0)
	    break;
	  cout<< "option "<<long_options[option_index].name;
	  if (optarg)
	    cout<<" with arg "<<optarg;
	  cout<<endl;
	  break;
     	case 'r':
	  o.numRun = atoi(optarg); 
	  break;
     	case 'l':
	  o.numLevel = atoi(optarg); 
	  break;
     	case 'n':
	  o.numIter = atoi(optarg); 
	  break;
	case 'x':
	  o.numThread = atoi(optarg); 
	  break;
	case 't':
	  o.timeout = atoi(optarg); 
	  break;
	case 's':
	  o.iterStats = 1;
	  break; 
	case 'S':
	  o.timerStats = 1;
	  break;
	case 'f':
	  o.statfilePrefix = optarg;
	  break; 
	case 'T':
	  o.tag = optarg;
	  break; 
	case 'p':
	  o.parallelLevel = atoi(optarg); 
	  break;
	case 'P':
	  o.parStrat = atoi(optarg); 
	  break;
	case 'q':
	  o.threadStats = true; 
	  break;
	case 'a':
	  o.seed = atoi(optarg); 
	  break;
	case 'h':
	  usage(argv[0]);
	  if(exitOnError) exit(1); 
	  break;
	case 'o':
	  parseOnly=true; 
	  break; 
	default:
	  cout<<"Unknown arg."<<endl;
	  if(exitOnError){
	    usage(argv[0]); 
	    exit(1);
	  }
	}
    }

  argc -= optind;
  argv += optind; 
  
  if(parseOnly){
    o.printAll(); 
    if(exitOnError) exit(1);
    else exit(0); 
  }
  else{
    o.print();
    return o;
  }
}

inline void Options::printAll(std::ostream &os, const std::string &prefix) const{
  os<<prefix<<"== Options ==\n"; 
  os<<prefix<<"numRun = "<<numRun<<"\n";
  os<<prefix<<"numLevel = "<<numLevel<<"\n";
  os<<prefix<<"numIter = "<<numIter<<"\n";
  os<<prefix<<"numThread = "<<numThread<<"\n";
  os<<prefix<<"timeout = "<<timeout<<"\n";
  os<<prefix<<"iterStats = "<<iterStats<<"\n"; 
  os<<prefix<<"timerStats = "<<iterStats<<"\n";
  os<<prefix<<"statfilePrefix = \""<<statfilePrefix<<"\"\n"; 
  os<<prefix<<"tag = \""<<tag<<"\"\n"; 
  os<<prefix<<"parallelLevel = "<<parallelLevel<<"\n"; 
  os<<prefix<<"parallelStrat = "<<parStrat<<"\n";
  os<<prefix<<"threadStats = "<<threadStats<<"\n"; 
  os<<prefix<<"seed = "<<seed<<"\n"; 
  os<<prefix<<"== End of options =="<<endl; 
}

inline void Options::print(std::ostream &os, const std::string &prefix) const{
  os<<prefix<<"== Options ==\n"; 
  os<<prefix<<"numRun = "<<numRun<<"\n";
  os<<prefix<<"numLevel = "<<numLevel<<"\n";
  os<<prefix<<"numIter = "<<numIter<<"\n";
  os<<prefix<<"numThread = "<<numThread<<"\n";
  os<<prefix<<"timeout = "<<timeout<<"\n";
  os<<prefix<<"parallelStrat = "<<parStrat<<"\n";
  os<<prefix<<"parallelLevel = "<<parallelLevel<<"\n";
  os<<prefix<<"== End of options =="<<endl; 
}


#endif //CLI_HPP
