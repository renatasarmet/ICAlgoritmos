#include <iostream>
#include <ctime>
#include <fstream>
#include "definitions.hpp"

#define QTY_NODES_TREE 13
#define QTY_CHILDREN_NODE 5 // inclui pocket and current //??

using namespace std;

#define DISPLAY_BASIC 1 // corresponde a exibicao da quantidade de movimentos
#define DISPLAY_MOVES 2 // corresponde a todos os cout quando um movimento é realizado de fato
#define DISPLAY_ACTIONS 3 // corresponde a todos os cout quando uma acao é feita. 
#define DISPLAY_DETAILS 4 // corresponde a todos os cout mais detalhados quando uma acao é feita. 
#define DISPLAY_TIME 5 // corresponde aos calculos de tempo 

#define DEBUG 3 // OPCOES DE DEBUG: 1 - MOSTRAR A QTD DE MOVIMENTOS, 2 PARA EXIBIR OS MOVIMENTOS REALIZADOS, 3 PARA EXIBIR ACOES, 4 PARA EXIBIR DETALHES DAS ACOES, 5 PARA EXIBIR TEMPO, 6 PARA EXIBIR AS MUDANÇAS NO GRAFO


// Retornar o valor da solucao
solutionType memetic(char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA){

	cout << fixed;
   	cout.precision(5);

	// Declaracao variaveis que indicam o tempo da funcao
	struct timespec start, finish;


	// INICIANDO A CONTAGEM DE TEMPO DA FUNCAO
	clock_gettime(CLOCK_REALTIME, &start);

	// FUTURO: SALVAR LOG E LOG_DETAILS


	// Declaracao dos nós da arvore. Cada nó é uma solucao
	solutionType nodes[QTY_NODES_TREE][QTY_CHILDREN_NODE];


	// Alocando memoria para as instalacoes abertas e instalacoes atribuidas de cada cliente
	for(int i=0;i<QTY_NODES_TREE;i++){
		for(int j=0;j<QTY_CHILDREN_NODE;j++){

			// indica as instalacoes abertas atualmente em cada solucao
			nodes[i][j].open_facilities = (int*) malloc((qty_facilities) * sizeof(int));

		    // indicara as instalacoes atribuidas a cada cliente na resposta
		    nodes[i][j].assigned_facilities = (int*) malloc((qty_clients) * sizeof(int));

		    if((!nodes[i][j].open_facilities)||(!nodes[i][j].assigned_facilities)){
		        cout << "Memory Allocation Failed";
		        exit(1);
		    }
		}
	}


	// assignment_cost - metor que indica o custo de atribuicao de cada cliente para cada instalacao
	double **assignment_cost = (double**) malloc((qty_clients) * sizeof(double*));

	// sorted_cijID - será o ID das instalacoes ordenadas pelo cij àquele cliente
	int **sorted_cijID = (int**) malloc((qty_clients) * sizeof(int*));

	for(int i = 0; i < qty_clients; i++) {
		assignment_cost[i] = (double *)malloc(qty_facilities * sizeof(double));
		sorted_cijID[i] = (int *)malloc(qty_facilities * sizeof(int));
	}

    if((!assignment_cost)||(!sorted_cijID)){
        cout << "Memory Allocation Failed";
        exit(1);
    }

    // Colocar os valores em assignment_cost e colocar valores ainda nao ordenados no sorted_cijID
    int counter = 0;
	for(int i=0;i<qty_clients;i++){
		for(int j=0;j<qty_facilities;j++){
			assignment_cost[i][j] = costA[counter]; // pegando valor vindo por parametro
			sorted_cijID[i][j] = j;
			counter++;
		}

    	// Ordenar sorted_cijID
    	mergeSortID(assignment_cost[i], sorted_cijID[i], 0, qty_facilities - 1);
	}


	// Inicializar as soluções iniciais - os 13 nós apenas 1 pocket
	set_initial_sol_G(&nodes[1][0], qty_facilities, qty_clients, costF, costA); // solucao com greedy
	set_initial_sol_LS_G(&nodes[0][0], solutionName, qty_facilities, qty_clients, costF, costA, nodes[1][0]); // solucao com local search com solucao inicial do greedy

	if(DEBUG >= DISPLAY_ACTIONS){
		cout << "Initial solutions:"<< endl;
		cout << "LS_G - node[0][0]: " << nodes[0][0].finalTotalCost << endl;
		cout << "Greedy - node[1][0]: " << nodes[1][0].finalTotalCost << endl;
	}

	for(int i=2;i<QTY_NODES_TREE;i++){
		set_initial_sol_RANDOM(&nodes[i][0], qty_facilities, qty_clients, costF, assignment_cost, i-2, sorted_cijID); // 11 solucoes aleatorias com sementes de 0 a 10
		
		if(DEBUG >= DISPLAY_ACTIONS){
			cout << "Random - node[" << i << "][0]:" << nodes[i][0].finalTotalCost << endl;
		}
	}


	// FINALIZANDO A CONTAGEM DE TEMPO DA FUNCAO
	clock_gettime(CLOCK_REALTIME, &finish);


	// Calculando o tempo gasto da funcao
	double timeSpent =  (finish.tv_sec - start.tv_sec);
	timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 

	if(DEBUG >= DISPLAY_TIME){
		cout << "Final Total Function Time: " << timeSpent << " seconds" << endl;
	}


	free(assignment_cost);
	free(sorted_cijID);

	return(nodes[0][0]);
}
