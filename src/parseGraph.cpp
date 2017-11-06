/* CSE6140 Project - Input parser */
#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <sstream>
#include "stdlib.h"
#include "graph.hpp"

using namespace std;

// parseGraph :: String -> Graph
// Takes an input .gr file and outputs a Graph pointer
Graph parseGraph(string graphFile) {

  // Initialize
  ifstream fHook;
  Graph initGraph;
  // Parse and open the .gr file
  fHook.open(graphFile.c_str());
  if (fHook.is_open()) {

    // Initialize the graph
    string input, elem;
    int E, V, dirFlag;
    getline(fHook, input);
    istringstream(input) >> V >> E >> dirFlag;
    initGraph = Graph(V,E);

    // Add edges
    int eNum = 0;
    int vNum = 1;
    while(getline(fHook,input)) {
      istringstream ss(input);
      while(getline(ss, elem, ' ')) {
        initGraph.addEdge(vNum, atoi(elem.c_str()));
      }
      eNum++;
      vNum++;
      try {
        if (eNum > E) {
          throw out_of_range("EXCEPTION: more edges in input file than specified!");
        }
      } catch (const out_of_range &oor){
        cerr << oor.what() << endl;
        exit(1);
      }
    }
    fHook.close();
  }
  return(initGraph);
}

// // --- Simple tests ---
// int main () {
//   Graph g = parseGraph("../input/karate.graph");
//   // Simple printing
//   g.printAdjacencyList();
//   vector < vector<int> > adjLst = g.getAdjacencyList();
//   cout << "Adjacency List Size=" << adjLst.size() << ", sizeV=" << g.sizeV << ", sizeE=" << g.sizeE << endl;
//   cout << "Neighbours of 1 => ";
//   g.printAdjacent(1);
//   cout << endl;
//   cout << "Removing Vertex 1..." << endl;
//   g.rmVertex(1);
//   g.printAdjacencyList();
//   vector<Edge> eSet = g.getEdges();
//   for (int k=0; k<eSet.size(); k++) {
//     eSet[k].print();
//   }
//   cout << "nEdges = " << g.sizeE << ", nESet=" << eSet.size() << endl;
//   return 1;
// }
