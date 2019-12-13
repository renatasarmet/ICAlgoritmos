#include <iostream>
#include <ctime>
#include <fstream>
#include "definitions.hpp"
#include "../greedy/definitions.hpp"
#include "../localSearch/definitions.hpp"
#include "../lateAcceptance/definitions.hpp"
#include "../tabuSearch/definitions.hpp"
#include "../localSearchCloseFac/definitions.hpp"

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
	int qtd_open = 0;

	// Verificando quais instalacoes estarão abertas: OPEN_RANDOM_RATE% de chance de cada uma
	for(int i=0; i<qty_facilities; i++){
		randNum = rand() % 100; // Generate a random number between 0 and 99

		if(randNum < OPEN_RANDOM_RATE){ // se ficou entre os OPEN_RANDOM_RATE%, abre essa instalacao
			node->open_facilities[i] = 1;
			qtd_open += 1;
		} 
		else{ //senao, deixa essa inst fechada
			node->open_facilities[i] = 0;
		}
	}

	// Se nao abriu nenhuma, abre pelo menos uma qualquer aleatoria
	if(qtd_open == 0){
		randNum = rand() % qty_facilities; // Generate a random number between 0 and qty_facilities

		node->open_facilities[randNum] = 1;
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


// Recebe node por referencia. Modificacoes feitas no node aqui refletem diretamente la
void call_local_search_close_fac(solutionType * node, char * solutionName, int qty_facilities, int qty_clients, double * costF, double ** assignment_cost, solutionType initial_sol){
	*node = LSCloseFac(solutionName, qty_facilities, qty_clients, costF, assignment_cost, initial_sol); 
}



// Recebe nodes por referencia. Modificacoes feitas no node aqui refletem diretamente la
void update_sub_pop(solutionType ** nodes, int id_parent, char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA){
	int index_child;
	int randNum;
	solutionType aux;
	for(int i=0; i< QTY_CHILDREN; i++){ // para todos os filhos
		index_child = id_parent * 3 + i + 1; // encontra o indice correto do filho
		if(nodes[index_child][INDEX_POCKET].finalTotalCost < nodes[id_parent][INDEX_POCKET].finalTotalCost){ // Se o best do filho for menor que o best do pai (pocket com pocket)
			if(DEBUG >= DISPLAY_MOVES){
				cout << "Swapping child " << index_child << "(" << nodes[index_child][INDEX_POCKET].finalTotalCost << ") with parent " << id_parent << " (" << nodes[id_parent][INDEX_POCKET].finalTotalCost << ")" << endl;
			}

			// Inverte
			aux = nodes[id_parent][INDEX_POCKET];
			nodes[id_parent][INDEX_POCKET] = nodes[index_child][INDEX_POCKET];
			nodes[index_child][INDEX_POCKET] = aux;

			// Update indice de pocket/current do filho (pocket sempre deve ser melhor que o current)
			if(nodes[index_child][INDEX_CURRENT].finalTotalCost < nodes[index_child][INDEX_POCKET].finalTotalCost){ // se agora o current é melhor que o pocket, atualiza
				// Inverte
				aux = nodes[index_child][INDEX_POCKET];
				nodes[index_child][INDEX_POCKET] = nodes[index_child][INDEX_CURRENT];
				nodes[index_child][INDEX_CURRENT] = aux;
			}

			// Se o que inverteu, o pai era a raiz (nodes[0][0]), então roda o tabu_search nele
			if(id_parent == 0){

				// randNum = rand() % QTY_NODES_TREE; // Generate a random number between 0 and QTY_NODES_TREE
				randNum = 0;

				if(DEBUG >= DISPLAY_MOVES){
					cout << "Calling TS for nodes[" << randNum << "][" << INDEX_POCKET << "] (" << nodes[randNum][INDEX_POCKET].finalTotalCost <<")" << endl;
				}

				call_tabu_search(&nodes[randNum][INDEX_POCKET], solutionName, qty_facilities, qty_clients, costF, costA, nodes[randNum][INDEX_POCKET]);

				if(DEBUG >= DISPLAY_MOVES){
					cout << "New value for nodes[" << randNum << "][" << INDEX_POCKET << "] = " << nodes[randNum][INDEX_POCKET].finalTotalCost << endl;
				}

				if(DEBUG >= DISPLAY_ACTIONS){
					cout << "Nodes[" << randNum << "][" << INDEX_POCKET << "] after tabu search: ";
					print_individual(nodes[randNum][INDEX_POCKET].open_facilities, qty_facilities);
				}
			}
		}
	}
}


// Recebe nodes por referencia. Modificacoes feitas no node aqui refletem diretamente la
void update_population(solutionType ** nodes, int QTY_SUBS, char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA){
	
	// compara o pocket do filho com o pocket do pai (best com best)
	// Para cada pai, chama para atualizar essa sub populacao
	for(int i = QTY_SUBS-1; i>=0; i--){
		update_sub_pop(nodes, i, solutionName, qty_facilities, qty_clients, costF, costA);
	}
}



void print_tree(solutionType ** nodes){
	for(int i=0;i<QTY_NODES_TREE;i++){	
		for(int j=0; j<QTY_SOLUTIONS_NODE; j++){
			cout << "node[" << i << "][" << j << "]:" << nodes[i][j].finalTotalCost << endl;
		}
		cout << endl;
	}
}


void print_tree_best(solutionType ** nodes){
	int index_child;

	cout << endl << "TREE BEST POCKET"<< endl;
	// Imprimindo a raiz
	cout << "									" << nodes[0][INDEX_POCKET].finalTotalCost << endl;

	// Imprimindo os nós intermediários (1,2,3)
	for(int i=0;i<QTY_CHILDREN;i++){  
		index_child = i + 1; // encontra o indice correto do filho
		cout << "		" << nodes[index_child][INDEX_POCKET].finalTotalCost << "				";
	}
	cout << endl;

	// Imprimindo as folhas
	for(int id_parent=1;id_parent<=QTY_CHILDREN;id_parent++){  
		for(int i=0;i<QTY_CHILDREN;i++){  
			index_child = id_parent * 3 + i + 1; // encontra o indice correto do filho
			cout << nodes[index_child][INDEX_POCKET].finalTotalCost << "	";
		}
		cout << "	";
	}
	cout << endl << endl;
}

void print_tree_complete(solutionType ** nodes){
	int index_child;

   	cout.precision(0);

	cout << endl << "TREE COMPLETE"<< endl;
	// Imprimindo a raiz
	cout << "									" << nodes[0][INDEX_POCKET].finalTotalCost << "("<< nodes[0][INDEX_CURRENT].finalTotalCost << ")" << endl;

	// Imprimindo os nós intermediários (1,2,3)
	for(int i=0;i<QTY_CHILDREN;i++){  
		index_child = i + 1; // encontra o indice correto do filho
		cout << "		" << nodes[index_child][INDEX_POCKET].finalTotalCost << "("<< nodes[index_child][INDEX_CURRENT].finalTotalCost << ")" << "				";
	}
	cout << endl;

	// Imprimindo as folhas
	for(int id_parent=1;id_parent<=QTY_CHILDREN;id_parent++){  
		for(int i=0;i<QTY_CHILDREN;i++){  
			index_child = id_parent * 3 + i + 1; // encontra o indice correto do filho
			cout << nodes[index_child][INDEX_POCKET].finalTotalCost << "("<< nodes[index_child][INDEX_CURRENT].finalTotalCost << ")" << "	";
		}
		cout << "	";
	}
	cout << endl << endl;

   	cout.precision(5);
}


void print_individual(int * open_facilities, int qty_facilities){
	cout << endl;

	for(int i=0;i<qty_facilities;i++){  
		cout << open_facilities[i] << " ";
	}
	cout << endl << endl;
}


void print_open_facilities(int * open_facilities, int qty_facilities){
	cout << endl << "Open facilities: ";

	for(int i=0;i<qty_facilities;i++){  
		if(open_facilities[i]){
			cout << i << " ";
		}
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
	int type_crossover = 0;

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

	if((CROSSOVER_TYPE == 1)||(type_crossover == 1)){ // Chama o uniform crossover
		uniform_crossover(child, mother, father, qty_facilities);
	}
	else if((CROSSOVER_TYPE == 2)||(type_crossover == 2)){ // Chama o one point crossover
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


// Recebe node child por referencia. Modificacoes feitas no node aqui refletem diretamente la.
void recombine(solutionType * child, solutionType mother, solutionType father, int qty_facilities, int QTY_INST_MUTATION, int qty_clients, int ** sorted_cijID, double * costF, double * costA, double ** assignment_cost, char * solutionName, int * map, double * new_costF, double * new_costA, int * temp_open_facilities){
	
	crossover_mutation(child, mother, father, qty_facilities, QTY_INST_MUTATION, qty_clients, sorted_cijID, costF, assignment_cost);

	// Se for do tipo uniform ou one-point crossover, entao chama o Late Acceptance
	if((CROSSOVER_TYPE == 1) ||(CROSSOVER_TYPE == 2)){
		// // Vai decidir se vai chamar o LA dessa vez ou não
		// int prob_la = rand() % 100; // Generate a random number between 0 and 99

		// if(prob_la < PROB_LA_RATE){	 // se o numero foi menor que o prob_la_rate, entao chama
			// LA em cada filho gerado
			call_late_acceptance(child, solutionName, qty_facilities, qty_clients, costF, costA, *child, false);

			if(DEBUG >= DISPLAY_ACTIONS){
				cout << "Child after late acceptance: ";
				print_individual(child->open_facilities, qty_facilities);
			}
		// }
		// else{
		// 	if(DEBUG >= DISPLAY_ACTIONS){		
		// 		cout << "We won't call LA this time. Random number: " << prob_la << " >= " << PROB_LA_RATE << endl;
		// 	}
		// }
	}
	// Senão, se for do tipo union, chama o LS_N0
	else if(CROSSOVER_TYPE == 3){
		// call_local_search_close_fac(child, solutionName, qty_facilities, qty_clients, costF, assignment_cost, *child);

		// if(DEBUG >= DISPLAY_ACTIONS){
		// 	cout << "Child after local search close fac: ";
		// 	print_individual(child->open_facilities, qty_facilities);
		// }

		// call_tabu_search(child, solutionName, qty_facilities, qty_clients, costF, costA, *child);

		// if(DEBUG >= DISPLAY_ACTIONS){
		// 	cout << "Child after tabu search: ";
		// 	print_individual(child->open_facilities, qty_facilities);
		// }

		map_and_call_TS(child, qty_facilities, qty_clients, costF, assignment_cost, map, new_costF, new_costA, solutionName, temp_open_facilities);

		if(DEBUG >= DISPLAY_ACTIONS){
			cout << "Child after map and tabu search: ";
			print_individual(child->open_facilities, qty_facilities);
		}

	}

}


// Imprime um vetor com do tamanho de intalacoes. Cada posicao é um inteiro indicando em quantas solucoes essa instalacao estava aberta.
void print_count_open_facilities(solutionType ** nodes, int qty_facilities){
	int qty_fac_open_all = 0;
	int qty_solutions = QTY_SOLUTIONS_NODE * QTY_NODES_TREE;
	int * count_open_facilities = (int*) malloc((qty_facilities) * sizeof(int));

  	if(!count_open_facilities){
        cout << "Memory Allocation Failed";
        exit(1);
    }

    // Zerando todas as posicoes
    for(int i=0; i<qty_facilities; i++){
    	count_open_facilities[i] = 0;
    }
 
	// Contando para cada nó
	for(int i = 0; i < QTY_NODES_TREE; i++){
		for(int k = 0; k < QTY_SOLUTIONS_NODE; k++){ // percorre o pocket e o current
			for(int j=0;j<qty_facilities;j++){
				count_open_facilities[j] += nodes[i][k].open_facilities[j];
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
void update_pop_change_root(solutionType ** nodes, int id_parent, char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA){
	int index_child;
	solutionType aux;

	// Verifico se ele é melhor do que a raiz atual
	if(nodes[id_parent][INDEX_POCKET].finalTotalCost < nodes[0][INDEX_POCKET].finalTotalCost){ // se for, substitui
		if(DEBUG >= DISPLAY_MOVES){
			cout << "Swapping (" << nodes[id_parent][INDEX_POCKET].finalTotalCost << ") with the root (" << nodes[0][INDEX_POCKET].finalTotalCost << ")" << endl;
		}

		// Inverte
		aux = nodes[id_parent][INDEX_POCKET];
		nodes[id_parent][INDEX_POCKET] = nodes[0][INDEX_POCKET];
		nodes[0][INDEX_POCKET] = aux;
	}
	else{ // se aconteceu o caso anterior, eu sei que o resto já está ordenado, pois só voltou pro mesmo lugar (ou veio um melhor ainda). Senao, entao tenho que ordenar essa subarvore

		// Atualiza o POCKET e CURRENT do pai primeiro. só esse novo que entrou pode ser pior do que o que ja era
		if(nodes[id_parent][INDEX_POCKET].finalTotalCost > nodes[id_parent][INDEX_CURRENT].finalTotalCost){ // se esse que chegou é pior que o current, atualiza 
			// Inverte
			aux = nodes[id_parent][INDEX_POCKET];
			nodes[id_parent][INDEX_POCKET] = nodes[id_parent][INDEX_CURRENT];
			nodes[id_parent][INDEX_CURRENT] = aux;
		}

		update_sub_pop(nodes, id_parent, solutionName, qty_facilities, qty_clients, costF, costA);
	}
}


// Recebe por referencia, entao as alteracoes sao salvas. Armazena a root em solution se ela for melhor que a que está lá e sobe a próxima para a root, atualizando o resto da populacao

void change_root(solutionType ** nodes, solutionType * solution, int * qty_changes_root, char * solutionName, int qty_facilities, int qty_clients, double * costF,  double * costA, double ** assignment_cost, int ** sorted_cijID){
	solutionType aux;

	if(DEBUG >= DISPLAY_MOVES){
		cout << "----------------------------------- Changing root " << endl;
	}

	// Salva a raiz, caso ela seja melhor do que a melhor salva até agora
	if(nodes[0][INDEX_POCKET].finalTotalCost < solution->finalTotalCost){
		*solution = nodes[0][INDEX_POCKET]; // atualiza a melhor encontrada até agora
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
		if(nodes[i][INDEX_POCKET].finalTotalCost < nodes[child_root][INDEX_POCKET].finalTotalCost){ // se for melhor, substitui
			child_root = i;
		}
	}

	// Substitui a raiz com o melhor filho escolhido
	nodes[0][INDEX_POCKET] = nodes[child_root][INDEX_POCKET];

	// Verifica se o pocket da raiz continua melhor que o current, senao inverte
	if(nodes[0][INDEX_POCKET].finalTotalCost > nodes[0][INDEX_CURRENT].finalTotalCost){ // se o pocket for pior que o current, inverte
		aux = nodes[0][INDEX_POCKET];
		nodes[0][INDEX_POCKET] = nodes[0][INDEX_CURRENT];
		nodes[0][INDEX_CURRENT] = aux;
	}

	// Atualiza o espaço que vagou no melhor filho escolhido com uma solucao vinda do random + LA
	set_initial_sol_RANDOM(&nodes[child_root][INDEX_POCKET], qty_facilities, qty_clients, costF, assignment_cost, sorted_cijID); 
			
	if(DEBUG >= DISPLAY_DETAILS){
		cout << "Random child " << child_root << ":" << nodes[child_root][INDEX_POCKET].finalTotalCost << endl;
	}

	// Roda LA ou LS completo para a solucao gerada com random
	// call_local_search(&nodes[i][INDEX_POCKET], solutionName, qty_facilities, qty_clients, costF, costA, nodes[i][INDEX_POCKET]); 
	call_late_acceptance(&nodes[child_root][INDEX_POCKET], solutionName, qty_facilities, qty_clients, costF, costA, nodes[child_root][INDEX_POCKET], false); 

	if(DEBUG >= DISPLAY_DETAILS){
		cout << "LS_R:" << nodes[child_root][INDEX_POCKET].finalTotalCost << endl << endl;
	}

	// Update população, levando esse novo gerado para o seu devido lugar
	update_pop_change_root(nodes, child_root, solutionName, qty_facilities, qty_clients, costF, costA);

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


void swap_pocket_current(solutionType * node){
	solutionType aux;

	// Inverte
	aux = node[INDEX_POCKET];
	node[INDEX_POCKET] = node[INDEX_CURRENT];
	node[INDEX_CURRENT] = aux;
}


// se o current for menor que o pocket, retorna negativo
// se for igual, fica 0
// se o current for maior, retorna positivo
double compare_pocket_current(solutionType * node){
	return node[INDEX_CURRENT].finalTotalCost - node[INDEX_POCKET].finalTotalCost; 
}


void update_refset(solutionType ** nodes, int qty_facilities, int qty_clients, double * costF, double ** assignment_cost, int ** sorted_cijID){
	double aux_comparison;
	bool ok = false;
	// Verificar se os currents entrarao no refSet ou nao
	// verifica se o current é melhor que pocket e se nao tem nada igual


	/* 
	Para a RAIZ 
	*/
	while(!ok){
		ok = true;
		aux_comparison = compare_pocket_current(nodes[0]);

		// Se o current for menor que o pocket, atualiza invertendo e pronto
		if(aux_comparison < 0){
			swap_pocket_current(nodes[0]);
		}
		// Senao, se for diferentes, o current é menor que o pocket entao está ok
		// Senao se forem iguais, se só o custo for igual, ta ok mantem como está
		// Senão se realmente forem iguais
		else if(!are_different(nodes[0][INDEX_CURRENT], nodes[0][INDEX_POCKET], qty_facilities)){
			// Gera uma solução aleatória e repete o processo
			set_initial_sol_RANDOM(&nodes[0][INDEX_CURRENT], qty_facilities, qty_clients, costF, assignment_cost, sorted_cijID); 
			
			if(DEBUG >= DISPLAY_DETAILS){
				cout << "Random - node[" << 0 << "][" << INDEX_CURRENT << "]:" << nodes[0][INDEX_CURRENT].finalTotalCost << endl;
			}

			ok = false;
		}
	}


	/*
	para todos pais intermediarios (1,2,3)
	*/

	for(int i = 1; i < QTY_CHILDREN+1; i++){ 
		ok = false;

		while(!ok){
			ok = true;

			// Compara com seu próprio pocket
			aux_comparison = compare_pocket_current(nodes[i]);

			// Se o custo for igual
			if(aux_comparison == 0){
				// Se só o custo for igual mas a solução for diferente, ta ok, pode deixar assim mesmo.. verificar com os outros pais intermediarios
				// Se realmente forem iguais
				if(!are_different(nodes[i][INDEX_CURRENT], nodes[i][INDEX_POCKET], qty_facilities)){
					// Gera uma solução aleatória e repete o processo
					set_initial_sol_RANDOM(&nodes[i][INDEX_CURRENT], qty_facilities, qty_clients, costF, assignment_cost, sorted_cijID); 
			
					if(DEBUG >= DISPLAY_DETAILS){
						cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].finalTotalCost << endl;
					}

					ok = false;
				}
			}

			// Se ja nao achou um problema, continuar procurando
			if(ok){
				// Compara com os outros pais intermediarios anteriores
				for(int j=i-1; j>0;j--){

					// Compara com o pocket e current 
					// se for menor que o pocket, ta ok
					// se for maior que o pocket, compara com o current
					if(nodes[i][INDEX_CURRENT].finalTotalCost > nodes[j][INDEX_POCKET].finalTotalCost){
						// se for igual ao current, gera aleatoria
						if(!are_different(nodes[i][INDEX_CURRENT], nodes[j][INDEX_CURRENT], qty_facilities)){
							// Gera uma solução aleatória e repete o processo
							set_initial_sol_RANDOM(&nodes[i][INDEX_CURRENT], qty_facilities, qty_clients, costF, assignment_cost, sorted_cijID); 
			
							if(DEBUG >= DISPLAY_DETAILS){
								cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].finalTotalCost << endl;
							}

							ok = false;
							break;
						}
					}
					// se for igual o pocket, ver se é igual mesmo, e gerar aletaroia
					else if(!are_different(nodes[i][INDEX_CURRENT], nodes[j][INDEX_POCKET], qty_facilities)){
						// Gera uma solução aleatória e repete o processo
						set_initial_sol_RANDOM(&nodes[i][INDEX_CURRENT], qty_facilities, qty_clients, costF, assignment_cost, sorted_cijID); 
			
						if(DEBUG >= DISPLAY_DETAILS){
							cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].finalTotalCost << endl;
						}

						ok = false;
						break;
					}
				}
			}

			// Se ja nao achou um problema, continuar procurando
			if(ok){
 				if(aux_comparison < 0){
					// Verifica se é melhor que o pocket da raiz
					if(nodes[i][INDEX_CURRENT].finalTotalCost < nodes[0][INDEX_POCKET].finalTotalCost){
						// Se sim, atualiza pocket <-> current
						swap_pocket_current(nodes[i]);
					}
					// se for maior
					else if(nodes[i][INDEX_CURRENT].finalTotalCost > nodes[0][INDEX_POCKET].finalTotalCost){
						// se for igual ao current, gera aleatoria
						if(!are_different(nodes[i][INDEX_CURRENT], nodes[0][INDEX_CURRENT], qty_facilities)){
							// Gera uma solução aleatória e repete o processo
							set_initial_sol_RANDOM(&nodes[i][INDEX_CURRENT], qty_facilities, qty_clients, costF, assignment_cost, sorted_cijID); 
			
							if(DEBUG >= DISPLAY_DETAILS){
								cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].finalTotalCost << endl;
							}

							ok = false;
						}
						// se for diferente, atualiza
						else {
							// atualiza pocket <-> current
							swap_pocket_current(nodes[i]);
						}
					}
					// se for igual mesmo ao pocket da raiz
					else if(!are_different(nodes[i][INDEX_CURRENT], nodes[0][INDEX_POCKET], qty_facilities)){
						// Gera uma solução aleatória e repete o processo
						set_initial_sol_RANDOM(&nodes[i][INDEX_CURRENT], qty_facilities, qty_clients, costF, assignment_cost, sorted_cijID); 
			
						if(DEBUG >= DISPLAY_DETAILS){
							cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].finalTotalCost << endl;
						}

						ok = false;
					}
				}
				// se for pior ou igual, só precisa confirmar que ele não é igual ao current da raiz
				else {
					// se for igual ao current, gera aleatoria
					if(!are_different(nodes[i][INDEX_CURRENT], nodes[0][INDEX_CURRENT], qty_facilities)){
						// Gera uma solução aleatória e repete o processo
						set_initial_sol_RANDOM(&nodes[i][INDEX_CURRENT], qty_facilities, qty_clients, costF, assignment_cost, sorted_cijID); 
			
						if(DEBUG >= DISPLAY_DETAILS){
							cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].finalTotalCost << endl;
						}

						ok = false;
					}
				}
			}
		}
	}


	/*
	para todos os filhos folhas (4,5,6,7,8,9,10,11,12)
	*/

	for(int i = QTY_CHILDREN + 1; i < QTY_NODES_TREE; i++){ 
		ok = false;

		while(!ok){
			ok = true;

			// Compara com seu próprio pocket
			aux_comparison = compare_pocket_current(nodes[i]);

			// Se o custo for igual
			if(aux_comparison == 0){
				// Se só o custo for igual mas a solução for diferente, ta ok, pode deixar assim mesmo.. verificar com os outros filhos folhas anteriores
				// Se realmente forem iguais
				if(!are_different(nodes[i][INDEX_CURRENT], nodes[i][INDEX_POCKET], qty_facilities)){
					// Gera uma solução aleatória e repete o processo
					set_initial_sol_RANDOM(&nodes[i][INDEX_CURRENT], qty_facilities, qty_clients, costF, assignment_cost, sorted_cijID); 
			
					if(DEBUG >= DISPLAY_DETAILS){
						cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].finalTotalCost << endl;
					}

					ok = false;
				}
			}

			// Se ja nao achou um problema, continuar procurando
			if(ok){

				// Compara com os outros filhos folha anteriores
				for(int j=i-1; j>QTY_CHILDREN;j--){

					// Compara com o pocket e current 
					// se for menor que o pocket, ta ok
					// se for maior que o pocket, compara com o current
					if(nodes[i][INDEX_CURRENT].finalTotalCost > nodes[j][INDEX_POCKET].finalTotalCost){
						// se for igual ao current, gera aleatoria
						if(!are_different(nodes[i][INDEX_CURRENT], nodes[j][INDEX_CURRENT], qty_facilities)){
							// Gera uma solução aleatória e repete o processo
							set_initial_sol_RANDOM(&nodes[i][INDEX_CURRENT], qty_facilities, qty_clients, costF, assignment_cost, sorted_cijID); 
			
							if(DEBUG >= DISPLAY_DETAILS){
								cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].finalTotalCost << endl;
							}

							ok = false;
							break;
						}
					}
					// se for igual o pocket, ver se é igual mesmo, e gerar aletaroia
					else if(!are_different(nodes[i][INDEX_CURRENT], nodes[j][INDEX_POCKET], qty_facilities)){
						// Gera uma solução aleatória e repete o processo
						set_initial_sol_RANDOM(&nodes[i][INDEX_CURRENT], qty_facilities, qty_clients, costF, assignment_cost, sorted_cijID); 
			
						if(DEBUG >= DISPLAY_DETAILS){
							cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].finalTotalCost << endl;
						}

						ok = false;
						break;
					}
				}
			}

			// Se ja nao achou um problema, continuar procurando
			if(ok){
				// Verificar igualdade com pocket e current do pai e dos tios
				for(int j=1; j<QTY_CHILDREN+1; j++){
					// Compara com o pocket desse nó
					// se for menor, ta ok, depois já vou ver o da raiz mesmo
					// se for maior
					if(nodes[i][INDEX_CURRENT].finalTotalCost > nodes[j][INDEX_POCKET].finalTotalCost){
						// compara com o current desse nó
						if(!are_different(nodes[i][INDEX_CURRENT], nodes[j][INDEX_CURRENT], qty_facilities)){
							// Gera uma solução aleatória e repete o processo
							set_initial_sol_RANDOM(&nodes[i][INDEX_CURRENT], qty_facilities, qty_clients, costF, assignment_cost, sorted_cijID); 
			
							if(DEBUG >= DISPLAY_DETAILS){
								cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].finalTotalCost << endl;
							}

							ok = false;
							break;
						}
					}
					// se nao, compara se sao iguais
					if(!are_different(nodes[i][INDEX_CURRENT], nodes[j][INDEX_POCKET], qty_facilities)){
						// Gera uma solução aleatória e repete o processo
						set_initial_sol_RANDOM(&nodes[i][INDEX_CURRENT], qty_facilities, qty_clients, costF, assignment_cost, sorted_cijID); 
			
						if(DEBUG >= DISPLAY_DETAILS){
							cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].finalTotalCost << endl;
						}

						ok = false;
						break;
					}
				}
			}

			// Se ja nao achou um problema, continuar procurando
			if(ok){
				// se ele for melhor que o pocket atual
				if(aux_comparison < 0){
					// Verifica se é melhor que o pocket da raiz
					if(nodes[i][INDEX_CURRENT].finalTotalCost < nodes[0][INDEX_POCKET].finalTotalCost){
						// Se sim, atualiza pocket <-> current
						swap_pocket_current(nodes[i]);
					}
					// se for maior
					else if(nodes[i][INDEX_CURRENT].finalTotalCost > nodes[0][INDEX_POCKET].finalTotalCost){
						// se for igual ao current, gera aleatoria
						if(!are_different(nodes[i][INDEX_CURRENT], nodes[0][INDEX_CURRENT], qty_facilities)){
							// Gera uma solução aleatória e repete o processo
							set_initial_sol_RANDOM(&nodes[i][INDEX_CURRENT], qty_facilities, qty_clients, costF, assignment_cost, sorted_cijID); 
			
							if(DEBUG >= DISPLAY_DETAILS){
								cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].finalTotalCost << endl;
							}

							ok = false;
						}
						// se forem diferentes, atualiza
						else{
							// atualiza pocket <-> current
							swap_pocket_current(nodes[i]);
						}
					}
					// se for igual mesmo ao pocket da raiz
					else if(!are_different(nodes[i][INDEX_CURRENT], nodes[0][INDEX_POCKET], qty_facilities)){
						// Gera uma solução aleatória e repete o processo
						set_initial_sol_RANDOM(&nodes[i][INDEX_CURRENT], qty_facilities, qty_clients, costF, assignment_cost, sorted_cijID); 
			
						if(DEBUG >= DISPLAY_DETAILS){
							cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].finalTotalCost << endl;
						}

						ok = false;
					}
				}
				// se for pior ou igual, só precisa confirmar que ele não é igual ao current da raiz
				else {
					// se for igual ao current, gera aleatoria
					if(!are_different(nodes[i][INDEX_CURRENT], nodes[0][INDEX_CURRENT], qty_facilities)){
						// Gera uma solução aleatória e repete o processo
						set_initial_sol_RANDOM(&nodes[i][INDEX_CURRENT], qty_facilities, qty_clients, costF, assignment_cost, sorted_cijID); 
			
						if(DEBUG >= DISPLAY_DETAILS){
							cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].finalTotalCost << endl;
						}

						ok = false;
					}
				}
			}
		}
	}
}


// Mapeando para chamar o TS ou o G com menos instalacoes que o total
// map[i] indica o valor que realmente corresponde
// retorna cont
int mapping(solutionType * solution, int qty_facilities, int qty_clients, double * costF, double ** assignment_cost, int * map, double * new_costF, double * new_costA){
	int cont = 0, cont2 = 0;
	bool update_assigned = false;
	// Mapear costF e open_facilities, salvando o map
	for(int i=0; i< qty_facilities; i++){
		// se a instalacao está aberta, ela vai entrar nesse conjunto para chamar
		if(solution->open_facilities[i]){
			map[cont] = i; // indica que na verdade essa inst será correspondente à i
			new_costF[cont] = costF[i];
			solution->open_facilities[cont] = true;
			cont += 1;
		}
	}
	// marcar as que nao foram mapeadas, como fechadas
	for(int i=cont;i<qty_facilities;i++){
		// map[i] = -1;
		solution->open_facilities[i] = false;
	}

	// Mapear costA e assigned_facilities
	for(int j=0;j < qty_clients; j++){
		update_assigned = false;
		for(int i=0;i<cont;i++){	
			new_costA[cont2] = assignment_cost[j][map[i]];
			cont2 += 1;
			if(!update_assigned){
				if(solution->assigned_facilities[j] == map[i]){
					solution->assigned_facilities[j] = i;
					update_assigned = true;
				}
			}
		}
	}

	if(DEBUG >= DISPLAY_DETAILS){
		for(int i=0; i< cont; i++){
			cout << "Map[" << i << "] = " << map[i] << endl;
		}
	}

	return cont;
}



// Desmapeando, depois que retornou do TS ou G com menos instalacoes
// map[i] indica o valor que realmente corresponde
void unmapping(solutionType * solution, int cont_facilities, int qty_facilities, int qty_clients, int * map, int * temp_open_facilities){
	// salva todos os valores em temp_open_facilities
	for(int i=0;i<cont_facilities;i++){
		temp_open_facilities[i] = solution->open_facilities[i];
	}
	// zera todas as instalacoes de solution->open_facilities
	for(int i=0; i<qty_facilities; i++){
		solution->open_facilities[i] = false;
	}
	// Coloca de volta as instalacoes aberta em solution->open_facilities
	for(int i = 0; i <cont_facilities; i++){
		if(DEBUG >= DISPLAY_DETAILS){
			cout << "Unmap " << i << " = " << map[i] << endl;
		}
		solution->open_facilities[map[i]] = temp_open_facilities[i];
	}
	// Atualizando as atribuicoes dos clientes
	for(int j=0; j<qty_clients; j++){
		solution->assigned_facilities[j] = map[solution->assigned_facilities[j]];
	}
}


void map_and_call_TS(solutionType * solution, int qty_facilities, int qty_clients, double * costF, double ** assignment_cost, int * map, double * new_costF, double * new_costA, char * solutionName, int * temp_open_facilities){
	if(DEBUG >= DISPLAY_MOVES){
		cout << "Initial TS_MAP solution = " << solution->finalTotalCost << endl; 
		if(DEBUG >= DISPLAY_DETAILS){
			print_open_facilities(solution->open_facilities, qty_facilities);
		}
	}

	int cont_facilities = mapping(solution, qty_facilities, qty_clients, costF, assignment_cost, map, new_costF, new_costA);

	if(DEBUG >= DISPLAY_MOVES){
		cout << "Calling TS mapping with " << cont_facilities << " open facilities" << endl;
	}

	call_tabu_search(solution, solutionName, cont_facilities, qty_clients, new_costF, new_costA, *solution);

	unmapping(solution, cont_facilities, qty_facilities, qty_clients, map, temp_open_facilities);

	if(DEBUG >= DISPLAY_MOVES){
		cout << "Final TS_MAP solution = " << solution->finalTotalCost << endl; 
		if(DEBUG >= DISPLAY_DETAILS){
			print_open_facilities(solution->open_facilities, qty_facilities);
		}
	}
}

