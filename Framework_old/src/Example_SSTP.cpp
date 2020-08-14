// instancias: https://www.ic.unicamp.br/~fusberti/problems/sstp/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <ostream>
#include <string>

#include "auxi/mylemonutils.h"
#include "SP_instance.h"
#include "SP_solution.h"
#include <lemon/kruskal.h>
#include <lemon/bfs.h>
#include "auxi/dijkstra_Steiner.h"
#include "EF_generic_solver.h"
#include "EF_local_search_solver.h"
#include "EF_manager.h"
#include "EF_second_stage_solver.h"
#include "auxi/timer.hpp"

using namespace std;
using namespace lemon;

void print_usage(char * exe_name, ostream& print_location);

Graph g;  // graph declaration
NodeName node_name(g); // name of graph nodes
Graph::NodeMap<int> node_id(g);
Graph::NodeMap<std::vector <bool> > is_terminal_node(g);// is_terminal[v][s]
std::vector<Graph::NodeMap <bool> * > is_terminal_scenario; // is_terminal_scenario[s][v]
std::vector<int> number_terminals_in_scenario;

//Vector of graphs containing only terminals per scenario
std::vector<Graph *> tg;
//g_node_from_tg_node[s][u] = v means that u in tg[s] corresponds to v in g
std::vector<Graph::NodeMap<Node> * > g_node_from_tg_node;
int number_of_nodes;

int get_node_id(Node n){return node_id[n];};
int get_number_of_nodes(){return number_of_nodes;};
int get_number_of_terminals_in_scenario(int s){return number_terminals_in_scenario[s];};
Node get_g_node_from_tg_node(int s, Node u){return (*g_node_from_tg_node[s])[u];};
Graph::NodeMap<bool>   * get_nodemap_is_terminal_scenario(int s){return is_terminal_scenario[s];};
Graph * get_terminal_graph(int s){return tg[s];};


/*Read instances in DIMACS format*/
void readInstanceDIMACS(EF::SP_instance &problem, char * instance_name){
	//Open the file
	std::ifstream myfile(instance_name);
	if (!myfile) {std::cerr << "Failed to open file " << instance_name << std::endl; exit(-1);}

	std::string line;
	//Read instance
	myfile >> line;
	while (line.compare("Graph") != 0) myfile >> line;
	myfile >> line;
	myfile.ignore(256000, ' ');
	myfile >> number_of_nodes;
	myfile >> line;
	myfile.ignore(256000, ' ');
	myfile >> problem.number_of_resources;
	myfile >> line;
	myfile.ignore(256000, ' ');
	myfile >> problem.number_of_scenarios;
	myfile >> line;
	myfile.ignore(256000, ' ');
	int root_id;
	myfile >> root_id;  root_id--; //the instances are 1-based, in this code we use 0-based vectors

	problem.id_final_to_reading_position.resize(problem.get_number_of_resources());  //To correct edges order
	problem.cost_first_stage.resize(problem.number_of_resources);
	problem.cost_in_scenario.resize(problem.number_of_resources);
	//Set of nodes
	Node V[number_of_nodes];

	Node V_aux[number_of_nodes]; //To correct edges order
	Graph g_aux; //To correct edges order
	int cost_first_stage_aux[problem.number_of_resources]; //To correct edges order
	int id_reading_to_final_position[problem.number_of_resources]; //To correct edges order

	for (int i=0;i<number_of_nodes;i++) {
		V[i] = g.addNode();    // create a node
		V_aux[i] = g_aux.addNode(); //To correct edges order
		node_name[V[i]] = IntToString(i);  // names it as "i"
		node_id[V[i]] = i;
	}

	for (int i = 0; i < problem.number_of_resources; i++){
		int head;
		int tail;
		double temp_d;
		myfile.ignore(256000, ' ');
		myfile >> head; head--; //the instances are 1-based, in this code we use 0-based vectors
		myfile >> tail; tail--; //the instances are 1-based, in this code we use 0-based vectors
		myfile >> temp_d;
		g_aux.addEdge(V_aux[head],V_aux[tail]); //To correct edges order

		cost_first_stage_aux[i] = (int) temp_d; //To correct edges order

		(problem.cost_in_scenario[i]).resize(problem.number_of_scenarios);
	}

	//To correct edges order
	for (Graph::EdgeIt e_aux(g_aux); e_aux != INVALID; ++e_aux){
		Node u_aux = g_aux.u(e_aux);
		Node v_aux = g_aux.v(e_aux);
		int u_id = g_aux.id(u_aux); //u from g has same id than u_aux from g_aux
		int v_id = g_aux.id(v_aux);
		Edge e = g.addEdge(V[u_id], V[v_id]);
		int id_final = g.id(e); //id_final indica a ordem de leitura do edgeMap
		int id_aux = g_aux.id(e_aux); //id_aux indica a ordem de leitura da entrada
		id_reading_to_final_position[id_aux] = id_final;
		problem.id_final_to_reading_position[id_final] = id_aux;
		(problem.cost_first_stage)[id_final] = cost_first_stage_aux[id_aux];
	}

	problem.probabilities_scenario.resize(problem.number_of_scenarios);
	number_terminals_in_scenario.resize(problem.number_of_scenarios);
	while (line.compare("StochasticProbabilities") != 0) myfile >> line;
	myfile.ignore(256000, ' ');
	for (int s = 0; s < problem.number_of_scenarios; s++){
		myfile >> problem.probabilities_scenario[s];
		number_terminals_in_scenario[s] = 0;
	}

	while (line.compare("StochasticWeights") != 0) myfile >> line;
	for (int i = 0; i < problem.number_of_resources; i++){
		myfile >> line;
		for (int s = 0; s < problem.number_of_scenarios; s++){
			myfile >> problem.cost_in_scenario[id_reading_to_final_position[i]][s];
		}
	}

	while (line.compare("StochasticTerminals") != 0) {
		myfile >> line;
	}

	for (int n = 0; n < number_of_nodes; n++){
		myfile.ignore(256000, ' '); //ignore ST
		myfile.ignore(256000, ' '); //ignore terminal id
		is_terminal_node[V[n]].resize(problem.number_of_scenarios);
		for (int s = 0; s < problem.number_of_scenarios; s++){
			int temp;
			myfile >> temp;
			is_terminal_node[V[n]][s] = (bool) temp;
			if(is_terminal_node[V[n]][s]  == true) {
				number_terminals_in_scenario[s]++;
			}
		}
		myfile.ignore(256000, 'T');
	}
}

// TODO: MEXER NISSO
void read_instance(EF::SP_instance &problem, char * instance_name, char instance_type){
	if (instance_type == 'D') readInstanceDIMACS(problem, instance_name);
	else {cerr << "Instance type " << instance_type << " invalid." << endl; exit(-1);}

	//this vector is used for optimization purposes
	is_terminal_scenario.resize(problem.get_number_of_scenarios());
	for(int s = 0; s < problem.get_number_of_scenarios() ; s++){
		is_terminal_scenario[s] = new Graph::NodeMap <bool> (g);
		for (Graph::NodeIt n(g); n!=INVALID; ++n){
			(*is_terminal_scenario[s])[n] = is_terminal_node[n][s];
		}
	}

	//this vector is used for optimization purposes
	problem.resource_cost_in_scenario.resize(problem.get_number_of_scenarios());
	for(int s = 0; s < problem.get_number_of_scenarios() ; s++){
		problem.resource_cost_in_scenario[s] = new std::vector<double>(problem.get_number_of_resources());
		for (int i = 0; i< problem.get_number_of_resources();i++){
			(*problem.resource_cost_in_scenario[s])[i] = problem.cost_in_scenario[i][s];
		}
	}

	tg.resize(problem.get_number_of_scenarios());
	g_node_from_tg_node.resize(problem.get_number_of_scenarios());
	for(int s = 0; s < problem.get_number_of_scenarios(); s++){
		tg[s] = new Graph();
		g_node_from_tg_node[s] = new Graph::NodeMap <Node> (*(tg[s]));
		//insert in the new graph tg only terminals
		for (Graph::NodeIt n(g); n!=INVALID; ++n) {
			if((*is_terminal_scenario[s])[n]){
				Node new_node = (*tg[s]).addNode();
				(*g_node_from_tg_node[s])[new_node] = n;
			}
		}
		for (Graph::NodeIt u(*tg[s]); u!=INVALID; ++u) {
			NodeIt v = u;
			for (++v; v!=INVALID; ++v) {
				(*tg[s]).addEdge(u, v);
			}
		}
	}
}

// TODO: mexer nisso
double decode_Ap(EF::SP_instance &problem, int s, std::vector <double>& costs_vector,
		std::vector <bool>& solution){
	//Original Graph
	Graph * g_pntr = &g;
	Graph * tg = get_terminal_graph(s);
	//Terminals graph
	Graph::EdgeMap<double> tg_costs(*tg);
	Graph::EdgeMap<bool> tree_map(*tg);
	Graph::EdgeMap<double> costs(*g_pntr);

	//clear solution
	for (int i = 0; i < problem.get_number_of_resources(); i++) solution[i] = false;

	for (int i = 0; i < problem.get_number_of_resources(); i++){
		Edge e = (*g_pntr).edgeFromId(i);
		costs[e] = costs_vector[i];
	}

	//Vector to store the corresponding edges of the original graph for each edge in terminal graph
	Graph::EdgeMap< std::vector <Edge> > original_edges_map(*tg);

	Dijkstra_Steiner<Graph, Graph::EdgeMap <double> > dijkstra_test(*g_pntr, costs);

	//create edges on tg corresponding to paths on g
	//for each pair (u, v) of tg terminals
	//corresponding to (original, original_target) in g
	std::vector<Node> removed_terminals;
	Graph::NodeMap<bool> * is_terminal_scenario = get_nodemap_is_terminal_scenario(s);
	int number_of_terminals_in_scenario = get_number_of_terminals_in_scenario(s);

	for (Graph::NodeIt u(*tg); u!=INVALID; ++u) {
		Node source = get_g_node_from_tg_node(s, u);//source is "u" in g

		//compute dijkstra between original and all other nodes
		dijkstra_test.run_Steiner(source, is_terminal_scenario, number_of_terminals_in_scenario);
		//for each node (greater)
		NodeIt v = u;
		for (++v; v!=INVALID; ++v) {
			Node target =  get_g_node_from_tg_node(s, v); //target is "v" in g
			Edge e = findEdge(*tg, u, v);
			tg_costs[e] = dijkstra_test.dist(target); //dist from source to target
			//maps each edge from tg in g edges
			for (Node x=target;x != source; x=dijkstra_test.predNode(x)) {
				Edge original_edge = findEdge(*g_pntr, x, dijkstra_test.predNode(x));
				original_edges_map[e].push_back(original_edge);
			}
		}
		// Removing visited terminals so upcoming dijkstras may finish faster
		removed_terminals.push_back(source);
		(*is_terminal_scenario)[source] = false;
		number_of_terminals_in_scenario--;
	}//end of for

	for(unsigned int i = 0; i < removed_terminals.size(); i++){
		(*is_terminal_scenario)[removed_terminals[i]] = true;
	}
	//Runs Kruskal on tg
	kruskal(*tg, tg_costs, tree_map);

	//For each edge used on tg minimum spanning tree insert (1 time) in the solution
	//the original edges of g

	for (Graph::EdgeIt e(*tg); e != INVALID; ++e){
		if(tree_map[e] == true){
			for (unsigned int i = 0; i < original_edges_map[e].size() ; i++){
				Edge original = original_edges_map[e][i];
				int id = (*g_pntr).id(original);
				if(solution[id] == false){
					solution[id] = true;
				}
			}
		}
	}

	return 0;
}


int main(int argc, char ** argv) {
	Timer timer;
	timer.restart();

	if (argc < 2) {print_usage(argv[0], std::cout); exit(0); }

	//Set default parameters
	char instance_type = 'D';
	int verbose = 1;
	double alpha = 0.4;
	int time_limit = -1; //time limit in seconds, -1 means unlimited
	int MAX_ITERATIONS_SINCE_LAST_IMPROVEMENT = 2;
	int MAX_ITERATIONS_SINCE_BEST_SOLUTION = 3;
	int NUMBER_OF_GENERATIONS = 25;
	int POPULATION_SIZE = 25;
	int TAIL_GENERATIONS_FACTOR = 3;
	double MINIMUM_IMPROVEMENT_RATIO = 0.001;

	//Read parameters
	//Read the instance name
	char * intance_file_name = argv[1];
	std::string instance_file_string ("");
	instance_file_string.append("");
	instance_file_string.append(argv[1]);

	if (argc >= 3) time_limit = atoi(argv[2]);
	if (argc >= 4) verbose = atoi(argv[3]);

	//Instance class carries all important information
	if (verbose >= 1) std::cout << "Reading instance " << instance_file_string << " of type " << instance_type << std::endl;
	EF::SP_instance problem;
	problem.decode_Ap = &decode_Ap;

	read_instance(problem, intance_file_name, instance_type);

	if (verbose >= 1) std::cout << "Finished instance reading" << std::endl;

	//Prints instance data
	if (verbose >= 1){
		std::cout << "Number of vertices = " << get_number_of_nodes() << std::endl;
		std::cout << "Number of edges (resources) = " << problem.get_number_of_resources() << std::endl;
		std::cout << "Number of scenarios = " << problem.get_number_of_scenarios() << std::endl;

		int total_number_of_terminals = 0;
		for (int s = 0; s < problem.get_number_of_scenarios(); s++){
			total_number_of_terminals += get_number_of_terminals_in_scenario(s);
		}
		std::cout << "Average number of terminals per scenario = " << total_number_of_terminals/problem.get_number_of_scenarios() << std::endl;

		double largest_inflation = 0;
		double average_inflation = 0;

		std::vector<double> * costs_in_first_stage = problem.get_vector_costs_in_first_stage();

		std::cout << "Probabilities of Scenarios: ";
		for(int s = 0; s < problem.get_number_of_scenarios(); s++){
			std::cout << "(" << s << ") " << problem.get_probabilities_scenario(s) << " | ";
		}
		std::cout << endl;

		for(int s = 0; s < problem.get_number_of_scenarios(); s++){
			double cumulative_inflation_in_scenario = 0;
			if (verbose >= 2) std::cout << "Scenario " << s;
			std::vector<double> * costs_in_scenario = problem.get_vector_costs_in_scenario(s);
			for (int i = 0; i < problem.get_number_of_resources(); i++){
				if (verbose >= 2) std::cout << " " << (*costs_in_scenario)[i];
				double edge_inflation = ((*costs_in_scenario)[i] / (*costs_in_first_stage)[i]) - 1;
				cumulative_inflation_in_scenario += edge_inflation;
				if (edge_inflation > largest_inflation) {
					largest_inflation = edge_inflation;
				}
			}
			if (verbose >= 2) std::cout << endl;
			double average_inflation_in_scenario = cumulative_inflation_in_scenario/problem.get_number_of_resources();
			average_inflation += average_inflation_in_scenario * problem.get_probabilities_scenario(s);
		}
		std::cout << "Average weighted inflation = " << average_inflation << std::endl;
		std::cout << "Largest inflation = " << largest_inflation << std::endl;
		std::cout << "Alpha = " << alpha << std::endl;
		std::cout << "MAX_ITERATIONS_SINCE_LAST_IMPROVEMENT " << MAX_ITERATIONS_SINCE_LAST_IMPROVEMENT  << std::endl;
		std::cout << "MAX_ITERATIONS_SINCE_BEST_SOLUTION " << MAX_ITERATIONS_SINCE_BEST_SOLUTION << std::endl;
		std::cout << "NUMBER_OF_GENERATIONS " << NUMBER_OF_GENERATIONS << std::endl;
		std::cout << "POPULATION_SIZE " << POPULATION_SIZE << std::endl;
		std::cout << "TAIL_GENERATIONS_FACTOR " << TAIL_GENERATIONS_FACTOR << std::endl;
	}

	if (verbose >= 1) cout << "Creating solution instance." << endl;
	EF::SP_solution solution_final(problem);

	if (verbose >= 1) cout << "Starting Evolutive Framework." << endl;
	EF::EF_manager ef_manager(problem, verbose,
			time_limit, alpha, MAX_ITERATIONS_SINCE_LAST_IMPROVEMENT,
			MAX_ITERATIONS_SINCE_BEST_SOLUTION, NUMBER_OF_GENERATIONS,
			POPULATION_SIZE, TAIL_GENERATIONS_FACTOR, MINIMUM_IMPROVEMENT_RATIO);

	ef_manager.solve(&decode_Ap, solution_final);

	std::cout << std::endl;
	std::cout << "Final time = " << timer.elapsed() << std::endl;
	std::cout << "Final solution cost = " << ef_manager.get_solution_value() << std::endl;


	return 0;
}

void print_usage(char * exe_name, ostream& print_location) {
	print_location << "Usage: " << exe_name
			<< " <instance_file> <time_limit (-1 if no time limit)> <verbose>"
			<< endl;
}


