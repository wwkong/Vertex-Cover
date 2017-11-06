/* CSE 6140 - LP Formulation for Vertex Cover */

/* Chosen method is Gurobi */
#include <stdio.h>
#include <sstream>
#include "gurobi_c++.h"
#include "graph.hpp"
using namespace std;

// Given a graph G, solve the LP relaxation of the Vertex Cover problem
GRBModel solveVC(Graph G) {
  // Grab the needed info
  int sizeV = G.sizeV;
  int sizeE = G.sizeE;
  vector<Edge> edges = G.getEdges();
  // Set up the model variables
  stringstream vName, sName, tName;
  vector<double> costs(sizeV,1.0);
  vector<string> varNames(sizeV);
  for (int i=1; i<=varNames.size(); i++) {
    vName << i;
    varNames[i] = "v" + vName.str();
    vName.str(string());
    vName.clear();
  }
  GRBEnv env = GRBEnv();
  GRBModel model = GRBModel(env);
  try {
    model.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);
    GRBVar *vars = model.addVars(NULL, NULL, &costs[0], "GRB_CONTINUOUS", &varNames[0], sizeV);
    // Add constraints
    int s, t;
    for (int j=0; j<edges.size(); j++) {
      s = edges[j].start;
      t = edges[j].end;
      sName << s;
      tName << t;
      model.addConstr(vars[s-1] + vars[t-1] <= 1, "e_"+sName.str()+"_"+tName.str());
      sName.str(string());
      sName.clear();
      tName.str(string());
      tName.clear();
    }
    // Optimize, print, and return
    model.optimize();
    printSolution(model, nCategories, nFoods, buy, nutrition);
  }
  // Catch errors
    catch(GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  } catch(...) {
    cout << "Exception during optimization" << endl;
  }
  return model;
}

int
main(int   argc,
     char *argv[])
{
  try {
    GRBEnv env = GRBEnv();

    GRBModel model = GRBModel(env);

    // Create variables

    GRBVar x = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "x");
    GRBVar y = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "y");
    GRBVar z = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "z");

    // Set objective: maximize x + y + 2 z

    model.setObjective(x + y + 2 * z, GRB_MAXIMIZE);

    // Add constraint: x + 2 y + 3 z <= 4

    model.addConstr(x + 2 * y + 3 * z <= 4, "c0");

    // Add constraint: x + y >= 1

    model.addConstr(x + y >= 1, "c1");

    // Optimize model

    model.optimize();

    cout << x.get(GRB_StringAttr_VarName) << " "
         << x.get(GRB_DoubleAttr_X) << endl;
    cout << y.get(GRB_StringAttr_VarName) << " "
         << y.get(GRB_DoubleAttr_X) << endl;
    cout << z.get(GRB_StringAttr_VarName) << " "
         << z.get(GRB_DoubleAttr_X) << endl;

    cout << "Obj: " << model.get(GRB_DoubleAttr_ObjVal) << endl;

  } catch(GRBException e) {
    cout << "Error code = " << e.getErrorCode() << endl;
    cout << e.getMessage() << endl;
  } catch(...) {
    cout << "Exception during optimization" << endl;
  }

  return 0;
}
