/* CSE6140 Project - Simple Graph Classes */

#include <vector>
#include <stdlib.h>
using namespace std;

#ifndef GRAPH_H
#define GRAPH_H

/* =============================== */
/* --- Simple graph structures --- */
/* =============================== */

class Edge {
public:
  int start, end;
};

class Graph {
public:
  // Number of nodes = V and number of edges = E
  int sizeV, sizeE;
  vector <Edge> edge;
};

#endif
