#include <lemon/list_graph.h>
#include <algorithm>
#include <iostream>
#include <ctime>
#include "definitions.hpp"
#define EPSL 0.00000001

using namespace lemon;
using namespace std;

#define DISPLAY_ACTIONS 1 // corresponde a todos os cout quando uma acao é feita. 
#define DISPLAY_TIME 2 // corresponde ao calculo de tempo 
#define DISPLAY_GRAPH 3 // corresponde a descricao dos clientes, instalacoes e arcos
#define DISPLAY_MATRIX 4 // corresponde à parte final, na criacao de Tlinha

#define DEBUG 0 // OPCOES DE DEBUG: 1 PARA EXIBIR ACOES, 2 PARA EXIBIR TEMPO, 3 PARA EXIBIR AS MUDANÇAS NO GRAFO, 4 PARA EXIBIR AS MUDANCAS NA MATRIZ DE ADJACENCIA NA CRIACAO DE TLINHA


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
            tempVector[k] = vector[j];
            tempVectorID[k] = vectorID[j];
            j++;
            k++;
        }
        else {
            if (j == endPosition + 1) {
                tempVector[k] = vector[i];
                tempVectorID[k] = vectorID[i];
                i++;
                k++;
            }
            else {
                if (vector[i] < vector[j]) {
                    tempVector[k] = vector[i];
                	tempVectorID[k] = vectorID[i];
                    i++;
                    k++;
                }
                else {
                    tempVector[k] = vector[j];
                	tempVectorID[k] = vectorID[j];
                    j++;
                    k++;
                }
            }
        }

    }
    for(i = startPosition; i <= endPosition; i++) {
        vector[i] = tempVector[i - startPosition];
        vectorID[i] = tempVectorID[i - startPosition];
    }
    free(tempVector);
    free(tempVectorID);
}


void deleting_non_active_clients(double *vector, int *vectorID, int vector_size, int *clients_delete, int qty_cli_delete){
	int qty_remaining = qty_cli_delete;
	for(int i=0;i<vector_size;i++){
		if(qty_remaining == 0){
			break;
		}
		
		for(int j=0;j<qty_cli_delete;j++){
			if(vectorID[i]==clients_delete[j]){
				vector[i] = -1; // coloca cij = -1, indicando que ja foi atribuido
				qty_remaining -= 1;
				break;
			}
		}
	}

	// Colocando novamente em ordem
	mergeSort(vector, vectorID, 0, (vector_size-1));
}



// Por referencia diz qual o melhor tamanho para Y e seu respectivo custo
void best_subset(int &best_size, double &best_cost, double *vector, double fi, double gain, int start_index, int vector_size, double biggest_cij){
	int current_size = 1;
	double sum_current_cost = 0;
	double current_cost = 0;
	best_cost = fi + biggest_cij + 1; // limitante superior


	// inicializando só para nao ficar nulo se algo der errado
	best_size = 1;

	// ira criar (vector_size - start_index) subconjuntos
	for(int i=start_index;i<vector_size;i++){
		sum_current_cost = fi - gain;
		// vendo a soma dos custos desse subconjunto
		for(int j=start_index;j<=i;j++){
			sum_current_cost += vector[j];
		}

		current_cost = sum_current_cost / current_size;

		if(current_cost <= best_cost){
			best_cost = current_cost;
			best_size = current_size;
		}

		current_size += 1;
	}

}

// Retornar o valor da solucao
solutionType greedy(int qty_clients, int qty_facilities, double * costF, double * costA){

	cout << fixed;
   	cout.precision(5);

	// Struct que vai retornar a solução
	solutionType solution;
	
	solution.finalTotalCost = 0; // inicializando o custo final
	
	/* Inicio declaracoes variaveis para calculo de tempo - finalidade eh encontrar gargalos */

	// Declaracao variaveis que indicam o tempo do programa como um todo
	struct timespec real_start, real_finish;

	// Declaracao variavel que marcara o tempo de execucao da funcao como um todo
	double timeSpent;

	/* Fim declaracoes para calculo de tempo */


	if(DEBUG >= DISPLAY_TIME){
		// INICIANDO A CONTAGEM DE TEMPO DA FUNCAO COMO UM TODO
		clock_gettime(CLOCK_REALTIME, &real_start);
	}

	// indica as instalacoes finais atribuidas a cada cliente
	solution.assigned_facilities = (int*) malloc((qty_clients) * sizeof(int));
    if(!solution.assigned_facilities){
        cout << "Memory Allocation Failed";
        exit(1);
    }

	// conjunto de clientes a serem removidos de g na iteração, pois deixaram de ser ativos
	int * delete_clients;
	delete_clients = (int*) malloc((qty_clients) * sizeof(int));
    if(!delete_clients){
        cout << "Memory Allocation Failed";
        exit(1);
    }


    int qty_non_active_cli = 0; // Indica quantos clientes ja foram conectados a alguma instalacao

	
	/* Sobre os grafos bipartidos, teremos
	
	Red para clientes

	Blue para instalações

	*/
	// Grafo que liga os clientes às instalações
	ListBpGraph g;

	// c_minX - será correnpondente a c(j,X) = min_{i \in X} cij, sendo X as inst abertas
	ListBpGraph::RedNodeMap<double> c_minX(g);

	// nearest_open_fac - será correnpondente ao ID da instalacao i tal que min_{i \in X} cij, sendo X as inst abertas
	ListBpGraph::RedNodeMap<int> nearest_open_fac(g);


	// active - indica se o cliente está ativo ainda ou nao, isto é, se ele ainda nao foi conectado a nenhuma instalacao aberta. if j \in g
	ListBpGraph::RedNodeMap<bool> active(g);

	// f - será o custo de instalação (fi)
	ListBpGraph::BlueNodeMap<double> f(g);


	// sorted_cij - será os clientes ativos ordenados pelo cij àquela instalação
	double **sorted_cij = (double**) malloc((qty_facilities) * sizeof(double*));

	// sorted_cijID - indica os ID dos clientes ativos ordenados pelo cij àquela instalação
	int **sorted_cijID = (int**) malloc((qty_facilities) * sizeof(int*));

	for(int i = 0; i < qty_facilities; i++) {
		sorted_cij[i] = (double *)malloc(qty_clients * sizeof(double));
		sorted_cijID[i] = (int *)malloc(qty_clients * sizeof(int));
	}

    if((!sorted_cij)||(!sorted_cijID)){
        cout << "Memory Allocation Failed";
        exit(1);
    }

	// Indica se a instalação está aberta ou não
	ListBpGraph::BlueNodeMap<bool> open(g);

	// Para identificar cada nó
	ListBpGraph::NodeMap<int> name(g);

	// Variavel que armazena o maior valor cij dado na entrada, para uso posterior
	double biggestCij = 0;

	// Variavel que armazena o maior valor fi dado na entrada, para uso posterior
	double biggestFi = 0;

	// Criação de nós clientes e atribuição de seus labels
	ListBpGraph::RedNode * clients;
	clients = (ListBpGraph::RedNode*) malloc((qty_clients) * sizeof(ListBpGraph::RedNode));
    if(!clients){
        cout << "Memory Allocation Failed";
        exit(1);
    }

	for(int i=0;i<qty_clients;i++){
		clients[i] = g.addRedNode();
		c_minX[clients[i]] = -1; // indica que nao está conectado com ninguém inicialmente //PROBLEMA: nao tenho ctz ainda qual a melhor inicializacao
		name[clients[i]] = i; // nomeia de acordo com a numeracao
		nearest_open_fac[clients[i]] = -1; // indica que nao está conectado com ninguém inicialmente
		active[clients[i]] = true;
	}


	// Criação de nós de instalações e atribuição de seus labels
	ListBpGraph::BlueNode * facilities;
	facilities = (ListBpGraph::BlueNode*) malloc((qty_facilities) * sizeof(ListBpGraph::BlueNode));
    if(!facilities){
        cout << "Memory Allocation Failed";
        exit(1);
    }


	for(int i=0;i<qty_facilities;i++){
		facilities[i] = g.addBlueNode();
		f[facilities[i]] = costF[i]; // pegando valor vindo por parametro
		open[facilities[i]] = false; // indica que a instalação não está aberta inicialmente
		name[facilities[i]] = i; // nomeia de acordo com a numeracao

		// Salvando o valor do maior Fi da entrada
		if(f[facilities[i]]>biggestFi){
			biggestFi = f[facilities[i]];
		}
	}


	// Custo de atribuição é o custo de atribuir o cliente associado à instalação associada
	ListBpGraph::EdgeMap<double> assignment_cost(g);


	// Criação de arcos e atribuição de seus labels
	ListBpGraph::Edge * edges; // conectando todos com todos
	edges = (ListBpGraph::Edge*) malloc((qty_clients*qty_facilities) * sizeof(ListBpGraph::Edge));
    if(!edges){
        cout << "Memory Allocation Failed";
        exit(1);
    }

	int counter = 0;
	for(int i=0;i<qty_clients;i++){
		for(int j=0;j<qty_facilities;j++){
			edges[counter] = g.addEdge(clients[i],facilities[j]);
			assignment_cost[edges[counter]] = costA[counter]; // pegando valor vindo por parametro

			// Salvando o valor do maior Cij da entrada
			if(assignment_cost[edges[counter]]>biggestCij){
				biggestCij = assignment_cost[edges[counter]];
			}

			counter++;
		}
	}


	if(DEBUG >= DISPLAY_ACTIONS){
		cout << "biggestCij from input : " << biggestCij << " biggestFi from input : " << biggestFi << endl;
	}

	int qty_non_active_cli_g = qty_clients; // Indica a quantidade de clientes ainda em g, os clientes ativos.


	if (DEBUG >= DISPLAY_GRAPH){
		// Percorrendo por todos os nós A - clientes
		cout << "Scrolling through all clients" << endl;
		for(ListBpGraph::RedNodeIt n(g); n != INVALID; ++n){
			cout << "node id: " << g.id(n)  << " - name: " << name[n];
		}

		// Percorrendo por todos os nós B - instalacoes
		cout << "Scrolling through all facilities" << endl;
		for(ListBpGraph::BlueNodeIt n(g); n != INVALID; ++n){
			cout << "node id: " << g.id(n)  << " - name: " << name[n] << " - f: " << f[n] << " - open: " << open[n] << endl;
		}

		// Percorrendo por todos os arcos
		cout << "Scrolling through all edges" << endl;
		for(ListBpGraph::EdgeIt e(g); e!= INVALID; ++e){
			cout << "edge id: " << g.id(e) ;
			cout << " - client: " << name[g.u(e)] << " - facility: " << name[g.v(e)];
			cout<< " - ac: " << assignment_cost[e] << endl;
		}
	}


	// colocando para cada instalacao a ordem dos cij
	for(ListBpGraph::BlueNodeIt n(g); n != INVALID; ++n){		// percorre as instalacoes
		counter = 0;
		// colocando no vetor que sera ordenado
		for (ListBpGraph::IncEdgeIt e(g, n); e != INVALID; ++e) { // Percorre todas arestas desse nó
			sorted_cijID[g.id(n)][counter] = g.id(g.u(e)); // linha da instalacao, coluna contando em counter
			sorted_cij[g.id(n)][counter] = assignment_cost[e]; // linha da instalacao, coluna contando em counter
			counter += 1;
		}

		// ordenando o vetor
		mergeSort(sorted_cij[g.id(n)], sorted_cijID[g.id(n)], 0, (counter-1));

		if (DEBUG >= DISPLAY_ACTIONS){
			cout << "sorted to facility " << g.id(n) << endl;
			for(int i=0;i<counter;i++){
				cout << sorted_cij[g.id(n)][i] << endl; // Adicionar o cliente j no conjunto de 
			}
			cout << "-------" << endl;
		}

	}


	int id_chosen_fac;
	double best_cost_chosen;
	int best_size_chosen;

	int current_Y_size;
	double current_Y_cost;

	double gain_cij;


	// ****** A partir daqui deve estar em um loop até nao ter mais clientes ativos:

	while(qty_non_active_cli_g > 0){

		if(DEBUG >= DISPLAY_ACTIONS){
			cout << endl << "------------------------------ STILL HAVE " << qty_non_active_cli_g << " ACTIVE CLIENTS ------------------------------" << endl << endl;
		}

		best_cost_chosen = biggestCij * qty_clients + biggestFi + 1; //limitante superior tranquilo 
		best_size_chosen = -1;
		id_chosen_fac = -1;
		
		for(ListBpGraph::BlueNodeIt n(g); n != INVALID; ++n){		// percorre as instalacoes
			// Calculando gain de troca de atribuições
			// \sum_{j notin g} (c(j,X), cij)+
			gain_cij = 0;
			
			for (ListBpGraph::IncEdgeIt e(g, n); e != INVALID; ++e) { // Percorre todas arestas desse nó
				if(!active[g.asRedNode(g.u(e))]){
					if(c_minX[g.asRedNode(g.u(e))] > assignment_cost[e]){
						gain_cij += c_minX[g.asRedNode(g.u(e))] - assignment_cost[e];
					}
				}
			}
			// cout << "fac = " << g.id(n) << " -- gain: " << gain_cij << endl;
			best_subset(current_Y_size, current_Y_cost, sorted_cij[g.id(n)], f[n], gain_cij, qty_non_active_cli, qty_clients, biggestCij);

			// Atualizando valores do melhor custo
			if(current_Y_cost < best_cost_chosen){
				best_cost_chosen = current_Y_cost;
				best_size_chosen = current_Y_size;
				id_chosen_fac = g.id(n);
			}

		}

		// cout << "Let's open fac: " << id_chosen_fac << " and assign to " << best_size_chosen << " clients with cost: " << best_cost_chosen << endl;
		solution.finalTotalCost += f[facilities[id_chosen_fac]]; // Somando os custos de abertura das instalacoes escolhidas para abrir

		// fi <- 0
		f[facilities[id_chosen_fac]] = 0;


		// g <- g - Y

		// criando o conjunto de clientes que serao apagados. inicia no primeiro ativo ate o tamanho escolhido na iteracao.
		for(int i = qty_non_active_cli; i < qty_non_active_cli + best_size_chosen; i++){
			delete_clients[i - qty_non_active_cli] = sorted_cijID[id_chosen_fac][i]; // delete_clients inicia na posicao 0.
		}

		qty_non_active_cli += best_size_chosen;
		qty_non_active_cli_g -= best_size_chosen;

		// Alterando o valor correspondente em sorted_cij e sorted_cijID para -1 em todas as instalacoes
		for(ListBpGraph::BlueNodeIt n(g); n != INVALID; ++n){		// percorre as instalacoes
			deleting_non_active_clients(sorted_cij[g.id(n)], sorted_cijID[g.id(n)], qty_clients, delete_clients, best_size_chosen);
		}

		// Atualizando o valor de c(j,X) para os clientes que ja nao eram ativos
		for(ListBpGraph::RedNodeIt n(g); n != INVALID; ++n){		// percorre os clientes
			if(!active[n]){
				if(c_minX[n] > assignment_cost[findEdge(g, n, facilities[id_chosen_fac])]){
					c_minX[n] = assignment_cost[findEdge(g, n, facilities[id_chosen_fac])];
					nearest_open_fac[n] = id_chosen_fac; // nao decidi ainda se aqui vai ser o ID ou o name

					// cout << "For ID = " << g.id(n) << " and fac: " << name[facilities[id_chosen_fac]] << endl;
					// cout << "Updating here: " << c_minX[n] << endl;
				}
			}
		}

		// Atualizando o valor de c(j,X) para todos os clientes agora atribuidos a i
		for(int i=0;i<best_size_chosen;i++){
			c_minX[clients[delete_clients[i]]] = assignment_cost[findEdge(g, clients[delete_clients[i]], facilities[id_chosen_fac])];
			nearest_open_fac[clients[delete_clients[i]]] = id_chosen_fac; // nao decidi ainda se aqui vai ser o ID ou o name
			active[clients[delete_clients[i]]] = false;
			// cout << "For ID = " << delete_clients[i] << " and fac: " << name[facilities[id_chosen_fac]] << endl;
			// cout << "we have cij = " << assignment_cost[findEdge(g, clients[delete_clients[i]], facilities[id_chosen_fac])] << endl;
			// cout << "Updating here: " << c_minX[clients[delete_clients[i]]] << " and id= " << nearest_open_fac[clients[delete_clients[i]]] << endl;
		}

	}

	// Somando os custos de conexao dos clientes a instalacao mais proxima aberta
	counter = 0;
	for(ListBpGraph::RedNodeIt n(g); n != INVALID; ++n){		// percorre os clientes
		solution.finalTotalCost += assignment_cost[findEdge(g, n, facilities[nearest_open_fac[n]])];

		// Colocando as instalacoes abertas mais proximas em um vetor pra retornar na solucao
		solution.assigned_facilities[counter] = nearest_open_fac[clients[counter]]; // esse loop percorre todos os clientes, então posso usar o counter normalmente aqui
		// cout << "client " << counter << " with fac " << solution.assigned_facilities[counter] << endl;

		counter += 1;
	}

	if(DEBUG >= DISPLAY_ACTIONS){
		cout << "FINAL TOTAL COST: " << solution.finalTotalCost << endl;
	}

	if(DEBUG >= DISPLAY_TIME){

		// FINALIZANDO A CONTAGEM DE TEMPO DA FUNCAO COMO UM TODO
		clock_gettime(CLOCK_REALTIME, &real_finish);

		// Calculando o tempo gasto total
		timeSpent =  (real_finish.tv_sec - real_start.tv_sec);
		timeSpent += (real_finish.tv_nsec - real_start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
		cout << "Final Total Function Time: " << timeSpent << " seconds" << endl;
	}

	free(clients);
	free(facilities);
	free(edges);

	return(solution);
}
