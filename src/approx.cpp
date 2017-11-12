#include "approx.hpp"

using namespace std;

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
    stack<int> S;
    S.push(1);
    while(!S.empty()){
        int curV = S.top();
        S.pop();
        if(!visited[curV]){
            visited[curV]=true;
            vector <int> incidentVs = graph.getAdjacent(curV);
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
    double real_time = get_time() - start_time;
    if(graph.isVC(solution)){
        cout<<"Find a VC!"<<endl;
    }

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
    trace_file << real_time << ","; 
    trace_file << solution.size() << endl;
    trace_file.close();


    return 0;
}
