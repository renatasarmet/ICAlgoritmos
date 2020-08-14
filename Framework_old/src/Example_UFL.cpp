// EXAMPLE TSP

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <ostream>
#include <string>
#include <math.h>

#include "auxi/mylemonutils.h"
#include "SP_instance.h"
#include "SP_solution.h"
#include <lemon/kruskal.h>
#include <lemon/bfs.h>
#include <lemon/matching.h>
#include "auxi/dijkstra_Steiner.h"
#include "EF_generic_solver.h"
#include "EF_local_search_solver.h"
#include "EF_manager.h"
#include "EF_second_stage_solver.h"
#include "auxi/timer.hpp"

// #define LOCAL_SEARCH 0

using namespace std;
using namespace lemon;

// int n_vertices;
// vector < vector<int> > indice_aresta;

// class Pilha{
//     int* vetor;
//     int topo;
//     int capacidade;
// public:
//     Pilha(int);
//     ~Pilha();
//     void push(int);
//     int pop();
//     bool vazia();
// };

// Pilha::Pilha(int sz)
// {
//     capacidade = sz;
//     vetor = new int[sz];
//     topo = -1;
// }

// Pilha::~Pilha()
// {
//     delete[] vetor;
// }

// void Pilha::push(int elem)
// {
//     topo++;
//     vetor[topo] = elem;
// }

// int Pilha::pop()
// {
//     int elem;
//     elem = vetor[topo];
//     topo--;
    
//     return elem;
// }

// bool Pilha::vazia()
// {
//     return topo == -1;
// }

void print_usage(char * exe_name, ostream& print_location);
// void ls_2opt(int[], int[],int, int, int);

void read_instance(EF::SP_instance &problem, char * instance_name, char instance_type){    
    ifstream my_file(instance_name);
    if(!my_file.is_open())
    {
        cout << "Erro ao ler arquivo" << endl;
        return;
    }
    my_file >> problem.number_of_scenarios;
    my_file >> n_vertices;
    problem.number_of_resources = (pow(n_vertices,2) - n_vertices)/2;
    problem.probabilities_scenario.resize(problem.number_of_scenarios);
    problem.resource_cost_in_scenario.resize(problem.number_of_scenarios);
    problem.cost_first_stage.resize(problem.number_of_resources);
    problem.cost_in_scenario.resize(problem.number_of_resources);
    indice_aresta.resize(n_vertices);

    problem.id_final_to_reading_position.resize(problem.number_of_resources);
    for(int i = 0; i < problem.number_of_resources; i++) problem.id_final_to_reading_position[i] = i;

    for(int i = 0; i < n_vertices; i++) indice_aresta[i].resize(n_vertices);

    for(int i = 0; i < problem.number_of_scenarios; i++) my_file >> problem.probabilities_scenario[i];
    
    int e = 0;
    for(int v = 0; v < n_vertices; v++)
    {
        for(int u = 0; u < n_vertices; u++)
        {
            double var;
            char lixo;

            if(u != n_vertices - 1){my_file >> var >> lixo;}
            else{my_file >> var;}
   
            if(u > v)
            {
                indice_aresta[u][v] = e;
                indice_aresta[v][u] = e;
                problem.cost_first_stage[e] = var;
                e++;
            }
        }    
    }

    int s = 0;

    for(int i = 0; i < problem.number_of_scenarios; i++)
    {
        problem.resource_cost_in_scenario[i] = new vector<double>(problem.number_of_resources);
        e = 0;
        for(int v = 0; v < n_vertices; v++)
        {
            for(int u = 0; u < n_vertices; u++)
            {
                double var;
                char lixo;

                if(u != n_vertices - 1){my_file >> var >> lixo;}
                else{my_file >> var;}

                if(u > v)
                {
                    (*problem.resource_cost_in_scenario[i])[e] = var;
                    e++;
                }
            }    
        }
    }
   
    for(int i = 0; i < problem.number_of_resources; i++){problem.cost_in_scenario[i].resize(problem.number_of_scenarios);}
   
    for(int i = 0; i < problem.number_of_scenarios; i++){
        for(int j = 0; j < problem.number_of_resources; j++){
            problem.cost_in_scenario[j][i] = (*problem.resource_cost_in_scenario[i])[j];
        }
    }
    /*
    cout << "N_scenarios: " << problem.number_of_scenarios << endl;
    cout << "N_vertices: " << n_vertices << endl;  
    cout << "Probabilidades: " << endl;  
    for(int i = 0; i < problem.number_of_scenarios; i++) cout << problem.probabilities_scenario[i] << " ";
    cout << endl;   
    cout << "Arestas primeiro estagio: " << endl;   
    for(int i = 0; i < problem.number_of_resources; i++) cout << problem.cost_first_stage[i] << " ";  
    cout << endl;
    for(int i = 0; i < problem.number_of_scenarios; i++){
        cout << "Arestas para o cenário: " << i + 1 << endl;
        for(int j = 0; j < problem.number_of_resources; j++){cout << (*problem.resource_cost_in_scenario[i])[j] << " ";}
        cout << endl;
    }*/
}

double decode_Ap(EF::SP_instance &problem, int s, std::vector <double>& costs_vector,
	std::vector <bool>& solution){
	ListGraph g, f;

    // costs_vector é só custo de instalação
    // O meu instance vai ser uma coisa global
    // O meu solution inicializa de novo aqui toda vez pra passar pro greedy
    // pegar os custos do costs_vector e mudar os valores da minha instancia
    // chamar o greedy
    // passar a minha solution para o solution do parametro, 
            // mas com os custos originais de problem do parametro
            // verificando se ela ja foi aberta no primeiro estagio ou nao 
            // (ainda nao tem essa info aqui, mas vai vir por parametro)
            // retorna um double do custo


	//clear solution
	for (int i = 0; i < problem.number_of_resources; i++) solution[i] = false;

    for(int i = 0; i < n_vertices; i++)
    {
        g.addNode();
        f.addNode();
    }

    for(int i = 0; i < n_vertices; i++)
    {
        for(int j = 0; j < n_vertices; j++)
        {
            if(j > i) {
                g.addEdge(g.nodeFromId(i), g.nodeFromId(j));
                f.addEdge(f.nodeFromId(i), f.nodeFromId(j));
            }
        }
    }
    
    ListGraph::EdgeMap<int> map(g);
    for(int i = 0; i < problem.number_of_resources; i++) map[g.edgeFromId(i)] = costs_vector[i];
    
    ListGraph::EdgeMap<bool> mst(g);
    ListGraph::EdgeMap<bool> mstf(f);
    kruskal(g,map,mst);

    for(ListGraph::EdgeIt e(g); e != INVALID; ++e)
            mstf[e] = mst[e];

    int odd[n_vertices];
    for(int i = 0; i < n_vertices; i++)
        odd[i] = 0;

    for(ListGraph::NodeIt n(g); n != INVALID; ++n) //find nodes with odd-degree
    {
        int cont = 0;
        for(ListGraph::EdgeIt e(g); e != INVALID; ++e)
        {
            if(mst[e] && (g.id(g.u(e)) == g.id(n) || g.id(g.v(e)) == g.id(n))) cont++;      
        }
        if(cont % 2 != 0) odd[g.id(n)] = 1;
    }
    
    for(int i = 0; i < n_vertices; i++){
        if(!odd[i]){
            for(ListGraph::NodeIt n(g); n != INVALID; ++n)
            {
                if(g.id(n) == i) g.erase(n);
            }
        }
    }

    for(ListGraph::EdgeIt e(g); e != INVALID; ++e)
        map[e] = -map[e];

    MaxWeightedPerfectMatching<ListGraph,ListGraph::EdgeMap<int>> MPM(g, map); //run minimum perfect matching algorithm
    MPM.run();

    for(ListGraph::EdgeIt e(f); e != INVALID; ++e) //add the MPM edges
        if(!mstf[e] && !MPM.matching(e)) f.erase(e);

    ListGraph::NodeMap<bool> visited(f);
    for(ListGraph::NodeIt n(f); n != INVALID; ++n)
        visited[n] = 0;
    
    int v_inicial;
    Pilha pilha(n_vertices);
    
    for(ListGraph::NodeIt n(f); n != INVALID; ++n) //select source
    {
        v_inicial = f.id(n);
        pilha.push(v_inicial);
        visited[n] = 1;
        break;
    }
    
    int node_id = -1;
    int caminho[n_vertices + 1];
    int pos = 0;
    float custo_sol = 0;

    while(!pilha.vazia()) 
    {
        node_id = pilha.pop();
        caminho[pos] = node_id;
        pos++;

        for(ListGraph::EdgeIt e(f); e != INVALID; ++e)
        {
            if(f.id(f.u(e)) == node_id && !visited[f.v(e)]){
                visited[f.v(e)] = 1;
                pilha.push(f.id(f.v(e)));
            }
            else if(f.id(f.v(e)) == node_id && !visited[f.u(e)]){
                visited[f.u(e)] = 1;
                pilha.push(f.id(f.u(e)));
            }
        }            
    }

    caminho[pos] = v_inicial;
    
    for(int i = 0; i < n_vertices; i++){custo_sol += costs_vector[indice_aresta[caminho[i]][caminho[i+1]]];}
    
    if(LOCAL_SEARCH){           
        int novo_caminho[n_vertices + 1];
        float nova_sol = 0;
        int melhorou = 0;
        do{
            melhorou = 0;
            for (int i = 1; i < n_vertices - 1; i++){
                for (int j = i + 1; j < n_vertices; j++){
                    nova_sol = 0;
                    ls_2opt(novo_caminho, caminho, i, j, n_vertices + 1);
                    for(int k = 0; k < n_vertices; k++) nova_sol += costs_vector[indice_aresta[novo_caminho[k]][novo_caminho[k+1]]];
                    if(nova_sol < custo_sol){
                        for(int it = 0; it <= n_vertices; it++) caminho[it] = novo_caminho[it];
                        custo_sol = nova_sol;
                        melhorou = 1;
                    }
                }
            }
        }while(melhorou);            
    }

    for(int i = 0; i < n_vertices; i++){ 
        solution[indice_aresta[caminho[i]][caminho[i+1]]] = true;
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
		//std::cout << "Number of vertices = " << get_number_of_nodes() << std::endl;
		std::cout << "Number of edges (resources) = " << problem.get_number_of_resources() << std::endl;
		std::cout << "Number of scenarios = " << problem.get_number_of_scenarios() << std::endl;

		/*int total_number_of_terminals = 0;
		for (int s = 0; s < problem.get_number_of_scenarios(); s++){
			total_number_of_terminals += get_number_of_terminals_in_scenario(s);
		}
		std::cout << "Average number of terminals per scenario = " << total_number_of_terminals/problem.get_number_of_scenarios() << std::endl;*/
        
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

void ls_2opt(int novo_caminho[], int caminho[],int a, int b, int sz)
{
    int pos = 0;
    for(int i = 0; i < a; i++) novo_caminho[pos++] = caminho[i];
    for(int i = b; i >= a; i--) novo_caminho[pos++] = caminho[i];
    for(int i = b+1; i < sz; i++) novo_caminho[pos++] = caminho[i];
}


