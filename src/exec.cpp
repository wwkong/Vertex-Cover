/* CSE 6140 Project - Main executable */

#include <cstring>
#include <string>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <getopt.h>
#include <math.h>
#include <time.h>
#include <iomanip> // setprecision
#include <fstream>
#include <sstream>
#include "graph.hpp"
#include "graph.cpp"
#include "parseGraph.cpp"
#include "approx.cpp"
#include "approx.hpp"
#include "LS1.cpp"
#include "branchAndBound.cpp"
using namespace std;

int main (int argc, char **argv) {

  // -----------------------------
  // Parse Options
  // -----------------------------
  string fileName;
  string algName;
  double cutoff;
  int seed;
  int c;

  opterr = 0;
  while (1)
    {
      static struct option long_options[] =
        {
          {"inst",  required_argument, 0, 'i'},
          {"alg",   required_argument, 0, 'a'},
          {"time",  required_argument, 0, 't'},
          {"seed",  required_argument, 0, 's'},
          {0, 0, 0, 0}
        };

      int option_index = 0;
      c = getopt_long_only (argc, argv, "i:a:t:s:",
                            long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;
      switch (c)
        {
          // Main flags
        case 0:
          cout << "No options detected!" << endl;
          break;
        case 'i':
          fileName = optarg;
          break;
        case 'a':
          if (! (strcmp(optarg,"BnB")    == 0 ||
                 strcmp(optarg,"Approx") == 0 ||
                 strcmp(optarg,"LS1")    == 0 ||
                 strcmp(optarg,"LS2")    == 0)) {
            fprintf (stderr, "Algorithm %s is invalid! You must choose one of [BNB|Approx|LS1|LS2]. \n", optarg);
            return 1;
          }
          else
            algName = string(optarg);
          break;
        case 't':
          cutoff = atof(optarg);
          break;
        case 's':
          seed = atoi(optarg);
          break;
          // Unknown inputs
        case '?':
          break;
        default:
          abort ();
        }
    }

  // For debugging purposes
  printf ("filename = %s, algorithm = %s, cutoff = %f, seed = %d\n",
          fileName.c_str(), algName.c_str(), cutoff, seed);

  // -----------------------------
  // Parse the inputs
  // -----------------------------
  Graph g = parseGraph(fileName);
  unsigned first = fileName.find_last_of("/");
  unsigned last = fileName.find(".graph");
  string instName = fileName.substr (first+1,last-first-1);

  // -----------------------------
  // Algorithms
  // -----------------------------
  int finalQuality = 0;
  timespec startTime, endTime;
  clock_gettime(CLOCK_REALTIME, &startTime);

  if (strcmp(algName.c_str(),"BnB") == 0) {
    branchAndBound(g, instName, cutoff);

  } else if (strcmp(algName.c_str(),"Approx") == 0) {
    approx(g, instName, cutoff);

  } else if (strcmp(algName.c_str(),"LS1") == 0) {
    ESA(g, instName, cutoff, seed);

  } else if (strcmp(algName.c_str(),"LS2") == 0) {
    // Do something

  } else {
    fprintf (stderr, "Algorithm %s is invalid! You must choose one of [BNB|Approx|LS1|LS2]. \n", optarg);
    return 1;
  }
  clock_gettime(CLOCK_REALTIME, &endTime);
  double sStart = startTime.tv_sec*1000.0;
  double sEnd = endTime.tv_sec*1000.0;
  double totalTime = ((sEnd + endTime.tv_nsec/1000000.0) - (sStart + startTime.tv_nsec/1000000.0))/1000;

}
