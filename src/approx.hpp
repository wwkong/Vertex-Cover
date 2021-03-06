#if !defined(APPROX_H)
#define      APPROX_H

#include "graph.hpp"
#include <stack>
#include <fstream>
#include <sys/time.h>
#include <sstream>
#include <iomanip>

double get_time();
int approx(Graph &graph, string &inst_name, double cutoff);

#endif
