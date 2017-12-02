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



vector<int> vc_approx(Graph G)
{
	vector<int> VC;
	for(int i = 1;i < G.adjacencyList.size() ; i++){
		int v = G.adjacencyList[i][0];
		vector<int> adj = G.getAdjacent(v);
		
		for( int j = 1;j<adj.size();j++){
			
			if(adj[j] < v || G.adjacencyList[adj[j]].size()==0)
				continue;
			G.adjacencyList[v].clear();

			G.adjacencyList[adj[j]].clear();
			
			VC.push_back(v);
			VC.push_back(adj[j]);
			break;
		}
	}
	return VC;
}



int cost(Graph gra, vector<int> candidate)
{
	int count = 0;
	
	for(int i = 1; i<gra.adjacencyList.size();i++){
		
		if(find(candidate.begin(),candidate.end(),gra.adjacencyList[i][0]) == candidate.end()){
			int v = gra.adjacencyList[i][0];
			//cout<<"source"<<v<<endl;
			vector<int> adj = gra.getAdjacent(v);
			for(int j = 1;j<adj.size();j++){
				if(adj[j] > v && find(candidate.begin(),candidate.end(),adj[j]) == candidate.end())
					count++;
			}
		}
	}
	return candidate.size() + count;
}

vector<int> construct(Graph gra, vector<int> candidate)
{
	vector<int> vert = gra.getVertices();
	for(auto each : candidate){
		gra.adjacencyList[each].clear();
	}
	for(int i =1; i<gra.adjacencyList.size();i++)
	{
		if(gra.adjacencyList[i].size() > 0){
			vector<int> adj = gra.getAdjacent(gra.adjacencyList[i][0]);
			for(int j = 1;j < adj.size();j++){
				if(gra.adjacencyList[adj[j]].size() > 0){
					candidate.push_back(i+1);
					break;
				}
			}
			gra.adjacencyList[gra.adjacencyList[i][0]].clear();
		}
	}
	return candidate;
}

void ESA(Graph graph, string instName, double cutoff, int seed)
{
	ofstream sol, trace;
	stringstream fss;
  	fss << instName << "_LS_" << cutoff;
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
				//cout<<"iter"<<*iter<<endl;
				neighbor.erase(iter);
				int delta = cost(graph,neighbor) - cost(graph,candidate);
				if(delta<= 0)
					candidate = neighbor;
				else{
					double P = exp(-delta * (1+graph.adjacencyList[selected].size() / float(graph.sizeV)) / T);
					double R = float(rand()) / RAND_MAX;

					if(R < P){
						candidate = neighbor;
					}
				}
			}
			else{
				neighbor.push_back(selected);
				int delta = cost(graph,neighbor) - cost(graph,candidate);
				if(delta <= 0)
					candidate = neighbor;
				else{
					double P = exp(-delta * (1 - graph.adjacencyList[selected].size()/ float(graph.sizeV))/T);
					double R = float(rand()) / RAND_MAX;

					if(R < P){
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
	if(best.size() > 0){
		for(int i =0; i<best.size()-1;i++)
			sol << best[i] <<",";
	}
	sol << best[best.size()-1];
	
	sol.close();
}

/*int main() {
   Graph g = parseGraph("../Data/star2.graph");
   ESA(g, "star",100,10);
   cout<<"Ended";
   return 1;
 }*/