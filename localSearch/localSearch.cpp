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

#define DISPLAY_MOVES 1 // corresponde a todos os cout quando um movimento é realizado de fato
#define DISPLAY_ACTIONS 2 // corresponde a todos os cout quando uma acao é feita. 
#define DISPLAY_TIME 3 // corresponde aos calculos de tempo 
#define DISPLAY_GRAPH 4 // corresponde a descricao dos clientes, instalacoes e arcos

#define DEBUG 0 // OPCOES DE DEBUG: 1 PARA EXIBIR OS MOVIMENTOS REALIZADOS, 2 PARA EXIBIR ACOES, 3 PARA EXIBIR TEMPO, 4 PARA EXIBIR AS MUDANÇAS NO GRAFO


// Retornar o valor da solucao
solutionType localSearch(int qty_facilities, int qty_clients, double * costF, double * costA, solutionType solution){

	cout << fixed;
   	cout.precision(5);

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

	// nearest_open_fac - será correnpondente ao ID da instalacao i tal que min_{i \in X} cij, sendo X as inst abertas
	ListBpGraph::RedNodeMap<int> nearest_open_fac(g);

	// temp_nearest_fac - será correnpondente ao ID da instalacao i mais proxima, depois da inst que ela ja esta conectada (temporario)
	ListBpGraph::RedNodeMap<int> temp_nearest_fac(g);

	// temp2_nearest_fac - será correnpondente ao ID da instalacao i mais proxima no movimento de troca, no for interno (temporario 2)
	ListBpGraph::RedNodeMap<int> temp2_nearest_fac(g);

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

	// Iteratores para os conjuntos 
	set <int, greater <int> > :: iterator itr; 
	set <int, greater <int> > :: iterator itr2; 


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


	// Variavel que indica se foi feito uma troca, se sim, devemos dar um break no for para voltar pro while
	bool swap_done = false;

	// double melhor_custo_escolhido;

	// Variavel que indica o custo extra de abrir ou fechar uma certa instalacao
	double extra_cost;

	// Variavel utilizada para calcular o custo completo extra do movimento de troca
	double complete_extra_cost;

	// Variavel que indica o melhor custo cij encontrado pra reatribuicao
	double best_cij_reassignment;


	// ****** A partir daqui deve estar em um loop até nao ter mais melhoras possiveis, isto eh, encontrar o otimo local


	// OBSERVACAO IMPORTANTE: SERA NECESSARIO COLOCAR UMA CONDICAO DE PARADA TAMBEM RELACIONADO A TEMPO OU QUANTIDADE DE ITERACOES


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
		// Apos isso, caso tenha encontrado alguma melhora, parte para a proxima iteracao do while e o for será repetido
		// Caso nenhuma melhora tenha sido encontrada, parte para o caso da troca

		// Apos testar todas as opcoes de add e delete e nada melhorar a solucao, vamos tentar a troca
		// Portanto havera um outro for, embaixo desse primeiro for, ainda dentro do while, no qual percorrera todas as inst
		// Caso a inst esteja aberta tenta fechar ela e haverá um for inteiro para tentar abrir as instalacoes SUBSEQUENTES (nao precisa repetir as anteriores, mas nao sei se consigo fazer isso no iterator) que estao fechadas atualmente
		// Caso esse movimento de troca resulte em uma melhora, será de fato feita a operacao
		// Do contrario, continuará percorrendo o for interno na busca por abrir a proxima inst fechada
		// Se nenhuma for encontrada, vai para a proxima iteracao do for externo e repete o processo
		// OBSERVAÇÃO IMPORTANTE: Quando encontrar o primeiro caso de troca bem sucedido, esse for para imediatamente, indo para a proxima iteracao do while
		// Assim serao testados novamente os movimentos de add ou delete, que sao mais simples
		// Só voltando novamente para a troca caso nenhum add nem delete melhore nada
		// OBSERVAÇÃO IMPORTANTE 2: Quando em nenhum caso de troca for encontrado melhora, encontramos de fato o minimo local!! E o while deve terminar


		// Podemos considerar que inicialmente estamos no otimo local ate que se prove o contrario. 
		// Entao, vou percorrer todas as instalacoes e, caso eu encontre alguma melhora no percurso, modifico esse otimo local para falso
		// Na proxima iteracao volto a considerar que estamos no otimo local e repito o processo
		// No momento que eu percorrer todas as instalacoes e nenhuma conseguir melhorar nada, entao eu realmente estava no otimo local e finalizo o while
		// Encaixando a operação de troca, será um for logo depois desse primeiro for, que só ira entrar caso finalize o primeiro for com local_optimum = true, pois indica que nada melhorou por ali
		local_optimum = true; 

		for(ListBpGraph::BlueNodeIt n(g); n != INVALID; ++n){		// percorre as instalacoes

			if(DEBUG >= DISPLAY_ACTIONS){
				cout << "Let's move facility " << name[n] - qty_clients << endl;
			}

			if(open[n]){ // caso a inst esteja aberta

				if(DEBUG >= DISPLAY_ACTIONS){
					cout << "It is OPEN, let's close it" << endl;
				}

				if(open_facilities.size() <= 1){
					if(DEBUG >= DISPLAY_ACTIONS){
						cout << "Sorry, you can't close it because it is the only one that is open." << endl;
					}
				}
				else{

					// Vamos fechar essa instalacao

					extra_cost = -f[n]; // subtrai o custo de abertura dessa inst

					for(ListBpGraph::RedNodeIt n_cli(g); n_cli != INVALID; ++n_cli){		// percorre os clientes

						if(nearest_open_fac[n_cli] == g.id(n)){ // caso esse cliente esteja conectado a essa instalacao

							best_cij_reassignment = biggestCij + 1; // limitante superior tranquilo

							for (itr = open_facilities.begin(); itr != open_facilities.end(); ++itr) { // percorrer todas as inst abertas

								if(*itr != g.id(n)){ // nao podemos olhar para a instalacao n, pq vamos fecha-la

									if(assignment_cost[findEdge(g, n_cli, facilities[*itr])] < best_cij_reassignment){ // caso essa seja a inst mais perto ate agr encontrada
										
										temp_nearest_fac[n_cli] = *itr; // atualizando a inst mais perto temporaria
										best_cij_reassignment = assignment_cost[findEdge(g, n_cli, facilities[*itr])];
									}
								}
							} 

							if(DEBUG >= DISPLAY_ACTIONS){
								cout << "client: " << name[n_cli] << " new nearest facility " << temp_nearest_fac[n_cli] << endl;
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

						if(DEBUG >= DISPLAY_MOVES){
							cout << "Move CLOSE facility: "<< name[n] - qty_clients << endl;
						}

						qty_moves += 1;

						for(ListBpGraph::RedNodeIt n_cli(g); n_cli != INVALID; ++n_cli){		// percorre os clientes

							if(nearest_open_fac[n_cli] == g.id(n)){ // caso esse cliente esteja conectado a essa instalacao
								if(DEBUG >= DISPLAY_ACTIONS){
									cout << "changing client: " << name[n_cli] << " nearest facility " << name[n] - qty_clients << endl;
								}

								nearest_open_fac[n_cli] = temp_nearest_fac[n_cli]; // reatribuindo com a nova inst mais proxima
								c_minX[n_cli] = assignment_cost[findEdge(g, n_cli, facilities[nearest_open_fac[n_cli]])]; // atualizando o menor cij desse cli

								solution.assigned_facilities[name[n_cli]] = nearest_open_fac[n_cli]; // salvando alteracoes sobre a inst mais proxima na solucao final
							}
						}

						open[n] = false; // fechando de fato a instalacao
						open_facilities.erase(g.id(n));
						closed_facilities.insert(g.id(n));

						solution.finalTotalCost += extra_cost; // Atualizando o custo total final

						local_optimum = false; // Como encontrei uma melhora, entao eu nao estava no otimo local, portanto o while deve continuar
					}
					else {
						if(DEBUG >= DISPLAY_ACTIONS){
							cout << "VISHHHH IT GOT WORSE" << endl;
						}
					}
				}
			}

			else { // caso a inst esteja fechada

				if(DEBUG >= DISPLAY_ACTIONS){
					cout <<"It is CLOSED, let's open it" << endl;
				}

				// Vamos abrir essa instalacao

				extra_cost = f[n]; // soma o custo de abertura dessa inst

				for (ListBpGraph::IncEdgeIt e(g, n); e != INVALID; ++e) { // Percorre todas arestas desse nó (ligam a clientes)

					if(assignment_cost[e] < c_minX[g.asRedNode(g.u(e))]){ // caso a nova inst seja a mais proxima aberta desse cliente

						if(DEBUG >= DISPLAY_ACTIONS){
							cout << "client: " << name[g.u(e)] << " new nearest facility " << g.id(n) << endl;
						}
			
						temp_nearest_fac[g.asRedNode(g.u(e))] = g.id(n); // atualizando a inst mais perto temporaria

						// Atualizamos o custo extra, subtraindo o custo de atribuicao da antiga inst e somando o custo com a nova
						extra_cost = extra_cost + assignment_cost[e] - c_minX[g.asRedNode(g.u(e))];
					}
					else { // senao, caso a nova inst nao seja mais proxima
						temp_nearest_fac[g.asRedNode(g.u(e))] = nearest_open_fac[g.asRedNode(g.u(e))]; // colocando isso apenas para uso posterior na checagem nao haver lixo
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

					if(DEBUG >= DISPLAY_MOVES){
						cout << "Move OPEN facility: "<< name[n] - qty_clients << endl;
					}

					qty_moves += 1;

					for(ListBpGraph::RedNodeIt n_cli(g); n_cli != INVALID; ++n_cli){		// percorre os clientes

						if(nearest_open_fac[n_cli] != temp_nearest_fac[n_cli]) { // caso esse cliente seja mais proximo dessa nova instalacao
							if(DEBUG >= DISPLAY_ACTIONS){
								cout << "changing client: " << name[n_cli] << " nearest facility " << name[n] - qty_clients << endl;
							}

							nearest_open_fac[n_cli] = temp_nearest_fac[n_cli]; // reatribuindo com a nova inst mais proxima
							c_minX[n_cli] = assignment_cost[findEdge(g, n_cli, facilities[nearest_open_fac[n_cli]])]; // atualizando o menor cij desse cli

							solution.assigned_facilities[name[n_cli]] = nearest_open_fac[n_cli]; // salvando alteracoes na solucao final
						}
					}

					open[n] = true; // abrindo de fato a instalacao
					closed_facilities.erase(g.id(n));
					open_facilities.insert(g.id(n));

					solution.finalTotalCost += extra_cost; // Atualizando o custo total final

					local_optimum = false; // Como encontrei uma melhora, entao eu nao estava no otimo local, portanto o while deve continuar
				}
				else {
					if(DEBUG >= DISPLAY_ACTIONS){
						cout << "VISHHHH IT GOT WORSE" << endl;
					}
				}
			}
		}


		// CASO DE TROCA
		if(local_optimum){ // Entra aqui se nao houve nenhuma melhora na tentativa de add ou delete

			if(DEBUG >= DISPLAY_ACTIONS){
				cout << "Let's try to swap it. Now the total cost is " << solution.finalTotalCost << endl;
			}

			for(ListBpGraph::BlueNodeIt n(g); n != INVALID; ++n){		// percorre as instalacoes

				if(DEBUG >= DISPLAY_ACTIONS){
					cout << "Let's move facility " << name[n] - qty_clients << endl;
				}



				if(open[n]){ // caso a inst esteja aberta

					if(DEBUG >= DISPLAY_ACTIONS){
						cout << "It is OPEN, let's swap it out" << endl;
					}

					if(open_facilities.size() <= 1){
						if(DEBUG >= DISPLAY_ACTIONS){
							cout << "Sorry, you can't close it because it is the only one that is open." << endl;
						}
					}
					else{

						// Vamos fechar essa instalacao

						extra_cost = -f[n]; // subtrai o custo de abertura dessa inst

						for(ListBpGraph::RedNodeIt n_cli(g); n_cli != INVALID; ++n_cli){		// percorre os clientes

							if(nearest_open_fac[n_cli] == g.id(n)){ // caso esse cliente esteja conectado a essa instalacao

								best_cij_reassignment = biggestCij + 1; // limitante superior tranquilo

								for (itr = open_facilities.begin(); itr != open_facilities.end(); ++itr) { // percorrer todas as inst abertas

									if(*itr != g.id(n)){ // nao podemos olhar para a instalacao n, pq vamos fecha-la

										if(assignment_cost[findEdge(g, n_cli, facilities[*itr])] < best_cij_reassignment){ // caso essa seja a inst mais perto ate agr encontrada

											temp_nearest_fac[n_cli] = *itr; // atualizando a inst mais perto temporaria
											best_cij_reassignment = assignment_cost[findEdge(g, n_cli, facilities[*itr])];
										}
									}
								} 

								if(DEBUG >= DISPLAY_ACTIONS){
									cout << "client: " << name[n_cli] << " new nearest facility " << temp_nearest_fac[n_cli] << endl;
								}

								// Atualizamos o custo extra, subtraindo o custo de atribuicao da antiga inst e somando o custo com a nova
								extra_cost = extra_cost + best_cij_reassignment - c_minX[n_cli];
							}
							else{
								temp_nearest_fac[n_cli] = nearest_open_fac[n_cli]; // colocando isso apenas para uso posterior na checagem nao haver lixo
							}
						}

						if(DEBUG >= DISPLAY_ACTIONS){
							cout << "--Partial extra cost:" << extra_cost << endl;
						}


						// Vamos abrir uma outra instalacao no lugar dessa que fechei

						for (itr = closed_facilities.begin(); itr != closed_facilities.end(); ++itr) { // percorrer todas as inst fechadas

							ListBpGraph::BlueNode n2 = g.asBlueNode(g.nodeFromId(*itr)); // Pega o no correspondente dessa inst

							if(DEBUG >= DISPLAY_ACTIONS){
								cout << "Let's swap in the facility " << name[n2] - qty_clients << endl;
							}

							complete_extra_cost = extra_cost + f[n2]; // soma o custo de abertura dessa inst

							for (ListBpGraph::IncEdgeIt e(g, n2); e != INVALID; ++e) { // Percorre todas arestas desse nó (ligam a clientes)

								if(assignment_cost[e] < assignment_cost[findEdge(g, g.asRedNode(g.u(e)), facilities[temp_nearest_fac[g.asRedNode(g.u(e))]])]){ // caso a nova inst seja a mais proxima aberta desse cliente

									if(DEBUG >= DISPLAY_ACTIONS){
										cout << "client: " << name[g.u(e)] << " new nearest facility " << g.id(n2) << endl;
									}
						
									temp2_nearest_fac[g.asRedNode(g.u(e))] = g.id(n2); // atualizando a inst mais perto temporaria

									// Atualizamos o custo extra, subtraindo o custo de atribuicao da antiga inst e somando o custo com a nova
									complete_extra_cost = complete_extra_cost + assignment_cost[e] - assignment_cost[findEdge(g, g.u(e), facilities[temp_nearest_fac[g.asRedNode(g.u(e))]])];
								}
								else { // senao, caso a nova inst nao seja mais proxima
									temp2_nearest_fac[g.asRedNode(g.u(e))] = temp_nearest_fac[g.asRedNode(g.u(e))]; // colocando isso apenas para uso posterior na checagem nao haver lixo
								}
							}

							if(DEBUG >= DISPLAY_ACTIONS){
								cout << "--Complete extra cost: " << complete_extra_cost << endl;
							}

							// Caso melhore o custo total final, vamos efetuar de fato essa alteracao
							if(complete_extra_cost < 0){

								if(DEBUG >= DISPLAY_ACTIONS){
									cout << "YEEES THE COST IS BETTER, LET'S DO IT " << endl;
								}

								if(DEBUG >= DISPLAY_MOVES){
									cout << "Move SWAP OUT facility: " << name[n] - qty_clients << " and SWAP IN facility: " << name[n2] - qty_clients << endl;
								}

								qty_moves += 1;

								for(ListBpGraph::RedNodeIt n_cli(g); n_cli != INVALID; ++n_cli){		// percorre os clientes

									if(nearest_open_fac[n_cli] != temp2_nearest_fac[n_cli]) { // caso esse cliente tenha mudado de atribuicao durante o percurso
										if(DEBUG >= DISPLAY_ACTIONS){
											cout << "changing client: " << name[n_cli] << " new nearest facility " << temp2_nearest_fac[n_cli] << endl;
										}

										nearest_open_fac[n_cli] = temp2_nearest_fac[n_cli]; // reatribuindo com a nova inst mais proxima
										c_minX[n_cli] = assignment_cost[findEdge(g, n_cli, facilities[nearest_open_fac[n_cli]])]; // atualizando o menor cij desse cli

										solution.assigned_facilities[name[n_cli]] = nearest_open_fac[n_cli]; // salvando alteracoes sobre a inst mais proxima na solucao final
									}
								}

								open[n] = false; // fechando de fato a primeira instalacao
								open_facilities.erase(g.id(n));
								closed_facilities.insert(g.id(n));


								open[n2] = true; // abrindo de fato a segunda instalacao
								open_facilities.insert(g.id(n2));
								closed_facilities.erase(g.id(n2));

								solution.finalTotalCost += complete_extra_cost; // Atualizando o custo total final

								if(DEBUG >= DISPLAY_ACTIONS){
									cout << "*After the swap move we have total cost: " << solution.finalTotalCost << endl;
								}

								local_optimum = false; // Como encontrei uma melhora, entao eu nao estava no otimo local, portanto o while deve continuar

								swap_done = true;
								break; // quero que volte a olhar os movimentos de add e delete, ao inves de ver outras possiveis trocas
							}
							else {
								if(DEBUG >= DISPLAY_ACTIONS){
									cout << "VISHHHH IT GOT WORSE" << endl;
								}
							}
						}
					}
				}


				else { // caso a inst esteja fechada

					if(DEBUG >= DISPLAY_ACTIONS){
						cout <<"It is CLOSED, let's open it and close another one." << endl;
					}

					// Vamos abrir essa instalacao

					extra_cost = f[n]; // soma o custo de abertura dessa inst

					for (ListBpGraph::IncEdgeIt e(g, n); e != INVALID; ++e) { // Percorre todas arestas desse nó (ligam a clientes)

						if(assignment_cost[e] < c_minX[g.asRedNode(g.u(e))]){ // caso a nova inst seja a mais proxima aberta desse cliente

							if(DEBUG >= DISPLAY_ACTIONS){
								cout << "client: " << name[g.u(e)] << " new nearest facility " << g.id(n) << endl;
							}
				
							temp_nearest_fac[g.asRedNode(g.u(e))] = g.id(n); // atualizando a inst mais perto temporaria

							// Atualizamos o custo extra, subtraindo o custo de atribuicao da antiga inst e somando o custo com a nova
							extra_cost = extra_cost + assignment_cost[e] - c_minX[g.asRedNode(g.u(e))];
						}
						else { // senao, caso a nova inst nao seja mais proxima
							temp_nearest_fac[g.asRedNode(g.u(e))] = nearest_open_fac[g.asRedNode(g.u(e))]; // colocando isso apenas para uso posterior na checagem nao haver lixo
						}
					}

					if(DEBUG >= DISPLAY_ACTIONS){
						cout << "--Partial extra cost:" << extra_cost << endl;
					}

					// Vamos fechar uma outra instalacao no lugar dessa que abri

					for (itr = open_facilities.begin(); itr != open_facilities.end(); ++itr) { // percorrer todas as inst abertas

						ListBpGraph::BlueNode n2 = g.asBlueNode(g.nodeFromId(*itr)); // Pega o no correspondente dessa inst

						if(DEBUG >= DISPLAY_ACTIONS){
							cout << "Let's swap out the facility " << name[n2] - qty_clients << endl;
						}

						complete_extra_cost = extra_cost - f[n2]; // subtrai o custo de abertura dessa inst

						for(ListBpGraph::RedNodeIt n_cli(g); n_cli != INVALID; ++n_cli){		// percorre os clientes

							if(temp_nearest_fac[n_cli] == g.id(n2)){ // caso esse cliente esteja conectado a essa instalacao

								best_cij_reassignment = biggestCij + 1; // limitante superior tranquilo

								for (itr2 = open_facilities.begin(); itr2 != open_facilities.end(); ++itr2) { // percorrer todas as inst abertas

									if(*itr2 != g.id(n2)){ // nao podemos olhar para a instalacao n, pq vamos fecha-la

										if(assignment_cost[findEdge(g, n_cli, facilities[*itr2])] < best_cij_reassignment){ // caso essa seja a inst mais perto ate agr encontrada
										
											temp2_nearest_fac[n_cli] = *itr2; // atualizando a inst mais perto temporaria
											best_cij_reassignment = assignment_cost[findEdge(g, n_cli, facilities[*itr2])];
										}

									}
								} 

								// Tambem devemos testar com a instalacao que pretendo abrir, que nao esta ainda no conjunto open facilities

								if(assignment_cost[findEdge(g, n_cli, n)] < best_cij_reassignment){ // caso essa seja a inst mais perto ate agr encontrada
									temp2_nearest_fac[n_cli] = name[n] - qty_clients; // atualizando a inst mais perto temporaria
									best_cij_reassignment = assignment_cost[findEdge(g, n_cli, n)];
								}


								if(DEBUG >= DISPLAY_ACTIONS){
									cout << "client: " << name[n_cli] << " new nearest facility " << temp2_nearest_fac[n_cli] << endl;
								}

								// Atualizamos o custo extra, subtraindo o custo de atribuicao da antiga inst e somando o custo com a nova
								complete_extra_cost = complete_extra_cost + best_cij_reassignment - assignment_cost[findEdge(g, n_cli, facilities[temp_nearest_fac[n_cli]])];
							}
							else{
								temp2_nearest_fac[n_cli] = temp_nearest_fac[n_cli]; // colocando isso apenas para uso posterior na checagem nao haver lixo
							}
						}


						if(DEBUG >= DISPLAY_ACTIONS){
							cout << "--Complete extra cost: " << complete_extra_cost << endl;
						}


						// Caso melhore o custo total final, vamos efetuar de fato essa alteracao
						if(complete_extra_cost < 0){

							if(DEBUG >= DISPLAY_ACTIONS){
								cout << "YEEES THE COST IS BETTER, LET'S DO IT " << endl;
							}

							if(DEBUG >= DISPLAY_MOVES){
								cout << "Move SWAP IN facility: " << name[n] - qty_clients << " and SWAP OUT facility: " << name[n2] - qty_clients << endl;
							}

							qty_moves += 1;

							for(ListBpGraph::RedNodeIt n_cli(g); n_cli != INVALID; ++n_cli){		// percorre os clientes

								if(nearest_open_fac[n_cli] != temp2_nearest_fac[n_cli]) { // caso esse cliente tenha mudado de atribuicao durante o percurso
									if(DEBUG >= DISPLAY_ACTIONS){
										cout << "changing client: " << name[n_cli] << " new nearest facility " << temp2_nearest_fac[n_cli] << endl;
									}

									nearest_open_fac[n_cli] = temp2_nearest_fac[n_cli]; // reatribuindo com a nova inst mais proxima
									c_minX[n_cli] = assignment_cost[findEdge(g, n_cli, facilities[nearest_open_fac[n_cli]])]; // atualizando o menor cij desse cli

									solution.assigned_facilities[name[n_cli]] = nearest_open_fac[n_cli]; // salvando alteracoes sobre a inst mais proxima na solucao final
								}
							}

							open[n] = true; // abrindo de fato a primeira instalacao
							closed_facilities.erase(g.id(n));
							open_facilities.insert(g.id(n));


							open[n2] = false; // fechando de fato a segunda instalacao
							closed_facilities.insert(g.id(n2));
							open_facilities.erase(g.id(n2));

							solution.finalTotalCost += complete_extra_cost; // Atualizando o custo total final

							if(DEBUG >= DISPLAY_ACTIONS){
								cout << "*After the swap move we have total cost: " << solution.finalTotalCost << endl;
							}

							local_optimum = false; // Como encontrei uma melhora, entao eu nao estava no otimo local, portanto o while deve continuar

							swap_done = true;
							break; // quero que volte a olhar os movimentos de add e delete, ao inves de ver outras possiveis trocas
						}
						else {
							if(DEBUG >= DISPLAY_ACTIONS){
								cout << "VISHHHH IT GOT WORSE" << endl;
							}
						}
					}

				}

				if(swap_done){
					swap_done = false;
					break;
				}
			}
		}
	}

	cout << "FINAL TOTAL COST: " << solution.finalTotalCost << endl;

	cout << "Total moves: " << qty_moves << endl;

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
