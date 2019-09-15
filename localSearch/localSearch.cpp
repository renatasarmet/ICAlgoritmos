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

	// Variavel que armazena o maior valor cij dado na entrada, para uso posterior
	double biggestCij = 0;
	
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

	// temp_nearest_fac - será correnpondente ao ID da instalacao i mais proxima, depois da inst que ela ja esta conectada (temporario)
	ListBpGraph::RedNodeMap<int> temp_nearest_fac(g);

	// f - será o custo de instalação (fi)
	ListBpGraph::BlueNodeMap<double> f(g);

	// open - Indica se a instalação está aberta ou não
	ListBpGraph::BlueNodeMap<bool> open(g);

	// name - Para identificar cada nó
	ListBpGraph::NodeMap<int> name(g);


	// conjunto de instalacoes que encontram-se abertas no momento
	set <int, greater <int> > open_facilities; 

	// conjunto de instalacoes que encontram-se fechadas no momento
	set <int, greater <int> > closed_facilities; 

	// Iterator para os conjuntos 
	set <int, greater <int> > :: iterator itr; 


	if(DEBUG >= DISPLAY_TIME){
		cout <<"[TIME] Starting time counter for graph and maps creation." << endl;
		//Iniciando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &start);
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

		if(DEBUG >= DISPLAY_ACTIONS){
			cout << "Client " << i << " - initial nearest open facility: " << nearest_open_fac[clients[i]] << endl;
		}
	}


	// Iniciando o conjunto de instalacoes abertas e fechadas
	for(int i=0;i<qty_facilities;i++){
		if(open[facilities[i]]){
			open_facilities.insert(i);
		}
		else{
			closed_facilities.insert(i);
		}
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

	// Atualizando valor c_minX para todos os clientes
	for(int i=0;i<qty_clients;i++){
		c_minX[clients[i]] = assignment_cost[findEdge(g, clients[i], facilities[solution.assigned_facilities[i]])]; 
	}


	if(DEBUG >= DISPLAY_ACTIONS){
		cout << "biggestCij from input : " << biggestCij << endl;
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


	// Variavel que indica que ja encontramos o otimo local -- condicao de parada do loop
	bool local_optimum = false;


	// double melhor_custo_escolhido;

	// Variavel que indica o custo extra de abrir ou fechar uma certa instalacao, ou efetuar uma troca (contando com as reatribuicoes)
	double extra_cost;

	// Variavel que indica o melhor custo cij encontrado pra reatribuicao
	double best_cij_reassignment;


	// ****** A partir daqui deve estar em um loop até nao ter mais melhoras possiveis, isto eh, encontrar o otimo local

	while(!local_optimum){

		if(DEBUG >= DISPLAY_ACTIONS){
			cout << endl << "------------------------------ NEXT MOVE ------------------------------" << endl << endl;
		}
		
		// Primeiramente vamos percorrer todas as instalacoes
		// Caso a instalacao esteja aberta, vamos tentar fechá-la e ver se há alguma melhora
		// Lembrando que temos que reatribuir todas os clientes a ela conectados, mudando para a prox inst mais proxima
		// Se houver melhora no custo total, efetuamos de fato a operacao
		// Da mesma forma, caso a instalacao esteja fechada, vamos tentar abri-la e ver se melhora
		// Lembrando que temos que buscar todos os clientes que possuem ela como inst aberta mais proxima para conecta-los
		// Esse teste sera sequencialmente feito com um for ate o final de percorrer as instalacoes
		// Apos isso, na proxima iteracao do while o for será repetido
		// Ainda faltará encaixar a operação de troca - FUTURO

		for(ListBpGraph::BlueNodeIt n(g); n != INVALID; ++n){		// percorre as instalacoes

			if(DEBUG >= DISPLAY_ACTIONS){
				cout << "Let's move facility " << name[n] - qty_clients << endl;
			}

			if(open[n]){ // caso a inst esteja aberta

				if(DEBUG >= DISPLAY_ACTIONS){
					cout << "It is open, let's close it" << endl;
				}

				// Vamos fechar essa instalacao

				extra_cost = -f[n]; // subtrai o custo de abertura dessa inst

				for(ListBpGraph::RedNodeIt n_cli(g); n_cli != INVALID; ++n_cli){		// percorre os clientes

					if(nearest_open_fac[n_cli] == g.id(n)){ // caso esse cliente esteja conectado a essa instalacao

						if(DEBUG >= DISPLAY_ACTIONS){
							cout << "client: " << name[n_cli] << " nearest facility " << name[n] - qty_clients << endl;
						}

						best_cij_reassignment = biggestCij + 1; // limitante superior tranquilo

						for (itr = open_facilities.begin(); itr != open_facilities.end(); ++itr) { // percorrer todas as inst abertas

							if(*itr != g.id(n)){ // nao podemos olhar para a instalacao n, pq vamos fecha-la

								if(assignment_cost[findEdge(g, n_cli, facilities[*itr])] < best_cij_reassignment){ // caso essa seja a inst mais perto ate agr encontrada
									
									temp_nearest_fac[n_cli] = *itr; // atualizando a inst mais perto temporaria
									best_cij_reassignment = assignment_cost[findEdge(g, n_cli, facilities[*itr])];
								}
							}
						} 

						// Atualizamos o custo extra, subtraindo o custo de atribuicao da antiga inst e somando o custo com a nova
						extra_cost = extra_cost + best_cij_reassignment - c_minX[n_cli];
					}
				}

				if(DEBUG >= DISPLAY_ACTIONS){
					cout << "--- EXTRA COST: " << extra_cost << endl;
				}

				// Caso melhore o custo total final, vamos efetuar de fato essa alteracao
				if(extra_cost < 0){

					if(DEBUG >= DISPLAY_ACTIONS){
						cout << "YEEES THE COST IS BETTER, LET'S DO IT " << endl;
					}

					for(ListBpGraph::RedNodeIt n_cli(g); n_cli != INVALID; ++n_cli){		// percorre os clientes

						if(nearest_open_fac[n_cli] == g.id(n)){ // caso esse cliente esteja conectado a essa instalacao
							if(DEBUG >= DISPLAY_ACTIONS){
								cout << "changing client: " << name[n_cli] << " nearest facility " << name[n] - qty_clients << endl;
							}

							nearest_open_fac[n_cli] = temp_nearest_fac[n_cli]; // reatribuindo com a nova inst mais proxima
							c_minX[n_cli] = assignment_cost[findEdge(g, n_cli, facilities[nearest_open_fac[n_cli]])]; // atualizando o menor cij desse cli

							open[n] = false; // fechando a instalacao
							open_facilities.erase(g.id(n));
							closed_facilities.insert(g.id(n));
						}
					}

					solution.finalTotalCost += extra_cost; // Atualizando o custo total final

					local_optimum = false; // AINDA NAO TENHO CTZ SE ISSO FICARA AQUI MESMO, OU EM OUTRO LUGAR MAIS PRA FORA DO ANINHAMENTO

				}
				else if(extra_cost == 0){
					if(DEBUG >= DISPLAY_ACTIONS){
						cout << "AAAA LOCAL OPTIMUM" << endl;
					}
					local_optimum = true; // AINDA NAO TENHO CTZ SE ISSO FICARA AQUI MESMO, OU EM OUTRO LUGAR MAIS PRA FORA DO ANINHAMENTO
				}
				else{
					if(DEBUG >= DISPLAY_ACTIONS){
						cout << "VISHHHH IT GOT WORSE" << endl;
					}
				}
			}
			else { // caso a inst esteja fechada

				if(DEBUG >= DISPLAY_ACTIONS){
					cout <<"It is closed, let's open, but in the future" << endl;
				}

			}

		}

		// VOU APAGAR ESSE BREAK DEPOIS, EH SO PRA NAO GERAR LOOP INFINITO E EU PODER TESTAR POR ENQUANTO
		break;
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
