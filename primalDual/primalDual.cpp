#include <lemon/list_graph.h>
#include <algorithm>
#include <iostream>
#include <set>
#include <iterator>
#include <ctime>
#include "definitions.hpp"
#define EPSL 0.00000001


// Casos de testes: http://resources.mpi-inf.mpg.de/departments/d1/projects/benchmarks/UflLib/index.html

// TODO: seria legal se tivesse uma fila de prioridades para ver o caso A (min cij) 


// Observacao importante: ao usar S.nodeFromId() eh necessario passar o ID geral, nao o blue ID nem red ID. 
// No caso do cliente, o red ID = ID geral, pois os clients foram os primeiros a serem adicionados no grafo.
// No caso da instalacao, o ID geral = blue ID + qty_clients, pois as facilities foram adicionadas no grafo logo apos todos os clients.

using namespace lemon;
using namespace std;

#define DISPLAY_ACTIONS 1 // corresponde a todos os cout quando uma acao é feita. 
#define DISPLAY_TIME 2 // corresponde aos calculos de tempo 
#define DISPLAY_GRAPH 3 // corresponde a descricao dos clients, facilities e edges
#define DISPLAY_ADJ_MATRIX 4 // corresponde à parte final, na criacao de Tline

#define DEBUG 0 // OPCOES DE DEBUG: 1 PARA EXIBIR ACOES, 2 PARA EXIBIR TEMPO, 3 PARA EXIBIR AS MUDANÇAS NO GRAFO, 4 PARA EXIBIR AS MUDANCAS NA MATRIZ DE ADJACENCIA NA CRIACAO DE Tline



bool equal(double i, double j){
    if((i > j-EPSL) && (i < j+EPSL))
        return true;
    return false;
}

bool greater_or_equal(double i, double j){
    // if(i >= j * (1-EPSL))
    if(i >= j - EPSL)
        return true;
    return false;
}


// Retornar o valor da solucao
solutionType primalDual(int qty_clients, int qty_facilities, double * costF, double * costA){

	cout << fixed;
   	cout.precision(5);

	// Struct que vai retornar a solução
	solutionType solution;

	solution.finalTotalCost = 0; // inicializando o custo final
	
	/* Inicio declaracoes variaveis para calculo de tempo - finalidade eh encontrar gargalos */

	// Declaracao variaveis que indicam o tempo no inicio e fim da execucao daquela parte desejada
	struct timespec start, finish;

	// Declaracao variaveis que indicam o tempo do programa como um todo
	struct timespec real_start, real_finish;

	// Declaracao variavel que marcara o tempo calculado daquela parte desejada
	double timeSpent;

	// Declaracao variavel que marcara o tempo de execucao da funcao como um todo
	double realTimeSpent;

	/* Fim declaracoes para calculo de tempo */


	if(DEBUG >= DISPLAY_TIME){
		// INICIANDO A CONTAGEM DE TEMPO DA FUNCAO COMO UM TODO
		clock_gettime(CLOCK_REALTIME, &real_start);
	}

	// indica as facilities finais atribuidas a cada cliente
	solution.assigned_facilities = (int*) malloc((qty_clients) * sizeof(int));
    if(!solution.assigned_facilities){
        cout << "Memory Allocation Failed";
        exit(1);
    }

	// conjunto de clients a serem removidos de S na iteração, pois deixaram de ser ativos
    set <int, greater <int> > delete_clients; 
    // Iterator para o conjunto 
    set <int, greater <int> > :: iterator itr; 

	/*

	ESTRUTURA QUE INDICA QUANTOS E QUAIS clients ESTAO CONTRIBUINDO (AUMENTANDO O W) 
	PARA UMA CERTA INSTALACAO. valor fixo, nao muda durante o algoritmo, só no final, na criaçao de Tline.
	Essa estrutura é importante pois perdemos os valores do grafo S.

	- cada linha indica um cliente
	- cada coluna indica uma instalacao
	- Valor 1 na linha i, coluna j se o cliente i contribui pra instalacao j

	*/
	
	// indica se o cliente alcançou a instalação
	int **adjacency_matrix = (int**) malloc((qty_clients) * sizeof(int*));
	for(int i = 0; i < qty_clients; i++) adjacency_matrix[i] = (int *)malloc(qty_facilities * sizeof(int));

    if(!adjacency_matrix){
        cout << "Memory Allocation Failed";
        exit(1);
    }

	/* Sobre os grafos bipartidos, teremos
	
	Red para clients

	Blue para instalações

	*/
	// Grafo que liga os clients às instalações
	ListBpGraph g;

	// v - será o quanto o cliente vai contribuir (v)
	ListBpGraph::RedNodeMap<double> v(g);

	// f - será o custo de instalação (fi)
	ListBpGraph::BlueNodeMap<double> f(g);

	// sumW - indica o quanto ja foi pago do custo de abrir a instalacao i... somatorio de todos os w correspondente a essa i
	ListBpGraph::BlueNodeMap<double> sumW(g);

	// qty_payers - será a quantidade de clients ativos que estao prontos para contribuir com a inst i
	ListBpGraph::BlueNodeMap<int> qty_payers(g);

	// Indica se a instalação está open ou não
	ListBpGraph::BlueNodeMap<bool> open(g);

	// Indica se a instalação está em Tline (util para o final)
	ListBpGraph::BlueNodeMap<bool> isInTline(g);

	// Para identificar cada nó
	ListBpGraph::NodeMap<int> name(g);

	// Variavel que armazena o maior valor cij dado na entrada, para uso posterior
	double biggestCij = 0;

	// Variavel que armazena o maior valor fi dado na entrada, para uso posterior
	double biggestFi = 0;

	if(DEBUG >= DISPLAY_TIME){
		cout <<"[TIME] Starting time count for graph creation and its maps" << endl;
		//Iniciando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &start);
	}

	// Criação de nós clients e atribuição de seus labels
	ListBpGraph::RedNode * clients;
	clients = (ListBpGraph::RedNode*) malloc((qty_clients) * sizeof(ListBpGraph::RedNode));
    if(!clients){
        cout << "Memory Allocation Failed";
        exit(1);
    }

	for(int i=0;i<qty_clients;i++){
		clients[i] = g.addRedNode();
		v[clients[i]] = 0; // v = inicialmente nao contribui com nada
		name[clients[i]] = i; // nameia de acordo com a numeracao
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
		sumW[facilities[i]] = 0; // no começo nada foi pago dessa instalacao
		qty_payers[facilities[i]] = 0; // no começo ninguem contribui pra ninguem
		open[facilities[i]] = false; // indica que a instalação não está open inicialmente
		isInTline[facilities[i]] = false; // no começo ninguem esta em Tline
		name[facilities[i]] = qty_clients + i; // nameia de acordo com a numeracao

		// Salvando o valor do maior Fi da entrada
		if(f[facilities[i]]>biggestFi){
			biggestFi = f[facilities[i]];
		}
	}


	// Custo de atribuição é o custo de atribuir o cliente associado à instalação associada
	ListBpGraph::EdgeMap<double> assignment_cost(g);

	// W é a parte do custo da instalação associada que o cliente associado vai contribuir
	ListBpGraph::EdgeMap<double> w(g);

	// Indica se aquele cliente ja está pagando todo seu custo de atribuicao àquela instalação, e esta pronto para começar a aumentar o w
	ListBpGraph::EdgeMap<bool> readyToPayW(g);


	// Criação de edges e atribuição de seus labels
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
			w[edges[counter]] = 0; // inicialmente nao contribui com nada
			readyToPayW[edges[counter]] = false; // inicia com falso em todos

			// Salvando o valor do maior Cij da entrada
			if(assignment_cost[edges[counter]]>biggestCij){
				biggestCij = assignment_cost[edges[counter]];
			}

			counter++;
		}
	}


	if(DEBUG >= DISPLAY_ACTIONS){
		cout << "biggestCij from input: " << biggestCij << endl << "biggestFi from input " << biggestFi << endl;
	}

	int qty_active_clients_S = qty_clients; // Indica a quantidade de clients ainda em S, os clients ativos.

	int qty_open_fac = 0; // indica a quantidade de facilities opens


	// Inicializando a matriz de adjacencia, todos com valor 0
	for(int i=0;i<qty_clients;i++){
		for(int j=0;j<qty_facilities;j++){
			adjacency_matrix[i][j] = 0;
		}
	}

	if(DEBUG >= DISPLAY_TIME){
		cout <<"[TIME] Ending time count for graph creation and its maps" << endl;
		//Finalizando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &finish);

		// Calculando o tempo gasto
		timeSpent =  (finish.tv_sec - start.tv_sec);
		timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
		cout << "[TIME] Time spent: " << timeSpent << " seconds" << endl;
	}

	ListBpGraph S; // Grafo que contem os clients os quais estamos aumentando as variaveis duais
	bpGraphCopy(g,S).run(); // copiando todas as informacoes de g para S
	

	if (DEBUG >= DISPLAY_GRAPH){
		// Percorrendo por todos os nós A - clients
		cout << "Scrolling through all clients" << endl;
		for(ListBpGraph::RedNodeIt n(S); n != INVALID; ++n){
			cout << "id: " << S.id(n)  << " - name: " << name[n] << " - v: " << v[n] << endl;
		}

		// Percorrendo por todos os nós B - facilities
		cout << "Scrolling through all facilities" << endl;
		for(ListBpGraph::BlueNodeIt n(S); n != INVALID; ++n){
			cout << "id: " << S.id(n)  << " - name: " << name[n] << " - f: " << f[n] << " - open: " << open[n] << " - qty_payers: " << qty_payers[n]<< endl;
		}


		// Percorrendo por todos os edges
		cout << "Scrolling through all edges" << endl;
		for(ListBpGraph::EdgeIt e(S); e!= INVALID; ++e){
			cout << "edge id: " << S.id(e) ;
			cout << " - client: " << name[S.u(e)] << " - facility: " << name[S.v(e)];
			cout<< " - ac: " << assignment_cost[e] << " w - " << w[e] << " readyToPayW - " << readyToPayW[e] << endl;
		}

		// Exibindo a matriz de adjacencia
		cout << "Scrolling through adjacency matrix" << endl;
		for(int i=0;i<qty_clients;i++){
			for(int j=0;j<qty_facilities;j++){
				cout<< adjacency_matrix[i][j] << " ";
			}
			cout << endl;
		}
	}
	

	// v <- 0, w <- 0 ja acontece na inicializacao 


	if(DEBUG >= DISPLAY_TIME){
		cout <<"[TIME] Starting time count for initial part execution, before loop" << endl;
		//Iniciando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &start);
	}


	// Aresta auxiliar, inicia com o valor da primeira aresta
	ListBpGraph::Edge min = edges[0]; 

	// Percorrer todas as arestas para ver qual o min cij
	for(ListBpGraph::EdgeIt e(S); e!= INVALID; ++e){
		if(assignment_cost[e] < assignment_cost[min]){
			min = e;
		}
	}

	if(DEBUG >= DISPLAY_ACTIONS){
		cout << "min : " << S.id(min) << " with cost: " << assignment_cost[min] << endl;
	}

	// Percorrer todos os clients para aumentar em todos esse valor
	for(ListBpGraph::RedNodeIt n(S); n != INVALID; ++n){
		v[n] += assignment_cost[min];
	}

	int fac_index;

	// Percorrer todas as arestas para ver quais bateram o custo de atribuicao
	// Entao, acionar a flag readyToPayW e colocar na matriz de adjacencia.
	for(ListBpGraph::EdgeIt e(S); e!= INVALID; ++e){
		if(equal(assignment_cost[e],assignment_cost[min])){ 
			readyToPayW[e] = true;
			qty_payers[S.asBlueNode(S.v(e))] += 1;

			// fac_index = name[S.v(e)] - qty_clients;
			// adjacency_matrix[name[S.u(e)]][fac_index] = 1;
		}
	}


	if(DEBUG >= DISPLAY_TIME){
		cout <<"[TIME] Ending time count for initial part execution, before loop" << endl;
		//Finalizando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &finish);

		// Calculando o tempo gasto
		timeSpent =  (finish.tv_sec - start.tv_sec);
		timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
		cout << "[TIME] Time spent: " << timeSpent << " seconds" << endl;
	}

	
	if (DEBUG >= DISPLAY_GRAPH){
		// Percorrendo por todos os nós A - clients
		cout << "Scrolling through all clients" << endl;
		for(ListBpGraph::RedNodeIt n(S); n != INVALID; ++n){
			cout << "id: " << S.id(n)  << " - name: " << name[n] << " - v: " << v[n] << endl;
		}

		// Percorrendo por todos os nós B - facilities
		cout << "Scrolling through all facilities" << endl;
		for(ListBpGraph::BlueNodeIt n(S); n != INVALID; ++n){
			cout << "id: " << S.id(n)  << " - name: " << name[n] << " - f: " << f[n] << " - open: " << open[n] << " - qty_payers: " << qty_payers[n]<< endl;
		}


		// Percorrendo por todos os edges
		cout << "Scrolling through all edges" << endl;
		for(ListBpGraph::EdgeIt e(S); e!= INVALID; ++e){
			cout << "edge id: " << S.id(e) ;
			cout << " - client: " << name[S.u(e)] << " - facility: " << name[S.v(e)];
			cout<< " - ac: " << assignment_cost[e] << " w - " << w[e] << " readyToPayW - " << readyToPayW[e] << endl;
		}

		// Exibindo a matriz de adjacencia
		cout << "Scrolling through adjacency matrix" << endl;
		for(int i=0;i<qty_clients;i++){
			for(int j=0;j<qty_facilities;j++){
				cout<< adjacency_matrix[i][j] << " ";
			}
			cout << endl;
		}
	}


	double minAB; // receberá o min valor entre o min de A ou o min de B

	double qty_minA;
	double current_qty;

	double qty_minB; 

	int itr_counter = 0; // Variavel auxiliar, que, em caso de DEBUG >= DISPLAY_ACTIONS indica quantas iteracoes houveram no while

	// ****** A partir daqui deve estar em um loop até nao ter mais clients ativos:

	while(qty_active_clients_S > 0){
		
		if(DEBUG >= DISPLAY_ACTIONS){
			cout << endl << "------------------------------ STILL HAVE " << qty_active_clients_S << " ACTIVE CLIENTS ------------------------------" << endl << endl;
		}


		if(DEBUG >= DISPLAY_TIME){
			itr_counter += 1;
			cout <<"[TIME] Starting time count for case A execution, iteration: " << itr_counter << endl;
			//Iniciando a contagem do tempo
			clock_gettime(CLOCK_REALTIME, &start);
		}

		// 	A - fazer um for percorrendo todos os clients e vendo o que resta de cij pra cada (achar o min) (min cij - vj)

		qty_minA = biggestCij + biggestFi; // inicia com o maior valor de cij + maior valor de fi dados na entrada

		for(ListBpGraph::RedNodeIt n(S); n != INVALID; ++n){		// percorre os clients
	
			for (ListBpGraph::IncEdgeIt e(S, n); e != INVALID; ++e) { // Percorre todas arestas desse nó
				if(!readyToPayW[e]){
					current_qty = assignment_cost[e] - v[n];
					if(current_qty < qty_minA){
						qty_minA = current_qty;
					}
				}
			}
		}
		
		if(DEBUG >= DISPLAY_ACTIONS){
			cout << "min qty A: " << qty_minA << endl;
		}


		if(DEBUG >= DISPLAY_TIME){
			cout <<"[TIME] Ending time count for case A execution, iteration: " << itr_counter << endl;
			//Finalizando a contagem do tempo
			clock_gettime(CLOCK_REALTIME, &finish);

			// Calculando o tempo gasto
			timeSpent =  (finish.tv_sec - start.tv_sec);
			timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
			cout << "[TIME] Time spent: " << timeSpent << " seconds" << endl;
		


			cout <<"[TIME] Starting time count for case B execution, iteration: " << itr_counter << endl;
			//Iniciando a contagem do tempo
			clock_gettime(CLOCK_REALTIME, &start);
		}

		// B - fazer um for percorrendo todas as instalações para ver (fi - somatório de tudo que foi contribuído ate então para fi ) / numero de clients prontos para contribuir para fi … os que ja alcançaram cij e ainda estão ativos

		qty_minB = biggestCij + biggestFi; // inicia com o maior valor de cij + maior valor de fi dados na entrada

		for(ListBpGraph::BlueNodeIt n(S); n != INVALID; ++n){ // percorre as facilities

			if(!open[n]){ // se a instalacao ainda nao estava open

				if(qty_payers[n] > 0){ // SE alguem ja esta pronto para contribuir pelo menos

					current_qty = (f[n] - sumW[n])/qty_payers[n]; //(fi - somatório de tudo que foi contribuído ate então para fi ) / (numero de clients prontos para contribuir para fi) 

					if(current_qty < qty_minB){
						qty_minB = current_qty;
					}
				}
			}
		}
		
		if(DEBUG >= DISPLAY_ACTIONS){
			cout << "min qty B: " << qty_minB  << endl;
		}


		if(DEBUG >= DISPLAY_TIME){
			cout <<"[TIME] Ending time count for case B execution, iteration: " << itr_counter << endl;
			//Finalizando a contagem do tempo
			clock_gettime(CLOCK_REALTIME, &finish);

			// Calculando o tempo gasto
			timeSpent =  (finish.tv_sec - start.tv_sec);
			timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
			cout << "[TIME] Time spent: " << timeSpent << " seconds" << endl;
		}

		//	ver o min entre A e B e aumentar esse valor em todos os clients (tanto no cij se ainda faltar quanto no wij se ja estiver contribuindo)

		if(qty_minA < qty_minB){
			minAB = qty_minA;
		}
		else { // inclui caso de empate
			minAB = qty_minB;
		}

		if(DEBUG >= DISPLAY_TIME){
			cout <<"[TIME] Starting time count to update values such as v, w, sumW, readyToPayW, qty_payers, and adjacency_matrix. Iteration: " << itr_counter << endl;
			//Iniciando a contagem do tempo
			clock_gettime(CLOCK_REALTIME, &start);
		}


		// Percorrer todos os clients para aumentar em todos esse valor em v
		for(ListBpGraph::RedNodeIt n(S); n != INVALID; ++n){
			v[n] += minAB;
		}


		// Percorrer todas as arestas para aumentar em w, caso o cliente estivesse pronto para contribuir
		// Aproveitar e ver quais bateram o custo de atribuicao
		// Entao, acionar a flag readyToPayW e colocar na matriz de adjacencia. 
		for(ListBpGraph::EdgeIt e(S); e!= INVALID; ++e){
			if(readyToPayW[e]){ // se está pronto para contribuir
				w[e] += minAB; 
				sumW[S.asBlueNode(S.v(e))] += minAB; // aumenta esse valor no somatorio da instalacao correspondente

				fac_index = name[S.v(e)] - qty_clients;
				adjacency_matrix[name[S.u(e)]][fac_index] = 1; // atribui esse novo cliente em sua lista
			}
			else if(equal(assignment_cost[e],v[S.asRedNode(S.u(e))])){ // SENAO SE: acabou de ficar pronto para contribuir (pagou o c.a.)
				readyToPayW[e] = true;
				qty_payers[S.asBlueNode(S.v(e))] += 1;

				// fac_index = name[S.v(e)] - qty_clients;
				// adjacency_matrix[name[S.u(e)]][fac_index] = 1; // atribui esse novo cliente em sua lista
			}
		}

		if(DEBUG >= DISPLAY_TIME){
			cout <<"[TIME] Ending time count to update values such as v, w, sumW, readyToPayW, qty_payers, and adjacency_matrix. Iteration: " << itr_counter << endl;
			//Finalizando a contagem do tempo
			clock_gettime(CLOCK_REALTIME, &finish);

			// Calculando o tempo gasto
			timeSpent =  (finish.tv_sec - start.tv_sec);
			timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
			cout << "[TIME] Time spent: " << timeSpent << " seconds" << endl;
		}

		// Repetir IF, tratar dos detalhes agora

		// SE FOR O CASO A ou empate: verificar se a instalação que aquele cliente alcançou ja estava open, se sim, remover ele dos ativos
		// if(qty_minA <= qty_minB){
		if(greater_or_equal(qty_minB,qty_minA)){

			if(DEBUG >= DISPLAY_TIME){
				cout <<"[TIME] Starting time count for case details A. Iteration: " << itr_counter << endl;
				//Iniciando a contagem do tempo
				clock_gettime(CLOCK_REALTIME, &start);
			}


			if(DEBUG >= DISPLAY_ACTIONS){
				cout << "Case A!" << endl;
			}

			for(ListBpGraph::EdgeIt e(S); e!= INVALID; ++e){ // percorrer todas as arestas
				if(readyToPayW[e]){ // verificar se o cliente j alcancou a inst i (se ta pronto para contribuir)
					if(open[S.asBlueNode(S.v(e))]){ // verificar se inst i ta open
						delete_clients.insert(S.id(S.u(e))); // Adicionar o cliente j no conjunto de futuros a apagar

						// Tirando ele dos contribuintes dessa instalacao
						readyToPayW[e] = false;
						qty_payers[S.asBlueNode(S.v(e))] -= 1;

						if(DEBUG >= DISPLAY_ACTIONS){
							cout<<"deleted client " << name[S.u(e)] << " from ready to pay of facility " << name[S.v(e)] << endl;
						}

						//Apagando ele da lista de contribuintes das outras facilities
						for (ListBpGraph::IncEdgeIt e2(S, S.u(e)); e2 != INVALID; ++e2) { // Percorre todas arestas desse nó cliente (ligam a facilities)
							if(readyToPayW[e2]){ // se o cliente estava pronto para contribuir com essa instalacao
								readyToPayW[e2] = false;
								qty_payers[S.asBlueNode(S.v(e2))] -= 1;

								if(DEBUG >= DISPLAY_ACTIONS){
									cout<<"deleted client " << name[S.u(e2)] << " from ready to pay of facility " << name[S.v(e2)] << endl;
								}
							}
						}
					}
				}
			}

			if(DEBUG >= DISPLAY_ACTIONS){
				cout << "In case A, size of delete_clients: " << delete_clients.size() << endl;
			}

			if(DEBUG >= DISPLAY_TIME){
				cout <<"[TIME] Ending time count for case details A. Iteration: " << itr_counter << endl;
				//Finalizando a contagem do tempo
				clock_gettime(CLOCK_REALTIME, &finish);

				// Calculando o tempo gasto
				timeSpent =  (finish.tv_sec - start.tv_sec);
				timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
				cout << "[TIME] Time spent: " << timeSpent << " seconds" << endl;
			}

		}

		// SE FOR O CASO B ou empate: caso B seja o min valor: abrir a instalação i e remover os seus contribuintes dos clients ativos (lembrando de remover eles das listas de contribuintes das outras instalações)
		// if(qty_minA >= qty_minB){ 
		if(greater_or_equal(qty_minA,qty_minB)){ 

			if(DEBUG >= DISPLAY_TIME){
				cout <<"[TIME] Starting time count for case B details. Iteration: " << itr_counter << endl;
				//Iniciando a contagem do tempo
				clock_gettime(CLOCK_REALTIME, &start);
			}


			if(DEBUG >= DISPLAY_ACTIONS){
				cout << "Case B!" << endl;
			}

			for(ListBpGraph::BlueNodeIt n(S); n != INVALID; ++n){ // percorrer todas as facilities
				if(!open[n]){ // se a instalacao ainda nao estava open
					cout.precision(10);
					if(greater_or_equal(sumW[n],f[n])){ // se a soma das partes completou o custo de abrir a instalacao, vamos abrir!
						open[n] = true;
						qty_open_fac += 1;

						if(DEBUG >= DISPLAY_ACTIONS){
							cout << "Facility " << name[n] << " should open!!!!" << endl;
							cout << "increasing open fac qty: " << qty_open_fac << endl;
						}

						//Remover os seus contribuintes dos clients ativos

						for (ListBpGraph::IncEdgeIt e(S, n); e != INVALID; ++e) { // Percorre todas arestas desse nó ( ligam a clients )

							if(readyToPayW[e]){ // se esse cliente está pronto para contribuir


								delete_clients.insert(S.id(S.u(e))); // Adicionar o cliente j no conjunto de futuros a apagar

								if(DEBUG >= DISPLAY_ACTIONS){
									cout<<"************* let's delete client " << name[S.u(e)] << endl;
									cout << "In case B, size of delete_clients: " << delete_clients.size() << endl;
								}

								//Apagando ele da lista de contribuintes das outras facilities
								for (ListBpGraph::IncEdgeIt e2(S, S.u(e)); e2 != INVALID; ++e2) { // Percorre todas arestas desse nó cliente (ligam a facilities)
									if(readyToPayW[e2]){ // se o cliente estava pronto para contribuir com essa instalacao
										readyToPayW[e2] = false;
										qty_payers[S.asBlueNode(S.v(e2))] -= 1;

										if(DEBUG >= DISPLAY_ACTIONS){
											cout<<"deleted client " << name[S.u(e2)] << " from ready to pay of facility " << name[S.v(e2)] << " now it has qty_payers = " << qty_payers[S.asBlueNode(S.v(e2))] << endl;
										}
									}
								}
							}
						}
					}
				}
			}

			if(DEBUG >= DISPLAY_TIME){
				cout <<"[TIME] Ending time count for case B details. Iteration:" << itr_counter << endl;
				//Finalizando a contagem do tempo
				clock_gettime(CLOCK_REALTIME, &finish);

				// Calculando o tempo gasto
				timeSpent =  (finish.tv_sec - start.tv_sec);
				timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
				cout << "[TIME] Time spent: " << timeSpent << " seconds" << endl;
			}

		}


		if(DEBUG >= DISPLAY_TIME){
			cout <<"[TIME] Starting time count to clear frozen clients. Iteration: " << itr_counter << endl;
			//Iniciando a contagem do tempo
			clock_gettime(CLOCK_REALTIME, &start);
		}


		for (itr = delete_clients.begin(); itr != delete_clients.end(); ++itr) { // percorrer todos os elementos do conjunto 
			if(DEBUG >= DISPLAY_ACTIONS){
				cout <<"Facility has been opened. Deleting client " << *itr << " from actives "<< endl;
			}

	        S.erase(S.nodeFromId(*itr));  // apagando dos clients ativos
			qty_active_clients_S -= 1;
	    }

	    //Apagar tudo do conjunto (limpar conjunto)
	    delete_clients.erase(delete_clients.begin(), delete_clients.end()); 


	    if(DEBUG >= DISPLAY_TIME){
			cout <<"[TIME] Ending time count to clear frozen clients. Iteration:" << itr_counter << endl;
			//Finalizando a contagem do tempo
			clock_gettime(CLOCK_REALTIME, &finish);

			// Calculando o tempo gasto
			timeSpent =  (finish.tv_sec - start.tv_sec);
			timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
			cout << "[TIME] Time spent: " << timeSpent << " seconds" << endl;
		}


		if (DEBUG >= DISPLAY_GRAPH){
			// Percorrendo por todos os nós A - clients
			cout << "Scrolling through all clients" << endl;
			for(ListBpGraph::RedNodeIt n(S); n != INVALID; ++n){
				cout << "id: " << S.id(n)  << " - name: " << name[n] << " - v: " << v[n] << endl;
			}

			// Percorrendo por todos os nós B - facilities
			cout << "Scrolling through all facilities" << endl;
			for(ListBpGraph::BlueNodeIt n(S); n != INVALID; ++n){
				cout << "id: " << S.id(n)  << " - name: " << name[n] << " - f: " << f[n] << " - open: " << open[n] << " - qty_payers: " << qty_payers[n]<< endl;
			}


			// Percorrendo por todos os edges
			cout << "Scrolling through all edges" << endl;
			for(ListBpGraph::EdgeIt e(S); e!= INVALID; ++e){
				cout << "edge id: " << S.id(e) ;
				cout << " - client: " << name[S.u(e)] << " - facility: " << name[S.v(e)];
				cout<< " - ac: " << assignment_cost[e] << " w - " << w[e] << " readyToPayW - " << readyToPayW[e] << endl;
			}

			// Exibindo a matriz de adjacencia
			cout << "Scrolling through adjacency matrix" << endl;
			for(int i=0;i<qty_clients;i++){
				for(int j=0;j<qty_facilities;j++){
					cout<< adjacency_matrix[i][j] << " ";
				}
				cout << endl;
			}
		}
	}

	/*

	CRIACAO DE Tline

	*/

	// grafo com as facilities opens (no futuro quero que tenha os clients conectados a suas facilities correspondentes)
	ListBpGraph Tline; 

	// Criação de nós de instalações em Tline
	ListBpGraph::BlueNode * facTline;
	facTline = (ListBpGraph::BlueNode*) malloc((qty_facilities) * sizeof(ListBpGraph::BlueNode));
    if(!facTline){
        cout << "Memory Allocation Failed";
        exit(1);
    }

	int qty_facTline = 0; // indica a quantidade de facilities ja em Tline

	// Para identificar cada nó
	ListBpGraph::NodeMap<int> nameTline(Tline);

	// fTline - será o custo de instalação das insts em Tline (fi)
	ListBpGraph::BlueNodeMap<double> fTline(g);

	if(DEBUG >= DISPLAY_ACTIONS){
		cout << endl<< "Criating Tline" << endl;
	}

	fac_index = 0;


	if(DEBUG >= DISPLAY_ADJ_MATRIX){
		// Exibindo a matriz de adjacencia
		cout << "Scrolling through adjacency matrix" << endl;
		for(int i=0;i<qty_clients;i++){
			for(int j=0;j<qty_facilities;j++){
				cout<< adjacency_matrix[i][j] << " ";
			}
			cout << endl;
		}
	}


	// int * vetor;
	// vetor = (int *) malloc(qty_clients * sizeof(int));

	// int counter2 = 0;

	// cout << "open facilities S: " << endl;

	// for(ListBpGraph::BlueNodeIt n(S); n != INVALID; ++n){
	// 	if(open[n]){
	// 		cout << "no id: " << S.id(n)  << " - name: " << name[n] << " - open: " << open[n] << endl;
	// 	}
	// }

	// cout << "LINHA: " << endl;
	// for(int i=0;i<qty_clients;i++){
	// 	if(adjacency_matrix[i][49]){
	// 		cout << i << " ";
	// 		vetor[counter2++] = i;
	// 	}
	// }
	// cout << endl;

	// for(int i=0;i<counter2;i++){
	// 	cout <<"client: " << vetor[i] << " - ";
	// 	for(int j=0;j<qty_facilities;j++){
	// 		if(adjacency_matrix[vetor[i]][j]){
	// 			cout << j << " "; 
	// 		}
	// 	}
	// 	cout << endl;
	// }


	if(DEBUG >= DISPLAY_TIME){
		cout <<"[TIME] Starting time count to model Tline" << endl;
		//Iniciando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &start);
	}

	// Enquanto houverem facilities opens em S ( no algoritmo: while T != vazio )
	ListBpGraph::BlueNodeIt current_fac(S);
	while(qty_open_fac > 0){
		while(current_fac != INVALID){
			if(open[current_fac]){ 
				// Escolha inst open de S
				if(DEBUG >= DISPLAY_ACTIONS){
					cout << "Chosen facility " << name[current_fac] << endl;
				}

				fac_index = name[current_fac] - qty_clients;
				// Tline <- Tline U {i}
				facTline[qty_facTline] = Tline.addBlueNode();
				nameTline[facTline[qty_facTline]] = name[current_fac]; // pega o name da instalacao
				fTline[facTline[qty_facTline]] = f[current_fac]; // pega o custo de abrir a instalacao


				// indica em g que ele está em Tline
				isInTline[current_fac] = true;

				qty_facTline += 1;

				++current_fac;
				break;
			}
			++current_fac;
		}


		if (DEBUG >= DISPLAY_GRAPH){
			// Percorrendo por todos os nós A - clients
			cout << "Scrolling through all clients" << endl;
			for(ListBpGraph::RedNodeIt n(S); n != INVALID; ++n){
				cout << "id: " << S.id(n)  << " - name: " << name[n] << " - v: " << v[n] << endl;
			}

			// Percorrendo por todos os nós B - facilities
			cout << "Scrolling through all facilities" << endl;
			for(ListBpGraph::BlueNodeIt n(S); n != INVALID; ++n){
				cout << "id: " << S.id(n)  << " - name: " << name[n] << " - f: " << f[n] << " - open: " << open[n] << " - qty_payers: " << qty_payers[n]<< endl;
			}


			// Percorrendo por todos os edges
			cout << "Scrolling through all edges" << endl;
			for(ListBpGraph::EdgeIt e(S); e!= INVALID; ++e){
				cout << "edge id: " << S.id(e) ;
				cout << " - client: " << name[S.u(e)] << " - facility: " << name[S.v(e)];
				cout<< " - ac: " << assignment_cost[e] << " w - " << w[e] << " readyToPayW - " << readyToPayW[e] << endl;
			}
		}


		// TODO: pensar em usar um conjunto aqui: para cada instalacao ter um conjunto dos clients que contribuem e para cada cliente ter um conjunto de facilities elas contribuem
		// talvez eliminar o for mais interno com a adicao de um novo vetor, apenas para indicar se ele ainda esta em T ou nao. Dentro do for ao inves de zerar a coluna, muda o valor desse vetor pra false

		//	Remove todas facilities i se algum cliente j contribuir a i e fac_index
		for(int i=0;i<qty_clients;i++){

			if(adjacency_matrix[i][fac_index]==1){ // se o cliente i contribui para a inst fac_index			

				adjacency_matrix[i][fac_index] = 0; 

				for(int j=0;j<qty_facilities;j++){ // ve todas as facilities que esse cliente contribui tambem e remove


					if(adjacency_matrix[i][j]){ // nao tem perigo de pegar a inst inicial pois ja zeramos o seu valor correspondente

						// Limpando a matriz de adjacencia naquela coluna
						for(int k=0;k<qty_clients;k++){ // marcar que todas os clients daquela instalacao agr nao contribuem mais pra ela
							adjacency_matrix[k][j] = 0;

							if (DEBUG >= DISPLAY_ADJ_MATRIX){

								// Exibindo a matriz de adjacencia
								cout << "Scrolling through adjacency matrix" << endl;
								for(int i2=0;i2<qty_clients;i2++){
									for(int j2=0;j2<qty_facilities;j2++){
										cout<< adjacency_matrix[i2][j2] << " ";
									}
									cout << endl;
								}
							}

						}

						if(open[S.asBlueNode(S.nodeFromId(j + qty_clients))]){ // Se a instalacao j está open

							if(DEBUG >= DISPLAY_ACTIONS){
								cout << "Deleting facility " << j << " from opens, since client " << i << " pays to her" << endl;
							}

							open[S.asBlueNode(S.nodeFromId(j + qty_clients))] = false;
							qty_open_fac -= 1;
						}
					}
				}
			}
		}

		if(DEBUG >= DISPLAY_ACTIONS){
			cout << "Deleting chosen facility " << fac_index + qty_clients << " from opens" << endl;		
		}

		open[S.asBlueNode(S.nodeFromId(fac_index + qty_clients))] = false;
		qty_open_fac -= 1;
	}


	if(DEBUG >= DISPLAY_TIME){
		cout <<"[TIME] Ending time count for Tline modeling" << endl;
		//Finalizando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &finish);

		// Calculando o tempo gasto
		timeSpent =  (finish.tv_sec - start.tv_sec);
		timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
		cout << "[TIME] Time spent: " << timeSpent << " seconds" << endl;
	}



	if(DEBUG >= DISPLAY_ADJ_MATRIX){
		cout << "Scrolling through adjacency matrix" << endl;
		for(int i=0;i<qty_clients;i++){
			for(int j=0;j<qty_facilities;j++){
				cout<< adjacency_matrix[i][j] << " ";
			}
			cout << endl;
		}
	}

	// // Percorrendo por todos os nós de Tline que sao facilities
	int qty_fac_Tline = 0;

	if(DEBUG >= DISPLAY_ACTIONS){

		cout<<"-- Tline facilities --" << endl;
		for(ListBpGraph::BlueNodeIt n(Tline); n != INVALID; ++n){
			qty_fac_Tline += 1;
			cout << "id: " << Tline.id(n) << " - name: " << nameTline[n] << endl;
		}
		cout<<"-------" << endl;
	}
	else if(DEBUG == 0){ // preciso fazer essa contagem de qualquer forma
		for(ListBpGraph::BlueNodeIt n(Tline); n != INVALID; ++n){
			qty_fac_Tline += 1;
		}
	}


	if(DEBUG >= DISPLAY_TIME){
		cout <<"[TIME] Starting time count to open everything from Tline and associate clients" << endl;
		//Iniciando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &start);
	}

	// Abrir todas as facilities em Tline e atribuir cada cliente à instalacao mais próxima

	// Criação de nós de clients em Tline
	ListBpGraph::RedNode * cliTline;
	cliTline = (ListBpGraph::RedNode*) malloc((qty_clients) * sizeof(ListBpGraph::RedNode));
    if(!cliTline){
        cout << "Memory Allocation Failed";
        exit(1);
    }


	int qty_cliTline = 0; // indica a quantidade de clients ja em Tline


	// acTline - em Tline indica o Custo de atribuição: é o custo de atribuir o cliente associado à instalação associada
	ListBpGraph::EdgeMap<double> acTline(g);

	// Criação de edges e atribuição de seus labels. Obs: criando no tamanho maximo necessario, limitado pelo numero de inst ja em Tline
	ListBpGraph::Edge * edgesTline;
	edgesTline = (ListBpGraph::Edge*) malloc((qty_clients*qty_fac_Tline) * sizeof(ListBpGraph::Edge));
    if(!edgesTline){
        cout << "Memory Allocation Failed";
        exit(1);
    }

	int qty_edgesTline = 0; // indica a quantidade de edges ja criados em Tline


	double minDistance = biggestCij;
	int nameCliMinDist = -1;
	int nameFacMinDist = -1;

	int idFacMinDist = -1;


	//TODO: Talvez mudar aqui o jeito de fazer pra percorrer menos gente
	// Percorrer todos os clients de g
	for(ListBpGraph::RedNodeIt n(g); n != INVALID; ++n){
		for (ListBpGraph::IncEdgeIt e(g, n); e != INVALID; ++e) { // Percorre todas arestas desse nó (ligam a facilities)
			if(isInTline[g.asBlueNode(g.v(e))]){ // se a instalacao correspondente está em Tline
				if(assignment_cost[e] <= minDistance){ // encontra a instalacao (que está em Tline) que possui a min distancia
					minDistance = assignment_cost[e];
					nameCliMinDist = name[g.u(e)];
					nameFacMinDist = name[g.v(e)];
				}
			}
		}

		// Adiciona esse cliente em Tline
		cliTline[qty_cliTline] = Tline.addRedNode();
		nameTline[cliTline[qty_cliTline]] = nameCliMinDist; // pega o name do cliente
		qty_cliTline += 1;


		//SOLUÇÃO: TALVEZ criar um vetor que dado o name da inst, tem o valor do ID de Tline.. criar isso quando cria o Tline
		//PROBLEMA: Gambiarra 
		// Descobrir o ID em Tline da instalacao correspondente a nameFacMinDist
		for(int i=0;i<qty_facTline;i++){
			if(nameTline[facTline[i]] == nameFacMinDist){
				idFacMinDist = i;
				break;
			}
		}

		// Cria aresta para associar esse cliente com a instalacao que possui a min distancia dele
		edges[qty_edgesTline] = Tline.addEdge(cliTline[qty_cliTline-1],facTline[idFacMinDist]);
		acTline[edges[qty_edgesTline]] = minDistance; // pega o custo de atribuicao

		qty_edgesTline += 1;

		// Zerando valores
		minDistance = biggestCij;
		nameCliMinDist = -1;
		nameFacMinDist = -1;
		idFacMinDist = -1;
	}

	if(DEBUG >= DISPLAY_TIME){
		cout <<"[TIME] Ending time count to open everything from Tline and associate clients" << endl;
		//Finalizando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &finish);

		// Calculando o tempo gasto
		timeSpent =  (finish.tv_sec - start.tv_sec);
		timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
		cout << "[TIME] Time spent: " << timeSpent << " seconds" << endl;
	
		cout <<"[TIME] Starting time count to display final answer" << endl;
		//Iniciando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &start);
	}


	//Resposta final: Grafo Tline
	if(DEBUG >= DISPLAY_ACTIONS){
		cout << endl <<  "Final answer: Tline graph" << endl;

		// Percorrendo por todos os nós A - clients
		cout << endl <<"Scrolling through all clients" << endl;
		for(ListBpGraph::RedNodeIt n(Tline); n != INVALID; ++n){
			cout << "id: " << S.id(n)  << " - name: " << name[n] << endl;
		}

	// Percorrendo por todos os nós B - facilities
		cout << endl << "Scrolling through all facilities" << endl;
	}

	for(ListBpGraph::BlueNodeIt n(Tline); n != INVALID; ++n){
		if(DEBUG >= DISPLAY_ACTIONS){
			cout << "id: " << Tline.id(n)  << " - name: " << nameTline[n] << " - f: " << fTline[n] << endl;
		}

		solution.finalTotalCost += fTline[n]; // acrescentando o valor do custo de abrir essa instalacao
	}


	// Percorrendo por todos os edges
	if(DEBUG >= DISPLAY_ACTIONS){
		cout << endl << "Scrolling through all edges" << endl;
	}

	counter = 0;
	for(ListBpGraph::EdgeIt e(Tline); e!= INVALID; ++e){
		if(DEBUG >= DISPLAY_ACTIONS){
			cout << "edge id: " << S.id(e) ;
				cout << " - client: " << name[S.u(e)] << " - facility: " << name[S.v(e)];
				cout<< " - ac: " << assignment_cost[e] << endl;
		}

		solution.finalTotalCost += acTline[e]; // acrescentando o valor de atribuicao desse cliente a essa instalacao

		// Colocando as facilities opens mais proximas em um vetor pra retornar na solucao
		fac_index = nameTline[Tline.v(e)] - qty_clients; // colocando -qty_clients para colocar o id correto da inst
		solution.assigned_facilities[counter] = fac_index;  // esse loop percorre todos os clients, então posso usar o counter normalmente aqui
		// cout << "cliente " << counter << " com inst " << solution.assigned_facilities[counter] << endl;

		counter += 1;
	}

	if(DEBUG >= DISPLAY_TIME){
		cout <<"[TIME] Ending time count to display final answer" << endl;
		//Finalizando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &finish);

		// Calculando o tempo gasto
		timeSpent =  (finish.tv_sec - start.tv_sec);
		timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
		cout << "[TIME] Time spent: " << timeSpent << " seconds" << endl;
	

		// FINALIZANDO A CONTAGEM DE TEMPO DA FUNCAO COMO UM TODO
		clock_gettime(CLOCK_REALTIME, &real_finish);

		// Calculando o tempo gasto total
		realTimeSpent =  (real_finish.tv_sec - real_start.tv_sec);
		realTimeSpent += (real_finish.tv_nsec - real_start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
		cout << "Final Total Function Time: " << realTimeSpent << " seconds" << endl;
	}
	

	free(adjacency_matrix);
	free(clients);
	free(facilities);
	free(edges);
	free(facTline);
	free(edgesTline);

	return(solution);
}
