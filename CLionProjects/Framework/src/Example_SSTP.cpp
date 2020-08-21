#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <ostream>
#include <string>


#include "SP_instance.h"
#include "SP_solution.h"
#include "EF_generic_solver.h"
#include "EF_local_search_solver.h"
#include "EF_manager.h"
#include "EF_second_stage_solver.h"
#include "auxi/timer.hpp"

#include "Instance.h"
#include "Solution.h"
#include "Greedy.h"

using namespace std;

double ** cost_connect_cli_fac; //cost_connect_cli_fac[CLIENTE][INSTALACAO]
vector < vector < int > > exist_cli_scenario; //exist_cli_scenario[CENARIO][CLIENTE]
int number_of_clients;

void print_usage(char * exe_name, ostream& print_location);

void read_instance(EF::SP_instance &problem, char * instance_name){
  std::ifstream myfile(instance_name);
  if (!myfile) {std::cerr << "Failed to open file " << instance_name << std::endl; exit(-1);}

  //Read instance
  myfile >> problem.number_of_resources;
  problem.cost_first_stage.resize(problem.number_of_resources);
  problem.cost_in_scenario.resize(problem.number_of_resources);

  myfile >> number_of_clients;
  for(int f = 0; f < problem.number_of_resources; f++){
    myfile >> problem.cost_first_stage[f];
  }

// alocacao de memoria
    cost_connect_cli_fac = new double*[number_of_clients];
    if (number_of_clients)
    {
        cost_connect_cli_fac[0] = new double[number_of_clients * problem.number_of_resources];
        for (int i = 1; i < number_of_clients; ++i) {
            cost_connect_cli_fac[i] = cost_connect_cli_fac[0] + i * problem.number_of_resources;
        }
    }

// Lendo instancia
    for(int c = 0; c < number_of_clients; c++){
        for(int f = 0; f < problem.number_of_resources; f++){
          double cost_c_f;
          myfile >> cost_c_f;
          cost_connect_cli_fac[c][f] = cost_c_f;
        }
    }


  //Scenarios
  myfile >> problem.number_of_scenarios;
  problem.probabilities_scenario.resize(problem.number_of_scenarios);

  for(int s = 0; s < problem.number_of_scenarios; s++){
    myfile >> problem.probabilities_scenario[s];
  }
  for(int f = 0; f < problem.number_of_resources; f++)
    (problem.cost_in_scenario[f]).resize(problem.number_of_scenarios);

  for(int s = 0; s < problem.number_of_scenarios; s++){
    vector < int > aux;
    for(int c = 0; c < number_of_clients; c++){
        int exist_cli;
        myfile >> exist_cli;
        aux.push_back(exist_cli);
    }
    exist_cli_scenario.push_back(aux);
  }

  for(int s = 0; s < problem.number_of_scenarios; s++){
    for(int f = 0; f < problem.number_of_resources; f++){
      myfile >> problem.cost_in_scenario[f][s];
    }
  }

  //this vector is used for optimization purposes
  problem.resource_cost_in_scenario.resize(problem.get_number_of_scenarios());
  for(int s = 0; s < problem.get_number_of_scenarios() ; s++){
    problem.resource_cost_in_scenario[s] = new std::vector<double>(problem.get_number_of_resources());
    for (int f = 0; f< problem.get_number_of_resources();f++){
      (*problem.resource_cost_in_scenario[s])[f] = problem.cost_in_scenario[f][s];
    }
  }
}

//costs_vector[i] is the perturbed cost of resource i, it will be 0 if
//i has been chosen on first stage (might also be zero in other situations).
//Your decoder must return the cost of this solution (for this scenario)
//considering the original cost, except those already chosen on first stage.
//Your decoder should also consider the exist_cli_scenario,
// that indicates whether each client exists or not in each scenario
double decode_Ap(EF::SP_instance &problem, int s, std::vector <bool>& selected_resources_first_stage, std::vector <double>& costs_vector,
    std::vector <bool>& solution){

    //  Descobrir quantos clientes realmente estão nessa instância
    int qty_cli_scenario = 0;
    for(int c = 0; c < number_of_clients; c++){
        if(exist_cli_scenario[s][c]){
            qty_cli_scenario+= 1;
        }
    }

    // Criando a instancia correta
    Instance instanceGreedy;
    instanceGreedy.readFrameworkInstance(
            problem.get_number_of_resources(),
            number_of_clients,
            qty_cli_scenario,
            exist_cli_scenario[s].data(),
            costs_vector.data(),
            (double **) &cost_connect_cli_fac[0]
            );

//    instanceGreedy.showInstance();

    Solution solutionGreedy(instanceGreedy);

    Greedy greedy;

    // Chamando a funcao que resolve o problema de fato
    greedy.allocate(&solutionGreedy);
    greedy.initialize(&solutionGreedy);

    double scenarioTotalCost = 0;

    // Calcular valor original das instalacoes e colocar solucao em solution
    for(int f = 0; f < problem.get_number_of_resources(); f++){
        if(solutionGreedy.getOpenFacilityJ(f)){
            solution[f] = true;
            // Se ele nao foi escolhido no primeiro estagio, soma o custo dele
            if(!selected_resources_first_stage[f]){
                scenarioTotalCost += problem.get_cost_in_scenario(f, s);
            }
        }
        else{
            solution[f] = false;
        }
    }

    // Somar o custo de todos os clientes atribuidos às respectivas instalacoes
    for(int c = 0; c < qty_cli_scenario; c++){
        scenarioTotalCost += instanceGreedy.getCostAIJ(c, solutionGreedy.getAssignedFacilityJ(c));
    }

    return scenarioTotalCost;


// Solucao do Pedro:
//  vector < pair < double,int > > cost;
//  for(int f = 0; f < problem.number_of_resources; f++){
//    cost.push_back(make_pair(costs_vector[f], f));
//  }
//
//  sort(cost.begin(), cost.end());
//
//  //Compro só a primeira instalação
//  int instalacao_mais_barata = cost[0].second;
//  solution[instalacao_mais_barata] = true;
//
//  double custo = 0;
//  //custo das conexões
//  for(int c = 0; c < number_of_clients; c++){
//    custo += cost_connect_cli_fac[c][instalacao_mais_barata];
//  }
//
//  //custo da instalacao
//  if(selected_resources_first_stage[instalacao_mais_barata] == true)
//    custo += 0; //ja comprou no primeiro estagio é de graça
//  else
//    custo += problem.get_cost_in_scenario(instalacao_mais_barata, s); //ou paga o custo original
//
//  return custo;
}




int main(int argc, char ** argv) {
  Timer timer;
  timer.restart();

  if (argc < 2) {print_usage(argv[0], std::cout); exit(0); }

  //Set default parameters
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
  if (verbose >= 1) std::cout << "Reading instance " << instance_file_string << std::endl;
  EF::SP_instance problem;
  problem.decode_Ap = &decode_Ap;

  read_instance(problem, intance_file_name);

  if (verbose >= 1) std::cout << "Finished instance reading" << std::endl;

  //Prints instance data
  if (verbose >= 1){
    std::cout << "Number of resources = " << problem.get_number_of_resources() << std::endl;
    std::cout << "Number of scenarios = " << problem.get_number_of_scenarios() << std::endl;


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
      if (verbose >= 2) std::cout << "Scenario " << s << ":";
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
  std::cout << "@Final_time " << timer.elapsed() << std::endl;
  std::cout << "@Final_solution_cost " << ef_manager.get_solution_value() << std::endl;


  return 0;
}

void print_usage(char * exe_name, ostream& print_location) {
  print_location << "Usage: " << exe_name
      << " <instance_file> <time_limit (-1 if no time limit)> <verbose>"
      << endl;
}


