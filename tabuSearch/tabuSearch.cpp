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


// Retornar o valor da solucao
solutionType tabuSearch(char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA, solutionType solution, int a1, int lc1, int lc2, int lo1, int lo2, int itr_limit){

	cout << fixed;
   	cout.precision(5);

   	// Semente do numero aleatorio
   	srand(0); // depois alterar aqui pra pegar por parametro

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


	// Vetor da short term memory que representa o numero do movimento
	int * t;
	t = (int*) malloc(qty_facilities * sizeof(int));
	if(!t){
		cout << "Memory Allocation Failed";
		exit(1);
	}

	// Vetor que vai indicar se a instalacao está flagged
	bool * flag;
	flag = (bool*) malloc(qty_facilities * sizeof(bool));
	if(!flag){
		cout << "Memory Allocation Failed";
		exit(1);
	}


	// extra_cost - será o delta z para cada iteracao, para cada instalacao
	double **extra_cost = (double**) malloc((itr_limit) * sizeof(double*));

	for(int i = 0; i < itr_limit; i++) {
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

	// Vai receber lc ou lo, dependendo da checagem if(open[]), para evitar duplicidade no codigo
	int aux_l;

	// Declaracao de variavel auxiliar para formacao do arquivo .log
	char completeLogSolName[250] = "";

	// Arquivo para salvar o log da solucao
	ofstream solLog;
	strcat(completeLogSolName,solutionName);
	strcat(completeLogSolName,".log");
	solLog.open(completeLogSolName, std::ofstream::out | std::ofstream::trunc);

	//Salvando o cabecalho
	solLog << "time spent so far, current solution cost, current qty moves" << endl;

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


	// Iniciando o conjunto de instalacoes abertas e fechadas, alem dos vetores t e flag
	for(int i=0;i<qty_facilities;i++){
		if(open[facilities[i]]){
			open_facilities.insert(i);
			t[i] = - lo;
		}
		else{
			closed_facilities.insert(i);
			t[i] = - lc;
		}
		flag[i] = false;
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

	// Variavel que indica se foi feito uma troca, se sim, devemos dar um break no for para voltar pro while
	bool swap_done = false;

	// double melhor_custo_escolhido;

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
	double cur_cost = 0;

	// Total cost of the best solution found in the current search cycle
	double best_cost_found = cur_cost;

	// Best delta in this iteration
	double best_extra_cost;

	// Which facility corresponds to the best delta in this iteration
	int fac_best_extra_cost;

	// The move number when best_cost_found is updated
	int k_last_best = qty_moves;

	// Represents the amount of open facilities
	int n1 = open_facilities.size();

	// Incates step 3.. if we will move in fact
	bool lets_move = false;


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



	// INICIANDO A CONTAGEM DE TEMPO DA FUNCAO
	clock_gettime(CLOCK_REALTIME, &start);



	/* 
	STEP 1
	*/

	// Select a facility that has de minimum extra_cost and is not flagged
	// TAMBEM TESTAR DEPOIS SE ACHAR UM QUE MELHORA JA FAZ

	best_extra_cost = DBL_MAX; // limitante superior, maior double possivel
	fac_best_extra_cost = -1; // indica invalidez
	for(int i=0;i<qty_facilities;i++){
		if((extra_cost[qty_moves][i] < best_extra_cost) && ( !flag[i] )){ // se essa for menor do que a ja encontrada ate agr e nao estiver marcada, atualiza
			best_extra_cost = extra_cost[qty_moves][i];
			fac_best_extra_cost = i;
		}
	}

	if(fac_best_extra_cost == -1){
		cout << "THERE ARE NO MOVES ANYMORE" << endl;
	}
	else{

		// Check the tabu status of the selected move
		if(open[facilities[fac_best_extra_cost]]){ // se a instalacao está aberta
			aux_l = lo;
		}
		else{ // se a instalacao está fechada
			aux_l = lc;
		}

		if(qty_moves - t[fac_best_extra_cost] < aux_l){ // Se for tabu

			/* 
			STEP 2
			*/

			// Check the aspiration criterion of the selected move
			if((cur_cost + best_extra_cost) < best_cost_found){ // Se satisfizer o aspiration criterion
				/* 
				go to STEP 3
				*/
				lets_move = true;
			}
			else{ // Se nao satisfizer o aspiration criterion
				// Mark facility as flagged
				flag[fac_best_extra_cost] = true;

				/* 
				go back to STEP 1 -->>>>> while
				*/
			}

		}
		else{ //  se nao for tabu
			/* 
			go to STEP 3
			*/
			lets_move = true;
		}	


		/* 
		STEP 3
		*/

		if(lets_move){

			if(open[facilities[fac_best_extra_cost]]){ // se a instalação estiver aberta
				n1 -= 1;
			}
			else { // se a instalação estiver aberta
				n1 += 1;
			}

			// Abrindo a insta se estiver fechada e fechando se estiver aberta
			open[facilities[fac_best_extra_cost]] = !open[facilities[fac_best_extra_cost]];

			// Atualizando o custo atual
			cur_cost += best_extra_cost;

			// Atualizando a lista tabu
			t[fac_best_extra_cost] = qty_moves;

			// Aumentando a contagem de movimentos
			qty_moves += 1;

			// Atualizando a melhor solucao encontrada ate agr
			if(cur_cost < best_cost_found){
				best_cost_found = cur_cost;
				k_last_best = qty_moves;
			}

			/* 
			STEP 4
			*/

			// Update extra_cost[qty_moves]


			// Mark each facility as unflagged

			// if qty_moves - k_last_best < a1*n
					/* 
					go back to STEP 1 -->>>>> while
					*/
			// else
					/* 
					go to STEP 8 
					*/


			/* 
			STEP 8
			*/

			// If a local optimum has not been found, select a facility according to (10 and go to step 3)

		}

	}

	
















	// // ****** A partir daqui deve estar em um loop até nao ter mais melhoras possiveis, isto é, encontrar o otimo local

	// // OBSERVACAO 1: tambem existe uma condicao de parada relacionada a tempo, feita logo no inicio do loop
	// // OBSERVACAO 2: solution.local_optimum (variavel que indica se ja encontramos o otimo local) foi inicializado com valor falso na leitura da solução inicial (handlesInput.cpp)

	// while(!solution.local_optimum){

	// 	// CHECANDO A CONTAGEM DE TEMPO GASTO ATÉ AGORA
	// 	clock_gettime(CLOCK_REALTIME, &time_so_far);

	// 	// Calculando o tempo gasto até agora
	// 	solution.timeSpent =  (time_so_far.tv_sec - start.tv_sec);
	// 	solution.timeSpent += (time_so_far.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 

	// 	if(DEBUG >= DISPLAY_TIME){
	// 		cout << "Total time spent so far: " << solution.timeSpent << " seconds" << endl;
	// 	}

	// 	// Se já passou o tempo limite, devemos parar
	// 	if(solution.timeSpent >= TIME_LIMIT){
	// 		// Acrescentando no solLog.txt o tempo final gasto nas iteracoes e o custo da solucao
	// 		solLog << solution.timeSpent << "," << solution.finalTotalCost << "," << qty_moves << endl;

	// 		break;
	// 	}
	// 	else if(solution.timeSpent >= timeCounter){
	// 		// aumentando o contador de tempo
	// 		timeCounter += TIME_COUNTER_STEP;

	// 		// Acrescentando no solLog.txt o tempo gasto nessa iteracao e o custo da solucao
	// 		solLog << solution.timeSpent << "," << solution.finalTotalCost << "," << qty_moves << endl;
	// 	}

	// 	if(DEBUG >= DISPLAY_ACTIONS){
	// 		cout << endl << "------------------------------ NEXT MOVE ------------------------------" << endl << endl;
	// 	}
		
	// 	// Primeiramente vamos percorrer todas as instalacoes
	// 	// Caso a instalacao esteja aberta, vamos tentar fechá-la e ver se há alguma melhora
	// 	// Lembrando que temos que reatribuir todas os clientes a ela conectados, mudando para a prox inst mais proxima
	// 	// Se houver melhora no custo total, efetuamos de fato a operacao
	// 	// Da mesma forma, caso a instalacao esteja fechada, vamos tentar abri-la e ver se melhora
	// 	// Lembrando que temos que buscar todos os clientes que possuem ela como inst aberta mais proxima para conecta-los
	// 	// Esse teste sera sequencialmente feito com um for ate o final de percorrer as instalacoes
	// 	// Apos isso, caso tenha encontrado alguma melhora, parte para a proxima iteracao do while e o for será repetido
	// 	// Caso nenhuma melhora tenha sido encontrada, parte para o caso da troca

	// 	// Apos testar todas as opcoes de add e delete e nada melhorar a solucao, vamos tentar a troca
	// 	// Portanto havera um outro for, embaixo desse primeiro for, ainda dentro do while, no qual percorrera todas as inst
	// 	// Caso a inst esteja aberta tenta fechar ela e haverá um for inteiro para tentar abrir as instalacoes SUBSEQUENTES (nao precisa repetir as anteriores, mas nao sei se consigo fazer isso no iterator) que estao fechadas atualmente
	// 	// Caso esse movimento de troca resulte em uma melhora, será de fato feita a operacao
	// 	// Do contrario, continuará percorrendo o for interno na busca por abrir a proxima inst fechada
	// 	// Se nenhuma for encontrada, vai para a proxima iteracao do for externo e repete o processo
	// 	// OBSERVAÇÃO IMPORTANTE: Quando encontrar o primeiro caso de troca bem sucedido, esse for para imediatamente, indo para a proxima iteracao do while
	// 	// Assim serao testados novamente os movimentos de add ou delete, que sao mais simples
	// 	// Só voltando novamente para a troca caso nenhum add nem delete melhore nada
	// 	// OBSERVAÇÃO IMPORTANTE 2: Quando em nenhum caso de troca for encontrado melhora, encontramos de fato o minimo local!! E o while deve terminar


	// 	// Podemos considerar que inicialmente estamos no otimo local ate que se prove o contrario. 
	// 	// Entao, vou percorrer todas as instalacoes e, caso eu encontre alguma melhora no percurso, modifico esse otimo local para falso
	// 	// Na proxima iteracao volto a considerar que estamos no otimo local e repito o processo
	// 	// No momento que eu percorrer todas as instalacoes e nenhuma conseguir melhorar nada, entao eu realmente estava no otimo local e finalizo o while
	// 	// Encaixando a operação de troca, será um for logo depois desse primeiro for, que só ira entrar caso finalize o primeiro for com solution.local_optimum = true, pois indica que nada melhorou por ali
	// 	solution.local_optimum = true; 

	// 	// VIZINHANCA 1: ABRIR OU FECHAR UMA INSTALACAO
	// 	for(ListBpGraph::BlueNodeIt n(g); n != INVALID; ++n){		// percorre as instalacoes

	// 		if(DEBUG >= DISPLAY_ACTIONS){
	// 			cout << "Let's move facility " << name[n] << endl;
	// 		}

	// 		if(open[n]){ // caso a inst esteja aberta

	// 			if(DEBUG >= DISPLAY_ACTIONS){
	// 				cout << "It is OPEN, let's close it" << endl;
	// 			}

	// 			if(open_facilities.size() <= 1){
	// 				if(DEBUG >= DISPLAY_ACTIONS){
	// 					cout << "Sorry, you can't close it because it is the only one that is open." << endl;
	// 				}
	// 			}
	// 			else{
	// 				// Vamos fechar essa instalacao

	// 				extra_cost2 = -f[n]; // subtrai o custo de abertura dessa inst

	// 				for(ListBpGraph::RedNodeIt n_cli(g); n_cli != INVALID; ++n_cli){		// percorre os clientes

	// 					if(nearest_open_fac[n_cli] == name[n]){ // caso esse cliente esteja conectado a essa instalacao

	// 						if(c2_minX[n_cli] >= 0){ // CASO EXISTA A SEGUNDA INST MAIS PROXIMA SALVA

	// 							// Atualizamos o custo extra, subtraindo o custo de atribuicao da antiga inst e somando o custo com a nova
	// 							extra_cost2 += c2_minX[n_cli] - c_minX[n_cli];

	// 							if(DEBUG >= DISPLAY_ACTIONS){
	// 								cout << "client: " << name[n_cli] << " new nearest facility " << nearest2_open_fac[n_cli] << endl;
	// 							}
	// 						}
	// 						else{
	// 							if(DEBUG >= DISPLAY_ACTIONS){
	// 								cout << "You shouldn't be here. There is just 1 open facility and you want to close it." << endl;
	// 							}
	// 							continue; // vai para a proxima iteracao
	// 						}
	// 					}
	// 				}

	// 				if(DEBUG >= DISPLAY_ACTIONS){
	// 					cout << "--- EXTRA COST: " << extra_cost2 << endl;
	// 				}

	// 				// Caso melhore o custo total final, vamos efetuar de fato essa alteracao
	// 				if(extra_cost2 < 0){

	// 					if(DEBUG >= DISPLAY_ACTIONS){
	// 						cout << "YEEES THE COST IS BETTER, LET'S DO IT " << endl;
	// 					}

	// 					if(DEBUG >= DISPLAY_MOVES){
	// 						cout << "Move CLOSE facility: "<< name[n] << endl;
	// 					}

	// 					qty_moves += 1;

	// 					for(ListBpGraph::RedNodeIt n_cli(g); n_cli != INVALID; ++n_cli){		// percorre os clientes

	// 						closed_nearest = false;

	// 						if(nearest_open_fac[n_cli] == name[n]){ // caso esse cliente esteja conectado a essa instalacao

	// 							closed_nearest = true;

	// 							if(DEBUG >= DISPLAY_ACTIONS){
	// 								cout << "changing client: " << name[n_cli] << " nearest facility " << name[n] << endl;
	// 							}

	// 							// ATUALIZANDO AQUI A INST MAIS PROXIMA
	// 							nearest_open_fac[n_cli] = nearest2_open_fac[n_cli]; // reatribuindo com a nova inst mais proxima
	// 							c_minX[n_cli] = c2_minX[n_cli]; // atualizando o menor cij desse cli

	// 							// SALVANDO A SOLUCAO NOVA
	// 							solution.assigned_facilities[name[n_cli]] = nearest_open_fac[n_cli]; // salvando alteracoes sobre a inst mais proxima na solucao final
	// 						}

	// 						if((closed_nearest)||(nearest2_open_fac[n_cli] == name[n])){ // caso esse cliente tinha essa instalacao como primeira ou segunda mais proxima
	// 							// ATUALIZANDO AQUI A SEGUNDA MAIS PROXIMA
	// 							c2_minX[n_cli] = biggestCij + 1; // limitante superior tranquilo

	// 							for (itr = open_facilities.begin(); itr != open_facilities.end(); ++itr) { // percorrer todas as inst abertas
	// 								aux_cij = assignment_cost[findEdge(g, n_cli, facilities[*itr])];
	// 								if((aux_cij < c2_minX[n_cli]) && (*itr != nearest_open_fac[n_cli]) && (*itr != name[n])){ // se for melhor que o salvo ate agr, mas nao melhor que o melhor de todos nem da que vai fechar
	// 									nearest2_open_fac[n_cli] = *itr;
	// 									c2_minX[n_cli] = aux_cij;
	// 								}
	// 							}
	// 							if(c2_minX[n_cli] > biggestCij){ // se só tinha 1 inst aberta depois do fechamento dessa iteracao, entao colocamos -1 para indicar invalidez
	// 								if(DEBUG >= DISPLAY_ACTIONS){
	// 									cout << "There is just 1 open facility" << endl;
	// 								}
	// 								c2_minX[n_cli] = -1;
	// 								nearest2_open_fac[n_cli] = -1;
	// 							}
	// 						}
	// 					}

	// 					open[n] = false; // fechando de fato a instalacao
	// 					open_facilities.erase(name[n]);
	// 					closed_facilities.insert(name[n]);

	// 					solution.finalTotalCost += extra_cost2; // Atualizando o custo total final

	// 					solution.local_optimum = false; // Como encontrei uma melhora, entao eu nao estava no otimo local, portanto o while deve continuar
	// 				}
	// 				else {
	// 					if(DEBUG >= DISPLAY_ACTIONS){
	// 						cout << "NO! IT GOT WORSE" << endl;
	// 					}
	// 				}
	// 			}
	// 		}

	// 		else { // caso a inst esteja fechada

	// 			if(DEBUG >= DISPLAY_ACTIONS){
	// 				cout <<"It is CLOSED, let's open it" << endl;
	// 			}

	// 			// Vamos abrir essa instalacao

	// 			extra_cost2 = f[n]; // soma o custo de abertura dessa inst

	// 			for (ListBpGraph::IncEdgeIt e(g, n); e != INVALID; ++e) { // Percorre todas arestas desse nó (ligam a clientes)

	// 				if(assignment_cost[e] < c_minX[g.asRedNode(g.u(e))]){ // caso a nova inst seja a mais proxima aberta desse cliente

	// 					if(DEBUG >= DISPLAY_ACTIONS){
	// 						cout << "client: " << name[g.u(e)] << " new nearest facility " << name[n] << endl;
	// 					}
			
	// 					temp_nearest_fac[g.asRedNode(g.u(e))] = name[n]; // atualizando a inst mais perto temporaria

	// 					// Atualizamos o custo extra, subtraindo o custo de atribuicao da antiga inst e somando o custo com a nova
	// 					extra_cost2 += assignment_cost[e] - c_minX[g.asRedNode(g.u(e))];
	// 				}
	// 				else { // senao, caso a nova inst nao seja mais proxima
	// 					temp_nearest_fac[g.asRedNode(g.u(e))] = nearest_open_fac[g.asRedNode(g.u(e))]; // colocando isso apenas para uso posterior na checagem nao haver lixo
	// 				}
	// 			}

	// 			if(DEBUG >= DISPLAY_ACTIONS){
	// 				cout << "--- EXTRA COST: " << extra_cost2 << endl;
	// 			}

	// 			// Caso melhore o custo total final, vamos efetuar de fato essa alteracao
	// 			if(extra_cost2 < 0){

	// 				if(DEBUG >= DISPLAY_ACTIONS){
	// 					cout << "YEEES THE COST IS BETTER, LET'S DO IT " << endl;
	// 				}

	// 				if(DEBUG >= DISPLAY_MOVES){
	// 					cout << "Move OPEN facility: "<< name[n] << endl;

	// 				}

	// 				qty_moves += 1;

	// 				for(ListBpGraph::RedNodeIt n_cli(g); n_cli != INVALID; ++n_cli){		// percorre os clientes

	// 					aux_cij = assignment_cost[findEdge(g, n_cli, n)];

	// 					if(nearest_open_fac[n_cli] != temp_nearest_fac[n_cli]) { // caso esse cliente seja mais proximo dessa nova instalacao
	// 						if(DEBUG >= DISPLAY_ACTIONS){
	// 							cout << "changing client: " << name[n_cli] << " nearest facility " << name[n] << endl;
	// 						}

	// 						// ATUALIZANDO A SEGUNDA INST MAIS PROXIMA
	// 						nearest2_open_fac[n_cli] = nearest_open_fac[n_cli];
	// 						c2_minX[n_cli] = c_minX[n_cli];

	// 						// ATUALIZANDO A INSTALACAO MAIS PROXIMA
	// 						nearest_open_fac[n_cli] = name[n]; // reatribuindo com a nova inst mais proxima
	// 						c_minX[n_cli] = aux_cij; // atualizando o menor cij desse cli

	// 						solution.assigned_facilities[name[n_cli]] = nearest_open_fac[n_cli]; // salvando alteracoes na solucao final
	// 					}
	// 					else if((aux_cij < c2_minX[n_cli])||(c2_minX[n_cli] < 0)) { // senao, caso esse cliente tenha essa nova instalacao como segunda mais proxima (ou nao existia segunda mais proxima)
	// 						// ATUALIZANDO A SEGUNDA INST MAIS PROXIMA
	// 						nearest2_open_fac[n_cli] = name[n];
	// 						c2_minX[n_cli] = aux_cij;
	// 					}
	// 				}

	// 				open[n] = true; // abrindo de fato a instalacao
	// 				closed_facilities.erase(name[n]);
	// 				open_facilities.insert(name[n]);

	// 				solution.finalTotalCost += extra_cost2; // Atualizando o custo total final

	// 				solution.local_optimum = false; // Como encontrei uma melhora, entao eu nao estava no otimo local, portanto o while deve continuar
	// 			}
	// 			else {
	// 				if(DEBUG >= DISPLAY_ACTIONS){
	// 					cout << "NO! IT GOT WORSE" << endl;
	// 				}
	// 			}
	// 		}
	// 	}
	// }

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

	// Acrescentando no solLog.txt o tempo gasto final da funcao e o custo final da solucao
	solLog << solution.timeSpent << "," << solution.finalTotalCost << "," << qty_moves << endl;

	solLog.close();

	free(clients);
	free(facilities);
	free(edges);

	return(solution);
}
