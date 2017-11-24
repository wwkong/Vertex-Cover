/* CSE 6140 Project - Branch and Bound Method */

#ifndef BNBMETHOD_H
#define BNBMETHOD_H

/* Chosen method is Gurobi */
#include <stdio.h>
#include <iomanip>  // setprecision
#include <sstream>
#include <ctime>
#include <fstream>
#include <math.h>
#include <algorithm>
#include <stack>
#include "gurobi_c++.h"
#include "graph.hpp"
#include "BnBInfo.hpp"
#include "parseGraph.cpp"
using namespace std;

// Utility
bool cmpSize(vector<int> a, vector<int> b) {
  return (a.size() < b.size()); // Larger sizes first
}

vector<int> approx2(Graph &graph){ // DFS approx
  vector <int> solution;
  solution.reserve(graph.sizeV);
  vector <bool> visited(graph.sizeV+1);
  int visited_vertex_num = 0;
  int start_vertex = 1;
  stack<int> S;
  /* Find DFS forest and record leaf nodes for each DFS tree */
  while(visited_vertex_num < graph.sizeV){
    for(int i=2; i< graph.sizeV;++i){
      if(!visited[i]){
        int adj_vertex_num = graph.adjacencyList[i].size(); 
        if(adj_vertex_num!=0){
          start_vertex = i;
          break;
        }
      }
    }
    S.push(start_vertex);
    while(!S.empty()){
      int curV = S.top();
      S.pop();
      if(!visited[curV]){
        visited[curV]=true;
        ++visited_vertex_num;
        vector <int> incidentVs = graph.adjacencyList[curV];
        int childsNum = 0;
        for(int it=0; it<incidentVs.size(); it++){
          if(!visited[incidentVs[it]]){
            S.push(incidentVs[it]);
            ++childsNum;
          }
        }
        if(childsNum>0){
          solution.push_back(curV);
        }
      }
    }
  }
  /* add the start vertex to the solution if it is not in the solution*/
  if(solution.front()!=start_vertex){
    solution.push_back(start_vertex);
  }
  return solution;
}

// Subroutine / Iterator which performs the branching
// branchAndBoundIter :: BnBInfo -> GRBModel* -> Double -> Bool -> BnBInfo
void branchAndBoundIter(BnBInfo *B, GRBModel *M) {

  // Check our runtime
  if (B->time >= 60*B->cutoff) {
    return;
  }

  // Helper variables
  timespec startTime, endTime;
  double diffSec;
  double sStart, sEnd, totalTime;
  clock_gettime(CLOCK_REALTIME, &startTime);

  // Check feasibility
  if (B->candidates.size() == 0) 
    return;
  // Check optimality
  if (B->isOptimal())
    return;

  // Check our runtime
  clock_gettime(CLOCK_REALTIME, &endTime);
  sStart = startTime.tv_sec*1000.0;
  sEnd = endTime.tv_sec*1000.0;
  diffSec = ((sEnd + endTime.tv_nsec/1000000.0) - (sStart + startTime.tv_nsec/1000000.0))/1000;
  clock_gettime(CLOCK_REALTIME, &startTime);
  B->time += diffSec;
  if (B->time >= B->cutoff) {
    B->time = B->cutoff;
    return;
  }

  // Gather info
  double inclObjVal = B->solution.size()+1;
  double exclObjVal = B->solution.size()+1;
  if (B->debug) {
    cout << "Gathering bound data..." << endl;
  }
  inclObjVal = B->getInclB(M);
  exclObjVal = B->getExclB(M);

  // ====================================
  // First Branch
  // ====================================

  // Check where we should branch
  bool goBranch;
  string branchType;
  if (B->debug) {
    cout << "Deciding branch direction..." <<endl;
  }
  if (inclObjVal < exclObjVal) {
    goBranch = B->inclVertex(M);
    branchType = "LEFT";
  } else {
    goBranch = B->exclVertex(M);
    branchType = "RIGHT";
  }

  // Check our runtime
  clock_gettime(CLOCK_REALTIME, &endTime);
  sStart = startTime.tv_sec*1000.0;
  sEnd = endTime.tv_sec*1000.0;
  diffSec = ((sEnd + endTime.tv_nsec/1000000.0) - (sStart + startTime.tv_nsec/1000000.0))/1000;
  clock_gettime(CLOCK_REALTIME, &startTime);
  B->time += diffSec;
  if (B->time >= B->cutoff) {
    B->time = B->cutoff;
    return;
  }

  // Update
  if (goBranch) {
    if(B->debug) {
      B->printCandidates();
      B->printVertexSet();
      B->printSolution();
      cout << "Current Solution Size = " << B->solution.size() << ", Branching " << branchType << endl << endl;
    }
    branchAndBoundIter(B, M);
    if (B->debug) {
      cout << "Result of Branching " << branchType << endl;
      B->printSolution();
      cout << endl;
    }
    B->restoreState(M);
  } else {
    return;
  }

  // Check our runtime
  if (B->time >= B->cutoff) {
    B->time = B->cutoff;
    return;
  }

  // Check optimality
  if (B->isOptimal())
    return;

  // =====================================
  // Second branch
  // =====================================

  // Reset the timer
  clock_gettime(CLOCK_REALTIME, &startTime);

  // Check where we should branch next
  if (inclObjVal >= exclObjVal) {
    goBranch = B->inclVertex(M);
    branchType = "LEFT";
  } else {
    goBranch = B->exclVertex(M);
    branchType = "RIGHT";
  }

  // Check our runtime
  clock_gettime(CLOCK_REALTIME, &endTime);
  sStart = startTime.tv_sec*1000.0;
  sEnd = endTime.tv_sec*1000.0;
  diffSec = ((sEnd + endTime.tv_nsec/1000000.0) - (sStart + startTime.tv_nsec/1000000.0))/1000;
  clock_gettime(CLOCK_REALTIME, &startTime);
  B->time += diffSec;
  if (B->time >= B->cutoff) {
    B->time = B->cutoff;
    return;
  }

  // Update
  if (goBranch) {
    if(B->debug) {
      B->printCandidates();
      B->printVertexSet();
      B->printSolution();
      cout << "Current Solution Size = " << B->solution.size() << ", Branching " << branchType << endl << endl;
    }
    branchAndBoundIter(B, M);
    if (B->debug) {
      cout << "Result of Branching " << branchType << endl;
      B->printSolution();
      cout << endl;
    }

    B->restoreState(M);
  } else {
    return;
  }

  // Check our runtime
  if (B->time >= B->cutoff) {
    B->time = B->cutoff;
    return;
  }

  // Check optimality
  if (B->isOptimal())
    return;

  // Go up a level
  return;

}

// Main Branch and Bound function
// branchAndBound :: Graph -> String -> Double -> Void
// **Note: cutoff is in minutes
void branchAndBound(Graph G, string instName, double cutoff) {

  // Set up main variables
  ofstream sol, trace;
  timespec startTime, endTime;
  double sStart, sEnd, totalTime;
  GRBModel lpInit = vcLpSolve(G, false);
  vector<int> verts = G.getVertices();
  sort(verts.begin(), verts.end());

  // Using a DFS approach, get the initial VC
  clock_gettime(CLOCK_REALTIME, &startTime);
  vector<int> vcInit;
  vcInit = approx2(G);
  clock_gettime(CLOCK_REALTIME, &endTime);
  sStart = startTime.tv_sec*1000.0;
  sEnd = endTime.tv_sec*1000.0;
  totalTime = ((sEnd + endTime.tv_nsec/1000000.0) - (sStart + startTime.tv_nsec/1000000.0))/1000;

  // Initialize trivial components
  BnBInfo BSol;
  BSol.debug = false;
  BSol.time = totalTime;
  BSol.cutoff = cutoff;
  BSol.sizeV = G.sizeV;
  BSol.sizeE = G.sizeE;
  BSol.G = G;
  BSol.edgeSet = G.getEdges();
  BSol.solution = vcInit;
  BSol.instName = instName;

  // Generate filestream info
  stringstream fss;
  fss << instName << "_BnB_" << cutoff;
  string solFName = fss.str()+".sol";
  string traceFName = fss.str()+".trace";
  fss.str(string());

  // Initialize trace file
  trace.open(traceFName.c_str());
  trace << fixed << setprecision(2) << totalTime << ", " << vcInit.size() << endl;
  trace.close();
  // Initial solution file
  sol.open(solFName.c_str());
  sol << vcInit.size() << endl;
  if (vcInit.size() > 0) {
    for (int i=0; i<vcInit.size()-1; i++) {
      sol << vcInit[i] << ",";
    }
    sol << vcInit[vcInit.size()-1];
  }
  sol.close();

  // Grab candidate vertices in ascending order of connections
  vector< vector<int> > aLst = G.getAdjacencyList();
  sort(aLst.begin()+1, aLst.end(), cmpSize);
  for (int i=1; i<aLst.size(); i++) {
    BSol.candidates.push_back(aLst[i][0]);
  }

  // Call the main iterator
  cout << "-------- SOLUTION FROM BRANCH AND BOUND SOLVER --------" << endl << endl;
  cout << "INSTANCE NAME" << endl;
  cout << instName << endl << endl;
  branchAndBoundIter(&BSol, &lpInit);
  sort(BSol.solution.begin(), BSol.solution.end());
  cout << "BRANCH AND BOUND SUMMARY" << endl;
  cout << "|V|=" << G.sizeV << ", |E|=" << G.sizeE << endl;
  BSol.printSolution();
  cout << "numVertices = " << BSol.solution.size() << endl;
  cout << "isVC = " << G.isVC(BSol.solution) << endl;
  cout << "Total time = " << BSol.time << " seconds"<< endl;

}

// Simple tests
int main() {
  Graph g = parseGraph("../input/football.graph");
  branchAndBound(g, "football", 120);
  return 1;
}

#endif
