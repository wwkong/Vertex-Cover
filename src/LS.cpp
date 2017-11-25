#include "graph.hpp"
#include <string>
#include <cstring>
#include <ctime>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <stack>
#include <time.h>
#include <cstddef>  
#include<algorithm> 
#include "parseGraph.hpp" 
#include<math.h>     // std::size_t
using namespace std;

vector<int> vc_approx(Graph &G)
{
	vector<int> VC;
	vector<int> v = G.getVertices();
	for( auto node : v)
	{
		for( int u : G.adjacencyList[node]){
			if(u<node || G.adjacencyList[u-1].size() == 0)
				continue;
			G.adjacencyList[u-1].clear();
			VC.push_back(node);
			VC.push_back(u);
			break;
		}
	}
	return VC;
}



int cost(Graph &gra, vector<int> candidate)
{
	int count = 0;
	vector<int> vertices = gra.getVertices();
	for(int i = 0; i<vertices.size();i++){
		if(find(candidate.begin(),candidate.end(),vertices[i]) == candidate.end()){
			for(auto each : gra.adjacencyList[vertices[i]]){
				if(each > i && find(candidate.begin(),candidate.end(),each) == candidate.end())
					count++;
			}
		}
	}
	return candidate.size() + count;
}

vector<int> construct(Graph &gra, vector<int> candidate)
{
	vector<int> vert = gra.getVertices();
	for(auto each : candidate){
		gra.adjacencyList[each-1].clear();
	}
	for(int i =0; i<vert.size();i++)
	{
		if(gra.adjacencyList[vert[i]].size() > 0){
			for(auto each : gra.adjacencyList[vert[i]]){
				if(gra.adjacencyList[each-1].size() > 0){
					candidate.push_back(i+1);
					break;
				}
			}
			gra.adjacencyList[vert[i]].clear();
		}
	}
	return candidate;
}

void ESA(Graph graph, string instName, double cutoff, int seed)
{
	ofstream sol, trace;
	stringstream fss;
  	fss << instName << "_LS1_" << cutoff;
  	string solFName = fss.str()+".sol";
  	string traceFName = fss.str()+".trace";
  	fss.str(string());
  	trace.open(traceFName.c_str());

	vector<int> candidate = vc_approx(graph);
	srand(seed);

	int step = 0;
	int iter = 20;
	double T = 1;

	int bestSofar = cost(graph, candidate);
	vector<int> best = candidate;

	clock_t start = clock(); 
	clock_t end = clock();
	float rt= (end - start) / (float) CLOCKS_PER_SEC;

	while(rt<cutoff && T>0){
		for(int i =0; i<iter;i++){
			vector<int> neighbor(candidate);
			int selected = rand() % graph.sizeV ;
			auto iter = find(neighbor.begin(), neighbor.end(),selected);
			if(iter!=neighbor.end()){
				neighbor.erase(iter);
				int delta = cost(graph,neighbor) - cost(graph,candidate);
				if(delta<= 0)
					candidate = neighbor;
				else{
					double P = exp(-delta * (1 + graph.adjacencyList[selected].size()/ float(graph.sizeV))/T);
					double R = float(rand()) / RAND_MAX;

					if(R<P){
						candidate = neighbor;
					}
				}
			}
			else{
				neighbor.push_back(selected);
				int delta = cost(graph,neighbor) - cost(graph,neighbor);
				if(delta <= 0)
					candidate = neighbor;
				else{
					double P = exp(-delta * (1 - graph.adjacencyList[selected].size()/ float(graph.sizeV))/T);
					double R = float(rand()) / RAND_MAX;

					if(R<P){
						candidate = neighbor;
					}
				}
			}
			end = clock();
			rt = (end-start) / (float) CLOCKS_PER_SEC;
			int temp = cost(graph,candidate);
			if(temp < bestSofar){
				bestSofar = temp;
				best = candidate;
				trace << rt << "," <<temp <<endl;
			}
			if(rt > cutoff)
				break;
		}
		T = 0.95 * T;
	}
	trace.close();
	best = construct(graph,best);

	sol.open(solFName.c_str());
	sol << best.size() << endl;
	for (auto each : best) {
		// cout << each << ",";
		sol << each << ",";
	}
	
	sol.close();
}

/*int main() {
   Graph g = parseGraph("../Data/football.graph");
   ESA(g, "foorball",2,1);
   cout<<"Ended";
   return 1;
 }*/
