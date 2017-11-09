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
#include "gurobi_c++.h"
#include "graph.hpp"
#include "BnBInfo.hpp"
#include "parseGraph.cpp"
using namespace std;

// Utility
bool cmpSize(vector<int> a, vector<int> b) {
  return (a.size() < b.size()); // Larger sizes first
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
  if (B->time >= B->cutoff*60) {
    B->time = B->cutoff*60;
    return;
  }

  // Gather info
  double inclObjVal = B->solution.size()+1;
  double exclObjVal = B->solution.size()+1;
  if (B->debug) {
    cout << "Gathering LB data..." << endl;
  }
  inclObjVal = B->getInclLB(M);
  exclObjVal = B->getExclLB(M);

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
  if (B->time >= B->cutoff*60) {
    B->time = B->cutoff*60;
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
  if (B->time >= B->cutoff*60) {
    B->time = B->cutoff*60;
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
  if (B->time >= B->cutoff*60) {
    B->time = B->cutoff*60;
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
  if (B->time >= B->cutoff*60) {
    B->time = B->cutoff*60;
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
  GRBModel lpInit = vcLpSolve(G, false);
  vector<int> verts = G.getVertices();
  sort(verts.begin(), verts.end());

  // Initialize trivial components
  BnBInfo BSol;
  BSol.debug = false;
  BSol.time = 0;
  BSol.cutoff = cutoff;
  BSol.sizeV = G.sizeV;
  BSol.sizeE = G.sizeE;
  BSol.edgeSet = G.getEdges();
  BSol.solution = verts;
  BSol.instName = instName;

  // Generate filestream info
  stringstream fss;
  fss << instName << "_BnB_" << cutoff;
  string solFName = fss.str()+".sol";
  string traceFName = fss.str()+".trace";
  fss.str(string());

  // Initialize trace file
  trace.open(traceFName.c_str());
  trace << fixed << setprecision(2) << "0.00, " << G.sizeV << endl;
  trace.close();
  // Initial solution file
  sol.open(solFName.c_str());
  sol << verts.size() << endl;
  if (verts.size() > 0) {
    for (int i=0; i<verts.size()-1; i++) {
      sol << verts[i] << ",";
    }
    sol << verts[verts.size()-1];
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
  branchAndBoundIter(&BSol, &lpInit);
  sort(BSol.solution.begin(), BSol.solution.end());
  cout << "BRANCH AND BOUND SUMMARY" << endl;
  cout << "|V|=" << G.sizeV << ", |E|=" << G.sizeE << endl;
  BSol.printSolution();
  cout << "numVertices = " << BSol.solution.size() << endl;
  cout << "isVC = " << G.isVC(BSol.solution) << endl;


}

// // Simple tests
// int main() {
//   Graph g = parseGraph("../input/football.graph");
//   branchAndBound(g, "football", 1);
//   return 1;
// }

#endif
