#include <iostream>
#include "../global_definitions.hpp"

#define QTY_NODES_TREE 13
#define QTY_POCKETS_NODE 5 
#define INDEX_CURRENT 5 // ocupa o sexto pocket de cada nó
#define QTY_CHILDREN 3 // quantos nós filhos cada nó pai tem

#define MUTATION_RATE 0.05 // 5% * qty_facilities

#define DISPLAY_BASIC 1 // corresponde a exibicao da quantidade de movimentos
#define DISPLAY_MOVES 2 // corresponde a todos os cout quando um movimento é realizado de fato
#define DISPLAY_ACTIONS 3 // corresponde a todos os cout quando uma acao é feita. 
#define DISPLAY_DETAILS 4 // corresponde a todos os cout mais detalhados quando uma acao é feita. 
#define DISPLAY_TIME 5 // corresponde aos calculos de tempo 


solutionType memetic(char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA);

void mergeSortID(double *vector, int *vectorID, int startPosition, int endPosition);

void connect_nearest(solutionType * node, int qty_clients, int ** sorted_cijID, double ** assignment_cost);

void set_initial_sol_G(solutionType * node, int qty_facilities, int qty_clients, double * costF, double * costA);

void call_local_search(solutionType * node, char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA, solutionType initial_sol);

void set_initial_sol_RANDOM(solutionType * node, int qty_facilities, int qty_clients, double * costF, double ** assignment_cost, int ** sorted_cijID);

void update_sub_pop(solutionType ** nodes, int * best_pocket_node, int id_parent);

void update_population(solutionType ** nodes, int * best_pocket_node);

void print_tree_best(solutionType ** nodes, int * best_pocket_node);

void print_tree_pockets(solutionType ** nodes);

void mutation(solutionType * child, int qty_facilities, int QTY_INST_MUTATION);

void crossover_mutation(solutionType * child, solutionType * father, solutionType * mother, int qty_facilities, int QTY_INST_MUTATION);
