#include <lemon/list_graph.h>
#include <algorithm>
#include <iostream>
#include <set>
#include <iterator>
#include <ctime>
#include "definitions.hpp"
#define EPSL 0.00000001

// Observacao importante: ao usar g.nodeFromId() eh necessario passar o ID geral, nao o blue ID nem red ID. 
// No caso do cliente, o red ID = ID geral, pois os clientes foram os primeiros a serem adicionados no grafo.
// No caso da instalacao, o ID geral = blue ID + qty_clients, pois as instalacoes foram adicionadas no grafo logo apos todos os clientes.

using namespace lemon;
using namespace std;

#define DISPLAY_ACTIONS 1 // corresponde a todos os cout quando uma acao é feita. 
#define DISPLAY_TIME 2 // corresponde aos calculos de tempo 
#define DISPLAY_GRAPH 3 // corresponde a descricao dos clientes, instalacoes e arcos

#define DEBUG 0 // OPCOES DE DEBUG: 1 PARA EXIBIR OS MOVIMENTOS REALIZADOS, 2 PARA EXIBIR ACOES, 3 PARA EXIBIR TEMPO, 4 PARA EXIBIR AS MUDANÇAS NO GRAFO


// Retornar o valor da solucao
solutionType randRounding(int qty_facilities, int qty_clients, double * costF, double * costA, double ** x_values, double * v_values){

	cout << fixed;
   	cout.precision(5);

	// Struct que vai retornar a solução
	solutionType solution;

	// Inicializando struct 
	solution.qty_clients = qty_clients;
	solution.finalTotalCost = 0;
	solution.timeSpent = 0;
	////// OBS: solution.assigned_facilities não está inicializado, ver se precisaria

	/* Inicio declaracoes variaveis para calculo de tempo */

	// Declaracao variaveis que indicam o tempo do programa como um todo
	struct timespec real_start, real_finish;

	// Declaracao variavel que marcara o tempo de execucao da funcao como um todo
	double realTimeSpent;

	/* Fim declaracoes para calculo de tempo */

	if(DEBUG >= DISPLAY_TIME){
		// INICIANDO A CONTAGEM DE TEMPO DA FUNCAO COMO UM TODO
		clock_gettime(CLOCK_REALTIME, &real_start);
	}

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

	// f - será o custo de instalação (fi)
	ListBpGraph::BlueNodeMap<double> f(g);

	// open - Indica se a instalação está aberta ou não
	ListBpGraph::BlueNodeMap<bool> open(g);

	// active - indica se o cliente está ativo ainda ou nao, isto é, se ele ainda nao foi conectado a nenhuma instalacao aberta. if j \in g
	ListBpGraph::RedNodeMap<bool> active(g);

	// name - Para identificar cada nó
	ListBpGraph::NodeMap<int> name(g);


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
		name[facilities[i]] = qty_clients + i; // nomeia de acordo com a numeracao
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
		nearest_open_fac[clients[i]] = -1; // indica que não está conectado com nenhuma inst inicialmente 
		c_minX[clients[i]] = -1; // indica que nao está conectado com ninguém inicialmente //PROBLEMA: nao tenho ctz ainda qual a melhor inicializacao
		active[clients[i]] = true;
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

			counter++;
		}
	}


	if (DEBUG >= DISPLAY_GRAPH){
		// Percorrendo por todos os nós A - clientes
		cout << "Scrolling through all clients" << endl;
		for(ListBpGraph::RedNodeIt n(g); n != INVALID; ++n){
			cout << "node id: " << g.id(n)  << " - name: " << name[n] << " - active: " << active[n] << endl;
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

		cout << "-----PRIMAL SOLUTION:" << endl;

		// Percorrendo por todos os x_values do primal
		cout << "Scrolling through all x_values" << endl;
		for(int i=0;i<qty_facilities;i++){
			for(int j=0;j<qty_clients;j++){
				cout << "x_values[" << i << "][" << j << "] = " << x_values[i][j] << endl;
			}
		}

		cout << "-----DUAL SOLUTION:" << endl;

		// Percorrendo por todos os v_values do dual
		cout << "Scrolling through all v_values" << endl;
		for(int j=0;j<qty_clients;j++){
			cout << "v_values[" << j << "] = " << v_values[j] << endl;
		}

	}


	int qty_active_clients = qty_clients; // Indica a quantidade de clientes ainda em g ativos.


	// ****** A partir daqui deve estar em um loop até todos os clientes se conectarem a alguma instalacao


	while(qty_active_clients > 0){

		if(DEBUG >= DISPLAY_ACTIONS){
			cout << endl << "------------------------------ WE STILL HAVE " << qty_active_clients << " ACTIVE CLIENTS ------------------------------" << endl << endl;
		}

		// VOU APAGAR ISSO, COLOCANDO SO PRA FINS DE TESTE
		qty_active_clients -= 1;
	}

	cout << "FINAL TOTAL COST: " << solution.finalTotalCost << endl;

	if(DEBUG >= DISPLAY_TIME){

		// FINALIZANDO A CONTAGEM DE TEMPO DA FUNCAO COMO UM TODO
		clock_gettime(CLOCK_REALTIME, &real_finish);

		// Calculando o tempo gasto total
		realTimeSpent =  (real_finish.tv_sec - real_start.tv_sec);
		realTimeSpent += (real_finish.tv_nsec - real_start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
		cout << "Final Total Function Time: " << realTimeSpent << " seconds" << endl;
	}

	free(clients);
	free(facilities);
	free(edges);

	return(solution);
}
