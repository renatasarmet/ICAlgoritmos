#include <lemon/list_graph.h>
#include <algorithm>
#include <iostream>
#include <set>
#include <iterator>
#include <ctime>
#include <fstream>
#include <cstring>
#include <cfloat>
#include "definitions.hpp"
#define EPSL 0.00000001

using namespace lemon;
using namespace std;

#define DISPLAY_BASIC 1 // corresponde a exibicao da quantidade de movimentos
#define DISPLAY_MOVES 2 // corresponde a todos os cout quando um movimento é realizado de fato
#define DISPLAY_ACTIONS 3 // corresponde a todos os cout quando uma acao é feita. 
#define DISPLAY_DETAILS 4 // corresponde a todos os cout mais detalhados quando uma acao é feita. 
#define DISPLAY_TIME 5 // corresponde aos calculos de tempo 
#define DISPLAY_GRAPH 6 // corresponde a descricao dos clientes, instalacoes e arcos

#define DEBUG 1 // OPCOES DE DEBUG: 1 - MOSTRAR A QTD DE MOVIMENTOS, 2 PARA EXIBIR OS MOVIMENTOS REALIZADOS, 3 PARA EXIBIR ACOES, 4 PARA EXIBIR DETALHES DAS ACOES, 5 PARA EXIBIR TEMPO, 6 PARA EXIBIR AS MUDANÇAS NO GRAFO

// Retornar o valor da solucao
solutionType tabuSearch(char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA, solutionType solution, bool best_fit, double a1, int lc1, int lc2, int lo1, int lo2, int seed){

	cout << fixed;
   	cout.precision(5);

   	// Semente do numero aleatorio
   	srand(seed);

	/* Inicio declaracoes variaveis para calculo de tempo */

	// Declaracao variaveis que indicam o tempo da funcao
	struct timespec start, finish, time_so_far;

	/* Fim declaracoes para calculo de tempo */


	// Variavel que armazena o maior valor cij dado na entrada, para uso posterior
	double biggestCij = 0;
	

	// Variavel que marca quantas movimentacoes foram feitas de fato
	int qty_moves = 0;


	/* Sobre os grafos bipartidos, teremos
	
	Red para clientes

	Blue para instalações

	*/
	// Grafo que liga os clientes às instalações
	ListBpGraph g;

	// c_minX - será correnpondente a c(j,X) = min_{i \in X} cij, sendo X as inst abertas
	ListBpGraph::RedNodeMap<double> c_minX(g);

	// c2_minX - será correnpondente ao segundo menor c(j,X) = min_{i \in X} cij, sendo X as inst abertas exceto a nearest_open_fac
	ListBpGraph::RedNodeMap<double> c2_minX(g);

	// nearest_open_fac - será correnpondente ao ID da instalacao i tal que min_{i \in X} cij, sendo X as inst abertas
	ListBpGraph::RedNodeMap<int> nearest_open_fac(g);

	// nearest2_open_fac - será correnpondente ao ID da segunda instalacao i tal que min_{i \in X} cij, sendo X as inst abertas exceto a nearest_open_fac (segunda instalacao aberta mais proxima)
	ListBpGraph::RedNodeMap<int> nearest2_open_fac(g);

	// temp_nearest_fac - será correnpondente ao ID da instalacao i mais proxima que sera conectada (temporario)
	ListBpGraph::RedNodeMap<int> temp_nearest_fac(g);

	// temp_nearest2_fac - será correnpondente ao ID da segunda instalacao i mais proxima que sera conectada (temporario)
	ListBpGraph::RedNodeMap<int> temp_nearest2_fac(g);

	// temp_c_minX - será correnpondente ao custo c_min do temp nearest
	ListBpGraph::RedNodeMap<double> temp_c_minX(g);

	// temp_c2_minX - será correnpondente ao custo c2_min do temp nearest2
	ListBpGraph::RedNodeMap<double> temp_c2_minX(g);

	// f - será o custo de instalação (fi)
	ListBpGraph::BlueNodeMap<double> f(g);

	// open - Indica se a instalação está aberta ou não
	ListBpGraph::BlueNodeMap<bool> open(g);

	// name - Para identificar cada nó
	ListBpGraph::NodeMap<int> name(g);


	// conjunto de instalacoes que encontram-se abertas no momento
	set <int, greater <int> > open_facilities; 

	// Iteratores para os conjuntos 
	set <int, greater <int> > :: iterator itr;


	// Vetor da short term memory que representa o numero do movimento
	int * t;
	t = (int*) malloc(qty_facilities * sizeof(int));
	if(!t){
		cout << "Memory Allocation Failed";
		exit(1);
	}

	// // Vetor que vai indicar se a instalacao está flagged
	// bool * flag;
	// flag = (bool*) malloc(qty_facilities * sizeof(bool));
	// if(!flag){
	// 	cout << "Memory Allocation Failed";
	// 	exit(1);
	// }

	// Serao utilizados para acessar o vetor extra_cost, funciona como modulo 2
	int cur_index_extra = 0;
	int old_index_extra = 0;

	// extra_cost - será o delta z para cada iteracao, para cada instalacao
	double **extra_cost = (double**) malloc((2) * sizeof(double*));

	for(int i = 0; i < 2; i++) {
		extra_cost[i] = (double *)malloc(qty_facilities * sizeof(double));
	}

    if(!extra_cost){
        cout << "Memory Allocation Failed";
        exit(1);
    }


	// Each facility is kept closed for at least lc moves after it's closed unless aspiration criterion is satisfied
	int lc = rand() % (lc2 - lc1 + 1) + lc1; // Generate the number between lc1 and lc2

	// Each facility is kept opened for at least lo moves after it's closed unless aspiration criterion is satisfied
	int lo = rand() % (lo2 - lo1 + 1) + lo1; // Generate the number between lo1 and lo2

	if(DEBUG >= DISPLAY_BASIC){
		if(best_fit)
			cout << "BEST FIT - ";
		else
			cout << "FIRST FIT - ";
		cout << "A1 criterion: " << int(a1 * qty_facilities) << " iterations without improvement" << endl;
	}

	// Vai receber lc ou lo, dependendo da checagem if(open[]), para evitar duplicidade no codigo
	int aux_l;

	// Represents the amount of open facilities
	int n1 = 0;

	// Declaracao de variavel auxiliar para formacao do arquivo .log
	char completeLogSolName[250] = "";

	// Declaracao de variavel auxiliar para formacao do arquivo .log
	char completeLogDetailName[300] = "";

	// Arquivo para salvar o log da solucao
	ofstream solLog;
	strcat(completeLogSolName,solutionName);
	strcat(completeLogSolName,".log");
	solLog.open(completeLogSolName, std::ofstream::out | std::ofstream::trunc);

	// Arquivo para salvar o log detail da solucao
	ofstream logDetail;
	strcat(completeLogDetailName,solutionName);
	strcat(completeLogDetailName,".log_detail");
	logDetail.open(completeLogDetailName, std::ofstream::out | std::ofstream::trunc);

	//Salvando o cabecalho
	solLog << "time spent so far, current solution cost, current qty moves" << endl;

	//Salvando o cabecalho
	logDetail << "time spent so far, current solution cost, current qty moves" << endl;

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
		name[facilities[i]] = i; // nomeia de acordo com a numeracao
		open[facilities[i]] = false; // indica que a instalação não está aberta inicialmente
	}

	// Criação de nós clientes e atribuição de seus labels
	ListBpGraph::RedNode * clients;
	clients = (ListBpGraph::RedNode*) malloc((qty_clients) * sizeof(ListBpGraph::RedNode));
	if(!clients){
		cout << "Memory Allocation Failed";
		exit(1);
	}

	for(int i=0;i<qty_clients;i++){
		clients[i] = g.addRedNode();
		name[clients[i]] = i; // nomeia de acordo com a numeracao
		temp_nearest_fac[clients[i]] = -1; // indica que nao há nenhuma inst temporaria ainda
		nearest_open_fac[clients[i]] = solution.assigned_facilities[i]; // indica com qual inst está conectado inicialmente 

		open[facilities[solution.assigned_facilities[i]]] = true; // indica que a instalação está aberta agora

		if(DEBUG >= DISPLAY_DETAILS){
			cout << "Client " << i << " - initial nearest open facility: " << nearest_open_fac[clients[i]] << endl;
		}
	}


	// Iniciando o conjunto de instalacoes abertas e fechadas, alem dos vetores t e flag
	for(int i=0;i<qty_facilities;i++){
		if(open[facilities[i]]){
			open_facilities.insert(i);
			n1 += 1;
			t[i] = - lo;
		}
		else{
			t[i] = - lc;
		}
		// flag[i] = false;
	}

	// assignment_cost - é o custo de atribuir o cliente associado à instalação associada
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

	// Atualizando valor c_minX para todos os clientes e iniciando c2_minX
	for(int i=0;i<qty_clients;i++){
		c_minX[clients[i]] = assignment_cost[findEdge(g, clients[i], facilities[solution.assigned_facilities[i]])]; 
		c2_minX[clients[i]] = biggestCij + 1; // limitante superior tranquilo
	}

	double aux_cij;

	// Iniciando os labels correspondentes à segunda instalacao aberta mais proxima de cada cliente (c2_minX)
	for(int i=0;i<qty_clients;i++){ // percorre todos os clientes
		for (itr = open_facilities.begin(); itr != open_facilities.end(); ++itr) { // percorrer todas as inst abertas
			aux_cij = assignment_cost[findEdge(g, clients[i], facilities[*itr])];
			if((aux_cij < c2_minX[clients[i]]) && (*itr != nearest_open_fac[clients[i]])){ // se for melhor que o salvo ate agr, mas nao melhor que o melhor de todos
				nearest2_open_fac[clients[i]] = *itr;
				c2_minX[clients[i]] = aux_cij;
			}
		}
		if(c2_minX[clients[i]] > biggestCij){ // se só tinha 1 inst aberta, entao colocamos -1 para indicar invalidez
			if(DEBUG >= DISPLAY_ACTIONS){
				cout << "Starting - There is just 1 open facility" << endl;
			}
			c2_minX[clients[i]] = -1;
			nearest2_open_fac[clients[i]] = -1;
		}

		if(DEBUG >= DISPLAY_DETAILS){
			cout << "Client " << i << " c1: " << c_minX[clients[i]] << " near: " << nearest_open_fac[clients[i]] << " c2: " << c2_minX[clients[i]] << " near2: " << nearest2_open_fac[clients[i]] << endl;
		}
	}

	if(DEBUG >= DISPLAY_DETAILS){
		cout << "biggestCij from input : " << biggestCij << endl;
	}

	// Variavel que indica se foi feito uma troca, se sim, devemos dar um break no for para voltar pro while
	bool swap_done = false;

	// Variavel que indica o custo extra de abrir ou fechar uma certa instalacao
	double extra_cost2;

	// Variavel utilizada para calcular o custo completo extra do movimento de troca
	double complete_extra_cost2;

	// Variavel auxiliar para ajudar quando houver substituicao de informacoes na troca
	bool closed_nearest = false; 


	if (DEBUG >= DISPLAY_GRAPH){
		// Percorrendo por todos os nós A - clientes
		cout << "Scrolling through all clients" << endl;
		for(ListBpGraph::RedNodeIt n(g); n != INVALID; ++n){
			cout << "node id: " << g.id(n)  << " - name: " << name[n] << " - nearest fac: " << nearest_open_fac[n] << endl;
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

	/*
	TABU SEARCH DEFINITIONS
	*/

	// Total cost of the current solution
	double cur_cost = solution.finalTotalCost;

	// Best delta in this iteration
	double best_extra_cost;

	// Which facility corresponds to the best delta in this iteration
	int fac_best_extra_cost;

	// The move number when solution.finalTotalCost is updated
	int k_last_best = qty_moves;

	// Incates step 3.. if we will move in fact
	bool lets_move = false;

	// Indica se continua ou nao o loop de busca
	bool keep_searching = true;

	// Indica se vamos atualizar nearest 1 ou 2
	bool update_near = false;

	// Auxiliares que indicam a terceira inst mais proxima (c3_minX = custo, nearest3_open_fac = id, aux_cij3 = auxiliar de custo)
	double c3_minX = -1;
	int nearest3_open_fac = -1;
	double aux_cij3 = -1;

	// cout <<"Initial assigned facilities" << endl;
	// for(ListBpGraph::RedNodeIt n_cli(g); n_cli != INVALID; ++n_cli){		// percorre os clientes
	// 	cout << "Client " << name[n_cli] << ": " << solution.assigned_facilities[name[n_cli]] << endl; 
	// }	

	/* 
	STEP 0 - algumas coisas ja foram feitas antes
	*/

	// Inicializando o vetor extra_cost[0] correspondente a delta z
	for(ListBpGraph::BlueNodeIt n(g); n != INVALID; ++n){		// percorre as instalacoes
		if(open[n]){ // se a instalacao está aberta, vamos ver a mudanca se fechar
			extra_cost[0][name[n]] = -f[n];
			for (ListBpGraph::IncEdgeIt e(g, n); e != INVALID; ++e) { // Percorre todas arestas desse nó (ligam a clientes)
				if(nearest_open_fac[g.asRedNode(g.u(e))] == name[n]){ // se essa inst for a mais proxima desse cli
					extra_cost[0][name[n]] += c2_minX[g.asRedNode(g.u(e))] - assignment_cost[e];
				}
			}
		}
		else{ // se a instalacao está fechada, vamos ver a mudanca se abrir
			extra_cost[0][name[n]] = f[n];
			for (ListBpGraph::IncEdgeIt e(g, n); e != INVALID; ++e) { // Percorre todas arestas desse nó (ligam a clientes)
				if(assignment_cost[e] < c_minX[g.asRedNode(g.u(e))]){ // se essa inst for a mais proxima desse cli
					extra_cost[0][name[n]] += assignment_cost[e] - c_minX[g.asRedNode(g.u(e))];
				}
			}
		}
	}



	// cout << "TESTE SE A SOLUCAO TA CERTA" << endl;
	// double coost = 0;

	// for (itr = open_facilities.begin(); itr != open_facilities.end(); ++itr) { // percorrer todas as inst abertas
	// 	coost += f[facilities[*itr]];
	// }

	// for(ListBpGraph::RedNodeIt n(g); n != INVALID; ++n){		// percorre as instalacoes
	// 	coost += assignment_cost[findEdge(g, n, facilities[solution.assigned_facilities[name[n]]])]; 
	// }

	// cout << "CUSTO INICIAL REAL: " << coost << endl;

	// CHECANDO A CONTAGEM DE TEMPO GASTO ATÉ AGORA
	clock_gettime(CLOCK_REALTIME, &time_so_far);

	// Calculando o tempo gasto até agora
	solution.timeSpent =  (time_so_far.tv_sec - start.tv_sec);
	solution.timeSpent += (time_so_far.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 

	// Acrescentando no solLog.txt o tempo e o custo inicial
	solLog << solution.timeSpent << "," << solution.finalTotalCost << "," << qty_moves << endl;

	// Acrescentando no logDetail.txt o tempo e o custo inicial
	logDetail << solution.timeSpent << "," << cur_cost << "," << qty_moves << endl;

	bool accept;

	// INICIANDO A CONTAGEM DE TEMPO DA FUNCAO
	clock_gettime(CLOCK_REALTIME, &start);

	// A partir daqui é loop até acabar a busca
	while(keep_searching){

		if(DEBUG >= DISPLAY_MOVES){
			cout << endl << "-------------------------- NEXT MOVE " << qty_moves << " ---------------------------" << endl << endl;
		}

		/* 
		STEP 1
		*/

		// Tecnica best fit
		// if(best_fit){
		accept = false;
		// Select a facility that has de minimum extra_cost and is not flagged (and extra_cost is not DBL_MAX ---> invalid)
		best_extra_cost = DBL_MAX; // limitante superior, maior double possivel
		fac_best_extra_cost = -1; // indica invalidez

		for(int i=0;i<qty_facilities;i++){
			if(!((open[facilities[i]])&&(n1 == 1))){ // se ela nao for a unica instalacao aberta
				if(extra_cost[cur_index_extra][i] < best_extra_cost){ // se essa for menor do que a ja encontrada ate agr, atualiza
					// Check the tabu status of the selected move
					if(qty_moves < t[i]){ // Se for tabu

						if(DEBUG >= DISPLAY_MOVES){
							cout << "It is tabu" << endl;
						}

						// Check the aspiration criterion of the selected move
						if((cur_cost + extra_cost[cur_index_extra][i]) < solution.finalTotalCost){ // Se satisfizer o aspiration criterion

							if(DEBUG >= DISPLAY_MOVES){
								cout << "It satisfies the aspiration criterion" << endl;
							}

							accept = true;
						}
					}
					else { // se nao for tabu

						if(DEBUG >= DISPLAY_MOVES){
							cout << "It is not tabu" << endl;
						}

						accept = true;
					}

					if(accept){
						accept = false;
						best_extra_cost = extra_cost[cur_index_extra][i];
						fac_best_extra_cost = i;
					}
				}
			}
		}

		// }
		// // Tecnica first fit que melhora, senao o best menos ruim
		// else{
		// 	// Select a facility that has a negative extra_cost and is not flagged or the minimum one positive
		// 	best_extra_cost = DBL_MAX; // limitante superior, maior double possivel
		// 	fac_best_extra_cost = -1; // indica invalidez
		// 	for(int i=0;i<qty_facilities;i++){
		// 		if(!((open[facilities[i]]) && (n1 == 1)) && ( !flag[i] )){ // se ela nao for a unica instalacao aberta e nao estiver flagged
		// 			if((extra_cost[cur_index_extra][i] < 0)){ // se ela melhorar a solucao final
		// 				best_extra_cost = extra_cost[cur_index_extra][i];
		// 				fac_best_extra_cost = i;
		// 				break; // sai com essa
		// 			}
		// 			else if((extra_cost[cur_index_extra][i] < best_extra_cost)){ // se essa for menor do que a ja encontrada ate agr, atualiza
		// 				best_extra_cost = extra_cost[cur_index_extra][i];
		// 				fac_best_extra_cost = i;
		// 			}
		// 		}
		// 	}
		// }

		if(DEBUG >= DISPLAY_MOVES){
			cout << "Facility " << fac_best_extra_cost << " best delta extra cost: " << best_extra_cost << endl;
		}

		if(fac_best_extra_cost == -1){
			if(DEBUG >= DISPLAY_BASIC){
				cout << "Stop criterion: THERE ARE NO MOVES ANYMORE" << endl;
			}
			keep_searching = false;
		}
		else{
			lets_move = true;
			if(DEBUG >= DISPLAY_MOVES){
				if(open[facilities[fac_best_extra_cost]]){ // se a instalacao está aberta
					cout << "We want to close it" << endl;
				}
				else{ // se a instalacao está fechada
					cout << "We want to open it" << endl;
				}
			}

		// 	// Check the tabu status of the selected move
		// 	if(qty_moves < t[fac_best_extra_cost]){ // Se for tabu

		// 		if(DEBUG >= DISPLAY_MOVES){
		// 			cout << "It is tabu" << endl;
		// 		}

		// 		/* 
		// 		STEP 2
		// 		*/

		// 		// Check the aspiration criterion of the selected move
		// 		if((cur_cost + best_extra_cost) < solution.finalTotalCost){ // Se satisfizer o aspiration criterion
		// 			if(DEBUG >= DISPLAY_MOVES){
		// 				cout << "It satisfies the aspiration criterion" << endl;
		// 			}
		// 			/* 
		// 			go to STEP 3
		// 			*/
		// 			lets_move = true;
		// 		}
		// 		else{ // Se nao satisfizer o aspiration criterion
		// 			if(DEBUG >= DISPLAY_MOVES){
		// 				cout << "It does not satisfy the aspiration criterion" << endl;
		// 			}
		// 			// Mark facility as flagged
		// 			flag[fac_best_extra_cost] = true;

		// 			/* 
		// 			go back to STEP 1 -->> while
		// 			*/
		// 			lets_move = false; // garantindo que o valor será falso
		// 			keep_searching = true; // garantindo que o valor será verdadeiro
		// 		}
		// 	}
		// 	else{ //  se nao for tabu
		// 		if(DEBUG >= DISPLAY_MOVES){
		// 			cout << "It is not tabu" << endl;
		// 		}

		// 		/* 
		// 		go to STEP 3
		// 		*/
		// 		lets_move = true;
		// 	}	


			/* 
			STEP 3
			*/

			if(lets_move){

				if(DEBUG >= DISPLAY_MOVES){
					cout << "Lets move!" << endl;
				}

				// Restaurando 
				lets_move = false;

				// Abrindo a inst se estiver fechada e fechando se estiver aberta
				open[facilities[fac_best_extra_cost]] = !open[facilities[fac_best_extra_cost]];

				if(open[facilities[fac_best_extra_cost]]){ // se a instalação estiver aberta agora
					n1 += 1;
					open_facilities.insert(fac_best_extra_cost);
					lo = rand() % (lo2 - lo1 + 1) + lo1; // Generate the number between lo1 and lo2
					aux_l = lo;
				}
				else { // senao, se a instalação estiver fechada agora
					n1 -= 1;
					open_facilities.erase(fac_best_extra_cost);
					lc = rand() % (lc2 - lc1 + 1) + lc1; // Generate the number between lc1 and lc2
					aux_l = lc;
				}


				// Atualizando o custo atual
				cur_cost += best_extra_cost;

				// Atualizando a lista tabu
				t[fac_best_extra_cost] = qty_moves + aux_l;

				// Aumentando a contagem de movimentos
				qty_moves += 1;

				// Atualizando os indices para acessar o vetor extra_cost
				cur_index_extra = qty_moves % 2;
				old_index_extra = !cur_index_extra;

				// CHECANDO A CONTAGEM DE TEMPO GASTO ATÉ AGORA
				clock_gettime(CLOCK_REALTIME, &time_so_far);

				// Calculando o tempo gasto até agora
				solution.timeSpent =  (time_so_far.tv_sec - start.tv_sec);
				solution.timeSpent += (time_so_far.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 

				if(DEBUG >= DISPLAY_TIME){
					cout << "Total time spent so far: " << solution.timeSpent << " seconds" << endl;
				}

				// Acrescentando no logDetail.txt o tempo gasto nessa iteracao e o custo da solucao
				logDetail << solution.timeSpent << "," << cur_cost << "," << qty_moves << endl;

				// Atualizando a melhor solucao encontrada ate agr
				if(cur_cost < solution.finalTotalCost){
					if(DEBUG >= DISPLAY_MOVES){
						cout << "Updating the best cost found so far: " << cur_cost << endl;
					}
					solution.finalTotalCost = cur_cost;
					k_last_best = qty_moves;
					
					// Acrescentando no solLog.txt o tempo gasto nessa iteracao e o custo da solucao
					solLog << solution.timeSpent << "," << solution.finalTotalCost << "," << qty_moves << endl;
				}
				else{
					if(DEBUG >= DISPLAY_ACTIONS){
						cout << "IT IS NOT BETTER. current cost: " << cur_cost << endl;
					}
				}


				/* 
				STEP 4
				*/

				/* 
				Update extra_cost[cur_index_extra]
				*/

				// If the facility was closed and we opened
				if(open[facilities[fac_best_extra_cost]]){

					if(DEBUG >= DISPLAY_ACTIONS){
						cout << "The facility was closed and we opened. Lets update delta extra cost from other facilities" << endl;
					}

					for(ListBpGraph::BlueNodeIt n2(g); n2 != INVALID; ++n2){		// percorre as instalacoes
						if(name[n2]!=fac_best_extra_cost){ // se nao for exatamente a inst que eu estou abrindo

							// Open facilities after another facility opened
							if(open[n2]){ // se a instalacao 2 está aberta

								if(DEBUG >= DISPLAY_DETAILS){
									cout << "OPEN FACILITY: " << name[n2] << endl;
								}
								if(n1 > 2){ // se ela nao era a unica instalacao aberta (entao extra_cost[qty_moves - 1] está definido)
									extra_cost[cur_index_extra][name[n2]] = extra_cost[old_index_extra][name[n2]]; // inicia com o valor da anterior

									for (ListBpGraph::IncEdgeIt e(g, n2); e != INVALID; ++e) { // Percorre todas arestas desse nó (ligam a clientes)
										if(nearest_open_fac[g.asRedNode(g.u(e))] == name[n2]){ // se esse cliente tinha a inst n2 como inst mais proxima

											//  Pegando o custo de atribuicao desse cliente com a inst que estou abrindo (cij')
											aux_cij = assignment_cost[findEdge(g, g.asRedNode(g.u(e)), facilities[fac_best_extra_cost])];

											// Tj1 = {i | di1 = j ^ cij' < cij}
											if(aux_cij < assignment_cost[e]){ // se a que abriu eh melhor que essa inst n2 (Tj1)
												extra_cost[cur_index_extra][name[n2]] += assignment_cost[e] - c2_minX[g.asRedNode(g.u(e))];

												// Updating d2 --> dj1 (nearest)
												temp_nearest2_fac[g.asRedNode(g.u(e))] = nearest_open_fac[g.asRedNode(g.u(e))];
												temp_c2_minX[g.asRedNode(g.u(e))] = c_minX[g.asRedNode(g.u(e))];

												// Updating d1 --> j' (fac_best_extra_cost)
												temp_nearest_fac[g.asRedNode(g.u(e))] = fac_best_extra_cost;
												temp_c_minX[g.asRedNode(g.u(e))] = aux_cij;
											}

											// Tj2 = {i | di1 = j ^ cij <= cij' < cidi2}
											else if((assignment_cost[e] <= aux_cij) && (aux_cij < c2_minX[g.asRedNode(g.u(e))])){ // senao, se a que abriu eh melhor que a segunda inst mais proxima (Tj2)
												extra_cost[cur_index_extra][name[n2]] += aux_cij - c2_minX[g.asRedNode(g.u(e))];

												// Saving d1 temp
												temp_nearest_fac[g.asRedNode(g.u(e))] = nearest_open_fac[g.asRedNode(g.u(e))];
												temp_c_minX[g.asRedNode(g.u(e))] = c_minX[g.asRedNode(g.u(e))];

												// Updating d2 --> j' (fac_best_extra_cost)
												temp_nearest2_fac[g.asRedNode(g.u(e))] = fac_best_extra_cost;
												temp_c2_minX[g.asRedNode(g.u(e))] = aux_cij;
											}
										}
									}
								}
								else { // se ela era a unica instalacao aberta
									if(DEBUG >= DISPLAY_ACTIONS){
										cout << "It was the only one open facility. We need to recompute the delta extra cost. Fac: " << name[n2] << endl;
									}
									/* 
									Recomputing extra_cost
									*/
									extra_cost[cur_index_extra][name[n2]] = -f[n2]; // inicia com o ganho ao fechar essa inst

									for (ListBpGraph::IncEdgeIt e(g, n2); e != INVALID; ++e) { // Percorre todas arestas desse nó (ligam a clientes)

										if(nearest_open_fac[g.asRedNode(g.u(e))] == name[n2]){ // se essa inst for a mais proxima desse cli. OBS: isso deve sempre ser verdadeiro pois só tinha ela aberta

											//  Pegando o custo de atribuicao desse cliente com a inst que estou abrindo (cij')
											aux_cij = assignment_cost[findEdge(g, g.asRedNode(g.u(e)), facilities[fac_best_extra_cost])];

											// Se a que abriu agora é a mais proxima
											if(aux_cij < c_minX[g.asRedNode(g.u(e))]){
												// Updating d2
												temp_nearest2_fac[g.asRedNode(g.u(e))] = nearest_open_fac[g.asRedNode(g.u(e))];
												temp_c2_minX[g.asRedNode(g.u(e))] = c_minX[g.asRedNode(g.u(e))];

												// Updating d1
												temp_nearest_fac[g.asRedNode(g.u(e))] = fac_best_extra_cost;
												temp_c_minX[g.asRedNode(g.u(e))] = aux_cij;
											}
											// Se nao, entao a que abriu é a segunda mais proxima
											else{

												// Saving d1 temp
												temp_nearest_fac[g.asRedNode(g.u(e))] = nearest_open_fac[g.asRedNode(g.u(e))];
												temp_c_minX[g.asRedNode(g.u(e))] = c_minX[g.asRedNode(g.u(e))];

												// Updating d2
												temp_nearest2_fac[g.asRedNode(g.u(e))] = fac_best_extra_cost;
												temp_c2_minX[g.asRedNode(g.u(e))] = aux_cij;

												// Complementa com o ganho de reatribuicao
												extra_cost[cur_index_extra][name[n2]] += temp_c2_minX[g.asRedNode(g.u(e))] - assignment_cost[e];
											}
										}
									}
								}
							}

							// Closed facilities after another facility opened
							else{ // se a instalacao 2 está fechada 
								if(DEBUG >= DISPLAY_DETAILS){
									cout << "CLOSED FACILITY: " << name[n2] << endl;
								}

								extra_cost[cur_index_extra][name[n2]] = extra_cost[old_index_extra][name[n2]]; // inicia com o valor da anterior

								for (ListBpGraph::IncEdgeIt e(g, n2); e != INVALID; ++e) { // Percorre todas arestas desse nó (ligam a clientes)

									update_near = false; // a principio nao sabemos se deveremos atualizar ou nao

									//  Pegando o custo de atribuicao desse cliente com a inst que estou abrindo (cij')
									aux_cij = assignment_cost[findEdge(g, g.asRedNode(g.u(e)), facilities[fac_best_extra_cost])];

									// Sj1 = {i | cij < cij' < cidi1}
									if((assignment_cost[e] < aux_cij) && (aux_cij < c_minX[g.asRedNode(g.u(e))])){ // se a que abriu eh melhor que a nearest mas pior que essa inst n2 (Sj1)
										extra_cost[cur_index_extra][name[n2]] += c_minX[g.asRedNode(g.u(e))] - aux_cij;
										update_near = true; // indica que vamos atualizar
									}
									// Sj2 = {i | cij' <= cij < cidi1}
									else if((aux_cij <= assignment_cost[e]) && (assignment_cost[e] < c_minX[g.asRedNode(g.u(e))])){ // senao, se a que abriu eh melhor que a nearest e essa n2, porem n2 melhor que nearest tbm (Sj2)
										extra_cost[cur_index_extra][name[n2]] += c_minX[g.asRedNode(g.u(e))] - assignment_cost[e];
										update_near = true; // indica que vamos atualizar
									}

									// Updating d1 and d2
									if(update_near){

										// Updating d2 --> dj1 (nearest)
										temp_nearest2_fac[g.asRedNode(g.u(e))] = nearest_open_fac[g.asRedNode(g.u(e))];
										temp_c2_minX[g.asRedNode(g.u(e))] = c_minX[g.asRedNode(g.u(e))];

										// Updating d1 --> j' (fac_best_extra_cost)
										temp_nearest_fac[g.asRedNode(g.u(e))] = fac_best_extra_cost;
										temp_c_minX[g.asRedNode(g.u(e))] = aux_cij;
									}
								}
							}

							// // Mark each facility as unflagged
							// flag[name[n2]] = false;
						}
					}

					// Updating extra_cost for this chosen facility -> don't need to recalculate
					extra_cost[cur_index_extra][fac_best_extra_cost] = - best_extra_cost;
					// extra_cost[cur_index_extra][fac_best_extra_cost] = -f[facilities[fac_best_extra_cost]];
					// for (ListBpGraph::IncEdgeIt e(g, facilities[fac_best_extra_cost]); e != INVALID; ++e) { // Percorre todas arestas desse nó (ligam a clientes)
					// 	if(temp_nearest_fac[g.asRedNode(g.u(e))] == name[facilities[fac_best_extra_cost]]){ // se essa inst for a mais proxima desse cli
					// 		extra_cost[cur_index_extra][fac_best_extra_cost] += temp_c2_minX[g.asRedNode(g.u(e))] - assignment_cost[e];
					// 	}
					// }
				}

				// Else, if the facility was opened and we closed
				else{

					if(DEBUG >= DISPLAY_ACTIONS){
						cout << "The facility was opened and we closed. Lets update delta extra cost from other facilities" << endl;
					}
					for(ListBpGraph::BlueNodeIt n2(g); n2 != INVALID; ++n2){		// percorre as instalacoes
						if(name[n2]!=fac_best_extra_cost){ // se nao for exatamente a inst que eu estou abrindo

							// Open facilities after another facility closed
							if(open[n2]){ // se a instalacao 2 está aberta

								if(DEBUG >= DISPLAY_DETAILS){
									cout << "OPEN FACILITY: " << name[n2] << endl;
								}

								if(n1 > 1){ // se ela nao eh a unica instalacao aberta 

									extra_cost[cur_index_extra][name[n2]] = extra_cost[old_index_extra][name[n2]]; // inicia com o valor da anterior

									for (ListBpGraph::IncEdgeIt e(g, n2); e != INVALID; ++e) { // Percorre todas arestas desse nó (ligam a clientes)

										//  Pegando o custo de atribuicao desse cliente com a inst que estou fechando (cij')
										aux_cij = assignment_cost[findEdge(g, g.asRedNode(g.u(e)), facilities[fac_best_extra_cost])];

										// Se for Tj1 ou Tj2
										// Tj1 = {i | di1 = j' ^ di2 = j} 		// Tj2 = {i | di1 = j ^ di2 = j'}
										if(((nearest_open_fac[g.asRedNode(g.u(e))] == fac_best_extra_cost) && (nearest2_open_fac[g.asRedNode(g.u(e))] == name[n2])) || ((nearest_open_fac[g.asRedNode(g.u(e))] == name[n2]) && (nearest2_open_fac[g.asRedNode(g.u(e))] == fac_best_extra_cost))) { // se estou fechando a mais proxima e n2 é a segunda mais proxima (Tj1)
											/*
											Defining nearest3_open_fac e c3_minX
											*/
											c3_minX = biggestCij + 1; // limitante superior tranquilo
											for(itr = open_facilities.begin(); itr != open_facilities.end(); ++itr) { // percorrer todas as inst abertas
												aux_cij3 = assignment_cost[findEdge(g, g.asRedNode(g.u(e)), facilities[*itr])];
												if((aux_cij3 < c3_minX) && (*itr != nearest_open_fac[g.asRedNode(g.u(e))]) && (*itr != nearest2_open_fac[g.asRedNode(g.u(e))])){ // se for melhor que o salvo ate agr, mas nao melhor que o melhor de todos nem o segundo melhor salvo
													nearest3_open_fac = *itr;
													c3_minX = aux_cij3;
												}
											}
											// incluindo a inst que fechamos na busca, ja q ela nao está mais em open_facilities
											if((c3_minX > biggestCij) || ((aux_cij < c3_minX) && (fac_best_extra_cost != nearest_open_fac[g.asRedNode(g.u(e))]) && (fac_best_extra_cost != nearest2_open_fac[g.asRedNode(g.u(e))]))) {
												nearest3_open_fac = fac_best_extra_cost;
												c3_minX = aux_cij;
											}

											// Se for Tj1
											// Tj1 = {i | di1 = j' ^ di2 = j}
											if((nearest_open_fac[g.asRedNode(g.u(e))] == fac_best_extra_cost) && (nearest2_open_fac[g.asRedNode(g.u(e))] == name[n2])){ // se estou fechando a mais proxima e n2 é a segunda mais proxima (Tj1)
												extra_cost[cur_index_extra][name[n2]] += c3_minX - assignment_cost[e];

												// Updating d1 --> dj2 (nearest2)
												temp_nearest_fac[g.asRedNode(g.u(e))] = nearest2_open_fac[g.asRedNode(g.u(e))];
												temp_c_minX[g.asRedNode(g.u(e))] = c2_minX[g.asRedNode(g.u(e))];	
											}
											// Senao, se for Tj2
											// Tj2 = {i | di1 = j ^ di2 = j'}
											else {  // se estou fechando a segunda mais proxima e n2 é a mais proxima (Tj2)
												extra_cost[cur_index_extra][name[n2]] += c3_minX - aux_cij;

												// saving temp d1
												temp_nearest_fac[g.asRedNode(g.u(e))] = nearest_open_fac[g.asRedNode(g.u(e))];
												temp_c_minX[g.asRedNode(g.u(e))] = c_minX[g.asRedNode(g.u(e))];

											}

											// Updating d2 --> d3 (nearest3), tanto para Tj1 quanto Tj2
											temp_nearest2_fac[g.asRedNode(g.u(e))] = nearest3_open_fac;
											temp_c2_minX[g.asRedNode(g.u(e))] = c3_minX;
										}
									}

								}
								else { // se ela eh a unica instalacao aberta
									// if(DEBUG >= DISPLAY_ACTIONS){
										cout << "It was the only one open facility. We cannot define a new delta extra cost. Fac: " << name[n2] << endl;
									// }
									/* 
									Not possible to have extra_cost because this facility cannot be closed
									*/

									extra_cost[cur_index_extra][name[n2]] = DBL_MAX; // indica invalidez

									for (ListBpGraph::IncEdgeIt e(g, n2); e != INVALID; ++e) { // Percorre todas arestas desse nó (ligam a clientes)
										// colocando ela como inst mais proxima de todos os clientes
										temp_nearest_fac[g.asRedNode(g.u(e))] = name[n2];
										temp_c_minX[g.asRedNode(g.u(e))] = assignment_cost[e];

										// indicando invalidez
										temp_nearest2_fac[g.asRedNode(g.u(e))] = -1;
										temp_c2_minX[g.asRedNode(g.u(e))] = -1;
									}
								}
							}

							// Closed facilities after another facility closed
							else{ // se a instalacao 2 está fechada 
								if(DEBUG >= DISPLAY_DETAILS){
									cout << "CLOSED FACILITY: " << name[n2] << endl;
								}

								extra_cost[cur_index_extra][name[n2]] = extra_cost[old_index_extra][name[n2]]; // inicia com o valor da anterior

								for (ListBpGraph::IncEdgeIt e(g, n2); e != INVALID; ++e) { // Percorre todas arestas desse nó (ligam a clientes)
								
									if(nearest_open_fac[g.asRedNode(g.u(e))] == fac_best_extra_cost){ // se esse cliente tinha a inst fac_best_extra_cost como inst mais proxima

										update_near = false; // a principio nao sabemos se deveremos atualizar ou nao

										// Pegando o custo de atribuicao desse cliente com a inst que estou fechando (cij')
										aux_cij = assignment_cost[findEdge(g, g.asRedNode(g.u(e)), facilities[fac_best_extra_cost])];

										// Sj1 = {i | di1 = j' ^ cij < cij'}
										if(assignment_cost[e] < aux_cij){ // se essa inst n2 eh melhor que a que fechou, que era a nearest (Sj1)
											extra_cost[cur_index_extra][name[n2]] += aux_cij - c2_minX[g.asRedNode(g.u(e))];
											update_near = true; // indica que vamos atualizar
										}

										// Sj2 = {i | di1 = j' ^ cij' <= cij < cidi2}
										else if((aux_cij <= assignment_cost[e]) && (assignment_cost[e] < c2_minX[g.asRedNode(g.u(e))])){ // senao, se essa inst n2 é melhor que a nearest2 porem nao melhor que a nearest que fechou(Sj2)
											extra_cost[cur_index_extra][name[n2]] += assignment_cost[e] - c2_minX[g.asRedNode(g.u(e))];
											update_near = true; // indica que vamos atualizar
										}

										// Updating d1 and d2
										if(update_near){

											// Updating d1 --> dj2 (nearest2)
											temp_nearest_fac[g.asRedNode(g.u(e))] = nearest2_open_fac[g.asRedNode(g.u(e))];
											temp_c_minX[g.asRedNode(g.u(e))] = c2_minX[g.asRedNode(g.u(e))];

											// Updating d2 --> search again
											
											temp_c2_minX[g.asRedNode(g.u(e))] = biggestCij + 1; // limitante superior tranquilo

											for(itr = open_facilities.begin(); itr != open_facilities.end(); ++itr) { // percorrer todas as inst abertas
												aux_cij = assignment_cost[findEdge(g, g.asRedNode(g.u(e)), facilities[*itr])];
												if((aux_cij < temp_c2_minX[g.asRedNode(g.u(e))]) && (*itr != temp_nearest_fac[g.asRedNode(g.u(e))])){ // se for melhor que o salvo ate agr, mas nao melhor que o melhor de todos
													temp_nearest2_fac[g.asRedNode(g.u(e))] = *itr;
													temp_c2_minX[g.asRedNode(g.u(e))] = aux_cij;
												}
											}
											if(temp_c2_minX[g.asRedNode(g.u(e))] > biggestCij){ // se só tinha 1 inst aberta, entao colocamos -1 para indicar invalidez
												if(DEBUG >= DISPLAY_ACTIONS){
													cout << "There is just 1 open facility" << endl;
												}
												temp_c2_minX[g.asRedNode(g.u(e))] = -1;
												temp_nearest2_fac[g.asRedNode(g.u(e))] = -1;
											}
										}
									}
								}
							}

							// // Mark each facility as unflagged
							// flag[name[n2]] = false;
						}
					}
					// Updating extra_cost for this chosen facility -> don't need to recalculate
					extra_cost[cur_index_extra][fac_best_extra_cost] = - best_extra_cost;
					// extra_cost[cur_index_extra][fac_best_extra_cost] = f[facilities[fac_best_extra_cost]];

					// for (ListBpGraph::IncEdgeIt e(g, facilities[fac_best_extra_cost]); e != INVALID; ++e) { // Percorre todas arestas desse nó (ligam a clientes)
					// 	if(assignment_cost[e] < temp_c_minX[g.asRedNode(g.u(e))]){ // se essa inst for a mais proxima desse cli
					// 		extra_cost[cur_index_extra][fac_best_extra_cost] += assignment_cost[e] - temp_c_minX[g.asRedNode(g.u(e))];
					// 	}
					// }

				}

				/*
				UPDATING NEAREST AND NEAREST 2
				*/

 				for(ListBpGraph::RedNodeIt n_cli(g); n_cli != INVALID; ++n_cli){		// percorre os clientes
 					if(temp_nearest_fac[n_cli]!= -1){ // se houve alguma mudanca com esse cliente
	 					nearest_open_fac[n_cli] = temp_nearest_fac[n_cli];
	 					c_minX[n_cli] = temp_c_minX[n_cli];

	 					nearest2_open_fac[n_cli] = temp_nearest2_fac[n_cli];
	 					c2_minX[n_cli] = temp_c2_minX[n_cli];

	 					temp_nearest_fac[n_cli] = -1; // resetando
 					}

 					// Se a solucao final foi atualizada nessa iteracao
 					if(k_last_best == qty_moves){ 
 						// SALVANDO A SOLUCAO NOVA
						solution.assigned_facilities[name[n_cli]] = nearest_open_fac[n_cli]; // salvando alteracoes sobre a inst mais proxima na solucao final
 					}
				}

				
				// Se ainda nao atingiu a condicao de parada respectivo ao a1
				if(qty_moves - k_last_best <= a1 * qty_facilities){
					/* 
					go back to STEP 1 -->> while
					*/
					keep_searching = true; // garantindo que o valor será verdadeiro
				}
						
				else{
					if(DEBUG >= DISPLAY_BASIC){
						cout << "Stop criterion a1" << endl;
					}
					/*
					STOP
					*/
					keep_searching = false;
				}
			}
		}
	}

	// FINALIZANDO A CONTAGEM DE TEMPO DA FUNCAO
	clock_gettime(CLOCK_REALTIME, &finish);

	if(DEBUG >= DISPLAY_ACTIONS){
		cout << "FINAL TOTAL COST: " << solution.finalTotalCost << endl;
	}

	// Exibir quais instalacoes foram abertas
	if(DEBUG >= DISPLAY_ACTIONS){
		cout << "OPEN FACILITIES: " << endl;
		for (itr = open_facilities.begin(); itr != open_facilities.end(); ++itr) { // percorrer todas as inst abertas
			cout << *itr << " ";
		}
		cout << endl;
	}

	if(DEBUG >= DISPLAY_BASIC){
		cout << "Total moves: " << qty_moves << endl;
		cout << "Last update final cost: " << k_last_best << endl;
	}


	// Calculando o tempo gasto da funcao
	solution.timeSpent =  (finish.tv_sec - start.tv_sec);
	solution.timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 

	if(DEBUG >= DISPLAY_TIME){
		cout << "Final Total Function Time: " << solution.timeSpent << " seconds" << endl;
	}

	// Acrescentando no solLog.txt o tempo gasto final da funcao e o custo final da solucao
	solLog << solution.timeSpent << "," << solution.finalTotalCost << "," << qty_moves << endl;

	solLog.close();
	logDetail.close();

	free(t);
	// free(flag);
	free(extra_cost);
	
	free(clients);
	free(facilities);
	free(edges);

	return(solution);
}
