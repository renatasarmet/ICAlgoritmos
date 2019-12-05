#include <iostream>
#include <ctime>
#include <fstream>
#include "definitions.hpp"
#include "../greedy/definitions.hpp"
#include "../localSearch/definitions.hpp"

using namespace std;

#define DEBUG 3 // OPCOES DE DEBUG: 1 - MOSTRAR A QTD DE MOVIMENTOS, 2 PARA EXIBIR OS MOVIMENTOS REALIZADOS, 3 PARA EXIBIR ACOES, 4 PARA EXIBIR DETALHES DAS ACOES, 5 PARA EXIBIR TEMPO, 6 PARA EXIBIR AS MUDANÇAS NO GRAFO

void mergeSortID(double *vector, int *vectorID, int startPosition, int endPosition) {

	int i, j, k, halfSize, *tempVectorID;
	double *tempVector;
	if(startPosition == endPosition) return;
	halfSize = (startPosition + endPosition ) / 2;

	mergeSortID(vector, vectorID, startPosition, halfSize);
	mergeSortID(vector, vectorID, halfSize + 1, endPosition);

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


// Conecta os clientes à instalacao aberta mais proxima. Parametro por referencia.
void connect_nearest(solutionType * node, int qty_clients, int ** sorted_cijID, double ** assignment_cost){
	int cont;
	for(int i=0;i<qty_clients;i++){
		cont = 0;
		// Percorrendo pelo vetor ordenado de ID do cij, quando encontrar a primeira inst que estiver aberta, para
		while(!node->open_facilities[sorted_cijID[i][cont]]){
			cont+=1;
		}
		// Atribui essa inst como inst aberta mais proxima
		node->assigned_facilities[i] = sorted_cijID[i][cont];

		// Aumenta no custo total final
		node->finalTotalCost += assignment_cost[i][sorted_cijID[i][cont]];
	}
}


// Recebe node por referencia. Modificacoes feitas no node aqui refletem diretamente la
void set_initial_sol_G(solutionType * node, int qty_facilities, int qty_clients, double * costF, double * costA){ 

	*node = greedy(qty_clients, qty_facilities, costF, costA);

	// Alocando memoria para as instalacoes abertas
	node->open_facilities = (int*) malloc((qty_facilities) * sizeof(int));
	if(!node->open_facilities){
		cout << "Memory Allocation Failed open_facilities";
		exit(1);
	}

	// Inicializando todas as inst fechadas
	for(int i=0; i<qty_facilities; i++){
		node->open_facilities[i] = false;
	}

	// Atualizando as instalacoes abertas
	for(int i=0;i<qty_clients;i++){
		node->open_facilities[node->assigned_facilities[i]] = true;
	}
}


// Recebe node por referencia. Modificacoes feitas no node aqui refletem diretamente la
void call_local_search(solutionType * node, char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA, solutionType initial_sol){
	*node = localSearch(solutionName, qty_facilities, qty_clients, costF, costA, initial_sol, 1); // tipo LS completo
}


// Recebe node por referencia. Modificacoes feitas no node aqui refletem diretamente la
void set_initial_sol_RANDOM(solutionType * node, int qty_facilities, int qty_clients, double * costF, double ** assignment_cost, int seed, int ** sorted_cijID){ // type: 0 para greedy, 1 para LS_G, 2 para aleatorio

	// Semente do numero aleatorio
	srand(seed);

	int randNum;
	bool used;

	// Inicializando struct 
	node->finalTotalCost = 0;

	// Verificando quais instalacoes estarão abertas: 50% de chance de cada uma
	for(int i=0; i<qty_facilities; i++){
		randNum = rand() % 2; // Generate a random number between 0 and 1
		node->open_facilities[i] = randNum;
	}

	// Conectando cada cliente com a instalacao aberta mais proxima
	connect_nearest(node, qty_clients, sorted_cijID, assignment_cost);

	// Fechar as instalacoes que nao foram conectadas a ninguem
	for(int i=0; i<qty_facilities; i++){
		used = false;
		if(node->open_facilities[i]){ // se essa instalacao estiver aberta
			for(int j=0;j<qty_clients;j++){ // percorre pelos clientes para ver se alguem está conectada nela
				if(node->assigned_facilities[j] == i){
					used = true;
					break; // encontrou alguem concetado, entao sai
				}
			}
			if(!used){ // se nao tem ninguem conectado nela, vamos fecha-la
				node->open_facilities[i] = false;
			}
			else{ // Se essa instalacao esta sendo usada, vai continuar aberta e devemos somar o custo
				node->finalTotalCost += costF[i];
			}
		}
	}
}


// Recebe nodes por referencia. Modificacoes feitas no node aqui refletem diretamente la
void update_sub_pop(solutionType ** nodes, int * best_pocket_node, int id_parent){
	int index_child;
	solutionType aux;
	for(int i=0; i< QTY_CHILDREN; i++){ // para todos os filhos
		index_child = id_parent * 3 + i + 1; // encontra o indice correto do filho
		if(nodes[index_child][best_pocket_node[index_child]].finalTotalCost < nodes[id_parent][best_pocket_node[id_parent]].finalTotalCost){ // Se o best do filho for menor que o best do pai
			if(DEBUG >= DISPLAY_MOVES){
				cout << "Swapping child " << index_child << "(" << nodes[index_child][best_pocket_node[index_child]].finalTotalCost << ") with parent " << id_parent << " (" << nodes[id_parent][best_pocket_node[id_parent]].finalTotalCost << ")" << endl;
			}


			// Inverte
			aux = nodes[id_parent][best_pocket_node[id_parent]];
			nodes[id_parent][best_pocket_node[id_parent]] = nodes[index_child][best_pocket_node[index_child]];
			nodes[index_child][best_pocket_node[index_child]] = aux;

			// Update indice de best_pocket do filho
			for(int j=0; j < QTY_POCKETS_NODE; j++){ // percorre por todos os pockets
				if(nodes[index_child][j].finalTotalCost < 0){ // pocket vazio, entao todos pra frente tambem estarão
					break;
				}
				else if(nodes[index_child][j].finalTotalCost < nodes[index_child][best_pocket_node[index_child]].finalTotalCost){ // se encontrou um melhor, atualiza 
					best_pocket_node[index_child] = j;
				}
			}
		}
	}
}


// Recebe nodes por referencia. Modificacoes feitas no node aqui refletem diretamente la
void update_population(solutionType ** nodes, int * best_pocket_node){
	// Compara best do filho com o best do pai
	int qty_subs = (QTY_NODES_TREE - 1) / QTY_CHILDREN;

	// Para cada pai, chama para atualizar essa sub populacao
	for(int i = qty_subs-1; i>=0; i--){
		update_sub_pop(nodes, best_pocket_node, i);
	}
}



void print_tree_pockets(solutionType ** nodes){
	for(int i=0;i<QTY_NODES_TREE;i++){	
		for(int j=0; j<QTY_POCKETS_NODE; j++){
			cout << "node[" << i << "][" << j << "]:" << nodes[i][j].finalTotalCost << endl;
		}
		cout << endl;
	}
}


void print_tree_best(solutionType ** nodes, int * best_pocket_node){
	int index_child;

	cout << endl << "TREE BEST POCKETS"<< endl;
	// Imprimindo a raiz
	cout << "									" << nodes[0][best_pocket_node[0]].finalTotalCost << endl;

	// Imprimindo os nós intermediários (1,2,3)
	for(int i=0;i<QTY_CHILDREN;i++){  
		index_child = i + 1; // encontra o indice correto do filho
		cout << "		" << nodes[index_child][best_pocket_node[index_child]].finalTotalCost << "				";
	}
	cout << endl;

	// Imprimindo as folhas
	for(int id_parent=1;id_parent<=QTY_CHILDREN;id_parent++){  
		for(int i=0;i<QTY_CHILDREN;i++){  
			index_child = id_parent * 3 + i + 1; // encontra o indice correto do filho
			cout << nodes[index_child][best_pocket_node[index_child]].finalTotalCost << "	";
		}
		cout << "	";
	}
	cout << endl << endl;

}

