#include <iostream>
#include <ctime>
#include <fstream>
#include "definitions.hpp"

using namespace std;

#define DEBUG 2 // OPCOES DE DEBUG: 1 - MOSTRAR A QTD DE MOVIMENTOS, 2 PARA EXIBIR OS MOVIMENTOS REALIZADOS, 3 PARA EXIBIR ACOES, 4 PARA EXIBIR DETALHES DAS ACOES, 5 PARA EXIBIR TEMPO, 6 PARA EXIBIR AS MUDANÇAS NO GRAFO


// Retornar o valor da solucao
solutionType memetic(char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA){

	cout << fixed;
   	cout.precision(5);

   	// Semente dos numeros aleatorios
	srand(0);

	// Declaracao variaveis que indicam o tempo da funcao
	struct timespec start, finish;


	// INICIANDO A CONTAGEM DE TEMPO DA FUNCAO
	clock_gettime(CLOCK_REALTIME, &start);

	solutionType solution;

	// FUTURO: SALVAR LOG E LOG_DETAILS


	// Declaracao dos nós da arvore. Cada nó é uma solucao
	solutionType **nodes = (solutionType**) malloc((QTY_NODES_TREE) * sizeof(solutionType*));

	for(int i = 0; i < QTY_NODES_TREE; i++) {
		nodes[i] = (solutionType *)malloc((QTY_POCKETS_NODE + 1) * sizeof(solutionType));
	}

	// best_pocket_node[i] Guarda o indice (de 0 a QTY_POCKETS_NODE-1) do node com melhor valor dentro daquele nó i (de 0 a QTY_NODES_TREE -1)
	int *best_pocket_node = (int*) malloc((QTY_NODES_TREE) * sizeof(int));

	// worst_pocket_node[i] Guarda o indice (de 0 a QTY_POCKETS_NODE-1) do node com pior valor dentro daquele nó i (de 0 a QTY_NODES_TREE -1)
	int *worst_pocket_node = (int*) malloc((QTY_NODES_TREE) * sizeof(int));

    if((!nodes)||(!best_pocket_node)||(!worst_pocket_node)){
        cout << "Memory Allocation Failed";
        exit(1);
    }


	// Alocando memoria para as instalacoes abertas e instalacoes atribuidas de cada cliente
	for(int i=0;i<QTY_NODES_TREE;i++){
		// Inicialmente o melhor e o pior pocket de todos os nós será da posiçao 0, pois só ele estará inicializado
		best_pocket_node[i] = 0;
		worst_pocket_node[i] = 0;

		for(int j=0;j< QTY_POCKETS_NODE+1; j++){

			// indica as instalacoes abertas atualmente em cada solucao
			nodes[i][j].open_facilities = (int*) malloc((qty_facilities) * sizeof(int));

		    // indicara as instalacoes atribuidas a cada cliente na resposta
		    nodes[i][j].assigned_facilities = (int*) malloc((qty_clients) * sizeof(int));

		    if((!nodes[i][j].open_facilities)||(!nodes[i][j].assigned_facilities)){
		        cout << "Memory Allocation Failed";
		        exit(1);
		    }

		    // Iniciando finalTotalCost com -1 para indicar que está vazio
		    nodes[i][j].finalTotalCost = -1;
		}
	}


	// assignment_cost - vetor que indica o custo de atribuicao de cada cliente para cada instalacao
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


	// Quantidade de instalacoes que sofrerão mutação quando um filho for gerado
	const int QTY_INST_MUTATION = qty_facilities * MUTATION_RATE; 

	// Quantidade de pais que existem (quantidade de sub arvores)
	const int QTY_SUBS = (QTY_NODES_TREE - 1) / QTY_CHILDREN;

	// Auxiliar para obtencao de index, principalmente de filhos
	int index_child;

	// Auxiliares para escolha do pocket da mae e do filho no crossover
	int pocket_mother, pocket_child;

	// Indica quantos pockets estao preenchidos em cada nós
	int used_pockets = 0;

	// indica a quantidade de gerações já feitas
	int qty_generations = 0;

	// indica a quantidade de gerações sem melhora
	int qty_gen_no_improv = 0;

	// marca o melhor custo até agora (finalTotalCost do node [0][0])
	double bestTotalCost = 0;

	if(DEBUG >= DISPLAY_MOVES){
		cout << "Initializing population" << endl;
	}

	// Inicializar as soluções iniciais - os 13 nós apenas 1 pocket
	set_initial_sol_G(&nodes[1][0], qty_facilities, qty_clients, costF, costA); // solucao com greedy
	call_local_search(&nodes[0][0], solutionName, qty_facilities, qty_clients, costF, costA, nodes[1][0]); // solucao com local search com solucao inicial do greedy

	if(DEBUG >= DISPLAY_MOVES){
		cout << "LS_G: ";
		cout << qty_open_facilities(nodes[0][0].open_facilities, qty_facilities) << " open facilities" << endl;	
		cout << "GREEDY: ";
		cout << qty_open_facilities(nodes[1][0].open_facilities, qty_facilities) << " open facilities" << endl;
	}


	if(DEBUG >= DISPLAY_DETAILS){
		cout << "LS_G - node[0][0]: " << nodes[0][0].finalTotalCost << endl;
		cout << "Greedy - node[1][0]: " << nodes[1][0].finalTotalCost << endl << endl;
	}

	for(int i=2;i<QTY_NODES_TREE;i++){
		set_initial_sol_RANDOM(&nodes[i][0], qty_facilities, qty_clients, costF, assignment_cost, sorted_cijID); // 11 solucoes aleatorias com sementes de 0 a 10
		
		if(DEBUG >= DISPLAY_DETAILS){
			cout << "Random - node[" << i << "][0]:" << nodes[i][0].finalTotalCost << endl;
		}

		// Roda LS completo para todas as solucoes geradas com random
		call_local_search(&nodes[i][0], solutionName, qty_facilities, qty_clients, costF, costA, nodes[i][0]); 

		if(DEBUG >= DISPLAY_DETAILS){
			cout << "LS_R -> node[" << i << "][0]:" << nodes[i][0].finalTotalCost << endl << endl;
		}

		if(DEBUG >= DISPLAY_MOVES){
			cout << "RANDOM after LS: ";
			cout << qty_open_facilities(nodes[i][0].open_facilities, qty_facilities) << " open facilities" << endl;
		}
	}

	if(DEBUG >= DISPLAY_MOVES){
		// Imprimindo a quantidade de vezes que cada inst estava aberta nessas solucoes iniciais
		print_count_open_facilities(nodes, 0, qty_facilities, used_pockets);
	}

	// Aumentando a contagem de pockets utilizados
	used_pockets += 1;

	if(DEBUG >= DISPLAY_MOVES){
		cout << endl << "Initial tree:";
		print_tree_best(nodes, best_pocket_node);
	}

	// A partir daqui entrará no loop de restart

	// Levando as melhores solucoes para cima -> Update Population
	update_population(nodes, best_pocket_node, worst_pocket_node, used_pockets, QTY_SUBS);
	
	if(DEBUG >= DISPLAY_MOVES){
		cout << endl << "Tree after updating population:";
		print_tree_best(nodes, best_pocket_node);

		if(DEBUG >= DISPLAY_DETAILS){
			print_tree_pockets(nodes, false);
		}
	}

	// Iniciando o melhor custo encontrado ate agora
	bestTotalCost = nodes[0][0].finalTotalCost;

	// A partir daqui estará em um loop até um número de iterações sem melhora for atingido
	while(qty_gen_no_improv < MAX_GEN_NO_IMPROVEMENT){

		if(DEBUG >= DISPLAY_MOVES){
			cout << endl << "------------------------------ NEXT GENERATION " << qty_generations << " ------------------------------" << endl << endl;
		}

		// crossover para cada par mãe e filho, seguido da mutacao de cada child gerado e reatribuicao de todos os clientes e por fim late acceptance em todos os filhos gerados
		// (o pocket será escolhido de forma aleatoria em cada nó)
		for(int id_parent = 0; id_parent < QTY_SUBS; id_parent++){ // para cada mãe
			if(DEBUG >= DISPLAY_ACTIONS){
				cout << "------ MOTHER " << id_parent << "-------" << endl;
			}
			for(int i=0; i < QTY_CHILDREN; i++){ // Para cada filho dessa mae

				index_child = id_parent * 3 + i + 1; // encontra o indice correto do filho

				if(DEBUG >= DISPLAY_ACTIONS){
					cout << "------ CHILD " << index_child << "-------" << endl;
				}

				if(id_parent == 0){ // Se for a raiz, só existe um pocket, entao escolhe ele
					pocket_mother = 0;
				}
				else{ // senao, sorteia o pocket da mae
					pocket_mother = rand() % (used_pockets); // Generate a random number between 0 and used_pockets - 1		
				}

				// sorteia o pocket do filho
				pocket_child = rand() % (used_pockets); // Generate a random number between 0 and used_pockets - 1


				if(DEBUG >= DISPLAY_DETAILS){
					cout << "Pockets: mother " << pocket_mother << " child " << pocket_child << endl;
				}

				crossover_mutation(&nodes[index_child][INDEX_CURRENT], nodes[id_parent][pocket_mother], nodes[index_child][pocket_child], qty_facilities, QTY_INST_MUTATION, qty_clients, sorted_cijID, costF, assignment_cost, 2);

				// LA em cada filho gerado
				call_late_acceptance(&nodes[index_child][INDEX_CURRENT], solutionName, qty_facilities, qty_clients, costF, costA, nodes[index_child][INDEX_CURRENT]);

				if(DEBUG >= DISPLAY_ACTIONS){
					// cout << "Child " << 1 < " after late acceptance: ";			
					cout << "Child after late acceptance: ";
					print_individual(nodes[index_child][INDEX_CURRENT].open_facilities, qty_facilities);
				}
			}
		} 

		if(DEBUG >= DISPLAY_MOVES){
			cout << endl << "POCKETS AFTER CROSSOVER, MUTATION AND LA" << endl;
			print_tree_pockets(nodes, true);

			if(DEBUG >= DISPLAY_DETAILS){
				print_tree_best(nodes, best_pocket_node);
			}
		}

		// Verificar se os currents entrarao no refSet ou nao e ja atualizar best_pocket_node e worst_pocket_node
		if(used_pockets < QTY_POCKETS_NODE){ // se ainda tiver pocket vazio, entra
			for(int i = 1; i < QTY_NODES_TREE; i++){ // para todos os nós, exceto a raiz
				nodes[i][used_pockets] = nodes[i][INDEX_CURRENT];

				// Atualiza o best_pocket_node
				if(nodes[i][used_pockets].finalTotalCost < nodes[i][best_pocket_node[i]].finalTotalCost){ // se esse current que entrou no refset é melhor que o melhor salvo, atualiza
					best_pocket_node[i] = used_pockets;
				}
			}
			used_pockets += 1;
		}
		else{ // senao, se todos os pockets estiverem ocupados, verifica se o current é melhor que o pior
			for(int i = 1; i < QTY_NODES_TREE; i++){ // para todos os nós, exceto a raiz
				if(nodes[i][INDEX_CURRENT].finalTotalCost < nodes[i][worst_pocket_node[i]].finalTotalCost){ // se o current for melhor que o pior, atualiza
					nodes[i][worst_pocket_node[i]] = nodes[i][INDEX_CURRENT];

					// Atualiza o best_pocket_node
					if(nodes[i][worst_pocket_node[i]].finalTotalCost < nodes[i][best_pocket_node[i]].finalTotalCost){ // se esse current que entrou no refset é melhor que o melhor salvo, atualiza
						best_pocket_node[i] = worst_pocket_node[i];
					}

					// Atualizar o worst_pocket_node
					for(int j=0; j < QTY_POCKETS_NODE; j++){ // percorre por todos os pockets utilizados
						if(nodes[i][j].finalTotalCost > nodes[i][worst_pocket_node[i]].finalTotalCost){ // se encontrou um pior, atualiza 
							worst_pocket_node[i] = j;
						}
					}
				}
			}
		}


		if(DEBUG >= DISPLAY_MOVES){
			cout << endl << "AFTER UPDATING REFSET";
			print_tree_best(nodes, best_pocket_node);

			if(DEBUG >= DISPLAY_DETAILS){
				print_tree_pockets(nodes, true);
			}
		}

		
		// Levando as melhores solucoes para cima -> Update Population
		update_population(nodes, best_pocket_node, worst_pocket_node, used_pockets, QTY_SUBS);
		
		if(DEBUG >= DISPLAY_MOVES){
			cout << endl << "AFTER UPDATING POPULATION";
			print_tree_best(nodes, best_pocket_node);

			if(DEBUG >= DISPLAY_DETAILS){
				print_tree_pockets(nodes, false);
			}
		}

		// Verificando se houve melhora
		if(nodes[0][0].finalTotalCost < bestTotalCost){ // se houve melhora, atualiza o bestTotal Cost e zera o qty_gen_no_improv
			bestTotalCost = nodes[0][0].finalTotalCost;
			qty_gen_no_improv = 0;

			if(DEBUG >= DISPLAY_MOVES){
				cout << "Updating the best cost found so far!!" << endl;
			}
		}
		else{ // se nao houve melhora, aumenta o contador qty_gen_no_improv
			qty_gen_no_improv += 1;
		}

		// Aumentando o contador de geracoes
		qty_generations += 1;
	}	

	// Vendo a diversidade da populacao final

	if(DEBUG >= DISPLAY_MOVES){
		cout << qty_open_facilities(nodes[0][0].open_facilities, qty_facilities) << " open facilities" << endl;

		for(int i=1;i<QTY_NODES_TREE;i++){
			for(int j=0;j<used_pockets;j++){
				cout << qty_open_facilities(nodes[i][j].open_facilities, qty_facilities) << " ";
			}
			cout << " open facilities" << endl;
		}
		
		// Imprimindo a quantidade de vezes que cada inst estava aberta nessas solucoes iniciais
		print_count_open_facilities(nodes, -1, qty_facilities, used_pockets);
	}


	// FINALIZANDO A CONTAGEM DE TEMPO DA FUNCAO
	clock_gettime(CLOCK_REALTIME, &finish);


	// Atualizando valor de solution
	solution = nodes[0][0];


	// Calculando o tempo gasto da funcao
	solution.timeSpent =  (finish.tv_sec - start.tv_sec);
	solution.timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 

	if(DEBUG >= DISPLAY_TIME){
		cout << "Final Total Function Time: " << solution.timeSpent << " seconds" << endl;
	}

	/* DESALOCAÇÃO DE MEMÓRIA */

	// desalocando todo o vetor de nodes
	for(int i = 0; i < QTY_NODES_TREE; i++) {
		// for(int j=0;j< QTY_POCKETS_NODE+1; j++){
		// 	free(nodes[i][j].open_facilities);
		// 	free(nodes[i][j].assigned_facilities);
		// }
		free(nodes[i]);
	}
	free(nodes);

	// desalocando vetor de assignment_cost e sorted_cijID
	for(int i = 0; i < qty_clients; i++) {
		free(assignment_cost[i]);
		free(sorted_cijID[i]);
	}
	free(assignment_cost);
	free(sorted_cijID);

	// Desalocando vetores best e worst pocket node
	free(best_pocket_node);
	free(worst_pocket_node);

	return(solution);
}
