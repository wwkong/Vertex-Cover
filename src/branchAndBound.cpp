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
  string instName;
  Graph G;
  vector<int> candidates;
  vector<int> vertexSet;
  vector<Edge> edgeSet; // Edges which still do not touch a vertex in vertexSet
  vector<int> solution;
  BnBInfo();
  void printCandidates();
  void printVertexSet();
  void printSolution();
  void updateEdgeSet();
};

// All constructors and methods
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
void BnBInfo::updateEdgeSet() {
  vector<Edge> newESet;
  vector<bool> vBoolArr(G.sizeV+1, false);
  for (int i=0; i<vertexSet.size(); i++) {
    vBoolArr[vertexSet[i]] = true;
  }
  Edge e;
  for (int i=0; i<edgeSet.size(); i++) {
    e = edgeSet[i];
    if ((vBoolArr[e.start] == false) && (vBoolArr[e.end] == false)) {
      newESet.push_back(e);
    }
  }
  edgeSet = newESet;
}

// Utility
bool cmpSize(vector<int> a, vector<int> b) {
  return (a.size() > b.size()); // Larger sizes first
}

// Subroutine / Iterator which performs the branching
// branchAndBoundIter :: BnBInfo -> GRBModel -> Double -> Bool -> BnBInfo
BnBInfo branchAndBoundIter(BnBInfo B, GRBModel M, double cutoff, bool debug) {

    // Check our runtime
    if (B.time >= 60*cutoff) {
      return B;
    }

    // Helper variables
    int nVars, nConstrs;
    vector<int> VBases;
    vector<int> CBases;
    stringstream ss;
    BnBInfo BVIn, BVOut, BLeft, BRight, BFinal;
    time_t startTimer, timer1, timer2, timer3, timer4;
    double diffSec, leftLB, rightLB;
    time(&startTimer);

    // Update the info
    int vSplit = B.candidates[0];
    B.candidates.erase(B.candidates.begin());

    // ===================================
    // Include the candidate vertex (LEFT)
    // ===================================

    // Copy and update BnBInfo
    BVIn = B;
    BVIn.vertexSet.push_back(vSplit);
    BVIn.updateEdgeSet();
    // Check optimality (BOTTLENECK)
    if (BVIn.edgeSet.empty() && BVIn.vertexSet.size() < BVIn.solution.size()) {
      BVIn.solution = BVIn.vertexSet;
      if (debug) {
        cout << "Adding vertex " << vSplit << "..." << endl;
        BVIn.printSolution();
        cout << "Solution Size = " << BVIn.solution.size() << endl;
        cout << "Fathom due to OPTIMALITY!" << endl << endl;
      }
      return BVIn;
    }

    // -------- Grab a LOWER BOUND ---------
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

      // Process the model output
      if (MIn.get(GRB_IntAttr_Status) == 3) {
        if (debug) {
          cout << "LEFT -> LP is infeasible!" << endl << endl;
        }
        leftLB = B.G.sizeV;
      } else {
        leftLB = ceil(MIn.get(GRB_DoubleAttr_ObjVal));
      }

    } else {
      leftLB = B.solution.size();
    }

    // ====================================
    // Exclude the candidate vertex (RIGHT)
    // ====================================

    // Copy BnBInfo
    BVOut = B;

    // -------- Grab a LOWER BOUND ---------
    GRBModel MOut = M;
    if(BVOut.solution.size() > 0) {

      // *** Note *** Removed the warm start since the presolver is faster usually
      // <--- Start New Gurobi Model --->
      ss << vSplit;
      MOut.addConstr(MOut.getVarByName("v"+ss.str()) == 0);
      ss.str(string());
      // MOut.getEnv().set(GRB_IntParam_OutputFlag, true);
      MOut.optimize();
      // <--- End New Gurobi Model --->

      // Process the model output
      if (MOut.get(GRB_IntAttr_Status) == 3) {
        if (debug) {
          cout << "RIGHT -> LP is infeasible!" << endl << endl;
        }
        rightLB = B.G.sizeV;
      } else {
        rightLB = ceil(MOut.get(GRB_DoubleAttr_ObjVal)); 
      }
    } else {
      rightLB = B.solution.size();
    }

    // Check our runtime
    time(&timer1);
    diffSec = difftime(timer1, startTimer);
    B.time += diffSec;
    if (B.time >= cutoff*60) {
      B.time = cutoff*60;
      return B;
    }

    // ====================================
    // Determine branching strategy
    // ====================================

    BLeft = B;
    BRight = B;

    // LEFT
    if (leftLB < B.solution.size()) {
      if (debug) {
        cout << "Attempting to add vertex " << vSplit << "..." << endl;
        BVIn.printCandidates();
        BVIn.printVertexSet();
        BVIn.printSolution();
        cout << "Current Solution Size = " << BVIn.solution.size() << ", Branching LEFT..." << endl << endl;
      }
      BLeft  = branchAndBoundIter(BVIn, MIn, cutoff, debug);
      if (debug) {
        cout << "Result of LEFT Branch..." << endl;
        BVOut.printSolution();
        cout << endl;
      }
      // Update
      BVOut.solution = BLeft.solution;
    } else {
      if (debug) {
        cout << "Failed to add vertex " << vSplit << "..." << endl;
        cout << "Left LB = " << leftLB << ", Solution Size = " << B.solution.size() << endl << endl;
      }
    }

    // Check our runtime
    time(&timer2);
    diffSec = difftime(timer2, startTimer);
    BLeft.time += diffSec;
    BRight.time += diffSec;
    if (BLeft.time >= cutoff*60) {
      BLeft.time = cutoff*60;
      return BLeft;
    }

    // RIGHT
    if (rightLB < B.solution.size()) {
      if (debug) {
        cout << "Attempting to remove vertex " << vSplit << "..." << endl;
        BVOut.printCandidates();
        BVOut.printVertexSet();
        BVOut.printSolution();
        cout << "Current Solution Size = " << BVOut.solution.size() << ", Branching RIGHT..." << endl << endl;
      }
      BRight = branchAndBoundIter(BVOut, MOut, cutoff, debug);
    } else {
      if (debug) {
        cout << "Failed to remove vertex " << vSplit << "..." << endl;
        cout << "Right LB = " << rightLB << ", Solution Size = " << B.solution.size() << endl << endl;
      }
    }

    // Check our runtime
    time(&timer3);
    diffSec = difftime(timer3, startTimer);
    BRight.time += diffSec;
    if (BRight.time >= cutoff*60) {
      BRight.time = cutoff*60;
      return BRight;
    }

    // Case where we cannot branch
    if (rightLB >= B.solution.size() && leftLB >= B.solution.size()) {
      if (debug) {
        cout << "Lower bound exceeds current solution size in branches!" << endl << endl;
      }
      return B;
    }

    // ====================================
    // Combine LEFT and RIGHT solutions
    // ====================================
    if (BLeft.solution.size() < BRight.solution.size()) {
      BFinal = BLeft;
    } else {
      BFinal = BRight;
    }
    if (debug) {
      BFinal.printCandidates();
      BFinal.printVertexSet();
      BFinal.printSolution();
      cout << "RETURN Combined Branches" << endl << endl;
    }
    return BFinal;

}

// Main Branch and Bound function
// branchAndBound :: Graph -> String -> Double -> Void
// **Note: cutoff is in minutes
void branchAndBound(Graph G, string instName, double cutoff) {

  // Set up main variables
  ofstream sol;
  BnBInfo BInit;
  GRBModel lpInit = vcLpSolve(G, false);
  vector<int> verts = G.getVertices();

  // // Print the exact solution to compare
  // cout << "-------- SOLUTION FROM IP SOLVER --------" << endl << endl;
  // GRBModel ipInit = lpInit;
  // ipInit.getEnv().set(GRB_IntParam_OutputFlag, true);
  // stringstream ss;
  // for (int i=1; i<verts.size()+1; i++) {
  //   ss << i;
  //   try {
  //   ipInit.getVarByName("v"+ss.str()).set(GRB_CharAttr_VType, 'B');
  //   } catch(GRBException e) {
  //     cout << "Error code = " << e.getErrorCode() << endl;
  //     cout << e.getMessage() << endl;
  //   } catch(...) {
  //     cout << "Exception during optimization" << endl;
  //   }
  //   ss.str(string());
  // }
  // ipInit.optimize();
  // cout << endl;
  // ipInit.getEnv().set(GRB_IntParam_OutputFlag, false);

  // Initialize trivial components
  BInit.time = 0;
  BInit.G = G;
  BInit.vertexSet = vector<int> ();
  BInit.edgeSet = G.getEdges();
  BInit.solution = verts;
  BInit.instName = instName;

  // Grab candidate vertices in descending order of connections
  vector< vector<int> > aLst = G.getAdjacencyList();
  sort(aLst.begin()+1, aLst.end(), cmpSize);
  for (int i=1; i<aLst.size(); i++) {
    BInit.candidates.push_back(aLst[i][0]);
  }

  // Call the main iterator
  cout << "-------- SOLUTION FROM BRANCH AND BOUND SOLVER --------" << endl << endl;
  BnBInfo BSol = branchAndBoundIter(BInit, lpInit, cutoff, false);
  cout << "BRANCH AND BOUND SUMMARY" << endl;
  cout << "|V|=" << G.sizeV << ", |E|=" << G.sizeE << endl;
  BSol.printSolution();
  cout << "numVertices = " << BSol.solution.size() << endl;
  cout << "isVC = " << G.isVC(BSol.solution) << endl;

  // Output info

}

// Simple tests
int main() {
  Graph g = parseGraph("../input/football.graph");
  branchAndBound(g, "", 1);
  return 1;
}
