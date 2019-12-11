#include <iostream>
#include <ctime>
#include <fstream>
#include "definitions.hpp"
#include "../greedy/definitions.hpp"
#include "../localSearch/definitions.hpp"
#include "../lateAcceptance/definitions.hpp"
#include "../tabuSearch/definitions.hpp"

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

	// Verificando quais instalacoes estarão abertas: 20% de chance de cada uma
	for(int i=0; i<qty_facilities; i++){
		randNum = rand() % 100; // Generate a random number between 0 and 99

		if(randNum < OPEN_RANDOM_RATE){ // se ficou entre os 20%, abre essa instalacao
			node->open_facilities[i] = 1;
		} 
		else{ //senao, deixa essa inst fechada
			node->open_facilities[i] = 0;
		}
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
void call_late_acceptance(solutionType * node, char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA, solutionType initial_sol, bool simple_ls){
	int lh = 10;
	if(simple_ls) // roda o LS_N1 simples
		lh = 1;
	*node = lateAcceptance(solutionName, qty_facilities, qty_clients, costF, costA, initial_sol, true, 2.5, 0.02, lh); // best_fit = true, a1 = 2.5, limit_idle = 0.02, lh = 10 or 1
}

// Recebe node por referencia. Modificacoes feitas no node aqui refletem diretamente la
void call_tabu_search(solutionType * node, char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA, solutionType initial_sol){
	int lc1 = 0.01 * qty_facilities;
	int lc2 = 0.05 * qty_facilities;
	*node = tabuSearch(solutionName, qty_facilities, qty_clients, costF, costA, initial_sol, true, 0.5, lc1, lc2, lc1, lc2, 0);  // best_fit = true, a1 = 0.5, (lc1 = lo1 = 0.01 * qty_facilities), (lc2 = lo2 = 0.05 * qty_facilities), seed = 0
}

// Recebe nodes por referencia. Modificacoes feitas no node aqui refletem diretamente la
void update_sub_pop_best(solutionType ** nodes, int * best_pocket_node, int * worst_pocket_node, int used_pockets, int id_parent){
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

			// // Se o que inverteu, o pai era a raiz (nodes[0][0]), então roda o tabu_search nele
			// if(id_parent == 0){
			// 	if(DEBUG >= DISPLAY_MOVES){
			// 		cout << "Calling TS for nodes[0][0]" << endl;
			// 	}
			// 	call_tabu_search(&nodes[0][0], solutionName, qty_facilities, qty_clients, costF, costA, nodes[0][0]);

			// 	if(DEBUG >= DISPLAY_ACTIONS){
			// 		cout << "Nodes[0][0] after tabu search: ";
			// 		print_individual(nodes[0][0].open_facilities, qty_facilities);
			// 	}
			// }
		}
	}
}


// Recebe nodes por referencia. Modificacoes feitas no node aqui refletem diretamente la
void update_sub_pop_worst(solutionType ** nodes, int * best_pocket_node, int * worst_pocket_node, int used_pockets, int id_parent){
	int index_child;
	solutionType aux;
	for(int i=0; i< QTY_CHILDREN; i++){ // para todos os filhos
		index_child = id_parent * 3 + i + 1; // encontra o indice correto do filho

		if(nodes[index_child][best_pocket_node[index_child]].finalTotalCost < nodes[id_parent][worst_pocket_node[id_parent]].finalTotalCost){ // Se o best do filho for menor que o worst do pai
			if(DEBUG >= DISPLAY_MOVES){
				cout << "Swapping child [" << index_child << "][" << best_pocket_node[index_child] << "](" << nodes[index_child][best_pocket_node[index_child]].finalTotalCost << ") with parent [" << id_parent << "][" << worst_pocket_node[id_parent] << "](" << nodes[id_parent][worst_pocket_node[id_parent]].finalTotalCost << ")" << endl;
			}

			// Inverte
			aux = nodes[id_parent][worst_pocket_node[id_parent]];
			nodes[id_parent][worst_pocket_node[id_parent]] = nodes[index_child][best_pocket_node[index_child]];
			nodes[index_child][best_pocket_node[index_child]] = aux;

			// Update indice de best_pocket do filho
			for(int j=0; j < used_pockets; j++){ // percorre por todos os pockets utilizados
				if(nodes[index_child][j].finalTotalCost < nodes[index_child][best_pocket_node[index_child]].finalTotalCost){ // se encontrou um melhor, atualiza 
					best_pocket_node[index_child] = j;
				}
			}
			// Update indice de best_pocket do pai. Só precisa comparar esse novo que veio com o best
			if(nodes[id_parent][worst_pocket_node[id_parent]].finalTotalCost < nodes[id_parent][best_pocket_node[id_parent]].finalTotalCost){ // se esse novo é melhor que o best, atualiza o best
				best_pocket_node[id_parent] = worst_pocket_node[id_parent];
			}
			// Update indice de worst do pai
			for(int j=0; j < used_pockets; j++){ // percorre por todos os pockets utilizados
				if(nodes[id_parent][j].finalTotalCost > nodes[id_parent][worst_pocket_node[id_parent]].finalTotalCost){ // se encontrou um pior, atualiza 
					worst_pocket_node[id_parent] = j;
				}
			}
		}
	}
}


// Recebe nodes por referencia. Modificacoes feitas no node aqui refletem diretamente la
void update_population(solutionType ** nodes, int * best_pocket_node, int * worst_pocket_node, int used_pockets, int QTY_SUBS){
	
	if(TYPE_UPDATE_POP == 1){ // Compara best do filho com o best do pai
		// Para cada pai, chama para atualizar essa sub populacao
		for(int i = QTY_SUBS-1; i>=0; i--){
			update_sub_pop_best(nodes, best_pocket_node, worst_pocket_node, used_pockets, i);
		}
	}
	else{ // compara o best do filho com o worst do pai
		// Para cada pai, chama para atualizar essa sub populacao
		for(int i = QTY_SUBS-1; i>=0; i--){
			update_sub_pop_worst(nodes, best_pocket_node, worst_pocket_node, used_pockets, i);
		}
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


// crossover uniforme: copia para o filho o que é igual e sorteia o que for diferente
void uniform_crossover(solutionType * child, solutionType mother, solutionType father, int qty_facilities){
	int randNum;
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
		cout << "Child after uniform crossover: ";
		print_individual(child->open_facilities, qty_facilities);
	}
}


// one point crossover: sorteia um numero (de 0 a qty_facilities -1) e copia para o filho da mae o que está na esquerda desse numero e do pai o resto
void one_point_crossover(solutionType * child, solutionType mother, solutionType father, int qty_facilities){
	int randNum;
	randNum = rand() % qty_facilities; // Generate a random number between 0 and qty_facilities - 1

	for(int i=0; i<randNum; i++){
		child->open_facilities[i] = mother.open_facilities[i];

		if(DEBUG >= DISPLAY_DETAILS){
			cout << "Facility " << i << " copy its mother: " << child->open_facilities[i] << endl;
		}
	}
	for(int i=randNum; i<qty_facilities; i++){
		child->open_facilities[i] = father.open_facilities[i];

		if(DEBUG >= DISPLAY_DETAILS){
			cout << "Facility " << i << " copy its father: " << child->open_facilities[i] << endl;
		}
	}

	if(DEBUG >= DISPLAY_ACTIONS){
		cout << "Child after one point crossover: ";
		print_individual(child->open_facilities, qty_facilities);
	}
}


// crossover união: copia para o filho todas as instalacoes abertas da mae e do pai, o resto fica fechado
void union_crossover(solutionType * child, solutionType mother, solutionType father, int qty_facilities){
	for(int i=0; i<qty_facilities; i++){
		if(father.open_facilities[i] || mother.open_facilities[i]){ // se no pai ou na mae está aberta, entao abre no filho
			child->open_facilities[i] = 1;

			if(DEBUG >= DISPLAY_DETAILS){
				cout << "Facility " << i << ": " << child->open_facilities[i] << endl;
			}
		}
		else{ // senao, entao deixa fechada
			child->open_facilities[i] = 0;

			if(DEBUG >= DISPLAY_DETAILS){
				cout << "Facility " << i << ": " << child->open_facilities[i] << endl;
			}
		}
	}

	if(DEBUG >= DISPLAY_ACTIONS){
		cout << "Child after union crossover: ";
		print_individual(child->open_facilities, qty_facilities);
	}
}


// Recebe node child por referencia. Modificacoes feitas no node aqui refletem diretamente la.
void crossover_mutation(solutionType * child, solutionType mother, solutionType father, int qty_facilities, int QTY_INST_MUTATION, int qty_clients, int ** sorted_cijID, double * costF, double ** assignment_cost){

	if(DEBUG >= DISPLAY_ACTIONS){
		cout << "CROSSOVER" << endl;
		cout << "Mother: ";
		print_individual(mother.open_facilities, qty_facilities);
		cout << "Father: ";
		print_individual(father.open_facilities, qty_facilities);
	}

	// Se for 0, indica que é para sortear qual o tipo de crossover devo fazer
	if(CROSSOVER_TYPE == 0){
		type_crossover = (rand() % 3) + 1; // Generate a random number between 1 and 3
	}

	if(CROSSOVER_TYPE == 1){ // Chama o uniform crossover
		uniform_crossover(child, mother, father, qty_facilities);
	}
	else if(CROSSOVER_TYPE == 2){ // Chama o one point crossover
		one_point_crossover(child, mother, father, qty_facilities);
	}
	else { // Chama o union crossover{ 
		union_crossover(child, mother, father, qty_facilities);
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


// Imprime um vetor com do tamanho de intalacoes. Cada posicao é um inteiro indicando em quantas solucoes essa instalacao estava aberta. pocket indica em qual pocket quer olhar de todos os nodes
void print_count_open_facilities(solutionType ** nodes, int pocket, int qty_facilities, int used_pockets){
	int qty_fac_open_all = 0;
	int qty_solutions = QTY_NODES_TREE;
	int * count_open_facilities = (int*) malloc((qty_facilities) * sizeof(int));

  	if(!count_open_facilities){
        cout << "Memory Allocation Failed";
        exit(1);
    }

    // Zerando todas as posicoes
    for(int i=0; i<qty_facilities; i++){
    	count_open_facilities[i] = 0;
    }

    // Contando a raiz
	for(int j=0;j<qty_facilities;j++){
		count_open_facilities[j] += nodes[0][0].open_facilities[j];
	}

    // indica que é para todos os pockets
    if(pocket < 0){
    	qty_solutions = used_pockets * (QTY_NODES_TREE - 1) + 1;
		// Contando para cada nó
		for(int i = 1; i< QTY_NODES_TREE; i++){
			for(int k = 0; k < used_pockets; k++){
				for(int j=0;j<qty_facilities;j++){
					count_open_facilities[j] += nodes[i][k].open_facilities[j];
				}
			}
		}
    }
    else{
    	// Contando para cada nó
		for(int i = 1; i< QTY_NODES_TREE; i++){
			for(int j=0;j<qty_facilities;j++){
				count_open_facilities[j] += nodes[i][pocket].open_facilities[j];
			}
		}
    }

    

	// Imprimindo o resultado
	cout << endl << "Number of sulutions each facility was open" << endl;
    for(int i=0; i<qty_facilities; i++){
    	cout << count_open_facilities[i] << " ";
    	if(count_open_facilities[i] == qty_solutions){
    		qty_fac_open_all += 1;
    	}
    }
    cout << endl << "Qty fac open in all (" << qty_solutions << ") solutions: " << qty_fac_open_all << endl;
    cout << endl;

	free(count_open_facilities);
}


// Recebe nodes por referencia. Modificacoes feitas no node aqui refletem diretamente la
void update_pop_change_root(solutionType ** nodes, int * best_pocket_node, int * worst_pocket_node, int used_pockets, int id_parent){
	int index_child;
	solutionType aux;

	// Verifico se ele é melhor do que a raiz atual
	if(nodes[id_parent][best_pocket_node[id_parent]].finalTotalCost < nodes[0][0].finalTotalCost){ // se for, substitui
		if(DEBUG >= DISPLAY_MOVES){
			cout << "Swapping (" << nodes[id_parent][best_pocket_node[id_parent]].finalTotalCost << ") with the root (" << nodes[0][0].finalTotalCost << ")" << endl;
		}

		// Inverte
		aux = nodes[id_parent][best_pocket_node[id_parent]];
		nodes[id_parent][best_pocket_node[id_parent]] = nodes[0][0];
		nodes[0][0] = aux;
	}
	else{ // se aconteceu o caso anterior, eu sei que o resto já está ordenado, pois só voltou pro mesmo lugar. Senao, entao tenho que ordenar essa subarvore


		// Atualiza o worst_pocket do pai primeiro. só esse novo que entrou pode ser pior do que o que ja era
		if(nodes[id_parent][best_pocket_node[id_parent]].finalTotalCost > nodes[id_parent][worst_pocket_node[id_parent]].finalTotalCost){ // se encontrou um pior, atualiza 
			worst_pocket_node[id_parent] = best_pocket_node[id_parent];
		}

		// Atualiza o best_pocket do pai 
		for(int j=0; j < used_pockets; j++){ // percorre por todos os pockets utilizados
			if(nodes[id_parent][j].finalTotalCost < nodes[id_parent][best_pocket_node[id_parent]].finalTotalCost){ // se encontrou um melhor, atualiza 
				best_pocket_node[id_parent] = j;
			}
		}

		if(TYPE_UPDATE_POP == 1){ // Compara best do filho com o best do pai
			update_sub_pop_best(nodes, best_pocket_node, worst_pocket_node, used_pockets, id_parent);
		}
		else{ // compara o best do filho com o worst do pai
			update_sub_pop_worst(nodes, best_pocket_node, worst_pocket_node, used_pockets, id_parent);
		}
	}
}


// Recebe por referencia, entao as alteracoes sao salvas. Armazena a root em solution se ela for melhor que a que está lá e sobe a próxima para a root, atualizando o resto da populacao

void change_root(solutionType ** nodes, solutionType * solution, int * best_pocket_node, int * worst_pocket_node, int used_pockets, int * qty_changes_root, char * solutionName, int qty_facilities, int qty_clients, double * costF,  double * costA, double ** assignment_cost, int ** sorted_cijID){

	if(DEBUG >= DISPLAY_MOVES){
		cout << "----------------------------------- Changing root " << endl;
	}

	// Salva a raiz, caso ela seja melhor do que a melhor salva até agora
	if(nodes[0][0].finalTotalCost < solution->finalTotalCost){
		*solution = nodes[0][0]; // atualiza a melhor encontrada até agora
		*qty_changes_root = 0; // zera o contador

		if(DEBUG >= DISPLAY_MOVES){
			cout << "Updating best solution found so far: " << solution->finalTotalCost << endl;
		}
	}
	else{
		*qty_changes_root += 1; // aumenta o contador
	}

	// Escolhe qual filho vai substituir a raiz
	int child_root = 1; // Inicia com o primeiro filho 

	for(int i=2;i<QTY_CHILDREN+1;i++){ // verifica se algum outro filho é melhor
		if(nodes[i][best_pocket_node[i]].finalTotalCost < nodes[child_root][best_pocket_node[child_root]].finalTotalCost){ // se for melhor, substitui
			child_root = i;
		}
	}

	// Substitui a raiz com o melhor filho escolhido
	nodes[0][0] = nodes[child_root][best_pocket_node[child_root]];

	// Atualiza o espaço que vagou no melhor filho escolhido com uma solucao vinda do random + LA
	set_initial_sol_RANDOM(&nodes[child_root][best_pocket_node[child_root]], qty_facilities, qty_clients, costF, assignment_cost, sorted_cijID); 
			
	if(DEBUG >= DISPLAY_DETAILS){
		cout << "Random child " << child_root << ":" << nodes[child_root][best_pocket_node[child_root]].finalTotalCost << endl;
	}

	// Roda LA ou LS completo para todas as solucoes geradas com random
	// call_local_search(&nodes[i][0], solutionName, qty_facilities, qty_clients, costF, costA, nodes[i][0]); 
	call_late_acceptance(&nodes[child_root][best_pocket_node[child_root]], solutionName, qty_facilities, qty_clients, costF, costA, nodes[child_root][best_pocket_node[child_root]], false); 

	if(DEBUG >= DISPLAY_DETAILS){
		cout << "LS_R:" << nodes[child_root][best_pocket_node[child_root]].finalTotalCost << endl << endl;
	}

	// Update população, levando esse novo gerado para o seu devido lugar
	update_pop_change_root(nodes, best_pocket_node, worst_pocket_node, used_pockets, child_root);

}


// Indica quantas instalacoes possuem valores diferentes, dado dois individuos
int qty_diversity(int * n1_open_facilities, int * n2_open_facilities, int qty_facilities){
	int qty = 0;
	for(int i=0; i< qty_facilities; i++){
		if(n1_open_facilities[i] != n2_open_facilities[i]){
			qty += 1;

			if(DEBUG >= DISPLAY_ACTIONS){
				cout << "** Dif fac: " << i << "**" << endl;
			}
		}
	}

	if(DEBUG >= DISPLAY_ACTIONS){
		print_individual(n1_open_facilities, qty_facilities);
		print_individual(n2_open_facilities, qty_facilities);
	}
	return qty;
}


// Indica se dois individuos são diferentes
bool are_different(solutionType n1, solutionType n2, int qty_facilities){
	if(n1.finalTotalCost != n2.finalTotalCost){
		return true;
	}
	else if(qty_diversity(n1.open_facilities, n2.open_facilities, qty_facilities) > 0){
		if(DEBUG >= DISPLAY_MOVES){
			cout << "***** It has the same cost (" << n1.finalTotalCost << "), but the config is different *****" << endl;
		}
		return true;
	}
	if(DEBUG >= DISPLAY_MOVES){
		cout << "***** IT IS EQUAL! It has the same cost (" << n1.finalTotalCost << ") and config! *****" << endl;
	}
	return false;
}


// Indica se o novo individuo é diferente de todos os outros do mesmo pocket
bool is_different_from_pockets(solutionType child, solutionType * node, int qty_facilities, int used_pockets){
	for(int i=0; i < used_pockets; i++){
		if(!are_different(child, node[i], qty_facilities)){ // se encontrou algum pocket que é igual, entao retorna falso
			return false;
		}
	}
	return true;
}
