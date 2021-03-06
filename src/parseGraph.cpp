/* CSE 6140 Project - Input parser */

#ifndef PARSEGRAPH_H
#define PARSEGRAPH_H

#include "parseGraph.hpp"

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
      if (eNum > E) {
        cout << "WARNING: more edges in input file than specified!" << endl;
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
//   vector<int> vSet = g.getVertices();
//   for (int k=0; k<vSet.size(); k++) {
//     cout << vSet[k] << endl;
//   }
//   return 1;
// }

#endif
