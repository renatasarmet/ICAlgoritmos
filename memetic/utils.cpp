#include <iostream>
#include <ctime>
#include <fstream>
#include "definitions.hpp"
#include "../greedy/definitions.hpp"
#include "../localSearch/definitions.hpp"
#include "../lateAcceptance/definitions.hpp"

using namespace std;

#define DEBUG 2 // OPCOES DE DEBUG: 1 - MOSTRAR A QTD DE MOVIMENTOS, 2 PARA EXIBIR OS MOVIMENTOS REALIZADOS, 3 PARA EXIBIR ACOES, 4 PARA EXIBIR DETALHES DAS ACOES, 5 PARA EXIBIR TEMPO, 6 PARA EXIBIR AS MUDANÇAS NO GRAFO

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


// Verifica quantas instalacoes estao abertas
int qty_open_facilities(int * open_facilities, int qty_facilities){
	int qty = 0;
	for(int i=0; i< qty_facilities; i++){
		if(open_facilities[i]){
			qty += 1;
		}
	}
	return qty;
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


// Chama a função que conecta os clientes à instalacao aberta mais proxima, fecha as instalacoes que nao tem ninguem conectado e atualiza o finalTotalCost
void connect_and_update_facilities(solutionType * node, int qty_facilities, int qty_clients, int ** sorted_cijID, double * costF, double ** assignment_cost){

	bool used;

	// Zera o custo total final
	node->finalTotalCost = 0;

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

				if(DEBUG >= DISPLAY_DETAILS){
					cout << "Closing facility " << i << " because no clients were assigned to it"<< endl;
				}
			}
			else{ // Se essa instalacao esta sendo usada, vai continuar aberta e devemos somar o custo
				node->finalTotalCost += costF[i];
			}
		}
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
void set_initial_sol_RANDOM(solutionType * node, int qty_facilities, int qty_clients, double * costF, double ** assignment_cost, int ** sorted_cijID){ // type: 0 para greedy, 1 para LS_G, 2 para aleatorio

	int randNum;

	// Verificando quais instalacoes estarão abertas: 50% de chance de cada uma
	for(int i=0; i<qty_facilities; i++){
		randNum = rand() % 2; // Generate a random number between 0 and 1
		node->open_facilities[i] = randNum;
	}

	if(DEBUG >= DISPLAY_MOVES){
		cout << "RANDOM before connecting clients: ";
		cout << qty_open_facilities(node->open_facilities, qty_facilities) << " open facilities" << endl;	
	}

	// Conectando cada cliente com a instalacao aberta mais proxima e fechando as instalacoes que nao foram conectadas a ninguem. Custo final também é atualizado
	connect_and_update_facilities(node, qty_facilities, qty_clients, sorted_cijID, costF, assignment_cost);

	if(DEBUG >= DISPLAY_MOVES){
		cout << "RANDOM after connecting clients: ";
		cout << qty_open_facilities(node->open_facilities, qty_facilities) << " open facilities" << endl;	
	}
}


// Recebe node por referencia. Modificacoes feitas no node aqui refletem diretamente la
void call_local_search(solutionType * node, char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA, solutionType initial_sol){
	*node = localSearch(solutionName, qty_facilities, qty_clients, costF, costA, initial_sol, 1); // tipo LS completo
}


// Recebe node por referencia. Modificacoes feitas no node aqui refletem diretamente la
void call_late_acceptance(solutionType * node, char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA, solutionType initial_sol){
	*node = lateAcceptance(solutionName, qty_facilities, qty_clients, costF, costA, initial_sol, true, 2.5, 0.02, 10); // best_fit = true, a1 = 2.5, limit_idle = 0.02, lh = 10
}


// Recebe nodes por referencia. Modificacoes feitas no node aqui refletem diretamente la
void update_sub_pop(solutionType ** nodes, int * best_pocket_node, int * worst_pocket_node, int used_pockets, int id_parent){
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
			for(int j=0; j < used_pockets; j++){ // percorre por todos os pockets utilizados
				if(nodes[index_child][j].finalTotalCost < nodes[index_child][best_pocket_node[index_child]].finalTotalCost){ // se encontrou um melhor, atualiza 
					best_pocket_node[index_child] = j;
				}
			}
			// Update indice de worst do pai, se ele era o pior
			if(worst_pocket_node[id_parent] == best_pocket_node[id_parent]){
				for(int j=0; j < used_pockets; j++){ // percorre por todos os pockets utilizados
					if(nodes[id_parent][j].finalTotalCost > nodes[id_parent][worst_pocket_node[id_parent]].finalTotalCost){ // se encontrou um pior, atualiza 
						worst_pocket_node[id_parent] = j;
					}
				}
			}
		}
	}
}


// Recebe nodes por referencia. Modificacoes feitas no node aqui refletem diretamente la
void update_population(solutionType ** nodes, int * best_pocket_node, int * worst_pocket_node, int used_pockets, int QTY_SUBS){
	// Compara best do filho com o best do pai

	// Para cada pai, chama para atualizar essa sub populacao
	for(int i = QTY_SUBS-1; i>=0; i--){
		update_sub_pop(nodes, best_pocket_node, worst_pocket_node, used_pockets, i);
	}
}



void print_tree_pockets(solutionType ** nodes, bool display_current){
	int qty_pockets_display = QTY_POCKETS_NODE;

	if(display_current){
		qty_pockets_display += 1;
	}

	for(int i=0;i<QTY_NODES_TREE;i++){	
		for(int j=0; j<qty_pockets_display; j++){
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


void print_individual(int * open_facilities, int qty_facilities){
	cout << endl;

	for(int i=0;i<qty_facilities;i++){  
		cout << open_facilities[i] << " ";
	}
	cout << endl << endl;
}


// Recebe nodes por referencia. Modificacoes feitas no node aqui refletem diretamente la. OBS: nao deve alterar o pai nem a mae aqui
void mutation(solutionType * child, int qty_facilities, int QTY_INST_MUTATION){
	// MUTATION_RATE % das instalações sofrerão mutação

	if(DEBUG >= DISPLAY_ACTIONS){
		cout << "MUTATION" << endl;
	}

	int randNum;

	// QTY_INST_MUTATION instalacoes sofrerao mutação
	for(int i=0; i<QTY_INST_MUTATION; i++){
		randNum = rand() % qty_facilities; // Generate a random number between 0 and qty_facilities
		child->open_facilities[randNum] = !child->open_facilities[randNum];

		if(DEBUG >= DISPLAY_DETAILS){
			cout << "Facility " << randNum << " changes open to: " << child->open_facilities[randNum] << endl;
		}
	}

	if(DEBUG >= DISPLAY_ACTIONS){
		cout << "Child after mutation: ";
		print_individual(child->open_facilities, qty_facilities);
	}
}



// Recebe node child por referencia. Modificacoes feitas no node aqui refletem diretamente la.
void crossover_mutation(solutionType * child, solutionType mother, solutionType father, int qty_facilities, int QTY_INST_MUTATION, int qty_clients, int ** sorted_cijID, double * costF, double ** assignment_cost){
	int randNum;

	if(DEBUG >= DISPLAY_ACTIONS){
		cout << "CROSSOVER" << endl;
		cout << "Mother: ";
		print_individual(mother.open_facilities, qty_facilities);
		cout << "Father: ";
		print_individual(father.open_facilities, qty_facilities);
	}

	// crossover uniforme: copia para o filho o que é igual e sorteia o que for diferente
	for(int i=0; i<qty_facilities; i++){
		if(father.open_facilities[i] == mother.open_facilities[i]){
			child->open_facilities[i] = mother.open_facilities[i];

			if(DEBUG >= DISPLAY_DETAILS){
				cout << "Facility " << i << " copy its parent: " << child->open_facilities[i] << endl;
			}
		}
		else{
			randNum = rand() % 2; // Generate a random number between 0 and 1
			child->open_facilities[i] = randNum;

			if(DEBUG >= DISPLAY_DETAILS){
				cout << "Facility " << i << " generate open: " << child->open_facilities[i] << endl;
			}
		}
	}

	if(DEBUG >= DISPLAY_ACTIONS){
		cout << "Child after crossover: ";
		print_individual(child->open_facilities, qty_facilities);
	}

	// mutation 
	mutation(child, qty_facilities, QTY_INST_MUTATION);

	if(DEBUG >= DISPLAY_ACTIONS){
		cout << "ASSIGNING CLIENTS" << endl;
	}

	// Conectando cada cliente com a instalacao aberta mais proxima e fechando as instalacoes que nao foram conectadas a ninguem. Custo final também é atualizado
	connect_and_update_facilities(child, qty_facilities, qty_clients, sorted_cijID, costF, assignment_cost);

	if(DEBUG >= DISPLAY_ACTIONS){
		cout << "Child after connecting clients: ";
		print_individual(child->open_facilities, qty_facilities);
	}

}



