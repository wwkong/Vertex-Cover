/* CSE 6140 Project - Branch and Bound Method */

#ifndef BNBINFO_H
#define BNBINFO_H

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
#include "vcLpSolve.cpp"
using namespace std;

class BnBInfo {
public:
  bool debug;
  double time; // in seconds
  string instName;
  int sizeV;
  int sizeE;
  // HISTORY
  vector<char>          prevTypes; // '1' = add vertex, '0' = remove vertex
  vector<int>           prevVertices;
  vector<Edge>          prevEdges;
  vector<string>        prevConstrNames;
  // END HISTORY
  vector<int> vertexSet;
  vector<Edge> edgeSet;
  vector<int> candidates;
  vector<int> solution;
  // Constructors
  BnBInfo();
  // Branching
  bool inclVertex(GRBModel*); // Should we include the last vertex in 'candidates'?
  bool exclVertex(GRBModel*); // Should we exclude the last vertex in 'candidates'
  void restoreState(GRBModel*);  // Revert to a state one level up
  // Utility
  void printCandidates();
  void printVertexSet();
  void printSolution();
  void updateEdgeSet();
};

// Constructors
BnBInfo::BnBInfo() {
  time = 0;
}

// ------------------ Branching (LEFT) ------------------
bool BnBInfo::inclVertex(GRBModel *MPtr) {

  // Grab the basis info
  int nVars = MPtr->get(GRB_IntAttr_NumVars);
  int nConstrs = MPtr->get(GRB_IntAttr_NumConstrs);
  vector<int> VBases (nVars);
  vector<int> CBases (nConstrs);
  MPtr->optimize();
  GRBConstr* MConstrs = MPtr->getConstrs();
  GRBVar* MVars = MPtr->getVars();
  for (int i=0; i<nVars; i++) {
    VBases[i] = MVars[i].get(GRB_IntAttr_VBasis);
  }
  for (int i=0; i<nConstrs; i++) {
    CBases[i] = MConstrs[i].get(GRB_IntAttr_CBasis);
  }

  // <--- Start new Gurobi model to get lower bound --->
  int vSplit = candidates.back();
  stringstream ss;
  ss << vSplit;
  string constrName = "v"+ss.str()+"_1";
  MPtr->addConstr(MPtr->getVarByName("v"+ss.str()) == 1, &constrName[0]);
  ss.str(string());
  // MPtr->getEnv().set(GRB_IntParam_OutputFlag, true);
  MPtr->update();
  MConstrs = MPtr->getConstrs();
  MVars = MPtr->getVars();
  for (int i=0; i<nVars; i++) {
    MVars[i].set(GRB_IntAttr_VBasis, VBases[i]);
  }
  for (int i=0; i<nConstrs+1; i++) {
    if (i<nConstrs) {
      MConstrs[i].set(GRB_IntAttr_CBasis, CBases[i]);
    } else {
      MConstrs[i].set(GRB_IntAttr_CBasis, 0);
    }
  }
  MPtr->optimize();
  // <--- End New Gurobi Model --->

  // Infeasible model
  if (MPtr->get(GRB_IntAttr_Status) == 3) {
    if (debug) {
      cout << "LEFT -> LP is infeasible!" << endl << endl;
    }
    MPtr->remove(MPtr->getConstrByName(constrName));
    return false;
  }
  // Lower bound exceeds current solution
  else if (ceil(MPtr->get(GRB_DoubleAttr_ObjVal)) > solution.size()){
    if (debug) {
      cout << "Failed to add vertex " << vSplit << "..." << endl;
      cout << "Left LB = " << ceil(MPtr->get(GRB_DoubleAttr_ObjVal));
      cout << ", Solution Size = " << solution.size() << endl << endl;
    }
    MPtr->remove(MPtr->getConstrByName(constrName));
    return false;
  }
  // There is potential for improvement
  else {
    // Update the components
    candidates.pop_back();
    vertexSet.push_back(vSplit);
    prevType.push_back('1');
    prevConstrNames.push_back(constrName);
    prevVertices.push_back(vSplit);
    // Edges in particular
    vector<Edge> newESet;
    vector<bool> vBoolArr(sizeV+1, false);
    for (int i=0; i<vertexSet.size(); i++) {
      vBoolArr[vertexSet[i]] = true;
    }
    Edge e;
    for (int i=0; i<edgeSet.size(); i++) {
      e = edgeSet[i];
      if ((vBoolArr[e.start] == false) && (vBoolArr[e.end] == false)) {
        newESet.push_back(e);
      } else {
        prevEdges.push_back(e);
      }
    }
    edgeSet = newESet;
    return true;
  }
}

// ------------------ Branching (RIGHT) ------------------
bool BnBInfo::exclVertex(GRBModel *MPtr) {

  // *** Note *** Removed the warm start since the presolver is usually faster
  // <--- Start new Gurobi model to get lower bound --->
  int vSplit = candidates.back();
  stringstream ss;
  ss << vSplit;
  string constrName = "v"+ss.str()+"_0";
  MPtr->addConstr(MPtr->getVarByName("v"+ss.str()) == 0, &constrName[0]);
  ss.str(string());
  // MPtr->getEnv().set(GRB_IntParam_OutputFlag, true);
  MPtr->update();
  MPtr->optimize();
  // <--- End New Gurobi Model --->

  // Infeasible model
  if (MPtr->get(GRB_IntAttr_Status) == 3) {
    if (debug) {
      cout << "RIGHT -> LP is infeasible!" << endl << endl;
    }
    MPtr->remove(MPtr->getConstrByName(constrName));
    return false;
  }
  // Lower bound exceeds current solution
  else if (ceil(MPtr->get(GRB_DoubleAttr_ObjVal)) > solution.size()){
    if (debug) {
      cout << "Failed to remove vertex " << vSplit << "..." << endl;
      cout << "Right LB = " << ceil(MPtr->get(GRB_DoubleAttr_ObjVal));
      cout << ", Solution Size = " << solution.size() << endl << endl;
    }
    MPtr->remove(MPtr->getConstrByName(constrName));
    return false;
  }
  // There is potential for improvement
  else {
    // Update the components
    candidates.pop_back();
    prevType.push_back('0');
    prevConstrName = constrName;
    prevVertex = vSplit;
    return true;
  }
}
void BnBInfo::restoreState(GRBModel *MPtr) {

  if (debug) {
    cout << "Attempting to restore state..." << endl;
    cout << "Previous Type = " << prevType.back() << endl;
    cout << "Previous Vertex = " << prevVertex << endl;
    cout << "Previous Constraint Name = " << prevConstrName << endl;
    cout << "Size of Previous Edges = " << prevEdges.size() << endl;
    printVertexSet();
    printCandidates();
    printSolution();
  }

  char type = prevType.back();
  prevType.pop_back();
  if (type == '1') {
    vertexSet.pop_back();
  }
  candidates.push_back(prevVertex);
  for (int i=0; i<prevEdges.size(); i++) {
    edgeSet.push_back(prevEdges[i]);
  }
  if (prevEdges.size() > 0) {
    prevEdges.clear();
  }
  prevVertex = -1;
  // Remove a constraint
  try {
    MPtr->remove(MPtr->getConstrByName(prevConstrName));
  }
  // Catch errors
  catch(GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  } catch(...) {
    cout << "Exception during optimization" << endl;
  }


  if (debug) {
    cout << "Success!" << endl << endl;
  }
}
// Utility
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
  vector<bool> vBoolArr(sizeV+1, false);
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

#endif
