/* CSE 6140 Project - Main executable */

#include <cstring>
#include <string>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <getopt.h>
#include <math.h>
#include <time.h>
#include <iomanip> // setprecision
#include <fstream>
#include <sstream>
#include "graph.hpp"
#include "graph.cpp"
#include "parseGraph.cpp"
#include "LS1.cpp"
#include "ls2.cpp"
using namespace std;

// Copy some algos and modify

/* my timer */
double get_time(){
  struct timeval time;
  double cur_time;
  gettimeofday(&time,NULL);
  cur_time=1000000*(time.tv_sec)+time.tv_usec;
  cur_time/=1000000;
  return cur_time;
}

// Local search 1
int LS1_qrtd(Graph graph, string instName, double cutoff, int seed,
              double minVC, double quality) {
	ofstream sol, trace;
	double temp = 1;
  double cooling = 0.95;
  double thresh = 0.000001;
  double relErr;

  srand(seed);

  stringstream fss;
  fss << instName << "_qrtd";
  string traceFName = fss.str()+".trace";
  fss.str(string());

	clock_t start = clock();
	clock_t end = clock();
	double elapsedfinal= (end - start) / (float) CLOCKS_PER_SEC;
	long diff;
	int iter = 20;
	vector<int> current = maxDegGreedy(graph);
	int currCost = current.size();
	vector<int> nextSol;

	trace.open(traceFName.c_str(), ios::app);
	vector<int> params(2,-2);
	while(elapsedfinal < cutoff && temp > 0)
	{
		for(int i =0;i<iter;i++){
			nextSol = getNextSol(current,graph,params);
			if(params[1]!= -1){
				diff = params[1] - currCost;
				if((diff<0) || (currCost > 0 && exp(-abs(diff)/temp ) > float(rand()) /RAND_MAX )){
					current = nextSol;
					currCost = params[1];
				}
			}
      end = clock();
      elapsedfinal = (end - start) / (float) CLOCKS_PER_SEC;

      // Update
      relErr = (current.size()-minVC) / minVC;
      elapsedfinal = min(elapsedfinal, cutoff);
      if (relErr <= quality) {
        trace << instName << ", LS1, ";
        trace << fixed << setprecision(5) << elapsedfinal << ", ";
        trace << quality << ", " << seed << endl;
        return 3;
      }
			if(elapsedfinal >= cutoff){
				break;
			}
		}
		temp = cooling * temp;
	}
  elapsedfinal = min(elapsedfinal, cutoff);
  trace << instName << ", LS1, ";
  trace << fixed << setprecision(5) << elapsedfinal << ", "; 
  trace << quality << ", " << seed << endl;
	trace.close();
  return 0;
}

// Local Search 2
int LS2_qrtd(Graph &graph, string &instName, double cutoff, int seed,
             double minVC, double quality)
{

  double relErr;
  int num_vertices = graph.sizeV;
  srand (seed);

  double elapsed_time = 0.0;
  double start_time = get_time();

  vector<int> loss(num_vertices + 1, 0);
  vector<int> C, VC;
  construct_vc(graph, loss, C);
  elapsed_time += get_time() - start_time;
  VC = C;

  ostringstream string_stream;
  string_stream << instName << "_qrtd" << ".trace";
  ofstream trace_file;
  string trace_file_name = string_stream.str();
  trace_file.open(trace_file_name, ios::app);

  // Test the waters
  relErr = (VC.size()-minVC)/minVC;
  if (relErr <= quality) {
    elapsed_time = min(elapsed_time, cutoff);
    trace_file << instName << ", LS2, ";
    trace_file << fixed << setprecision(5) << elapsed_time << ", "; 
    trace_file << quality << ", " << seed << endl;
    return 2;
  }

  vector<int> gain(num_vertices + 1, 0);
  vector< pair<int,int> > uncovered_edges;

  while (elapsed_time < cutoff) {
    start_time = get_time();
    if (graph.isVC(C)) {
      VC = C;
      relErr = (VC.size()-minVC)/minVC;
      if (relErr <= quality) {
        trace_file << instName << ", LS2, ";
        trace_file << fixed << setprecision(5) << elapsed_time << ", "; 
        trace_file << quality << ", " << seed << endl;
        return 3;
      }
      int min_loss_vertex_index = find_min_loss_vertex(loss, C); 
      int min_loss_vertex = C[min_loss_vertex_index];
      vector<int>::iterator min_loss_vertex_it = C.begin() + min_loss_vertex_index;
      /* Update loss and gain */
      update_gain_loss_before_remove(graph, gain, loss, C, uncovered_edges, min_loss_vertex);
      C.erase(min_loss_vertex_it);
      elapsed_time += get_time() - start_time;
      continue;
    }
    int best_vertex_index = choose_rm_vertex(C, loss);
    int best_vertex = C[best_vertex_index];
    vector<int>::iterator best_vertex_it = C.begin() + best_vertex_index;
    /* Update loss and gain */
    update_gain_loss_before_remove(graph, gain, loss, C, uncovered_edges, best_vertex);
    C.erase(best_vertex_it);

    pair<int,int> random_uncovered_edge = uncovered_edges[rand() % uncovered_edges.size()];
    int new_vertex = gain[random_uncovered_edge.first] >= gain[random_uncovered_edge.second] ? random_uncovered_edge.first : random_uncovered_edge.second;
    C.push_back(new_vertex);
    /* Update loss and gain */
    update_gain_loss_after_add(graph, gain, loss, C, uncovered_edges,  new_vertex);
    elapsed_time += get_time() - start_time;
  }

  trace_file << instName << ", LS2, ";
  trace_file << fixed << setprecision(5) << cutoff << ", ";
  trace_file << quality << ", " << seed << endl;
  return 0;
}

// Main call
int main (int argc, char **argv) {

  // -----------------------------
  // Parse Options
  // -----------------------------
  string fileName;
  int c;

  opterr = 0;
  while (1)
    {
      static struct option long_options[] =
        {
          {"inst",  required_argument, 0, 'i'},
          {0}
        };

      int option_index = 0;
      c = getopt_long_only (argc, argv, "i:",
                            long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;
      switch (c)
        {
          // Main flags
        case 0:
          cout << "No options detected!" << endl;
          break;
        case 'i':
          fileName = optarg;
          break;
          // Unknown inputs
        case '?':
          break;
        default:
          abort ();
        }
    }

  // For debugging purposes
  printf ("filename = %s\n", fileName.c_str());
  ifstream ifile(fileName);
  if (!ifile) {
    cout << "Input file does not exist!" << endl;
    return 0;
  }

  // -----------------------------
  // Parse the inputs
  // -----------------------------

  cout << "Reading inputs..." << endl;
  Graph g = parseGraph(fileName);
  unsigned first = fileName.find_last_of("/");
  unsigned last = fileName.find(".graph");
  string instName = fileName.substr (first+1,last-first-1);

  // -----------------------------
  // Algorithms
  // -----------------------------

  // Global params
  int nRuns;
  vector<double> qVec_LS1, qVec_LS2;

  // Reset file
  ofstream trace;
  stringstream fss;
  fss << instName << "_qrtd";
  string traceFName = fss.str()+".trace";
  fss.str(string());
	trace.open(traceFName.c_str());
  trace << "instName, algorithm, time, quality, seed" << endl;
  trace.close();

  // Set params based on instName
  double minVC, cutoff_LS1, cutoff_LS2;
  if (strcmp(instName.c_str(), "power") == 0) {
    nRuns=50;
    minVC = 2203.0;
    cutoff_LS1 = 30.0;
    cutoff_LS2 = 1.0;

    qVec_LS1.push_back(0.10);
    qVec_LS1.push_back(0.25);
    qVec_LS1.push_back(0.75);
    qVec_LS1.push_back(1.50);

    qVec_LS2.push_back(0.01);
    qVec_LS2.push_back(0.05);
    qVec_LS2.push_back(0.10);
    qVec_LS2.push_back(0.20);

  } else if (strcmp(instName.c_str(), "star2") == 0) {
    nRuns=50;
    nRuns = 1;
    minVC = 4542.0;
    cutoff_LS1 = 120.0;
    cutoff_LS2 = 10.0;

    qVec_LS1.push_back(1.50);
    qVec_LS1.push_back(2.00);
    qVec_LS1.push_back(2.50);
    qVec_LS1.push_back(3.00);

    qVec_LS2.push_back(0.05);
    qVec_LS2.push_back(0.10);
    qVec_LS2.push_back(0.20);
    qVec_LS2.push_back(0.50);

  } else {
    cout << "Invalid graph file!" << endl;
    return 0;
  }

  // Main runs
  int seedI;
  double qJ;
  cout << "Running algorithms..." << endl;
  for (int i=0; i<nRuns; i++) {
    seedI = 777 + 1000*i;
    for (int j=0; j<qVec_LS1.size(); j++) {
      qJ = qVec_LS1[j];
      LS1_qrtd(g, instName, cutoff_LS1, seedI, minVC, qJ);
    }
    for (int j=0; j<qVec_LS2.size(); j++) {
      qJ = qVec_LS2[j];
      LS2_qrtd(g, instName, cutoff_LS2, seedI, minVC, qJ);
    }
  }

}
