#include "graph.hpp"

// Constructors
Edge::Edge() {
  start = 0;
  end = 0;
}
Edge::Edge(int s, int t) {
  start = s;
  end = t;
}
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
void Graph::addEdge(Edge e) {
  adjacencyList[e.start].push_back(e.end);
}
void Graph::addEdges(vector<Edge> es) {
  sizeE += es.size();
  for(int i=0; i<es.size(); i++) {
    addEdge(es[i].start, es[i].end);
  }
}

void Graph::rmVertex(int vertex) {
  vector<int> neighbours = adjacencyList[vertex];
  sizeE -= adjacencyList[vertex].size()-1;
  sizeV -= 1;
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
vector<Edge> Graph::getEdges() {
  vector<Edge> eSet;
  for(int i=0; i<adjacencyList.size(); i++) {
    for (int j=0; j<adjacencyList[i].size(); j++) {
      if (i<adjacencyList[i][j])
        eSet.push_back(Edge(i,adjacencyList[i][j]));
    }
  }
  return eSet;
}
vector<int> Graph::getVertices() {
  int v;
  vector<int> vSet;
  vector<bool> vCheck(sizeV+1,false);
  for(int i=0; i<adjacencyList.size(); i++) {
    for (int j=0; j<adjacencyList[i].size(); j++) {
      v = adjacencyList[i][j];
      if (!vCheck[v]) {
        vCheck[v] = true;
        vSet.push_back(v);
      }
    }
  }
  return vSet;
}

// Utility
void Edge::print() {
  cout << "start=" << start << ", end="<< end << endl;;
}
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
// Check if a set of vertices is a vertex cover
bool Graph::isVC(vector<int> vSet) {
  vector<Edge> eSet = getEdges();
  vector<bool> vBoolArr(sizeV+1, false);
  for (int i=0; i<vSet.size(); i++) {
    vBoolArr[vSet[i]] = true;
  }
  Edge e;
  for (int j=0; j<eSet.size(); j++) {
    e = eSet[j];
    if ((vBoolArr[e.start] == false) && (vBoolArr[e.end] == false))
      return false;
  }
  return true;
}
