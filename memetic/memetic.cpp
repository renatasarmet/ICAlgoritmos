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
	struct timespec start, finish, start_part, finish_part;

	double timeSpent;


	// INICIANDO A CONTAGEM DE TEMPO DA FUNCAO
	clock_gettime(CLOCK_REALTIME, &start);

	solutionType solution;

	// FUTURO: SALVAR LOG E LOG_DETAILS


	// Declaracao dos nós da arvore. Cada nó é um agente, que possui 1 pocket e 1 current
	solutionType **nodes = (solutionType**) malloc((QTY_NODES_TREE) * sizeof(solutionType*));

	for(int i = 0; i < QTY_NODES_TREE; i++) {
		nodes[i] = (solutionType *)malloc((QTY_SOLUTIONS_NODE) * sizeof(solutionType));
	}


    if(!nodes){
        cout << "Memory Allocation Failed";
        exit(1);
    }


	// Alocando memoria para as instalacoes abertas e instalacoes atribuidas de cada cliente
	for(int i=0;i<QTY_NODES_TREE;i++){

		for(int j=0;j< QTY_SOLUTIONS_NODE; j++){

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

	// indica a quantidade de gerações já feitas
	int qty_generations = 0;

	// indica a quantidade de gerações sem melhora
	int qty_gen_no_improv = 0;

	// indica a quantidade vezes que mudou o root seguida
	int qty_changes_root = 0;

	// Auxiliares para identificação dos filhos no crossover
	int s1, s2, s3;

	// Auxiliar para inversao
	solutionType aux;

	// indica o melhor custo encontrado com aquele root (se atualizou o root, o valor nao fica com o antigo)
	double tempBestTotalCost;

	if(DEBUG >= DISPLAY_MOVES){
		cout << "Initializing population" << endl;
	}

	// VERIFICAR COMO VOU FAZER A GERACAO DA POPULACAO INICIAL

	// Iniciando os nós 0 e 1, pocket 0

	clock_gettime(CLOCK_REALTIME, &start_part);

	set_initial_sol_G(&nodes[1][INDEX_POCKET], qty_facilities, qty_clients, costF, costA); // solucao com greedy

	clock_gettime(CLOCK_REALTIME, &finish_part);

	// Calculando o tempo gasto até agora
	timeSpent =  (finish_part.tv_sec - start_part.tv_sec);
	timeSpent += (finish_part.tv_nsec - start_part.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 

	cout << "Time generate greedy: " << timeSpent << " seconds" << endl;

	clock_gettime(CLOCK_REALTIME, &start_part);

	call_local_search(&nodes[0][INDEX_POCKET], solutionName, qty_facilities, qty_clients, costF, costA, nodes[1][INDEX_POCKET]); // solucao com local search com solucao inicial do greedy
	// call_late_acceptance(&nodes[0][INDEX_POCKET], solutionName, qty_facilities, qty_clients, costF, costA, nodes[1][INDEX_POCKET], false); // solucao com late acceptance com solucao inicial do greedy

	clock_gettime(CLOCK_REALTIME, &finish_part);

	// Calculando o tempo gasto até agora
	timeSpent =  (finish_part.tv_sec - start_part.tv_sec);
	timeSpent += (finish_part.tv_nsec - start_part.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 

	cout << "Time LS greedy: " << timeSpent << " seconds" << endl;

	if(DEBUG >= DISPLAY_MOVES){
		cout << "LS_G: ";
		cout << qty_open_facilities(nodes[0][INDEX_POCKET].open_facilities, qty_facilities) << " open facilities" << endl;	
		cout << "GREEDY: ";
		cout << qty_open_facilities(nodes[1][INDEX_POCKET].open_facilities, qty_facilities) << " open facilities" << endl;
	}

	if(DEBUG >= DISPLAY_DETAILS){
		cout << "LS_G - node[0][0]: " << nodes[0][INDEX_POCKET].finalTotalCost << endl;
		cout << "Greedy - node[1][0]: " << nodes[1][INDEX_POCKET].finalTotalCost << endl << endl;
	}

	// Inicial todos os pockets de todos os nós, com solução aleatória

	// Preenchendo os currents dos nós 0 e 1
	for(int j=0;j<2;j++){ // para os nós 0 e 1

		clock_gettime(CLOCK_REALTIME, &start_part);

		set_initial_sol_RANDOM(&nodes[j][INDEX_CURRENT], qty_facilities, qty_clients, costF, assignment_cost, sorted_cijID); // preenche o current

		clock_gettime(CLOCK_REALTIME, &finish_part);

		// Calculando o tempo gasto até agora
		timeSpent =  (finish_part.tv_sec - start_part.tv_sec);
		timeSpent += (finish_part.tv_nsec - start_part.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 

		cout << "Time Random: " << timeSpent << " seconds" << endl;
		
		if(DEBUG >= DISPLAY_DETAILS){
			cout << "Random - node[" << j << "] << [" << INDEX_CURRENT << "]:" << nodes[j][INDEX_CURRENT].finalTotalCost << endl;
		}

		if(LA_INITIAL_POP){
			clock_gettime(CLOCK_REALTIME, &start_part);

			// Roda LA completo para todas as solucoes geradas com random
			call_late_acceptance(&nodes[j][INDEX_CURRENT], solutionName, qty_facilities, qty_clients, costF, costA, nodes[j][INDEX_CURRENT], false); 

			clock_gettime(CLOCK_REALTIME, &finish_part);

			// Calculando o tempo gasto até agora
			timeSpent =  (finish_part.tv_sec - start_part.tv_sec);
			timeSpent += (finish_part.tv_nsec - start_part.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 

			cout << "Time LA random: " << timeSpent << " seconds" << endl;

			if(DEBUG >= DISPLAY_DETAILS){
				cout << "LS_R -> node[1][" << j << "]:" <<nodes[j][INDEX_CURRENT].finalTotalCost << endl << endl;
			}

			if(DEBUG >= DISPLAY_MOVES){
				cout << "RANDOM after LA: ";
				cout << qty_open_facilities(nodes[j][INDEX_CURRENT].open_facilities, qty_facilities) << " open facilities" << endl;
			}
		}
	}

	// Preenchendo os pockets e o currents do restante dos nós (de 2 a 12)
	for(int i=2;i<QTY_NODES_TREE;i++){ // para todos os nós a partir de 2

		if(DEBUG >= DISPLAY_MOVES){
			cout << "Node " << i << endl;
		}

		for(int j=0;j<QTY_SOLUTIONS_NODE;j++){ // para o pocket e current

			set_initial_sol_RANDOM(&nodes[i][j], qty_facilities, qty_clients, costF, assignment_cost, sorted_cijID); 
			
			if(DEBUG >= DISPLAY_DETAILS){
				cout << "Random - node[" << i << "][" << j << "]:" << nodes[i][j].finalTotalCost << endl;
			}

			if(LA_INITIAL_POP){
				// Roda LA completo para todas as solucoes geradas com random
				call_late_acceptance(&nodes[i][j], solutionName, qty_facilities, qty_clients, costF, costA, nodes[i][j], false); 

				if(DEBUG >= DISPLAY_DETAILS){
					cout << "LS_R -> node[" << i << "][" << j << "]:" <<nodes[i][j].finalTotalCost << endl << endl;
				}

				if(DEBUG >= DISPLAY_MOVES){
					cout << "RANDOM after LA: ";
					cout << qty_open_facilities(nodes[i][j].open_facilities, qty_facilities) << " open facilities" << endl;
				}
			}
		}
	}


	if(DEBUG >= DISPLAY_MOVES){
		// Imprimindo a quantidade de vezes que cada inst estava aberta nessas solucoes iniciais
		print_count_open_facilities(nodes, qty_facilities);
	}


	if(DEBUG >= DISPLAY_MOVES){
		cout << endl << "Initial tree:";
		print_tree_best(nodes);
	}

	// A partir daqui entrará no loop de restart

	// Levando as melhores solucoes para cima -> Update Population
	update_population(nodes, QTY_SUBS, solutionName, qty_facilities, qty_clients, costF, costA);
	

	if(DEBUG >= DISPLAY_MOVES){
		cout << endl << "Tree after updating population:";
		print_tree_best(nodes);

		if(DEBUG >= DISPLAY_DETAILS){
			print_tree(nodes);
		}
	}

	// Iniciando o melhor custo encontrado ate agora
	solution = nodes[0][INDEX_POCKET];

	// Atualiza o best para o valor desse root
	tempBestTotalCost = nodes[0][INDEX_POCKET].finalTotalCost;

	// A partir daqui estará em um loop até um número de iterações sem melhora for atingido
	while(qty_changes_root < MAX_CHANGES_ROOT){

		if(DEBUG >= DISPLAY_MOVES){
			cout << endl << "------------------------------ NEXT GENERATION " << qty_generations << " ------------------------------" << endl << endl;
		}

		// Se atingiu MAX_GEN_NO_IMPROVEMENT geracoes sem melhora, armazena a root em solution se ela for melhor que a que está lá e sobe a próxima para a root
		if(qty_gen_no_improv > MAX_GEN_NO_IMPROVEMENT){
			qty_gen_no_improv = 0; // zera o contador
			change_root(nodes, &solution, &qty_changes_root, solutionName, qty_facilities, qty_clients, costF, costA, assignment_cost, sorted_cijID);
		
			// Atualiza o best para o valor desse root
			tempBestTotalCost = nodes[0][INDEX_POCKET].finalTotalCost;
		}


		// crossover 
		// Combinacoes entre cada mãe e filhos,
		// seguido da mutacao de cada child gerado e reatribuicao de todos os clientes e por fim local search close facilities em todos os filhos gerados
		for(int id_parent = 0; id_parent < QTY_SUBS; id_parent++){ // para cada mãe
			if(DEBUG >= DISPLAY_ACTIONS){
				cout << "------ PARENT " << id_parent << "-------" << endl;
			}

			// encontra o indice correto dos filhos
			s1 = id_parent * 3 + 1;
			s2 = id_parent * 3 + 2;
			s3 = id_parent * 3 + 3;


			// current[id_parent] = Recombine(pocket[id_parent], current[s1])

			if(DEBUG >= DISPLAY_ACTIONS){
				cout << "current[" << id_parent << "] = Recombine(pocket[" << id_parent << "], current[" << s1 << "]) " << endl;
			}

			recombine(&nodes[id_parent][INDEX_CURRENT], nodes[id_parent][INDEX_POCKET], nodes[s1][INDEX_CURRENT], qty_facilities, QTY_INST_MUTATION, qty_clients, sorted_cijID, costF, costA, assignment_cost, solutionName);


			// current[s3] = Recombine(pocket[s3], current[id_parent])

			if(DEBUG >= DISPLAY_ACTIONS){
				cout << "current[" << s3 << "] = Recombine(pocket[" << s3 << "], current[" << id_parent << "]) " << endl;
			}

			recombine(&nodes[s3][INDEX_CURRENT], nodes[s3][INDEX_POCKET], nodes[id_parent][INDEX_CURRENT], qty_facilities, QTY_INST_MUTATION, qty_clients, sorted_cijID, costF, costA, assignment_cost, solutionName);


			// current[s1] = Recombine(pocket[s1], current[s2])

			if(DEBUG >= DISPLAY_ACTIONS){
				cout << "current[" << s1 << "] = Recombine(pocket[" << s1 << "], current[" << s2 << "]) " << endl;
			}

			recombine(&nodes[s1][INDEX_CURRENT], nodes[s1][INDEX_POCKET], nodes[s2][INDEX_CURRENT], qty_facilities, QTY_INST_MUTATION, qty_clients, sorted_cijID, costF, costA, assignment_cost, solutionName);


			// current[s2] = Recombine(pocket[s2], current[s3])

			if(DEBUG >= DISPLAY_ACTIONS){
				cout << "current[" << s2 << "] = Recombine(pocket[" << s2 << "], current[" << s3 << "]) " << endl;
			}

			recombine(&nodes[s2][INDEX_CURRENT], nodes[s2][INDEX_POCKET], nodes[s3][INDEX_CURRENT], qty_facilities, QTY_INST_MUTATION, qty_clients, sorted_cijID, costF, costA, assignment_cost, solutionName);

		} 


		if(DEBUG >= DISPLAY_MOVES){
			cout << endl << "TREE AFTER CROSSOVER, MUTATION AND LS_N0" << endl;
			print_tree(nodes);

			if(DEBUG >= DISPLAY_DETAILS){
				print_tree_best(nodes);
			}
		}


		if(CHECK_DIVERSITY){
			// Primeiro verificar se o filho for igual a alguem do pocket, vai tentar fazer umas mutações nele // PROBLEMA: e se mesmo assim ficou igual? Por enquanto ele é colocado de qualquer forma
			for(int i = 1; i < QTY_NODES_TREE; i++){ // para todos os nós, exceto a raiz

				if(!are_different(nodes[i][INDEX_POCKET], nodes[i][INDEX_CURRENT], qty_facilities)){

					mutation(&nodes[i][INDEX_CURRENT], qty_facilities, QTY_INST_MUTATION); // sofre mutação

					// Conectando cada cliente com a instalacao aberta mais proxima e fechando as instalacoes que nao foram conectadas a ninguem. Custo final também é atualizado
					connect_and_update_facilities(&nodes[i][INDEX_CURRENT], qty_facilities, qty_clients, sorted_cijID, costF, assignment_cost);

					// Roda o late_acceptance simples, como local search
					call_late_acceptance(&nodes[i][INDEX_CURRENT], solutionName, qty_facilities, qty_clients, costF, costA, nodes[i][INDEX_CURRENT], true);
			
					if(DEBUG >= DISPLAY_MOVES){
						cout << "It was equal to pockets! Node (" << i << ") Final cost after mutation an LS: " << nodes[i][INDEX_CURRENT].finalTotalCost << endl;

						if(DEBUG >= DISPLAY_ACTIONS){
							cout << "It was equal pockets. Child after simple local search: ";
							print_individual(nodes[i][INDEX_CURRENT].open_facilities, qty_facilities);
						}
					}
				}
			}
		}
		

		// Verificar se os currents entrarao no refSet ou nao
		// verifica se o current é melhor que pocket
		for(int i = 0; i < QTY_NODES_TREE; i++){ // para todos os nós

			if(nodes[i][INDEX_CURRENT].finalTotalCost < nodes[i][INDEX_POCKET].finalTotalCost){ // se o current for melhor que o pocket, atualiza
				// Inverte
				aux = nodes[i][INDEX_POCKET];
				nodes[i][INDEX_POCKET] = nodes[i][INDEX_CURRENT];
				nodes[i][INDEX_CURRENT] = aux;
			}
		}


		if(DEBUG >= DISPLAY_MOVES){
			cout << endl << "AFTER UPDATING REFSET";
			print_tree_best(nodes);

			if(DEBUG >= DISPLAY_DETAILS){
				print_tree(nodes);
			}
		}

		
		// Levando as melhores solucoes para cima -> Update Population
		update_population(nodes, QTY_SUBS, solutionName, qty_facilities, qty_clients, costF, costA);
		
		if(DEBUG >= DISPLAY_MOVES){
			cout << endl << "AFTER UPDATING POPULATION";
			print_tree_best(nodes);

			if(DEBUG >= DISPLAY_DETAILS){
				print_tree(nodes);
			}
		}

		// Verificando se houve melhora
		if(nodes[0][INDEX_POCKET].finalTotalCost < tempBestTotalCost){ // se houve melhora, atualiza o tempBestTotalCost e zera o qty_gen_no_improv
			tempBestTotalCost = nodes[0][INDEX_POCKET].finalTotalCost;
			qty_gen_no_improv = 0;

			if(DEBUG >= DISPLAY_MOVES){
				cout << "Updating the best cost in this tree!!" << endl;
			}
			if(nodes[0][INDEX_POCKET].finalTotalCost < solution.finalTotalCost){ // se houve melhora na melhor solução encontrada até agora
				qty_changes_root = 0; // zera a quantidade de mudanças para garantir que vai continuar a busca, pois sabe que logo vai alterar a raiz e zerar de qualquer forma

				if(DEBUG >= DISPLAY_MOVES){
					cout << "Also it is the best cost found so far!!" << endl;
				}
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
		for(int i=0;i<QTY_NODES_TREE;i++){
			for(int j=0;j<QTY_SOLUTIONS_NODE;j++){
				cout << qty_open_facilities(nodes[i][j].open_facilities, qty_facilities) << " ";
			}
			cout << " open facilities" << endl;
		}
		
		// Imprimindo a quantidade de vezes que cada inst estava aberta nessas solucoes
		print_count_open_facilities(nodes, qty_facilities);
	}


	// FINALIZANDO A CONTAGEM DE TEMPO DA FUNCAO
	clock_gettime(CLOCK_REALTIME, &finish);


	// Atualizando valor de solution
	if(nodes[0][INDEX_POCKET].finalTotalCost < solution.finalTotalCost){
		solution = nodes[0][INDEX_POCKET];
	}

	// Calculando o tempo gasto da funcao
	solution.timeSpent =  (finish.tv_sec - start.tv_sec);
	solution.timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 

	if(DEBUG >= DISPLAY_TIME){
		cout << "Final Total Function Time: " << solution.timeSpent << " seconds" << endl;
	}

	/* DESALOCAÇÃO DE MEMÓRIA */

	// desalocando todo o vetor de nodes
	for(int i = 0; i < QTY_NODES_TREE; i++) {
		// for(int j=0;j< QTY_SOLUTIONS_NODE+1; j++){
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

	return(solution);
}
