
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


bool cmpSize(vector<int> a, vector<int> b) {
	return (a.size() < b.size()); // Larger sizes first
}

vector<int> getNextSol(vector<int> current, Graph &gra, vector<int> &params )
{
	vector<int> copycurrent(current);
	
	int index = rand() % copycurrent.size() ;
	int vertex = copycurrent[index];
	int result = 1;
	std::vector<int>::iterator it;

	vector<int> e = gra.getAdjacent(vertex);
	for(auto ed : e)
	{	it = find(copycurrent.begin(),copycurrent.end(),ed);
		if(it==copycurrent.end())
		{
			result = 0;
			break;
		}
	}
	if(result == 1){
		std::vector<int>::iterator it1 = find(copycurrent.begin(),copycurrent.end(),vertex);
		copycurrent.erase(it1);
		params[1] = gra.adjacencyList[vertex].size();
	}
	else{
		params[1] = -1;
	}
	params[0] = result;
	return copycurrent;

}

vector<int> sortVertices(Graph &gra)
{
	vector<int> vertices = gra.getVertices();
	std::sort(vertices.begin(),vertices.end(),[gra](int &a, int &b){return gra.adjacencyList[a].size() < gra.adjacencyList[b].size();});
	//std::sort(vertices.begin(),vertices.end(),cpmsize());
	return vertices;
}

void removeEdges(vector<Edge> &edges, int temp)
{	vector<Edge> copy(edges);
	for(int i =0;i<edges.size();i++)
	{
		if(edges[i].start == temp || edges[i].end == temp)
		{
			edges.erase(edges.begin()+i);

		}
	}	
}


vector<int> maxDegGreedy(Graph &gra){
	vector<int> output;
	vector<Edge> edges = gra.getEdges();
	vector<int> sortedVertices = sortVertices(gra);
	int i =0;
	int temp;
	while(!edges.empty() && i < sortedVertices.size())
	{
		temp = sortedVertices[i];
		output.push_back(temp);
		removeEdges(edges,temp);
		i++;
	}
	return output;
}

void ESA(Graph graph, string instName, double cutoff, int seed)
{	
	ofstream sol, trace;
	double temp = 10000.0;
    double cooling = 0.9999;
    double thresh = 0.0001;

    srand(seed);

    stringstream fss;
  	fss << instName << "_LS1_" << cutoff;
  	string solFName = fss.str()+".sol";
  	string traceFName = fss.str()+".trace";
  	fss.str(string());

	clock_t start = clock(); 
	clock_t end = clock();
	float rt = (end - start) / (float) CLOCKS_PER_SEC;

	long diff;

	vector<int> current = maxDegGreedy(graph);
	int currCost = current.size();
	vector<int> nextSol;
	trace.open(traceFName.c_str());
	vector<int> params(2,-2);
	while(temp > thresh && rt < cutoff)
	{
		nextSol = getNextSol(current,graph,params);
		if(params[1]!= -1){
			diff = params[1] - currCost;
			if((diff<0) || (currCost > 0 && exp(-abs(diff)/temp > rand()) )){
				current = nextSol;
				currCost = params[1];
			}
		}
		temp *= cooling;
		if(params[0] == 1){
			end = clock();
			float elapsed = (end - start) / (float) CLOCKS_PER_SEC;
			trace << elapsed<< ", " << current.size() << endl;
		}
	}
	//end = clock();
	//float elapsed= (end - start) / (float) CLOCKS_PER_SEC;
	//trace << elapsed<< ", " << current.size() << endl;
	trace.close();

	sol.open(solFName.c_str());
	sol << current.size() << endl;
	if (current.size() > 0) {
    for (int i=0; i<current.size()-1; i++) {
      sol << current[i] << ",";
    }
    sol << current[current.size()-1];
  }
  sol.close();
}

/*int main() {
   Graph g = parseGraph("../Data/jazz.graph");
   ESA(g, "jaz", 2,100);
   cout<<"Ended";
   return 1;
 }*/
