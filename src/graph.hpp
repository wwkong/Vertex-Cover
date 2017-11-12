<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 57337a755c473bc725452695aeddf0dc5c3f029c
/* CSE 6140 Project - Simple Graph Classes */

#ifndef GRAPH_H
#define GRAPH_H

#include <algorithm> // remove(vector)
#include <iostream>
#include <vector>
#include <stdlib.h>
using namespace std;

/* =============================== */
/* --- Simple graph structures --- */
/* =============================== */

// We will assume 1-Indexing
class Edge {
public:
  int start, end;
  // Constructors
  Edge();
  Edge(int, int);
  // Utility
  void print();
};
class Graph {
public:
  vector< vector<int> > adjacencyList;
  // Number of nodes = V and number of edges = E
  int sizeV, sizeE;
  // Constructors
  Graph();
  Graph(int);
  Graph(int,int);
  // Set
  void addEdge(int, int);
  void addEdge(Edge);
  void addEdges(int, vector<int>);
  void addEdges(vector<Edge>);
  void rmVertex(int);
  // Get
  vector< vector<int> > getAdjacencyList();
  vector<int> getAdjacent(int);
  vector<Edge> getEdges();
  vector<int> getVertices();
  // Utility
  void printAdjacencyList();
  void printAdjacent(int);
  bool isVC(vector<int>);
};

#endif
