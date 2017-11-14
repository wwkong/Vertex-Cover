#include "approx.hpp"

using namespace std;

/* my timer */
double get_time(){
    struct timeval time;  
    double cur_time;  
    gettimeofday(&time,NULL);  
    cur_time=1000000*(time.tv_sec)+time.tv_usec;  
    cur_time/=1000000;
    return cur_time;
}

int approx(Graph &graph, string &instName, double cutoff){
    double start_time = get_time();

    vector <int> solution;
    solution.reserve(graph.sizeV);
    vector <bool> visited(graph.sizeV+1);
    int visited_vertex_num = 0;
    int start_vertex = 1;
    stack<int> S;

    /* Find DFS forest and record leaf nodes for each DFS tree */
    while(visited_vertex_num < graph.sizeV){
        for(int i=2; i< graph.sizeV;++i){
            if(!visited[i]){
                int adj_vertex_num = graph.adjacencyList[i].size(); 
                if(adj_vertex_num!=0){
                    start_vertex = i;
                    break;
                }
            }
        }
        S.push(start_vertex);
        while(!S.empty()){
            int curV = S.top();
            S.pop();
            if(!visited[curV]){
                visited[curV]=true;
                ++visited_vertex_num;
                vector <int> incidentVs = graph.adjacencyList[curV];
                int childsNum = 0;
                for(const auto it: incidentVs){
                    if(!visited[it]){
                        S.push(it);
                        ++childsNum;
                    }
                }
                if(childsNum>0){
                    solution.push_back(curV);
                }
            }
        }
    }

    /* add the start vertex to the solution if it is not in the solution*/
    if(solution.front()!=start_vertex){
        solution.push_back(start_vertex);
    }

    double real_time = get_time() - start_time;
    if(graph.isVC(solution)){
        cout<<"Find a VC!"<<endl;
    }

    /* Output the results to files */
    ofstream solution_file;
    ostringstream string_stream;
    string_stream << instName << "_Approx_" << cutoff << ".sol";
    string solution_file_name;
    solution_file_name = string_stream.str();
    solution_file.open(solution_file_name);
    solution_file << to_string(solution.size()) << endl;
    for(const auto it: solution){
        solution_file << it;
        if(it != solution.back())
            solution_file << ",";
    }
    solution_file << endl;
    solution_file.close();
    
    string_stream.clear();
    string_stream.str(string());
    string_stream << instName << "_Approx_" << cutoff <<".trace";
    ofstream trace_file;
    string trace_file_name = string_stream.str();
    trace_file.open(trace_file_name);
    trace_file << fixed << setprecision(2) << real_time << ","; 
    trace_file << solution.size() << endl;
    trace_file.close();

    return 0;
}
