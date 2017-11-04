/* Main executable */

#include <cstring>
#include <string>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "graph.hpp"
using namespace std;

int main (int argc, char **argv) {

  // -----------------------------
  // Parse Inputs
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
          break;
        case 'i':
          // printf("option -inst with input %s \n", optarg);
          fileName = optarg;
          break;
        case 'a':
          // printf("option -alg with input %s \n", optarg);
          if (! (strcmp(optarg,"BnB") == 0 || strcmp(optarg,"Approx") == 0 || strcmp(optarg,"LS1") == 0 || strcmp(optarg,"LS2") == 0)) {
            fprintf (stderr, "Algorithm %s is invalid! You must choose one of [BNB|Approx|LS1|LS2]. \n", optarg);
            return 1;
          }
          else
            algName = string(optarg);
          break;
        case 't':
          // printf("option -time with input %s \n", optarg);
          cutoff = atof(optarg);
          break;
        case 's':
          // printf("option -seed with input %s \n", optarg);
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
  // Parsing the inputs
  // -----------------------------
  // Graph g = parseInput(fileName);

  // -----------------------------
  // Algorithms
  // -----------------------------
  if (strcmp(algName.c_str(),"BnB") == 0) {
    // Do something
  } else if (strcmp(algName.c_str(),"Approx") == 0) {
    // Do something
  } else if (strcmp(algName.c_str(),"LS1") == 0) {
    // Do something
  } else if (strcmp(algName.c_str(),"LS2") == 0) {
    // Do something
  } else {
    fprintf (stderr, "Algorithm %s is invalid! You must choose one of [BNB|Approx|LS1|LS2]. \n", optarg);
    return 1;
  }

}
