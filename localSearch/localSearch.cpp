#include <lemon/list_graph.h>
#include <algorithm>
#include <iostream>
#include <set>
#include <iterator>
#include <ctime>
#include <fstream>
#include <cstring>
#include "definitions.hpp"
#define EPSL 0.00000001

using namespace lemon;
using namespace std;

#define DISPLAY_BASIC 1 // corresponde a exibicao da quantidade de movimentos
#define DISPLAY_MOVES 2 // corresponde a todos os cout quando um movimento é realizado de fato
#define DISPLAY_ACTIONS 3 // corresponde a todos os cout quando uma acao é feita. 
#define DISPLAY_TIME 4 // corresponde aos calculos de tempo 
#define DISPLAY_GRAPH 5 // corresponde a descricao dos clientes, instalacoes e arcos

#define DEBUG 1 // OPCOES DE DEBUG: 1 - MOSTRAR A QTD DE MOVIMENTOS, 2 PARA EXIBIR OS MOVIMENTOS REALIZADOS, 3 PARA EXIBIR ACOES, 4 PARA EXIBIR TEMPO, 5 PARA EXIBIR AS MUDANÇAS NO GRAFO

#define TIME_LIMIT 900 //15 minutos

#define TIME_COUNTER_STEP 5 // 5 segundos para testar as inst ga250a //30 // 30 segundos. Isso indicara que vai salvar a melhor solucao encontrada a cada minuto

// // Função para embaralhar o vetor com indices das facilities
// void shuffle(int *vet, int vetSize){
// 	int r, temp;
// 	for(int i=0; i<vetSize; i++){
// 		r = rand() % vetSize;
// 		temp = vet[i];
// 		vet[i] = vet[r];
// 		vet[r] = temp;
// 	}
// }


// Retornar o valor da solucao
solutionType localSearch(char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA, solutionType solution, int ls_type){

	cout << fixed;
   	cout.precision(5);

	/* Inicio declaracoes variaveis para calculo de tempo */

	// Declaracao variaveis que indicam o tempo da funcao
	struct timespec start, finish, time_so_far;

	// Declaracao da variavel que vai indicar o contador para salvar a solucao atual
	double timeCounter = 0;

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

	// // Declaracao de variavel auxiliar para formacao do arquivo .log
	// char completeLogSolName[250] = "";

	// // Arquivo para salvar o log da solucao
	// ofstream solLog;
	// strcat(completeLogSolName,solutionName);
	// strcat(completeLogSolName,".log");
	// solLog.open(completeLogSolName, std::ofstream::out | std::ofstream::trunc);

	// //Salvando o cabecalho
	// solLog << "time spent so far, current solution cost, current qty moves" << endl;

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
				cout << "There is just 1 open facility" << endl;
			}
			c2_minX[clients[i]] = -1;
			nearest2_open_fac[clients[i]] = -1;
		}

		if(DEBUG >= DISPLAY_ACTIONS){
			cout << "Client " << i << " c1: " << c_minX[clients[i]] << " near: " << nearest_open_fac[clients[i]] << " c2: " << c2_minX[clients[i]] << " near2: " << nearest2_open_fac[clients[i]] << endl;
		}
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

	// Variavel que indica se foi feito uma troca, se sim, devemos dar um break no for para voltar pro while
	bool swap_done = false;

	// Variavel que indica o custo extra de abrir ou fechar uma certa instalacao
	double extra_cost;

	// Variavel utilizada para calcular o custo completo extra do movimento de troca
	double complete_extra_cost;

	// Variavel auxiliar para ajudar quando houver substituicao de informacoes na troca
	bool closed_nearest = false; 


	// Decidindo a ordem que as instalacoes 
	//PROBLEMA: NAO ADIANTA PQ EU PERCORRO PELO GRAFO, NAO PELO VETOR... VAI TER QUE MUDAR ISSO MESMO


	// INICIANDO A CONTAGEM DE TEMPO DA FUNCAO
	clock_gettime(CLOCK_REALTIME, &start);

	// ****** A partir daqui deve estar em um loop até nao ter mais melhoras possiveis, isto é, encontrar o otimo local

	// OBSERVACAO 1: tambem existe uma condicao de parada relacionada a tempo, feita logo no inicio do loop
	// OBSERVACAO 2: solution.local_optimum (variavel que indica se ja encontramos o otimo local)

	solution.local_optimum = false;

	while(!solution.local_optimum){

		// CHECANDO A CONTAGEM DE TEMPO GASTO ATÉ AGORA
		clock_gettime(CLOCK_REALTIME, &time_so_far);

		// Calculando o tempo gasto até agora
		solution.timeSpent =  (time_so_far.tv_sec - start.tv_sec);
		solution.timeSpent += (time_so_far.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 

		if(DEBUG >= DISPLAY_TIME){
			cout << "Total time spent so far: " << solution.timeSpent << " seconds" << endl;
		}

		// Se já passou o tempo limite, devemos parar
		if(solution.timeSpent >= TIME_LIMIT){
			// // Acrescentando no solLog.txt o tempo final gasto nas iteracoes e o custo da solucao
			// solLog << solution.timeSpent << "," << solution.finalTotalCost << "," << qty_moves << endl;

			break;
		}
		else if(solution.timeSpent >= timeCounter){
			// aumentando o contador de tempo
			timeCounter += TIME_COUNTER_STEP;

			// // Acrescentando no solLog.txt o tempo gasto nessa iteracao e o custo da solucao
			// solLog << solution.timeSpent << "," << solution.finalTotalCost << "," << qty_moves << endl;
		}

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
		// Encaixando a operação de troca, será um for logo depois desse primeiro for, que só ira entrar caso finalize o primeiro for com solution.local_optimum = true, pois indica que nada melhorou por ali
		solution.local_optimum = true; 

		if((ls_type==1)||(ls_type==2)){
			// VIZINHANCA 1: ABRIR OU FECHAR UMA INSTALACAO
			for(ListBpGraph::BlueNodeIt n(g); n != INVALID; ++n){		// percorre as instalacoes

				if(DEBUG >= DISPLAY_ACTIONS){
					cout << "Let's move facility " << name[n] << endl;
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

							if(nearest_open_fac[n_cli] == name[n]){ // caso esse cliente esteja conectado a essa instalacao

								if(c2_minX[n_cli] >= 0){ // CASO EXISTA A SEGUNDA INST MAIS PROXIMA SALVA

									// Atualizamos o custo extra, subtraindo o custo de atribuicao da antiga inst e somando o custo com a nova
									extra_cost += c2_minX[n_cli] - c_minX[n_cli];

									if(DEBUG >= DISPLAY_ACTIONS){
										cout << "client: " << name[n_cli] << " new nearest facility " << nearest2_open_fac[n_cli] << endl;
									}
								}
								else{
									if(DEBUG >= DISPLAY_ACTIONS){
										cout << "You shouldn't be here. There is just 1 open facility and you want to close it." << endl;
									}
									continue; // vai para a proxima iteracao
								}
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
								cout << "Move CLOSE facility: "<< name[n] << endl;
							}

							qty_moves += 1;

							for(ListBpGraph::RedNodeIt n_cli(g); n_cli != INVALID; ++n_cli){		// percorre os clientes

								closed_nearest = false;

								if(nearest_open_fac[n_cli] == name[n]){ // caso esse cliente esteja conectado a essa instalacao

									closed_nearest = true;

									if(DEBUG >= DISPLAY_ACTIONS){
										cout << "changing client: " << name[n_cli] << " nearest facility " << name[n] << endl;
									}

									// ATUALIZANDO AQUI A INST MAIS PROXIMA
									nearest_open_fac[n_cli] = nearest2_open_fac[n_cli]; // reatribuindo com a nova inst mais proxima
									c_minX[n_cli] = c2_minX[n_cli]; // atualizando o menor cij desse cli

									// SALVANDO A SOLUCAO NOVA
									solution.assigned_facilities[name[n_cli]] = nearest_open_fac[n_cli]; // salvando alteracoes sobre a inst mais proxima na solucao final
								}

								if((closed_nearest)||(nearest2_open_fac[n_cli] == name[n])){ // caso esse cliente tinha essa instalacao como primeira ou segunda mais proxima
									// ATUALIZANDO AQUI A SEGUNDA MAIS PROXIMA
									c2_minX[n_cli] = biggestCij + 1; // limitante superior tranquilo

									for (itr = open_facilities.begin(); itr != open_facilities.end(); ++itr) { // percorrer todas as inst abertas
										aux_cij = assignment_cost[findEdge(g, n_cli, facilities[*itr])];
										if((aux_cij < c2_minX[n_cli]) && (*itr != nearest_open_fac[n_cli]) && (*itr != name[n])){ // se for melhor que o salvo ate agr, mas nao melhor que o melhor de todos nem da que vai fechar
											nearest2_open_fac[n_cli] = *itr;
											c2_minX[n_cli] = aux_cij;
										}
									}
									if(c2_minX[n_cli] > biggestCij){ // se só tinha 1 inst aberta depois do fechamento dessa iteracao, entao colocamos -1 para indicar invalidez
										if(DEBUG >= DISPLAY_ACTIONS){
											cout << "There is just 1 open facility" << endl;
										}
										c2_minX[n_cli] = -1;
										nearest2_open_fac[n_cli] = -1;
									}
								}
							}

							open[n] = false; // fechando de fato a instalacao
							solution.open_facilities[name[n]] = false;
							open_facilities.erase(name[n]);
							closed_facilities.insert(name[n]);

							solution.finalTotalCost += extra_cost; // Atualizando o custo total final

							solution.local_optimum = false; // Como encontrei uma melhora, entao eu nao estava no otimo local, portanto o while deve continuar
						}
						else {
							if(DEBUG >= DISPLAY_ACTIONS){
								cout << "NO! IT GOT WORSE" << endl;
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
								cout << "client: " << name[g.u(e)] << " new nearest facility " << name[n] << endl;
							}
				
							temp_nearest_fac[g.asRedNode(g.u(e))] = name[n]; // atualizando a inst mais perto temporaria

							// Atualizamos o custo extra, subtraindo o custo de atribuicao da antiga inst e somando o custo com a nova
							extra_cost += assignment_cost[e] - c_minX[g.asRedNode(g.u(e))];
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
							cout << "Move OPEN facility: "<< name[n] << endl;

						}

						qty_moves += 1;

						for(ListBpGraph::RedNodeIt n_cli(g); n_cli != INVALID; ++n_cli){		// percorre os clientes

							aux_cij = assignment_cost[findEdge(g, n_cli, n)];

							if(nearest_open_fac[n_cli] != temp_nearest_fac[n_cli]) { // caso esse cliente seja mais proximo dessa nova instalacao
								if(DEBUG >= DISPLAY_ACTIONS){
									cout << "changing client: " << name[n_cli] << " nearest facility " << name[n] << endl;
								}

								// ATUALIZANDO A SEGUNDA INST MAIS PROXIMA
								nearest2_open_fac[n_cli] = nearest_open_fac[n_cli];
								c2_minX[n_cli] = c_minX[n_cli];

								// ATUALIZANDO A INSTALACAO MAIS PROXIMA
								nearest_open_fac[n_cli] = name[n]; // reatribuindo com a nova inst mais proxima
								c_minX[n_cli] = aux_cij; // atualizando o menor cij desse cli

								solution.assigned_facilities[name[n_cli]] = nearest_open_fac[n_cli]; // salvando alteracoes na solucao final
							}
							else if((aux_cij < c2_minX[n_cli])||(c2_minX[n_cli] < 0)) { // senao, caso esse cliente tenha essa nova instalacao como segunda mais proxima (ou nao existia segunda mais proxima)
								// ATUALIZANDO A SEGUNDA INST MAIS PROXIMA
								nearest2_open_fac[n_cli] = name[n];
								c2_minX[n_cli] = aux_cij;
							}
						}

						open[n] = true; // abrindo de fato a instalacao
						solution.open_facilities[name[n]] = true;
						closed_facilities.erase(name[n]);
						open_facilities.insert(name[n]);

						solution.finalTotalCost += extra_cost; // Atualizando o custo total final

						solution.local_optimum = false; // Como encontrei uma melhora, entao eu nao estava no otimo local, portanto o while deve continuar
					}
					else {
						if(DEBUG >= DISPLAY_ACTIONS){
							cout << "NO! IT GOT WORSE" << endl;
						}
					}
				}
			}
		}

		if((ls_type==1)||(ls_type==3)){
			// VIZINHANCA 2: CASO DE TROCA
			if(solution.local_optimum){ // Entra aqui se nao houve nenhuma melhora na tentativa de add ou delete

				if(DEBUG >= DISPLAY_ACTIONS){
					cout << "Let's try to swap it. Now the total cost is " << solution.finalTotalCost << endl;
				}

				for(ListBpGraph::BlueNodeIt n(g); n != INVALID; ++n){		// percorre as instalacoes

					if(DEBUG >= DISPLAY_ACTIONS){
						cout << "Let's move facility " << name[n] << endl;
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

								if(nearest_open_fac[n_cli] == name[n]){ // caso esse cliente esteja conectado a essa instalacao

									if(c2_minX[n_cli] >= 0){ // CASO EXISTA A SEGUNDA INST MAIS PROXIMA SALVA

										temp_nearest_fac[n_cli] = nearest2_open_fac[n_cli]; // atualizando a inst mais perto temporaria								

										if(DEBUG >= DISPLAY_ACTIONS){
											cout << "client: " << name[n_cli] << " new nearest facility " << temp_nearest_fac[n_cli] << endl;
										}

										// Atualizamos o custo extra, subtraindo o custo de atribuicao da antiga inst e somando o custo com a nova
										extra_cost += c2_minX[n_cli] - c_minX[n_cli];
									}
									else{
										if(DEBUG >= DISPLAY_ACTIONS){
											cout << "You shouldn't be here. There is just 1 open facility and you want to close it." << endl;
										}
										continue; // vai para a proxima iteracao
									}
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
									cout << "Let's swap in the facility " << name[n2] << endl;
								}

								complete_extra_cost = extra_cost + f[n2]; // soma o custo de abertura dessa inst

								for (ListBpGraph::IncEdgeIt e(g, n2); e != INVALID; ++e) { // Percorre todas arestas desse nó (ligam a clientes)

									aux_cij = assignment_cost[findEdge(g, g.asRedNode(g.u(e)), facilities[temp_nearest_fac[g.asRedNode(g.u(e))]])];

									if(assignment_cost[e] < aux_cij){ // caso a nova inst seja a mais proxima aberta desse cliente

										if(DEBUG >= DISPLAY_ACTIONS){
											cout << "client: " << name[g.u(e)] << " new nearest facility " << name[n2] << endl;
										}
							
										temp2_nearest_fac[g.asRedNode(g.u(e))] = name[n2]; // atualizando a inst mais perto temporaria

										// Atualizamos o custo extra, subtraindo o custo de atribuicao da antiga inst e somando o custo com a nova
										complete_extra_cost += assignment_cost[e] - aux_cij;
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
										cout << "Move SWAP OUT facility: " << name[n] << " and SWAP IN facility: " << name[n2] << endl;
									}

									qty_moves += 1;

									for(ListBpGraph::RedNodeIt n_cli(g); n_cli != INVALID; ++n_cli){		// percorre os clientes

										// SE A NOVA INST QUE ABRIU É A MAIS PROXIMA
										if(name[n2] == temp2_nearest_fac[n_cli]){

											// SE A QUE FECHOU NÃO ERA A MAIS PROXIMA ANTES
											if(name[n] != nearest_open_fac[n_cli]){
												nearest2_open_fac[n_cli] = nearest_open_fac[n_cli]; // atualiza a segunda mais proxima com a mais proxima. obs: se tiver fechado a mais proxima, entao a segunda mais proxima continua igual
												c2_minX[n_cli] = c_minX[n_cli];
											}

											nearest_open_fac[n_cli] = name[n2]; // atualiza a mais proxima com a que abriu
											c_minX[n_cli] = assignment_cost[findEdge(g, n_cli, n2)]; // atualizando o menor cij desse cli
										}
										// SENAO, SE A NOVA É A SEGUNDA MAIS PROXIMA
										else if((assignment_cost[findEdge(g, n_cli, n2)] < c2_minX[n_cli])||(c2_minX[n_cli] < 0)) { // senao, caso esse cliente tenha essa nova instalacao como segunda mais proxima ou nao exista segunda salva

											// SE A QUE FECHOU ERA A MAIS PROXIMA // eu sei que existe pelo menos mais outra, senao nao ia permitir fechar essa, entao nearest2 existe
											if(name[n] == nearest_open_fac[n_cli]){
												nearest_open_fac[n_cli] = name[n2]; // a mais proxima recebe a nova que abriu
												c_minX[n_cli] = assignment_cost[findEdge(g, n_cli, n2)];
											}
											else{ 
												nearest2_open_fac[n_cli] = name[n2]; // a segunda mais proxima recebe a nova que abriu
												c2_minX[n_cli] = assignment_cost[findEdge(g, n_cli, n2)];
											}
										}
										// SENAO, ELA NAO TEM NENHUMA RELACAO, NEM COMO PRIMEIRA NEM COMO SEGUNDA
										else { // eu sei que existe nearest2 pois tinha pelo menos 2 instalacoes ja la

											// Variavel auxiliar para nao ter perigo de perder informacao importante na substituicao
											closed_nearest = false;

											// SE A QUE FECHOU ERA A MAIS PROXIMA
											if(name[n] == nearest_open_fac[n_cli]){
												nearest_open_fac[n_cli] = nearest2_open_fac[n_cli]; // atualiza a instalacao mais proxima
												c_minX[n_cli] = c2_minX[n_cli];
												closed_nearest = true;
											}

											// SE A QUE FECHOU ERA A MAIS PROXIMA OU A SEGUNDA MAIS PROXIMA
											if((closed_nearest) || (name[n] == nearest2_open_fac[n_cli])){

												// Percorre para atualizar a segunda mais proxima
												c2_minX[n_cli] = biggestCij + 1; // limitante superior tranquilo


												// ERRO AQUI:: DEVERIA SER ITR2

												for (itr = open_facilities.begin(); itr != open_facilities.end(); ++itr) { // percorrer todas as inst abertas
													aux_cij = assignment_cost[findEdge(g, n_cli, facilities[*itr])];
													if((aux_cij < c2_minX[n_cli]) && (*itr != nearest_open_fac[n_cli]) && (*itr != name[n])){ // se for melhor que o salvo ate agr, mas nao melhor que o melhor de todos nem da que vai fechar
														nearest2_open_fac[n_cli] = *itr;
														c2_minX[n_cli] = aux_cij;
													}
												}

												// nao precisa conferir se a nova nao é a segunda mais proxima (ja q ela nao esta ainda no open_facilities) pois senao entraria no else if anterior
												
												if(c2_minX[n_cli] > biggestCij){ // se só tinha 1 inst aberta, entao colocamos -1 para indicar invalidez
													if(DEBUG >= DISPLAY_ACTIONS){
														cout << "There is just 1 open facility" << endl;
													}
													c2_minX[n_cli] = -1;
													nearest2_open_fac[n_cli] = -1;
												}
											}
										}

										solution.assigned_facilities[name[n_cli]] = nearest_open_fac[n_cli]; // salvando alteracoes sobre a inst mais proxima na solucao final
									}

									open[n] = false; // fechando de fato a primeira instalacao
									solution.open_facilities[name[n]] = false;
									open_facilities.erase(name[n]);
									closed_facilities.insert(name[n]);


									open[n2] = true; // abrindo de fato a segunda instalacao
									solution.open_facilities[name[n2]] = true; 
									open_facilities.insert(name[n2]);
									closed_facilities.erase(name[n2]);

									solution.finalTotalCost += complete_extra_cost; // Atualizando o custo total final

									if(DEBUG >= DISPLAY_ACTIONS){
										cout << "*After the swap move we have total cost: " << solution.finalTotalCost << endl;
									}

									solution.local_optimum = false; // Como encontrei uma melhora, entao eu nao estava no otimo local, portanto o while deve continuar

									swap_done = true;
									break; // quero que volte a olhar os movimentos de add e delete, ao inves de ver outras possiveis trocas
								}
								else {
									if(DEBUG >= DISPLAY_ACTIONS){
										cout << "NO! IT GOT WORSE" << endl;
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
									cout << "client: " << name[g.u(e)] << " new nearest facility " << name[n] << endl;
								}
					
								temp_nearest_fac[g.asRedNode(g.u(e))] = name[n]; // atualizando a inst mais perto temporaria

								// Atualizamos o custo extra, subtraindo o custo de atribuicao da antiga inst e somando o custo com a nova
								extra_cost += assignment_cost[e] - c_minX[g.asRedNode(g.u(e))];
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
								cout << "Let's swap out the facility " << name[n2] << endl;
							}

							complete_extra_cost = extra_cost - f[n2]; // subtrai o custo de abertura dessa inst

							for(ListBpGraph::RedNodeIt n_cli(g); n_cli != INVALID; ++n_cli){		// percorre os clientes

								if(temp_nearest_fac[n_cli] == name[n2]){ // caso esse cliente esteja conectado a essa instalacao

									// Devemos testar nearest2 com a instalacao que pretendo abrir e ver qual é a mais proxima
									aux_cij = assignment_cost[findEdge(g, n_cli, n)];
									if((aux_cij < c2_minX[n_cli])||(c2_minX[n_cli] < 0)){ // caso a nova seja a inst mais perto ou nao exista nearest2
										temp2_nearest_fac[n_cli] = name[n]; // atualizando a inst mais perto temporaria
									} 
									else{ // senao, entao a segunda mais proxima salva ser a mais proxima
										temp2_nearest_fac[n_cli] = nearest2_open_fac[n_cli]; // atualizando a inst mais perto temporaria
										aux_cij = c2_minX[n_cli];
									}

									if(DEBUG >= DISPLAY_ACTIONS){
										cout << "client: " << name[n_cli] << " new nearest facility " << temp2_nearest_fac[n_cli] << endl;
									}

									// Atualizamos o custo extra, subtraindo o custo de atribuicao da antiga inst e somando o custo com a nova
									complete_extra_cost += aux_cij - assignment_cost[findEdge(g, n_cli, facilities[temp_nearest_fac[n_cli]])];
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
									cout << "Move SWAP IN facility: " << name[n] << " and SWAP OUT facility: " << name[n2] << endl;
								}

								qty_moves += 1;

								for(ListBpGraph::RedNodeIt n_cli(g); n_cli != INVALID; ++n_cli){		// percorre os clientes

									// SE A NOVA INST QUE ABRIU É A MAIS PROXIMA
									if(name[n] == temp2_nearest_fac[n_cli]){

										// SE A QUE FECHOU NÃO ERA A MAIS PROXIMA ANTES
										if(name[n2] != nearest_open_fac[n_cli]){
											nearest2_open_fac[n_cli] = nearest_open_fac[n_cli]; // atualiza a segunda mais proxima com a mais proxima. obs: se tiver fechado a mais proxima, entao a segunda mais proxima continua igual
											c2_minX[n_cli] = c_minX[n_cli];
										}

										nearest_open_fac[n_cli] = name[n]; // atualiza a mais proxima com a que abriu
										c_minX[n_cli] = assignment_cost[findEdge(g, n_cli, n)]; // atualizando o menor cij desse cli
									}

									// SENAO, SE A NOVA É A SEGUNDA MAIS PROXIMA
									else if((assignment_cost[findEdge(g, n_cli, n)] < c2_minX[n_cli])||(c2_minX[n_cli] < 0)) { // senao, caso esse cliente tenha essa nova instalacao como segunda mais proxima ou nao exista segunda salva

										// SE A QUE FECHOU ERA A MAIS PROXIMA // nearest2 continuara sendo a mesma, mesmo se fosse inexistente
										if(name[n2] == nearest_open_fac[n_cli]){
											nearest_open_fac[n_cli] = name[n]; // a mais proxima recebe a nova que abriu
											c_minX[n_cli] = assignment_cost[findEdge(g, n_cli, n)];
										}
										else{
											nearest2_open_fac[n_cli] = name[n]; // a segunda mais proxima recebe a nova que abriu
											c2_minX[n_cli] = assignment_cost[findEdge(g, n_cli, n)];
										}
									}
									// SENAO, ELA NAO TEM NENHUMA RELACAO, NEM COMO PRIMEIRA NEM COMO SEGUNDA
									else { // eu sei que existe nearest2 pois tinha pelo menos 2 instalacoes ja la

										// Variavel auxiliar para nao ter perigo de perder informacao importante na substituicao
										closed_nearest = false;

										// SE A QUE FECHOU ERA A MAIS PROXIMA
										if(name[n2] == nearest_open_fac[n_cli]){
											nearest_open_fac[n_cli] = nearest2_open_fac[n_cli]; // atualiza a instalacao mais proxima
											c_minX[n_cli] = c2_minX[n_cli];
											closed_nearest = true;
										}

										// SE A QUE FECHOU ERA A MAIS PROXIMA OU A SEGUNDA MAIS PROXIMA
										if((closed_nearest) || (name[n2] == nearest2_open_fac[n_cli])){

											// Percorre para atualizar a segunda mais proxima
											c2_minX[n_cli] = biggestCij + 1; // limitante superior tranquilo


											// ERRO AQUI:: DEVERIA SER ITR2



											for (itr = open_facilities.begin(); itr != open_facilities.end(); ++itr) { // percorrer todas as inst abertas
												aux_cij = assignment_cost[findEdge(g, n_cli, facilities[*itr])];
												if((aux_cij < c2_minX[n_cli]) && (*itr != nearest_open_fac[n_cli]) && (*itr != name[n2])){ // se for melhor que o salvo ate agr, mas nao melhor que o melhor de todos nem da que vai fechar
													nearest2_open_fac[n_cli] = *itr;
													c2_minX[n_cli] = aux_cij;
												}
											}

											// nao precisa conferir se a nova nao é a segunda mais proxima (ja q ela nao esta ainda no open_facilities) pois senao entraria no else if anterior

											if(c2_minX[n_cli] > biggestCij){ // se só tinha 1 inst aberta, entao colocamos -1 para indicar invalidez
												if(DEBUG >= DISPLAY_ACTIONS){
													cout << "There is just 1 open facility" << endl;
												}
												c2_minX[n_cli] = -1;
												nearest2_open_fac[n_cli] = -1;
											}
										}
									}

									solution.assigned_facilities[name[n_cli]] = nearest_open_fac[n_cli]; // salvando alteracoes sobre a inst mais proxima na solucao final
								}

								open[n] = true; // abrindo de fato a primeira instalacao
								solution.open_facilities[name[n]] = true;
								closed_facilities.erase(name[n]);
								open_facilities.insert(name[n]);


								open[n2] = false; // fechando de fato a segunda instalacao
								solution.open_facilities[name[n2]] = false; 
								closed_facilities.insert(name[n2]);
								open_facilities.erase(name[n2]);

								solution.finalTotalCost += complete_extra_cost; // Atualizando o custo total final

								if(DEBUG >= DISPLAY_ACTIONS){
									cout << "*After the swap move we have total cost: " << solution.finalTotalCost << endl;
								}

								solution.local_optimum = false; // Como encontrei uma melhora, entao eu nao estava no otimo local, portanto o while deve continuar

								swap_done = true;
								break; // quero que volte a olhar os movimentos de add e delete, ao inves de ver outras possiveis trocas
							}
							else {
								if(DEBUG >= DISPLAY_ACTIONS){
									cout << "NO! IT GOT WORSE" << endl;
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
	}

	// Calculando o tempo gasto da funcao
	solution.timeSpent =  (finish.tv_sec - start.tv_sec);
	solution.timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 

	if(DEBUG >= DISPLAY_TIME){
		cout << "Final Total Function Time: " << solution.timeSpent << " seconds" << endl;
	}

	// // Acrescentando no solLog.txt o tempo gasto final da funcao e o custo final da solucao
	// solLog << solution.timeSpent << "," << solution.finalTotalCost << "," << qty_moves << endl;

	// solLog.close();

	free(clients);
	free(facilities);
	free(edges);

	return(solution);
}
