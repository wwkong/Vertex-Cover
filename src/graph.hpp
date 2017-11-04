/* CSE6140 Project - Simple Graph Classes */

#include <iostream>
#include <vector>
#include <stdlib.h>
using namespace std;

#ifndef GRAPH_H
#define GRAPH_H

/* =============================== */
/* --- Simple graph structures --- */
/* =============================== */

// We will assume 1-Indexing
class Graph {
  vector< vector<int> > adjacencyList;
public:
  // Number of nodes = V and number of edges = E
  int sizeV, sizeE;
  // Constructors
  Graph();
  Graph(int);
  Graph(int,int);
  // Add
  void addEdge(int, int);
  void addEdges(int, vector<int>);
  // Get
  vector< vector<int> > getAdjancencyList();
  // Utility
  void printAdjancencyList();
};

// Constructors
Graph::Graph() {
  sizeV = 0;
  sizeE = 0;
  vector< vector<int> > adjacencyList;
}
Graph::Graph(int numVertices) {
  sizeV = numVertices;
  sizeE = 0;
  vector< vector<int> > tmp(numVertices+1);
  adjacencyList = tmp;
}
Graph::Graph(int numVertices, int numEdges) {
  sizeV = numVertices;
  sizeE = numEdges;
  vector< vector<int> > tmp(numVertices+1);
  adjacencyList = tmp;
}

// Add
void Graph::addEdge(int start, int end) {
  adjacencyList[start].push_back(end);
}
void Graph::addEdges(int start, vector<int> ends) {
  sizeE += ends.size();
  for(int i=0; i<ends.size(); i++) {
    addEdge(start, ends[i]);
  }
}

// Get
vector< vector<int> > Graph::getAdjancencyList() {
  return adjacencyList;
};

// Utility
void Graph::printAdjancencyList() {
  for (int i=1; i < adjacencyList.size(); i++) {
    cout << i << " -> ";
    for (int j=0; j < adjacencyList[i].size(); j++) {
      cout << adjacencyList[i][j] << " ";
    }
    cout << endl;
  }
}

#endif
