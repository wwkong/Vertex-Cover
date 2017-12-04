#include "ls2.hpp"

using namespace std;

int max_k = 50;

int construct_vc(Graph &graph, vector<int> &loss, vector<int> &C)
{
    int num_vertices = graph.sizeV;

    C.reserve(num_vertices);
    vector<int> tmp_C;
    tmp_C.reserve(num_vertices);

    /* Add the endpoint of uncovered edges with higher degree into VC */
    const vector< vector<int> > adjacency_list = graph.adjacencyList;
    for (int i = 1; i <= num_vertices; ++i) {
        vector<int>::iterator v_it = find(tmp_C.begin(), tmp_C.end(), i);
        if (v_it == tmp_C.end()) {
            for (int j = 1; j < adjacency_list[i].size(); ++j) {
                int cur_vertex = adjacency_list[i][j];
                //cout << "j: " << j << ", cur_v: " << cur_vertex << endl;
                vector<int>::iterator cur_it = find(tmp_C.begin(), tmp_C.end(), cur_vertex);
                if (cur_it == tmp_C.end()) {
                    if (adjacency_list[i].size() >= adjacency_list[cur_vertex].size()) {
                       tmp_C.push_back(i);
                       //cout << "j: " << j << ", i:" << i << ", degree: " << adjacency_list[i].size() << ", cur_v: " << cur_vertex << ", degree: " << adjacency_list[cur_vertex].size() << ", push: " << i <<endl;
                       break;
                    } else {
                        tmp_C.push_back(cur_vertex);
                        //cout << "j: " << j << ", i:" << i << ", degree: " << adjacency_list[i].size() << ", cur_v: " << cur_vertex << ", degree: " << adjacency_list[cur_vertex].size() << ", push1: " << cur_vertex <<endl;
                    }
                }
            }
        } 
    }

    /* Calculate loss of vertices in VC  */
    for (int i = 0; i < tmp_C.size(); ++i) {
        for (int j = 1; j < adjacency_list[tmp_C[i]].size(); ++j) {
            int cur_vertex = adjacency_list[tmp_C[i]][j];
            vector<int>::iterator cur_it = find(tmp_C.begin(), tmp_C.end(), cur_vertex);
            if (cur_it == tmp_C.end()) {
                ++loss[tmp_C[i]];
            } 
        }
    }
    
    /* Remove redundant vertices */
    for (int i = 0; i < tmp_C.size(); ++i) {
        if (loss[tmp_C[i]] != 0) {
            C.push_back(tmp_C[i]);
        } else {
            for (int j = 1; j < adjacency_list[tmp_C[i]].size(); ++j) {
                int cur_vertex = adjacency_list[tmp_C[i]][j];
                vector<int>::iterator cur_it = find(tmp_C.begin(), tmp_C.end(), cur_vertex);
                if (cur_it != tmp_C.end()) {
                    ++loss[cur_vertex];
                } 
            }
        }
    }

    return 0;
}

int find_min_loss_vertex(const vector<int> &loss, const vector<int> &C)
{
    int min = 0; 
    int min_loss = loss[C[min]];

    for (int i = 1; i < C.size(); ++i) {
        if (loss[C[i]] < min_loss) {
            min = i;
            min_loss = loss[C[i]];
        }
    }
    return min;
}

int choose_rm_vertex(vector<int> &C, vector<int> &loss, vector<uint64_t> &time_steps) 
{
    int best_vertex_index =rand() % C.size(); 
    for (int i = 0; i < max_k; ++i) {
        int cur_vertex_index =rand() % C.size();
        if (loss[C[cur_vertex_index]] < loss[C[best_vertex_index]]) {
            best_vertex_index = cur_vertex_index;
        } else if (loss[C[cur_vertex_index]] == loss[C[best_vertex_index]] && time_steps[C[cur_vertex_index]] < time_steps[C[best_vertex_index]]) {
            best_vertex_index = cur_vertex_index;
        }
    }
    return best_vertex_index;
}

int update_gain_loss_before_remove(const Graph &graph, vector<int> &gain, vector<int> &loss, const vector<int> &C, const vector<bool> &isInC, vector< pair<int,int> > &uncovered_edges, const int vertex)
{
    loss[vertex] = 0;
    gain[vertex] = 0;
    vector<int> adjacency_list = graph.adjacencyList[vertex];
    uncovered_edges.reserve(uncovered_edges.size() + adjacency_list.size());
    for (int j = 1; j < adjacency_list.size(); ++j) {
        int cur_vertex = adjacency_list[j];
        if (!isInC[cur_vertex]) {
            pair<int,int> tmp_pair(vertex, cur_vertex);
            uncovered_edges.push_back(tmp_pair);
            ++gain[vertex];
            ++gain[cur_vertex];
        } else {
            ++loss[cur_vertex];
        }
    }
    return 0;
}

int update_gain_loss_after_add(const Graph &graph, vector<int> &gain, vector<int> &loss, const vector<int> &C, const vector<bool> &isInC,  vector< pair<int,int> > &uncovered_edges, const int vertex)
{
    gain[vertex] = 0;
    loss[vertex] = 0;
    const vector<int> adjacency_list = graph.adjacencyList[vertex];
    vector< pair<int,int> > tmp_uncovered_edges;
    tmp_uncovered_edges.reserve(uncovered_edges.size());
    for (int i = 0; i < uncovered_edges.size(); ++i) {
        if(uncovered_edges[i].first != vertex && uncovered_edges[i].second != vertex) {
            tmp_uncovered_edges.push_back(uncovered_edges[i]);
        }
    }
    swap(uncovered_edges, tmp_uncovered_edges);
            
    for (int j = 1; j < adjacency_list.size(); ++j) {
        int cur_vertex = adjacency_list[j];
        if (!isInC[cur_vertex]) {
            --gain[cur_vertex];
            if (gain[cur_vertex] < 0) {
                gain[cur_vertex] = 0;
            }
            ++loss[vertex];
        } else {
            --loss[cur_vertex];
            if (loss[cur_vertex] < 0) {
                loss[cur_vertex] = 0;
            }
        }
    }
    return 0;
}



int local_search2(Graph &graph, string &instName, double cutoff, int seed)
{

    int num_vertices = graph.sizeV;
    srand (seed);

    vector<int> loss(num_vertices + 1, 0);
    vector<int> C, VC;
    construct_vc(graph, loss, C);
    VC = C;
    max_k = 50 < num_vertices ? 50 : num_vertices;
//#ifdef DEBUG
    cout << "Construct VC successfully! The size of VC is " << C.size() << "." <<endl;
    if (graph.isVC(C)) {
        cout << "Construct a VC!" << endl;
    }
//#endif

    ostringstream string_stream;
    string_stream << instName << "_LS2_" << cutoff << "_" << seed << ".trace";
    ofstream trace_file;
    string trace_file_name = string_stream.str();
    trace_file.open(trace_file_name);
    trace_file << fixed << setprecision(2) << .0 << ", "; 
    trace_file << VC.size() << endl;


    vector<int> gain(num_vertices + 1, 0);
    vector< pair<int,int> > uncovered_edges;
    vector<bool> isInC(num_vertices + 1, false);
    vector<uint64_t> time_steps(num_vertices + 1, 0);
    uint64_t step = 0;
    for (int i = 0; i < C.size(); ++i) {
        isInC[C[i]] = true;
    }

    double elapsed_time = 0.0;
    while (elapsed_time < cutoff) {
        double start_time = get_time();
        if (uncovered_edges.size() == 0) { //graph.isVC(C)) {
            if (C.size() < VC.size()) {
                VC = C;
                //cout << "Find a VC! The size of VC is " << VC.size() << ", time: " << elapsed_time << endl;  
                trace_file << fixed << setprecision(2) << elapsed_time << ", "; 
                trace_file << VC.size() << endl;
            }
            int min_loss_vertex_index = find_min_loss_vertex(loss, C); 
            int min_loss_vertex = C[min_loss_vertex_index];
            //cout << "min_loss_vertex: " << min_loss_vertex << endl;
            vector<int>::iterator min_loss_vertex_it = C.begin() + min_loss_vertex_index;
            //assert(min_loss_vertex_it != C.end());
            /* Update loss and gain */
            update_gain_loss_before_remove(graph, gain, loss, C, isInC,  uncovered_edges, min_loss_vertex);
            isInC[min_loss_vertex] = false;;
            C.erase(min_loss_vertex_it);
            time_steps[min_loss_vertex] = step;
            elapsed_time += get_time() - start_time;
            continue;
        }
        int best_vertex_index = choose_rm_vertex(C, loss, time_steps);
        int best_vertex = C[best_vertex_index];
        //cout << "best_vertex: " << best_vertex << endl;
        vector<int>::iterator best_vertex_it = C.begin() + best_vertex_index;
        /* Update loss and gain */
        update_gain_loss_before_remove(graph, gain, loss, C, isInC,  uncovered_edges, best_vertex);
        //assert(best_vertex_it != C.end());
        isInC[best_vertex] = false;
        time_steps[best_vertex] = step;
        C.erase(best_vertex_it);
        //assert(uncovered_edges.size()>0);

        pair<int,int> random_uncovered_edge = uncovered_edges[rand() % uncovered_edges.size()];
        int v1 = random_uncovered_edge.first;
        int v2 = random_uncovered_edge.second;
        int new_vertex = v2;
        if (gain[v1] > gain[v2]) {
            new_vertex = v1;
        } else if (gain[v1] == gain[v2] && time_steps[v1] < time_steps[v2]) {
            new_vertex = v1;
        } 

        //cout << "new_vertex: " << new_vertex << endl; 
        //cout << endl;
        C.push_back(new_vertex);
        time_steps[new_vertex] = step;
        isInC[new_vertex] = true;
        /* Update loss and gain */
        update_gain_loss_after_add(graph, gain, loss, C, isInC,  uncovered_edges,  new_vertex);
        //if (uncovered_edges.size()>1)
        //    printf("number of uncovered edges: %d.\n", uncovered_edges.size());
        ++step;
        elapsed_time += get_time() - start_time;
    }

    //if (graph.isVC(VC)) {
    //    cout << "Find a VC! The size of VC is " << VC.size() << ", time: " << elapsed_time << endl;  
    //}

    /* Output the results to files */
    ofstream solution_file;
    string_stream.clear();
    string_stream.str(string());
    string_stream << instName << "_LS2_" << cutoff << "_" << seed << ".sol";
    string solution_file_name;
    solution_file_name = string_stream.str();
    solution_file.open(solution_file_name);
    solution_file << to_string(VC.size()) << endl;
    for(const auto it: VC){
        solution_file << it;
        if(it != VC.back())
            solution_file << ", ";
    }
    solution_file << endl;
    solution_file.close();
    trace_file.close();


    return 0;
}
