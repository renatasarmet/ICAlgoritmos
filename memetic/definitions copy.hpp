#include <iostream>
#include "../global_definitions.hpp"

#define QTY_NODES_TREE 13
#define QTY_POCKETS_NODE 5 
#define INDEX_CURRENT QTY_POCKETS_NODE // ocupa o sexto pocket de cada nó
#define QTY_CHILDREN 3 // quantos nós filhos cada nó pai tem

#define CHECK_DIVERSITY false // indica se vai verificar se o filho gerado é diferente dos pcokets ou nao
#define OPEN_RANDOM_RATE 50 // 10% // probabilidade de abrir uma instalacao na geração de solucoes random
#define MUTATION_RATE 0.01 // 1% 0.1// 10% //0.05 // 5% * qty_facilities
#define CROSSOVER_TYPE 3 // 1 para uniform, 2 para one-point, 3 para union, 0 para aleatorio cada vez
#define PROB_LA_RATE 50 // 50%
#define MAX_GEN_NO_IMPROVEMENT 5 // quantidade de geracoes sem melhora, para atualizar o root
#define MAX_CHANGES_ROOT 2 // indica a quantidade de vezes seguidas que pode atualizar o root sem melhorar a best salva
#define TYPE_UPDATE_POP 2 // 1 para best, 2 para worst
#define INITIAL_COMPLETE_POCKET true // inidica se inicial vai povoar todos os pockets ou só o 0

#define DISPLAY_BASIC 1 // corresponde a exibicao da quantidade de movimentos
#define DISPLAY_MOVES 2 // corresponde a todos os cout quando um movimento é realizado de fato
#define DISPLAY_ACTIONS 3 // corresponde a todos os cout quando uma acao é feita. 
#define DISPLAY_DETAILS 4 // corresponde a todos os cout mais detalhados quando uma acao é feita. 
#define DISPLAY_TIME 5 // corresponde aos calculos de tempo 


solutionType memetic(char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA);

void mergeSortID(double *vector, int *vectorID, int startPosition, int endPosition);

int qty_open_facilities(int * open_facilities, int qty_facilities);

void connect_nearest(solutionType * node, int qty_clients, int ** sorted_cijID, double ** assignment_cost);

void connect_and_update_facilities(solutionType * node, int qty_facilities, int qty_clients, int ** sorted_cijID, double * costF, double ** assignment_cost);

void set_initial_sol_G(solutionType * node, int qty_facilities, int qty_clients, double * costF, double * costA);

void set_initial_sol_RANDOM(solutionType * node, int qty_facilities, int qty_clients, double * costF, double ** assignment_cost, int ** sorted_cijID);

void call_local_search(solutionType * node, char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA, solutionType initial_sol);

void call_late_acceptance(solutionType * node, char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA, solutionType initial_sol, bool simple_ls);

void call_tabu_search(solutionType * node, char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA, solutionType initial_sol);

void call_local_search_close_fac(solutionType * node, char * solutionName, int qty_facilities, int qty_clients, double * costF, double ** assignment_cost, solutionType initial_sol);

void update_sub_pop_best(solutionType ** nodes, int * best_pocket_node, int * worst_pocket_node, int used_pockets, int id_parent);

void update_sub_pop_worst(solutionType ** nodes, int * best_pocket_node, int * worst_pocket_node, int used_pockets, int id_parent, char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA);

void update_population(solutionType ** nodes, int * best_pocket_node, int * worst_pocket_node, int used_pockets, int QTY_SUBS, char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA);

void print_tree_best(solutionType ** nodes, int * best_pocket_node);

void print_tree_pockets(solutionType ** nodes, bool display_current);

void print_individual(int * open_facilities, int qty_facilities);

void mutation(solutionType * child, int qty_facilities, int QTY_INST_MUTATION);

void uniform_crossover(solutionType * child, solutionType mother, solutionType father, int qty_facilities);

void one_point_crossover(solutionType * child, solutionType mother, solutionType father, int qty_facilities);

void crossover_mutation(solutionType * child, solutionType mother, solutionType father, int qty_facilities, int QTY_INST_MUTATION, int qty_clients, int ** sorted_cijID, double * costF, double ** assignment_cost);

void print_count_open_facilities(solutionType ** nodes, int pocket, int qty_facilities, int used_pockets);

void update_pop_change_root(solutionType ** nodes, int * best_pocket_node, int * worst_pocket_node, int used_pockets, int id_parent, char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA);

void change_root(solutionType ** nodes, solutionType * solution, int * best_pocket_node, int * worst_pocket_node, int used_pockets, int * qty_changes_root, char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA, double ** assignment_cost, int ** sorted_cijID);

int qty_diversity(int * n1_open_facilities, int * n2_open_facilities, int qty_facilities);

bool are_different(solutionType n1, solutionType n2, int qty_facilities);

bool is_different_from_pockets(solutionType child, solutionType * node, int qty_facilities, int used_pockets);



