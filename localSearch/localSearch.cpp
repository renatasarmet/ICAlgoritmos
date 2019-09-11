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
#define DISPLAY_ADJACENCY_MATRIX 4 // corresponde à parte final, na criacao de Tlinha

#define DEBUG 0 // OPCOES DE DEBUG: 1 PARA EXIBIR ACOES, 2 PARA EXIBIR TEMPO, 3 PARA EXIBIR AS MUDANÇAS NO GRAFO, 4 PARA EXIBIR AS MUDANCAS NA MATRIZ DE ADJACENCIA NA CRIACAO DE TLINHA

// void mergeSort(int *vetor, int *vetorID, int posicaoInicio, int posicaoFim) {

//     int i, j, k, metadeTamanho, *vetorTemp, *vetorTempID;
//     if(posicaoInicio == posicaoFim) return;
//     metadeTamanho = (posicaoInicio + posicaoFim ) / 2;

//     mergeSort(vetor, vetorID, posicaoInicio, metadeTamanho);
//     mergeSort(vetor, vetorID, metadeTamanho + 1, posicaoFim);

//     i = posicaoInicio;
//     j = metadeTamanho + 1;
//     k = 0;
//     vetorTemp = (int *) malloc(sizeof(int) * (posicaoFim - posicaoInicio + 1));
//     vetorTempID = (int *) malloc(sizeof(int) * (posicaoFim - posicaoInicio + 1));

//     while(i < metadeTamanho + 1 || j  < posicaoFim + 1) {
//         if (i == metadeTamanho + 1 ) { 
//             vetorTemp[k] = vetor[j];
//             vetorTempID[k] = vetorID[j];
//             j++;
//             k++;
//         }
//         else {
//             if (j == posicaoFim + 1) {
//                 vetorTemp[k] = vetor[i];
//                 vetorTempID[k] = vetorID[i];
//                 i++;
//                 k++;
//             }
//             else {
//                 if (vetor[i] < vetor[j]) {
//                     vetorTemp[k] = vetor[i];
//                 	vetorTempID[k] = vetorID[i];
//                     i++;
//                     k++;
//                 }
//                 else {
//                     vetorTemp[k] = vetor[j];
//                 	vetorTempID[k] = vetorID[j];
//                     j++;
//                     k++;
//                 }
//             }
//         }

//     }
//     for(i = posicaoInicio; i <= posicaoFim; i++) {
//         vetor[i] = vetorTemp[i - posicaoInicio];
//         vetorID[i] = vetorTempID[i - posicaoInicio];
//     }
//     free(vetorTemp);
//     free(vetorTempID);
// }


// void excluindo_clientes_nao_ativos(int *vetor, int *vetorID, int tam_vetor, int *clientes_apagar, int quantidade_apagar){
// 	int qtd_restante = quantidade_apagar;
// 	for(int i=0;i<tam_vetor;i++){
// 		if(qtd_restante == 0){
// 			break;
// 		}
		
// 		for(int j=0;j<quantidade_apagar;j++){
// 			if(vetorID[i]==clientes_apagar[j]){
// 				// cout << "VAMOS APAGAR O CLINETE " << clientes_apagar[j] << endl;
// 				// vetorID[i] = -1; // ID -1 indica que nao esta ativo
// 				vetor[i] = -1; // coloca cij = -1, indicando que ja foi atribuido
// 				qtd_restante -= 1;
// 				break;
// 			}
// 		}
// 	}

// 	// Colocando novamente em ordem
// 	mergeSort(vetor, vetorID, 0, (tam_vetor-1));
// }



// // Por referencia diz qual o melhor tamanho para Y e seu respectivo custo
// void melhor_subconjunto(int &melhor_tamanho, double &melhor_custo, int *vetor, double fi, double ganho, int index_inicio, int tam_vetor, double maior_cij){
// 	int tamanho_atual = 1;
// 	double soma_custo_atual = 0;
// 	double custo_atual = 0;
// 	melhor_custo = fi + maior_cij + 1; // limitante superior


// 	// inicializando só para nao ficar nulo se algo der errado
// 	melhor_tamanho = 1;

// 	// ira criar (tam_vetor - index_inicio) subconjuntos
// 	for(int i=index_inicio;i<tam_vetor;i++){
// 		soma_custo_atual = fi - ganho;
// 		// vendo a soma dos custos desse subconjunto
// 		for(int j=index_inicio;j<=i;j++){
// 			soma_custo_atual += vetor[j];
// 		}

// 		custo_atual = soma_custo_atual / tamanho_atual;

// 		if(custo_atual <= melhor_custo){
// 			melhor_custo = custo_atual;
// 			melhor_tamanho = tamanho_atual;
// 		}

// 		tamanho_atual += 1;
// 	}

// }


// bool igual(double i, double j){
//     if((i > j-EPSL) && (i < j+EPSL))
//         return true;
//     return false;
// }

// bool maior_igual(double i, double j){
//     if(i >= j * (1-EPSL))
//         return true;
//     return false;
// }


// Retornar o valor da solucao
solutionType localSearch(int qtyFac, double * costF, double * costA, solutionType solution){

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

	int qty_clients = solution.qty_clients; // Indica quantidade de clientes
	int qty_facilities = qtyFac; // Indica quantidade de instalacoes

	
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

	// name - Para identificar cada nó
	ListBpGraph::NodeMap<int> name(g);


	// // ordem_cij - será os clientes ativos ordenados pelo cij àquela instalação
	// int **ordem_cij = (int**) malloc((qty_facilities) * sizeof(int*));

	// // ordem_cijID - indica os ID dos clientes ativos ordenados pelo cij àquela instalação
	// int **ordem_cijID = (int**) malloc((qty_facilities) * sizeof(int*));

	// for(int i = 0; i < qty_facilities; i++) {
	// 	ordem_cij[i] = (int *)malloc(qty_clients * sizeof(int));
	// 	ordem_cijID[i] = (int *)malloc(qty_clients * sizeof(int));
	// }

    // if((!ordem_cij)||(!ordem_cijID)){
    //     cout << "Memory Allocation Failed";
    //     exit(1);
    // }
	

	if(DEBUG >= DISPLAY_TIME){
		cout <<"[TIME] Starting time counter for graph and maps creation." << endl;
		//Iniciando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &start);
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
		c_minX[clients[i]] = -1; // ERRADO AQUI, VER OQ FAREI.. mas inicialmente ja esta conectado com alguem salvo em solution.assigned_facilities[i]
		name[clients[i]] = i; // nomeia de acordo com a numeracao
		nearest_open_fac[clients[i]] = solution.assigned_facilities[i]; // indica com qual inst está conectado inicialmente 

		if(DEBUG >= DISPLAY_ACTIONS){
			cout << "Client " << i << " - initial nearest open facility: " << nearest_open_fac[clients[i]] << endl;
		}
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
		name[facilities[i]] = qty_clients + i; // nomeia de acordo com a numeracao

		// ERRADOOOO, JA TEM UMAS INST ABERTA INICIALMENTE.. VER COMO VOU FAZER
		open[facilities[i]] = false; // indica que a instalação não está aberta inicialmente ERRADOOOOOO
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


	if(DEBUG >= DISPLAY_TIME){
		cout <<"[TIME] Finishing time counter for graph and maps creation." << endl;
		//Finalizando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &finish);

		// Calculando o tempo gasto
		timeSpent =  (finish.tv_sec - start.tv_sec);
		timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
		cout << "[TIME] Time spent: " << timeSpent << " seconds" << endl;
	}
	

	if (DEBUG >= DISPLAY_GRAPH){
		// Percorrendo por todos os nós A - clientes
		cout << "Scrolling through all clients" << endl;
		for(ListBpGraph::RedNodeIt n(g); n != INVALID; ++n){
			cout << "no id: " << g.id(n)  << " - nome: " << name[n] << endl;
		}

		// Percorrendo por todos os nós B - instalacoes
		cout << "Scrolling through all facilities" << endl;
		for(ListBpGraph::BlueNodeIt n(g); n != INVALID; ++n){
			cout << "no id: " << g.id(n)  << " - nome: " << name[n] << " - f: " << f[n] << " - aberta: " << open[n] << endl;
		}

		// Percorrendo por todos os arcos
		cout << "Scrolling through all edges" << endl;
		for(ListBpGraph::EdgeIt e(g); e!= INVALID; ++e){
			cout << "edge id: " << g.id(e) ;
			cout << " - client: " << name[g.u(e)] << " - facility: " << name[g.v(e)];
			cout<< " - ac: " << assignment_cost[e] << endl;
		}
	}


	// // colocando para cada instalacao a ordem dos cij
	// for(ListBpGraph::BlueNodeIt n(g); n != INVALID; ++n){		// percorre as instalacoes
	// 	counter = 0;
	// 	// colocando no vetor que sera ordenado
	// 	for (ListBpGraph::IncEdgeIt e(g, n); e != INVALID; ++e) { // Percorre todas arestas desse nó
	// 		ordem_cijID[g.id(n)][counter] = g.id(g.u(e)); // linha da instalacao, coluna contando em counter
	// 		ordem_cij[g.id(n)][counter] = assignment_cost[e]; // linha da instalacao, coluna contando em counter
	// 		counter += 1;
	// 	}

	// 	// ordenando o vetor
	// 	mergeSort(ordem_cij[g.id(n)], ordem_cijID[g.id(n)], 0, (counter-1));

	// 	if (DEBUG >= DISPLAY_ACTIONS){
	// 		cout << "SORTED BY FACILITY " << g.id(n) << endl;
	// 		for(int i=0;i<counter;i++){
	// 			cout << ordem_cij[g.id(n)][i] << endl; // Adicionar o cliente j no conjunto de 
	// 		}
	// 		cout << "-------" << endl;
	// 	}

	// }


	// int id_inst_escolhida;
	// double melhor_custo_escolhido;

	// double ganho_cij;

	// double finalTotalCost = 0;


	// ****** A partir daqui deve estar em um loop até NAO TER MAIS MELHORAS POSSIVEIS

	// while( NAO TEM MAIS MELHORAS ?? ){

		// if(DEBUG >= DISPLAY_ACTIONS){
		// 	cout << endl << "------------------------------ PROXIMA ITERACAO ------------------------------" << endl << endl;
		// }

		// id_inst_escolhida = -1;
		
		// for(ListBpGraph::BlueNodeIt n(g); n != INVALID; ++n){		// percorre as instalacoes
		// 	// Calculando ganho de troca de atribuições
		// 	// \sum_{j notin g} (c(j,X), cij)+
		// 	ganho_cij = 0;
			
		// 	for (ListBpGraph::IncEdgeIt e(g, n); e != INVALID; ++e) { // Percorre todas arestas desse nó
		// 		if(c_minX[g.asRedNode(g.u(e))] > assignment_cost[e]){
		// 			ganho_cij += c_minX[g.asRedNode(g.u(e))] - assignment_cost[e];
		// 		}
		// 	}
		// 	// cout << "inst = " << g.id(n) << " -- GANHO: " << ganho_cij << endl;

		// 	// TALVEZ AQUI VAI SER "MELHOR OPERACAO"
		// 	// melhor_subconjunto(tam_Y_atual, custo_Y_atual, ordem_cij[g.id(n)], f[n], ganho_cij, qtd_cli_nao_ativos, qty_clients, maiorCij);

		// 	// // Atualizando valores do melhor custo
		// 	// if(custo_Y_atual < melhor_custo_escolhido){
		// 	// 	melhor_custo_escolhido = custo_Y_atual;
		// 	// 	id_inst_escolhida = g.id(n);
		// 	// }

		// }

		// cout << "Vamos abrir a instalacao " << id_inst_escolhida << " e conectar " << melhor_tam_escolhido << " clientes com custo " << melhor_custo_escolhido << endl;
		// finalTotalCost += f[facilities[id_inst_escolhida]]; // Somando os custos de abertura das instalacoes escolhidas para abrir



		// ERRADO .. ACHO QUE NAO VAI USAR ISSOO
		// // Alterando o valor correspondente em ordem_cij e ordem_cijID para -1 em todas as instalacoes
		// for(ListBpGraph::BlueNodeIt n(g); n != INVALID; ++n){		// percorre as instalacoes
		// 	excluindo_clientes_nao_ativos(ordem_cij[g.id(n)], ordem_cijID[g.id(n)], qty_clients, apagar_clientes, melhor_tam_escolhido);
		// }


		// // Atualizando o valor de c(j,X) para os clientes que ja nao eram ativos
		// for(ListBpGraph::RedNodeIt n(g); n != INVALID; ++n){		// percorre os clientes
		// 	if(c_minX[n] > assignment_cost[findEdge(g, n, facilities[id_inst_escolhida])]){
		// 		c_minX[n] = assignment_cost[findEdge(g, n, facilities[id_inst_escolhida])];
		// 		nearest_open_fac[n] = id_inst_escolhida; // nao decidi ainda se aqui vai ser o ID ou o nome

		// 		// cout << "Para ID = " << g.id(n) << " e inst: " << name[facilities[id_inst_escolhida]] << endl;
		// 		// cout << "atualizando aqui: " << c_minX[n] << endl;
		// 	}
		// }

		// // Atualizando o valor de c(j,X) para todos os clientes agora atribuidos a i
		// for(int i=0;i<melhor_tam_escolhido;i++){
		// 	c_minX[clients[apagar_clientes[i]]] = assignment_cost[findEdge(g, clients[apagar_clientes[i]], facilities[id_inst_escolhida])];
		// 	nearest_open_fac[clients[apagar_clientes[i]]] = id_inst_escolhida; // nao decidi ainda se aqui vai ser o ID ou o nome
		// 	// cout << "Para ID = " << apagar_clientes[i] << " e inst: " << name[facilities[id_inst_escolhida]] << endl;
		// 	// cout << "temos cij = " << assignment_cost[findEdge(g, clients[apagar_clientes[i]], facilities[id_inst_escolhida])] << endl;
		// 	// cout << "atualizando aqui: " << c_minX[clients[apagar_clientes[i]]] << " e id= " << nearest_open_fac[clients[apagar_clientes[i]]] << endl;
		// }

	// }



	cout << "FINAL TOTAL COST: " << solution.finalTotalCost << endl;


	if(DEBUG >= DISPLAY_TIME){

		// FINALIZANDO A CONTAGEM DE TEMPO DA FUNCAO COMO UM TODO
		clock_gettime(CLOCK_REALTIME, &real_finish);

		// Calculando o tempo gasto total
		realTimeSpent =  (real_finish.tv_sec - real_start.tv_sec);
		realTimeSpent += (real_finish.tv_nsec - real_start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
		cout << "Final Total Function Time: " << realTimeSpent << " seconds" << endl;
	}

	// free(ordem_cij);
	// free(ordem_cijID);
	free(clients);
	free(facilities);
	free(edges);

	return(solution);
}
