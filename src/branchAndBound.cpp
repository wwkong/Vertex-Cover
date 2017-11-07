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

    // *** FATHOM *** via infeasibility
    if (B.candidates.empty()) {
      cout << "Fathom due to infeasibility!" << endl;
      return B;
    }

    // Helper variables
    int nVars, nConstrs;
    vector<int> VBases;
    vector<int> CBases;
    stringstream vName;
    BnBInfo BVIn, BVOut, BLeft, BRight, BFinal;
    time_t startTimer, timer1, timer2, timer3, timer4;
    double diffSec;
    time(&startTimer);

    // Update the info
    int vSplit = B.candidates[0];
    vName << "v" << vSplit;

    // ===================================
    // Include the candidate vertex (LEFT)
    // ===================================

    // Copy and update BnBInfo and check if solution is better than the incumbent
    BVIn = B;
    vector<int> adjV = B.G.getAdjacent(vSplit);
    BVIn.vertexSet.push_back(vSplit);
    // *** FATHOM *** via optimality or bounding
    if (BVIn.G.isVC(BVIn.vertexSet)) {
      cout << "VIn -> Fathom due to optimality or bounding!" << endl;
      if (BVIn.vertexSet.size() < BVIn.incumbentValue) {
        BVIn.solution = BVIn.vertexSet;
        BVIn.incumbentValue = BVIn.solution.size();
        cout << "VIn -> Fathom due to OPTIMALITY!" << endl;
      }
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
      cout << "LEFT LP Status: " << MIn.get(GRB_IntAttr_Status) << endl;
      // <--- End New Gurobi Model --->

      // *** FATHOM *** via bounding
      if (MIn.get(GRB_DoubleAttr_ObjVal) >= BVIn.incumbentValue) {
        cout << "LEFT -> LP fathom due to bounding!" << endl;
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
    try {
      BVIn.candidates.erase(BVIn.candidates.begin());
      BLeft  = branchAndBoundIter(BVIn,  MIn,  cutoff);
    }
    // Catch errors
    catch(GRBException e) {
      cout << "Error code = " << e.getErrorCode() << endl;
      cout << e.getMessage() << endl;
    } catch(...) {
      cout << "Exception during optimization" << endl;
    }

    // Check our runtime (again)
    time(&timer2);
    diffSec = difftime(timer2, timer1);
    BLeft.time += diffSec;
    if (BLeft.time >= cutoff*60) {
      BLeft.time = cutoff*60;
      return BLeft;
    }

    // ====================================
    // Exclude the candidate vertex (RIGHT)
    // ====================================

    // Copy BnBInfo
    BVOut = B;
    BVOut.incumbentValue = BLeft.incumbentValue;
    BVOut.solution = BLeft.solution;
    // When we still need to search potential solutions and have a solution, start LP building
    GRBModel MOut = M;
    if(BVOut.solution.size() > 0) {

      // *** Note *** Removed the warm start since the presolver is faster usually
    
      // <--- Start New Gurobi Model ---> 
      MOut.addConstr(MOut.getVarByName(vName.str()) == 0);
      // MOut.getEnv().set(GRB_IntParam_OutputFlag, true);
      MOut.update();
      MOut.optimize();
      cout << "RIGHT LP Status: " << MOut.get(GRB_IntAttr_Status) << endl;
      // <--- End New Gurobi Model --->

      // *** FATHOM *** via bounding
      if (MOut.get(GRB_DoubleAttr_ObjVal) >= BVOut.incumbentValue) { 
        cout << "RIGHT -> LP fathom due to bounding!" << endl;
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
    try {
      BVOut.candidates.erase(BVOut.candidates.begin());
      BRight = branchAndBoundIter(BVOut, MOut, cutoff);
    }
    // Catch errors
    catch(GRBException e) {
      cout << "Error code = " << e.getErrorCode() << endl;
      cout << e.getMessage() << endl;
    } catch(...) {
      cout << "Exception during optimization" << endl;
    }

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
    if (BLeft.incumbentValue < BRight.incumbentValue) {
      BFinal = BLeft;
    } else {
      BFinal = BRight;
    }
    return BFinal;
  // }
  // Catch errors
  // catch(GRBException e) {
  //   cout << "Error code = " << e.getErrorCode() << endl;
  //   cout << e.getMessage() << endl;
  // } catch(...) {
  //   cout << "Exception during optimization" << endl;
  // }

  return B;
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
  cout << "|V|=" << G.sizeV << ", |E|=" << G.sizeE << endl;
  cout << "Solution:" << endl;
  BOut.printVertexSet();
  cout << "numVertices = " << BOut.incumbentValue << endl;
  cout << "isVC = " << G.isVC(BOut.vertexSet) << endl;

  // Output info

}

// Simple tests
int main() {
  Graph g = parseGraph("../input/karate.graph");
  branchAndBound(g, "", 0.1);
  return 1;
}
