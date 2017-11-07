/* CSE 6140 Project - Branch and Bound Method */

/* Chosen method is Gurobi */
#include <stdio.h>
#include <sstream>
#include <ctime>
#include <fstream>
#include <math.h>
#include <algorithm>
#include "gurobi_c++.h"
#include "global.hpp"
using namespace std;

class BnBInfo {
public:
  double time; // in seconds
  Graph G;
  int incumbentValue;
  vector<int> candidates;
  vector<bool> vertexCheck;
  vector<int> vertexSet;
  vector<int> solution;
  vector< vector<double> > history;
  BnBInfo();
  void printCandidates();
  void printVertexSet();
};

BnBInfo::BnBInfo() {
  time = 0;
}
void BnBInfo::printCandidates() {
  cout << "Candidates = " ;
  for(int i=0; i<candidates.size(); i++) {
    cout << candidates[i] << " ";
  }
  cout << endl;
}
void BnBInfo::printVertexSet() {
  cout << "Vertex Set = " ;
  for(int i=0; i<vertexSet.size(); i++) {
    cout << vertexSet[i] << " ";
  }
  cout << endl;
}
// Utility
bool cmpSize(vector<int> a, vector<int> b) {
  return (a.size() > b.size()); // Larger sizes first
}

// Subroutine / Iterator which performs the branching
// branchAndBoundIter :: BnBInfo -> GRBModel -> Double -> BnBInfo
BnBInfo branchAndBoundIter(BnBInfo B, GRBModel M, double cutoff) {

  // Check our runtime
  if (B.time >= 60*cutoff) {
    return B;
  }

  // Helper variables
  int nVars, nConstrs;
  vector<int> VBases;
  vector<int> CBases;
  stringstream vName;
  BnBInfo BVIn, BVOut, BLeft, BRight, BFinal, BInter;
  time_t startTimer, timer1, timer2, timer3, timer4;
  double diffSec;
  time(&startTimer);

  // Update the info
  int vSplit = B.candidates[0];
  B.candidates.erase(B.candidates.begin());
  vName << "v" << vSplit;

  // ===================================
  // Include the candidate vertex (LEFT)
  // ===================================

  // Copy and update BnBInfo and check if solution is better than the incumbent
  BVIn = B;
  vector<int> adjV = B.G.getAdjacent(vSplit);
  BVIn.vertexSet.push_back(vSplit);
  BVIn.vertexCheck[vSplit] = true;
  for (int i=0; i<adjV.size(); i++) {
    BVIn.vertexCheck[adjV[i]] = true;
  }
  // *** FATHOM *** via optimality or bounding
  if (find(BVIn.vertexCheck.begin(),
           BVIn.vertexCheck.end()  , false) == BVIn.vertexCheck.end()) {
    cout << "Fathom due to optimality or bounding!" << endl;
    if (BVIn.vertexSet.size() < BVIn.incumbentValue) {
      BVIn.solution = BVIn.vertexSet;
      BVIn.incumbentValue = BVIn.solution.size();
      cout << "Fathom due to OPTIMALITY!" << endl;
    }
    return BVIn;
  }
  // *** FATHOM *** via infeasibility
  if (BVIn.candidates.empty()) {
    cout << "Fathom due to infeasibility!" << endl;
    cin.get();
    return BVIn;
  }

  // When we still need to search potential solutions and have a solution, start LP building
  GRBModel MIn  = M;
  if(B.solution.size() > 0) {

    // Grab the basis info
    nVars = M.get(GRB_IntAttr_NumVars);
    nConstrs = M.get(GRB_IntAttr_NumConstrs);
    vector<int> VBases (nVars);
    vector<int> CBases (nConstrs);
    M.optimize();
    GRBConstr* MConstrs = M.getConstrs();
    GRBVar* MVars = M.getVars();
    for (int i=0; i<nVars; i++) {
      VBases[i] = MVars[i].get(GRB_IntAttr_VBasis);
    }
    for (int i=0; i<nConstrs; i++) {
      CBases[i] = MConstrs[i].get(GRB_IntAttr_CBasis);
    }

    // <--- Start New Gurobi Model ---> 
    MIn.addConstr(MIn.getVarByName(vName.str()) == 1);
    // MIn.getEnv().set(GRB_IntParam_OutputFlag, true);
    MIn.update();
    int nVarsIn = MIn.get(GRB_IntAttr_NumVars);
    int nConstrsIn = MIn.get(GRB_IntAttr_NumConstrs);
    GRBConstr* MInConstrs = MIn.getConstrs();
    GRBVar* MInVars = MIn.getVars();
    for (int i=0; i<nVarsIn; i++) {
      if (i<nVars) {
        MInVars[i].set(GRB_IntAttr_VBasis, VBases[i]);
      } else {
        MInVars[i].set(GRB_IntAttr_VBasis, 0);
      }
    }
    for (int i=0; i<nConstrsIn; i++) {
      if (i<nConstrs) {
        MInConstrs[i].set(GRB_IntAttr_CBasis, CBases[i]);
      } else {
        MInConstrs[i].set(GRB_IntAttr_CBasis, 0);
      }
    }
    MIn.optimize();
    // <--- End New Gurobi Model --->

    // *** FATHOM *** via bounding
    cout << "LEFT -> LP fathom due to bounding!" << endl;
    if (MIn.get(GRB_DoubleAttr_ObjVal) > BVIn.incumbentValue) {
      return BVIn;
    }
  }

  // Check our runtime
  time(&timer1);
  diffSec = difftime(timer1, startTimer);
  BVIn.time += diffSec;
  if (BVIn.time >= cutoff*60) {
    BVIn.time = cutoff*60;
    return BVIn;
  }

  // Last resort -> we MUST branch
  BVIn.printCandidates();
  BVIn.printVertexSet();
  cout << "IncumbentValue=" << BVIn.incumbentValue << "... Branching LEFT..." << endl;
  BLeft  = branchAndBoundIter(BVIn,  MIn,  cutoff);
  // Check our runtime (again)

  time(&timer2);
  diffSec = difftime(timer2, timer1);
  BLeft.time += diffSec;
  if (BLeft.time >= cutoff*60) {
    BLeft.time = cutoff*60;
    return BLeft;
  }
  // ====================================
  // Combine LEFT and OLD solutions
  // ====================================
  if (BLeft.incumbentValue < B.incumbentValue) {
    BInter = BLeft;
  } else {
    BInter = B;
  }

  // ====================================
  // Exclude the candidate vertex (RIGHT)
  // ====================================

  // Copy BnBInfo
  BVOut = BInter;
  // *** FATHOM *** via infeasibility
  if (BVOut.candidates.empty()) {
    cout << "Fathom due to infeasibility!" << endl;
    return BVOut;
  }
  // When we still need to search potential solutions and have a solution, start LP building
  GRBModel MOut  = M;
  if(BInter.solution.size() > 0) {

    // Grab the basis info
    nVars = M.get(GRB_IntAttr_NumVars);
    nConstrs = M.get(GRB_IntAttr_NumConstrs);
    vector<int> VBases (nVars);
    vector<int> CBases (nConstrs);
    M.optimize();
    GRBConstr* MConstrs = M.getConstrs();
    GRBVar* MVars = M.getVars();
    for (int i=0; i<nVars; i++) {
      VBases[i] = MVars[i].get(GRB_IntAttr_VBasis);
    }
    for (int i=0; i<nConstrs; i++) {
      CBases[i] = MConstrs[i].get(GRB_IntAttr_CBasis);
    }
    
    // <--- Start New Gurobi Model ---> 
    MOut.addConstr(MOut.getVarByName(vName.str()) == 0);
    // MOut.getEnv().set(GRB_IntParam_OutputFlag, true);
    MOut.update();
    int nVarsOut = MOut.get(GRB_IntAttr_NumVars);
    int nConstrsOut = MOut.get(GRB_IntAttr_NumConstrs);
    GRBConstr* MOutConstrs = MOut.getConstrs();
    GRBVar* MOutVars = MOut.getVars();
    for (int i=0; i<nVarsOut; i++) {
      if (i<nVars) {
        MOutVars[i].set(GRB_IntAttr_VBasis, VBases[i]);
      } else {
        MOutVars[i].set(GRB_IntAttr_VBasis, 0);
      }
    }
    for (int i=0; i<nConstrsOut; i++) {
      if (i<nConstrs) {
        MOutConstrs[i].set(GRB_IntAttr_CBasis, CBases[i]);
      } else {
        MOutConstrs[i].set(GRB_IntAttr_CBasis, 0);
      }
    }
    MOut.optimize();
    // <--- End New Gurobi Model --->

    // *** FATHOM *** via bounding
    cout << "RIGHT -> LP fathom due to bounding!" << endl;
    if (MOut.get(GRB_DoubleAttr_ObjVal) > BVOut.incumbentValue) { 
      return BVOut;
    }
  }

  // Check our runtime
  time(&timer3);
  diffSec = difftime(timer3, startTimer);
  BVOut.time += diffSec;
  if (BVOut.time >= cutoff*60) {
    BVOut.time = cutoff*60;
    return BVOut;
  }

  // Last resort -> we MUST branch
  BVOut.printCandidates();
  BVOut.printVertexSet();
  cout << "IncumbentValue=" << BVOut.incumbentValue << "... Branching RIGHT..." << endl;
  BRight = branchAndBoundIter(BVOut, MOut, cutoff);

  // Check our runtime (again)
  time(&timer4);
  diffSec = difftime(timer4, timer3);
  BRight.time += diffSec;
  if (BRight.time >= cutoff*60) {
    BRight.time = cutoff*60;
    return BRight;
  }

  // ====================================
  // Combine INTER and RIGHT solutions
  // ====================================
  if (BInter.incumbentValue < BRight.incumbentValue) {
    BFinal = BInter;
  } else {
    BFinal = BRight;
  }
  return BFinal;
}

// Main Branch and Bound function
// branchAndBound :: Graph -> String -> Double -> Void
// **Note: cutoff is in minutes
void branchAndBound(Graph G, string instName, double cutoff) {

  // Set up main variables
  ofstream sol, trace;
  BnBInfo BInit;
  GRBModel lpInit = vcLpSolve(G, false);

  // Initialize trivial components
  BInit.time = 0;
  BInit.G = G;
  BInit.incumbentValue = G.sizeV;
  BInit.vertexCheck = vector<bool> (G.sizeV+1, false);
  BInit.vertexCheck[0] = true; // Exclude checking the trivial vertex
  BInit.vertexSet = vector<int> ();
  BInit.solution = vector<int> ();
  BInit.history = vector< vector<double> > ();

  // Grab candidate vertices in descending order of connections
  vector< vector<int> > aLst = G.getAdjacencyList();
  sort(aLst.begin()+1, aLst.end(), cmpSize);
  for (int i=1; i<aLst.size(); i++) {
    BInit.candidates.push_back(aLst[i][0]);
  }

  // Call the main iterator
  BnBInfo BOut = branchAndBoundIter(BInit, lpInit, cutoff);
  cout << "Solution:" << endl;
  BOut.printVertexSet();
  cout << "numVertices = " << BOut.incumbentValue << endl;

  // Output info

}

// Simple tests
int main() {
  Graph g = parseGraph("../input/jazz.graph");
  branchAndBound(g, "karate", 1);
  return 1;
}
