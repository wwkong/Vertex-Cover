/* CSE6140 Project - Simple Graph Classes */

#include <algorithm> // remove(vector)
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
  // Set
  void addEdge(int, int);
  void addEdges(int, vector<int>);
  void rmVertex(int);
  // Get
  vector< vector<int> > getAdjacencyList();
  vector<int> getAdjacent(int);
  // Utility
  void printAdjacencyList();
  void printAdjacent(int);
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
  vector< vector<int> > adjacencyList(numVertices+1);
  for (int i=1; i<=numVertices; i++) {
    adjacencyList[i].push_back(i);
  }
}
Graph::Graph(int numVertices, int numEdges) {
  sizeV = numVertices;
  sizeE = numEdges;
  vector< vector<int> > tmp(numVertices+1);
  for (int i=1; i<=numVertices; i++) {
    tmp[i].push_back(i);
  }
  adjacencyList = tmp;
}

// Set
void Graph::addEdge(int start, int end) {
  adjacencyList[start].push_back(end);
}
void Graph::addEdges(int start, vector<int> ends) {
  sizeE += ends.size();
  for(int i=0; i<ends.size(); i++) {
    addEdge(start, ends[i]);
  }
}
void Graph::rmVertex(int vertex) {
  vector<int> neighbours = adjacencyList[vertex];
  adjacencyList[vertex].clear();
  for (int i = 0; i<neighbours.size(); i++) {
    for (int j = 0; j<adjacencyList[i].size(); j++) {
      if (adjacencyList[i][j] == vertex)
        adjacencyList[i].erase(adjacencyList[i].begin()+j);
    }
  }
}

// Get
vector< vector<int> > Graph::getAdjacencyList() {
  return adjacencyList;
};
vector<int> Graph::getAdjacent(int start) {
  return adjacencyList[start];
};

// Utility
void Graph::printAdjacencyList() {
  for (int i=1; i<adjacencyList.size(); i++) {
    cout << i << " -> ";
    for (int j=0; j<adjacencyList[i].size(); j++) {
      cout << adjacencyList[i][j] << " ";
    }
    cout << endl;
  }
}
void Graph::printAdjacent(int start) {
  vector<int> adjNodes = getAdjacent(start);
  cout << start << " -> ";
  for (int j=0; j < adjNodes.size(); j++) {
    cout << adjNodes[j] << " ";
  }
  cout << endl;
}
#endif
