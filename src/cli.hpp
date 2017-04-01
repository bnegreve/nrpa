// nrpa.inl
// Made by Benjamin Negrevergne 
// Started on <2017-03-28 Tue>

#ifndef CLI_HPP
#define CLI_HPP
#include <string>

#include <unistd.h> //GETOPT
#include <getopt.h>

extern char *optarg;
extern int optind, opterr, optopt;



struct Options{

  int numRun = 4;
  int numLevel = 4;
  int numIter = 10;
  int numThread = 0;
  int timeout = -1;
  int stats = 0; 
  std::string tag = ""; // name for this run, will be used to generate trace data file 
  int parallelLevel = 1; 
  bool threadStats = false; 

  static Options parse(int &argc, char **&argv); 
  void print() const; 

}; 

inline Options Options::parse(int &argc, char **&argv){
  using namespace std; 
  Options o; 
  int c;
     
  while (1)
    {
      static struct option long_options[] =
	{
	  {"num-run",   required_argument,       0, 'r'},
	  {"num-level",   required_argument,       0, 'l'},
	  {"num-iter",   required_argument,       0, 'n'},
	  {"num-thread",    required_argument,       0, 'x'},
	  {"timeout",    required_argument,       0, 't'},
	  {"stats",    optional_argument,       0, 's'},
	  {"parallel-level", required_argument, 0, 'p'}, 
	  {"thread-stats", no_argument, 0, 'q'}, 
	  {0, 0, 0, 0}
	};

      int option_index = 0;
      c = getopt_long (argc, argv, "r:l:n:x:t:s::p:q",
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
	  o.stats = 1;
	  if(optarg)
	    o.tag = optarg; 
	  break;
	case 'p':
	  o.parallelLevel = atoi(optarg); 
	  break;
	case 'q':
	  o.threadStats = true; 
	  break;
	default:
	  cout<<"Unknown arg."<<endl;
	  exit(1); 
	}
    }

  argc -= optind;
  argv += optind; 
  
  o.print();
  return o; 
}

inline void Options::print() const{
  cout<<"== Options =="<<endl;
  cout<<"numRun = "<<numRun<<endl;
  cout<<"numLevel = "<<numLevel<<endl;
  cout<<"numIter = "<<numIter<<endl;
  cout<<"numThread = "<<numThread<<endl;
  cout<<"timeout = "<<timeout<<endl;
  cout<<"== End of options =="<<endl;
}


#endif //CLI_HPP
