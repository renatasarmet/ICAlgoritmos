#include <lemon/list_graph.h>
#include <algorithm>
#include <iostream>
#include <set>
#include <ctime>
#include <fstream>
#include <cstring>
#include <cfloat>
#include "definitions.hpp"

#define QTY_NODES_TREE 13
#define QTY_CHILDREN_NODE 5 // inclui pocket and current //??

using namespace lemon;
using namespace std;

#define DISPLAY_BASIC 1 // corresponde a exibicao da quantidade de movimentos
#define DISPLAY_MOVES 2 // corresponde a todos os cout quando um movimento é realizado de fato
#define DISPLAY_ACTIONS 3 // corresponde a todos os cout quando uma acao é feita. 
#define DISPLAY_DETAILS 4 // corresponde a todos os cout mais detalhados quando uma acao é feita. 
#define DISPLAY_TIME 5 // corresponde aos calculos de tempo 
#define DISPLAY_GRAPH 6 // corresponde a descricao dos clientes, instalacoes e arcos

#define DEBUG 1 // OPCOES DE DEBUG: 1 - MOSTRAR A QTD DE MOVIMENTOS, 2 PARA EXIBIR OS MOVIMENTOS REALIZADOS, 3 PARA EXIBIR ACOES, 4 PARA EXIBIR DETALHES DAS ACOES, 5 PARA EXIBIR TEMPO, 6 PARA EXIBIR AS MUDANÇAS NO GRAFO


void mergeSort(double *vector, int *vectorID, int startPosition, int endPosition) {

    int i, j, k, halfSize, *tempVectorID;
    double *tempVector;
    if(startPosition == endPosition) return;
    halfSize = (startPosition + endPosition ) / 2;

    mergeSort(vector, vectorID, startPosition, halfSize);
    mergeSort(vector, vectorID, halfSize + 1, endPosition);

    i = startPosition;
    j = halfSize + 1;
    k = 0;
    tempVector = (double *) malloc(sizeof(double) * (endPosition - startPosition + 1));
    tempVectorID = (int *) malloc(sizeof(int) * (endPosition - startPosition + 1));

    while(i < halfSize + 1 || j  < endPosition + 1) {
        if (i == halfSize + 1 ) { 
            tempVector[k] = vector[vectorID[j]];
            tempVectorID[k] = vectorID[j];
            j++;
            k++;
        }
        else {
            if (j == endPosition + 1) {
                tempVector[k] = vector[vectorID[i]];
                tempVectorID[k] = vectorID[i];
                i++;
                k++;
            }
            else {
                if (vector[vectorID[i]] < vector[vectorID[j]]) {
                    tempVector[k] = vector[vectorID[i]];
                    tempVectorID[k] = vectorID[i];
                    i++;
                    k++;
                }
                else {
                    tempVector[k] = vector[vectorID[j]];
                    tempVectorID[k] = vectorID[j];
                    j++;
                    k++;
                }
            }
        }
    }
    for(i = startPosition; i <= endPosition; i++) {
        vectorID[i] = tempVectorID[i - startPosition];
    }
    free(tempVector);
    free(tempVectorID);
}




// Retornar o valor da solucao
solutionType memetic(char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA, solutionType solution, bool best_fit, double a1, int lc1, int lc2, int lo1, int lo2, int seed){

	cout << fixed;
   	cout.precision(5);

	// Declaracao variaveis que indicam o tempo da funcao
	struct timespec start, finish;

	// FUTURO: SALVAR LOG E LOG_DETAILS



	// Declaracao dos nós da arvore. Cada nó é uma solucao
	solutionType nodes[QTY_NODES_TREE][QTY_CHILDREN_NODE];

	// Inicializando todas as solucoes com todas as inst fechadas
	for(int i=0;i<QTY_NODES_TREE;i++){
		for(int j=0;j<QTY_CHILDREN_NODE;j++){
			for(int k=0;k<qty_facilities;k++){
				nodes[i][j].open_facilities[k] = false;
			}
		}
	}

	// TO DO: VERIFICAR POR CADA NÓ, MARCAR COMO ABERTA TODAS AS INSTALACOES QUE APARECEREM EM ALGUM ASSIGNED_FACILITIES[i]


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

    // Colocar os valores em assignment_cost
    int counter = 0;
	for(int i=0;i<qty_clients;i++){
		for(int j=0;j<qty_facilities;j++){
			assignment_cost[i][j] = costA[counter]; // pegando valor vindo por parametro
			counter++;
		}
	}


    // Colocar valores ainda nao ordenados no sorted_cijID


    // Ordenar sorted_cijID




	// INICIANDO A CONTAGEM DE TEMPO DA FUNCAO
	clock_gettime(CLOCK_REALTIME, &start);
	

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

	return(solution);
}
