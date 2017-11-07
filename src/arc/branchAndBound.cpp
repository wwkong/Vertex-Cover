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
  vector<int> vertexSet;
  vector<int> solution;
  vector< vector<double> > history;
  BnBInfo();
  void printCandidates();
  void printVertexSet();
  void printSolution();
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
void BnBInfo::printSolution() {
  cout << "Solution = " ;
  for(int i=0; i<solution.size(); i++) {
    cout << solution[i] << " ";
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

    // *** FATHOM *** via infeasibility
    if (B.candidates.empty()) {
      cout << "EMPTY BRANCH -> Fathom due to infeasibility!" << endl;
      return B;
    }

    // Helper variables
    int nVars, nConstrs;
    vector<int> VBases;
    vector<int> CBases;
    stringstream ss;
    BnBInfo BVIn, BVOut, BLeft, BRight, BFinal;
    time_t startTimer, timer1, timer2, timer3, timer4;
    double diffSec;
    time(&startTimer);

    // Update the info
    int vSplit = B.candidates[0];
    B.candidates.erase(B.candidates.begin());
    BVIn = B;
    BVOut = B;

    // ===================================
    // Case for when we have a solution
    // ===================================

    GRBModel MIn  = M;
    GRBModel MOut = M;
    if(B.solution.size() > 0) {

      // ==========================================
      // Include vertex
      // ==========================================

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
      ss << vSplit;
      MIn.addConstr(MIn.getVarByName("v"+ss.str()) == 1);
      ss.str(string());
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

      // ==========================================
      // Exclude vertex
      // ==========================================

      // *** Note *** Removed the warm start since the presolver is faster usually
    
      // <--- Start New Gurobi Model --->
      ss << vSplit;
      MOut.addConstr(MOut.getVarByName("v"+ss.str()) == 0);
      ss.str(string());
      // MOut.getEnv().set(GRB_IntParam_OutputFlag, true);
      MOut.optimize();
      // <--- End New Gurobi Model --->

      // ==========================================
      // Combine model outputs
      // ==========================================

      // Check infeasibility
      double leftObj, rightObj;
      if (MIn.get(GRB_IntAttr_Status) == 3) { 
        cout << "LEFT -> LP is infeasible!" << endl << endl;
        leftObj = B.G.sizeV;
      } else {
        leftObj = ceil(MIn.get(GRB_DoubleAttr_ObjVal));
      }
      if (MOut.get(GRB_IntAttr_Status) == 3) { 
        cout << "RIGHT -> LP is infeasible!" << endl << endl;
        rightObj = B.G.sizeV;
      } else {
        rightObj = ceil(MOut.get(GRB_DoubleAttr_ObjVal));
      }

      // *** FATHOM *** via bounding
      if (leftObj >= BVIn.incumbentValue) {
        cout << "ObjVal = " << leftObj;
        cout << ", IncumbentValue=" << BVIn.incumbentValue << endl;
        cout << "LEFT -> LP fathom due to bounding!" << endl << endl;
        return BVIn;
      }
      if (rightObj >= BVOut.incumbentValue) {
        cout << "ObjVal = " << rightObj;
        cout << ", IncumbentValue=" << BVOut.incumbentValue << endl;
        cout << "RIGHT -> LP fathom due to bounding!" << endl << endl;
        return BVOut;
      }

      // Last resort -> we MUST branch
      BVIn.printCandidates();
      BVIn.printVertexSet();
      BVIn.printSolution();
      cout << "IncumbentValue = " << BVIn.incumbentValue << "... Branching LEFT..." << endl << endl;
      BLeft  = branchAndBoundIter(BVIn,  MIn,  cutoff);
      BVOut.printCandidates();
      BVOut.printVertexSet();
      BVOut.printSolution();
      cout << "IncumbentValue = " << BVOut.incumbentValue << "... Branching RIGHT..." << endl << endl;
      BRight = branchAndBoundIter(BVOut, MOut, cutoff);


    }

    // =======================================
    // Case for when we do NOT have a solution
    // =======================================

    else {
    
      // ===================================
      // Include vertex; no known solution
      // ===================================

      // *** FATHOM *** via optimality or bounding
      if (BVIn.vertexSet.size()+1 >= BVIn.incumbentValue) {
        cout << "VIn -> Fathom due to bounding!" << endl << endl;
        return BVIn;
      } else {
        BVIn.vertexSet.push_back(vSplit);
      }
      // Check feasibility
      if (BVIn.G.isVC(BVIn.vertexSet)) {
        BVIn.solution = BVIn.vertexSet;
        BVIn.incumbentValue = BVIn.solution.size();
        cout << "VIn -> Fathom due to OPTIMALITY!" << endl << endl;
        return BVIn;
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
      BVIn.printSolution();
      cout << "IncumbentValue = " << BVIn.incumbentValue << "... Branching LEFT..." << endl << endl;
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
      // Exclude vertex; no current solution
      // ====================================

      // Copy BnBInfo
      BVOut.incumbentValue = BLeft.incumbentValue;
      BVOut.solution = BLeft.solution;

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
      BVOut.printSolution();
      cout << "IncumbentValue = " << BVOut.incumbentValue << "... Branching RIGHT..." << endl << endl;
      BRight = branchAndBoundIter(BVOut, MOut, cutoff);

      // Check our runtime (again)
      time(&timer4);
      diffSec = difftime(timer4, timer3);
      BRight.time += diffSec;
      if (BRight.time >= cutoff*60) {
        BRight.time = cutoff*60;
        return BRight;
      }
    }

    // ====================================
    // Combine LEFT and RIGHT solutions
    // ====================================
    if (BLeft.incumbentValue < BRight.incumbentValue) {
      BFinal = BLeft;
    } else {
      BFinal = BRight;
    }
    BFinal.printSolution();
    cout << "RETURN Combined Branches" << endl << endl;
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

  // Print the exact solution to compare
  cout << "-------- SOLUTION FROM IP SOLVER --------" << endl << endl;
  GRBModel ipInit = lpInit;
  ipInit.getEnv().set(GRB_IntParam_OutputFlag, true);
  vector<int> verts = G.getVertices();
  stringstream ss;
  for (int i=1; i<verts.size()+1; i++) {
    ss << i;
    try {
    ipInit.getVarByName("v"+ss.str()).set(GRB_CharAttr_VType, 'B');
    } catch(GRBException e) {
      cout << "Error code = " << e.getErrorCode() << endl;
      cout << e.getMessage() << endl;
    } catch(...) {
      cout << "Exception during optimization" << endl;
    }
    ss.str(string());
  }
  ipInit.optimize();
  cout << endl;
  ipInit.getEnv().set(GRB_IntParam_OutputFlag, false);

  // Initialize trivial components
  BInit.time = 0;
  BInit.G = G;
  BInit.incumbentValue = G.sizeV;
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
  cout << "-------- SOLUTION FROM BRANCH AND BOUND SOLVER --------" << endl << endl;
  BnBInfo BSol = branchAndBoundIter(BInit, lpInit, cutoff);
  cout << "BRANCH AND BOUND SUMMARY" << endl;
  cout << "|V|=" << G.sizeV << ", |E|=" << G.sizeE << endl;
  BSol.printSolution();
  cout << "numVertices = " << BSol.incumbentValue << endl;
  cout << "isVC = " << G.isVC(BSol.solution) << endl;

  // Output info

}

// Simple tests
int main() {
  Graph g = parseGraph("../input/karate.graph");
  branchAndBound(g, "", 1);
  return 1;
}
