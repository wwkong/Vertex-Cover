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
  vector<int>           prevEdgesNum;
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
  // Optimality
  bool isOptimal(double);
  // Branching
  bool inclVertex(GRBModel*); // Should we include the last vertex in 'candidates'?
  bool exclVertex(GRBModel*); // Should we exclude the last vertex in 'candidates'
  void restoreState(GRBModel*);  // Revert to a state one level up
  // Utility
  void printCandidates();
  void printVertexSet();
  void printSolution();
  void printPrevEdgeNum();
};

// Constructors
BnBInfo::BnBInfo() {
  time = 0;
}

// Optimality
bool BnBInfo::isOptimal(double cutoff) {

  stringstream ss, fss;
  ofstream trace;
  string traceFName;
  if (edgeSet.empty() && vertexSet.size() < solution.size()) {

    // Update and add to the trace log
    solution = vertexSet;
    if (debug) {
      printSolution();
      cout << "Solution Size = " << solution.size() << endl;
      cout << "Fathom due to OPTIMALITY!" << endl << endl;
    }

    // Output
    fss << instName << "_BnB_" << cutoff;
    traceFName = fss.str()+".trace";
    fss.str(string());
    trace.open(traceFName.c_str(), ios_base::app);
    trace << fixed << setprecision(2) << solution.size() << " " << time << endl;
    trace.close();

    return true;
  } else {
    return false;
  }
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
    prevTypes.push_back('1');
    prevConstrNames.push_back(constrName);
    prevVertices.push_back(vSplit);
    // Edges in particular
    vector<Edge> newESet;
    vector<bool> vBoolArr(sizeV+1, false);
    for (int i=0; i<vertexSet.size(); i++) {
      vBoolArr[vertexSet[i]] = true;
    }
    Edge e;
    int pEdgesN = 0;
    for (int i=0; i<edgeSet.size(); i++) {
      e = edgeSet[i];
      if ((vBoolArr[e.start] == false) && (vBoolArr[e.end] == false)) {
        newESet.push_back(e);
      } else {
        prevEdges.push_back(e);
        pEdgesN++;
      }
    }
    edgeSet = newESet;
    prevEdgesNum.push_back(pEdgesN);
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
    prevTypes.push_back('0');
    prevConstrNames.push_back(constrName);
    prevVertices.push_back(vSplit);
    prevEdgesNum.push_back(0);
    return true;
  }
}
void BnBInfo::restoreState(GRBModel *MPtr) {

  if (debug) {
    cout << "Attempting to restore state..." << endl;
    cout << "Previous Type = " << prevTypes.back() << endl;
    cout << "Previous Vertex = " << prevVertices.back() << endl;
    cout << "Previous Constraint Name = " << prevConstrNames.back() << endl;
    cout << "Size of Previous Edges = " << prevEdges.size() << endl;
    cout << "Size of Edge Set = " << edgeSet.size() << endl;
    cout << "Size of Previous Edges Num = " << prevEdgesNum.back() << endl;
    cout << "Solution Size = " << solution.size() << endl;
    printVertexSet();
    printCandidates();
    printSolution();
    printPrevEdgeNum();
  }

  // Get the latest info
  int eNum = prevEdgesNum.back();
  int vertex = prevVertices.back();
  char type = prevTypes.back();
  string constr = prevConstrNames.back();
  prevEdgesNum.pop_back();
  prevVertices.pop_back();
  prevTypes.pop_back();
  prevConstrNames.pop_back();
  // Case where we have updated vertexSet and edgeSet
  if (type == '1') {
    vertexSet.pop_back();
  }
  // Reinstate candidates and edges
  candidates.push_back(vertex);
  for(int i=0; i<eNum; i++) {
    edgeSet.push_back(prevEdges.back());
    prevEdges.pop_back();
  }
  // Remove a constraint
  try {
    MPtr->remove(MPtr->getConstrByName(constr));
  }
  // Catch errors
  catch(GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  } catch(...) {
    cout << "Exception during optimization" << endl;
  }

  // Sucessful restore!
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
void BnBInfo::printPrevEdgeNum() {
  cout << "Previous Edge Num = " ;
  for(int i=0; i<prevEdgesNum.size(); i++) {
    cout << prevEdgesNum[i] << " ";
  }
  cout << endl;
}

#endif
