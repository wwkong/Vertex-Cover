#if !defined(LS2_H)
#define      LS2_H

#include "graph.hpp"
#include <fstream>
#include <sys/time.h>
#include <sstream>
#include <iomanip>
#include <assert.h>
#include <string>
#include "approx.hpp"
#include <stdint.h>

int local_search2(Graph &graph, string &instName, double cutoff, int seed);

#endif
