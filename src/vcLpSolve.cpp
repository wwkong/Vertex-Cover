/* CSE 6140 - LP Formulation for Vertex Cover */

#ifndef VCLP_H
#define VCLP_H

/* Chosen method is Gurobi */
#include <stdio.h>
#include <sstream>
#include <math.h>
#include <unistd.h>
#include "gurobi_c++.h"
#include "graph.hpp"
#include "parseGraph.cpp"
using namespace std;

// Given a graph G, solve the LP relaxation of the Vertex Cover problem
GRBModel vcLpSolve(Graph G, bool printFlag) {
  // Grab the needed info
  vector<Edge> edges = G.getEdges();
  vector<int> vertices = G.getVertices();
  int sizeE = edges.size();
  int sizeV = vertices.size();
  // Set up the model variables
  stringstream vName, sName, tName;
  vector<double> costs(sizeV,1.0);
  vector<string> varNames(sizeV);
  vector<char> types(sizeV);
  for (int i=0; i<sizeV; i++) {
    vName << vertices[i];
    varNames[i] = "v" + vName.str();
    types[i] = GRB_CONTINUOUS;
    vName.str(string());
    vName.clear();
  }
  // Supress stdout
  int o = dup(fileno(stdout));
  freopen ("/dev/null", "w", stdout);
  GRBEnv *env = new GRBEnv();
  dup2(o,fileno(stdout));
  close(o);
  // Re-open stdout
  GRBModel model = GRBModel(*env);
  try {
    model.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);
    GRBVar *vars = model.addVars(NULL, NULL, &costs[0], &types[0], &varNames[0], sizeV);
    model.update();
    // Add constraints
    int s, t;
    GRBVar sVar, tVar;
    for (int j=0; j<edges.size(); j++) {
      s = min(edges[j].start, edges[j].end);
      t = max(edges[j].start, edges[j].end);
      sName << s;
      tName << t;
      sVar = model.getVarByName("v"+sName.str());
      tVar = model.getVarByName("v"+tName.str());
      model.addConstr(sVar + tVar >= 1, "e_"+sName.str()+"_"+tName.str());
      sName.str(string());
      sName.clear();
      tName.str(string());
      tName.clear();
    }
    // Optimize, print, and return
    model.getEnv().set(GRB_IntParam_OutputFlag, printFlag);
    model.update();
    model.optimize();
    return model;
  }
  // Catch errors
    catch(GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  } catch(...) {
    cout << "Exception during optimization" << endl;
  }
  delete env;
  return model;
}

// // Simple tests
// int main(int argc, char *argv[]) {
//   Graph g = parseGraph("../input/jazz.graph");
//   GRBModel vcModel = vcLpSolve(g, false);
//   cout << "ObjVal: " << vcModel.get(GRB_DoubleAttr_ObjVal) << endl;
//   return 0;
// }

#endif
