#include "approx.hpp"
#include <stack>

using namespace std;

int approx(Graph &graph){
    vector <int> solution;
    solution.reserve(graph.sizeV);
    vector <bool> visited(graph.sizeV+1);
    stack<int> S;
    S.push(1);
    while(!S.empty()){
        int curV = S.top();
        S.pop();
        vector <int> incidentVs = graph.getAdjacent(curV);
        int childsNum = 0;
        for(const auto it: incidentVs){
            if(!visited[it]){
                S.push(it);
                visited[it] = true;
                ++childsNum;
            }
        }
        if(childsNum>1){
            solution.push_back(curV);
        }
    }
    cout<<endl;
    cout<< solution.size()<<endl;
    for(const auto it: solution){
        cout<< it <<",";
    }
    cout<<endl;
    return 0;
}
