/* CSE6140 Project - Global header file */

#ifndef GLOBAL_H
#define GLOBAL_H

#include "parseGraph.cpp"
#include "vcLpSolve.cpp"
#include "gurobi_c++.h"

Graph parseGraph(string graphFile);
GRBModel vcLpSolve(Graph G, bool printFlag);

#endif
