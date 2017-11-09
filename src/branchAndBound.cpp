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
void branchAndBoundIter(BnBInfo *B, GRBModel *M, double cutoff) {

  // Check our runtime
  if (B->time >= 60*cutoff) {
    return;
  }

  // Helper variables
  int nVars, nConstrs;
  vector<int> VBases;
  vector<int> CBases;
  stringstream ss, fss;
  ofstream trace;
  string traceFName;
  BnBInfo BVIn, BVOut, BLeft, BRight, BFinal;
  timespec startTime, endTime;
  double diffSec, leftLB, rightLB;
  double sStart, sEnd, totalTime;
  clock_gettime(CLOCK_REALTIME, &startTime);

  // =============================
  // Check optimality (BOTTLENECK)
  // =============================
  if (B->edgeSet.empty() && B->vertexSet.size() < B->solution.size()) {

    // Update and add to the trace log
    B->solution = B->vertexSet;
    if (B->debug) {
      B->printSolution();
      cout << "Solution Size = " << B->solution.size() << endl;
      cout << "Fathom due to OPTIMALITY!" << endl << endl;
    }

    // Update time
    clock_gettime(CLOCK_REALTIME, &endTime);
    sStart = startTime.tv_sec*1000.0;
    sEnd = endTime.tv_sec*1000.0;
    diffSec = ((sEnd + endTime.tv_nsec/1000000.0) - (sStart + startTime.tv_nsec/1000000.0))/1000;
    clock_gettime(CLOCK_REALTIME, &startTime);
    B->time += diffSec;
    if (B->time >= cutoff*60) {
      B->time = cutoff*60;
    }

    // Output
    fss << B->instName << "_BnB_" << cutoff;
    traceFName = fss.str()+".trace";
    fss.str(string());
    trace.open(traceFName.c_str(), ios_base::app);
    trace << fixed << setprecision(2) << B->solution.size() << " " << diffSec + B->time << endl;
    trace.close();
    return;

  }

  // ====================================
  // Include the candidate vertex? (LEFT)
  // ====================================

  // Check if we should branch left
  bool goLeft = B->inclVertex(M);

  // Check our runtime
  clock_gettime(CLOCK_REALTIME, &endTime);
  sStart = startTime.tv_sec*1000.0;
  sEnd = endTime.tv_sec*1000.0;
  diffSec = ((sEnd + endTime.tv_nsec/1000000.0) - (sStart + startTime.tv_nsec/1000000.0))/1000;
  clock_gettime(CLOCK_REALTIME, &startTime);
  B->time = diffSec;
  if (B->time >= cutoff*60) {
    B->time = cutoff*60;
    return;
  }

  // Update
  if (goLeft) {
    if(B->debug) {
      B->printCandidates();
      B->printVertexSet();
      B->printSolution();
      cout << "Current Solution Size = " << B->solution.size() << ", Branching LEFT..." << endl << endl;
    }
    branchAndBoundIter(B, M, cutoff);
    if (B->debug) {
      cout << "Result of LEFT Branch..." << endl;
      B->printSolution();
      cout << endl;
    }
    B->restoreState(M);
  } else {
    return;
  }

  // Check our runtime
  if (B->time >= cutoff*60) {
    B->time = cutoff*60;
    return;
  }

  // =====================================
  // Exclude the candidate vertex? (RIGHT)
  // =====================================

  // Check if we should branch right
  bool goRight = B->exclVertex(M);

  // Check our runtime
  clock_gettime(CLOCK_REALTIME, &endTime);
  sStart = startTime.tv_sec*1000.0;
  sEnd = endTime.tv_sec*1000.0;
  diffSec = ((sEnd + endTime.tv_nsec/1000000.0) - (sStart + startTime.tv_nsec/1000000.0))/1000;
  clock_gettime(CLOCK_REALTIME, &startTime);
  B->time = diffSec;
  if (B->time >= cutoff*60) {
    B->time = cutoff*60;
    return;
  }

  // Update
  if (goRight) {
    if(B->debug) {
      B->printCandidates();
      B->printVertexSet();
      B->printSolution();
      cout << "Current Solution Size = " << B->solution.size() << ", Branching RIGHT..." << endl << endl;
    }
    branchAndBoundIter(B, M, cutoff);
    if (B->debug) {
      cout << "Result of RIGHT Branch..." << endl;
      B->printSolution();
      cout << endl;
    }

    B->restoreState(M);
  } else {
    return;
  }

  // Check our runtime
  if (B->time >= cutoff*60) {
    B->time = cutoff*60;
    return;
  }

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

  // Initialize trivial components
  BnBInfo BSol;
  BSol.debug = true;
  BSol.time = 0;
  BSol.sizeV = G.sizeV;
  BSol.sizeE = G.sizeE;
  BSol.prevVertex = -1;
  BSol.prevEdges = vector<Edge> ();
  BSol.vertexSet = vector<int> ();
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
  trace << fixed << setprecision(2) << G.sizeV << " 0.00" << endl;
  trace.close();

  // Grab candidate vertices in ascending order of connections
  vector< vector<int> > aLst = G.getAdjacencyList();
  sort(aLst.begin()+1, aLst.end(), cmpSize);
  for (int i=1; i<aLst.size(); i++) {
    BSol.candidates.push_back(aLst[i][0]);
  }

  // Call the main iterator
  cout << "-------- SOLUTION FROM BRANCH AND BOUND SOLVER --------" << endl << endl;
  branchAndBoundIter(&BSol, &lpInit, cutoff);
  sort(BSol.solution.begin(), BSol.solution.end());
  cout << "BRANCH AND BOUND SUMMARY" << endl;
  cout << "|V|=" << G.sizeV << ", |E|=" << G.sizeE << endl;
  BSol.printSolution();
  cout << "numVertices = " << BSol.solution.size() << endl;
  cout << "isVC = " << G.isVC(BSol.solution) << endl;

  // Output solution info
  sol.open(solFName.c_str());
  sol << BSol.solution.size() << endl;
  if (BSol.solution.size() > 0) {
    for (int i=0; i<BSol.solution.size()-1; i++) {
      sol << BSol.solution[i] << ",";
    }
    sol << BSol.solution[BSol.solution.size()-1];
  }
  sol.close();

}

// Simple tests
int main() {
  Graph g = parseGraph("../input/karate.graph");
  branchAndBound(g, "karate", 1);
  return 1;
}

#endif
