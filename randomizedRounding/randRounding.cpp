#include <lemon/list_graph.h>
#include <algorithm>
#include <iostream>
#include <ctime>
#include "definitions.hpp"
#define EPSL 0.00000001

using namespace lemon;
using namespace std;

#define DISPLAY_ACTIONS 1 // corresponde a todos os cout quando uma acao é feita. 
#define DISPLAY_TIME 2 // corresponde aos calculos de tempo 
#define DISPLAY_GRAPH 3 // corresponde a descricao dos clientes, instalacoes e arcos

#define DEBUG 0 // OPCOES DE DEBUG: 1 PARA EXIBIR ACOES, 2 PARA EXIBIR TEMPO, 3 PARA EXIBIR AS MUDANÇAS NO GRAFO


double randZeroToOne(){
	return ((double) rand() / (RAND_MAX));
}


void mergeSort(double *vetor, int *vetorID, int posicaoInicio, int posicaoFim) {

    int i, j, k, metadeTamanho, *vetorTempID;
    double *vetorTemp;

    if(posicaoInicio == posicaoFim) return;
    metadeTamanho = (posicaoInicio + posicaoFim ) / 2;

    mergeSort(vetor, vetorID, posicaoInicio, metadeTamanho);
    mergeSort(vetor, vetorID, metadeTamanho + 1, posicaoFim);

    i = posicaoInicio;
    j = metadeTamanho + 1;
    k = 0;
    vetorTemp = (double *) malloc(sizeof(double) * (posicaoFim - posicaoInicio + 1));
    vetorTempID = (int *) malloc(sizeof(int) * (posicaoFim - posicaoInicio + 1));

    while(i < metadeTamanho + 1 || j  < posicaoFim + 1) {
        if (i == metadeTamanho + 1 ) { 
            vetorTemp[k] = vetor[j];
            vetorTempID[k] = vetorID[j];
            j++;
            k++;
        }
        else {
            if (j == posicaoFim + 1) {
                vetorTemp[k] = vetor[i];
                vetorTempID[k] = vetorID[i];
                i++;
                k++;
            }
            else {
                if (vetor[i] < vetor[j]) {
                    vetorTemp[k] = vetor[i];
                	vetorTempID[k] = vetorID[i];
                    i++;
                    k++;
                }
                else {
                    vetorTemp[k] = vetor[j];
                	vetorTempID[k] = vetorID[j];
                    j++;
                    k++;
                }
            }
        }

    }
    for(i = posicaoInicio; i <= posicaoFim; i++) {
        vetor[i] = vetorTemp[i - posicaoInicio];
        vetorID[i] = vetorTempID[i - posicaoInicio];
    }
    free(vetorTemp);
    free(vetorTempID);
}



// Retornar o valor da solucao
solutionType randRounding(int qty_facilities, int qty_clients, double * costF, double * costA, double ** x_values, double * v_values){

	cout << fixed;
   	cout.precision(5);

   	// Semente do numero aleatorio
   	srand((unsigned)time(NULL));

	// Struct que vai retornar a solução
	solutionType solution;

	// Inicializando struct 
	solution.qty_clients = qty_clients;
	solution.finalTotalCost = 0;
	solution.timeSpent = 0;

	// indicara as instalacoes atribuidas a cada cliente na resposta
	solution.assigned_facilities = (int*) malloc((qty_clients) * sizeof(int));
	if(!solution.assigned_facilities){
		cout << "Memory Allocation Failed";
		exit(1);
	}

	/* Inicio declaracoes variaveis para calculo de tempo */

	// Declaracao variaveis que indicam o tempo do programa como um todo
	struct timespec real_start, real_finish;

	// Declaracao variavel que marcara o tempo de execucao da funcao como um todo
	double realTimeSpent;

	/* Fim declaracoes para calculo de tempo */

	if(DEBUG >= DISPLAY_TIME){
		// Iniciando a contagem de tempo da funcao como um todo
		clock_gettime(CLOCK_REALTIME, &real_start);
	}

	/* Sobre os grafos bipartidos, teremos
	
	Red para clientes

	Blue para instalações

	*/
	// Grafo que liga os clientes às instalações
	ListBpGraph g;

	// v_C - será correnpondente a vj* + Cj* , sendo Cj* = sum_{i \in F} (cij xij*)
	ListBpGraph::RedNodeMap<double> v_C(g);

	// index_vC - será correnpondente ao indice que esse cliente está no vetor ordenado sorted_vC
	ListBpGraph::RedNodeMap<int> index_vC(g);

	// f - será o custo de instalação (fi)
	ListBpGraph::BlueNodeMap<double> f(g);

	// open - Indica se a instalação está aberta ou não
	ListBpGraph::BlueNodeMap<bool> open(g);


	// sorted_vC - será os clientes ativos ordenados pelo v_C 
	double *sorted_vC = (double*) malloc((qty_clients) * sizeof(double));

	// sorted_vC_ID - indica os ID dos clientes ativos ordenados pelo v_C
	int *sorted_vC_ID = (int*) malloc((qty_clients) * sizeof(int));

    if((!sorted_vC)||(!sorted_vC_ID)){
        cout << "Memory Allocation Failed";
        exit(1);
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
		v_C[clients[i]] = v_values[i]; // inicia com o valor de vj
		index_vC[clients[i]] = -1; // inicialmente nao está em nenhuma posicao na ordenacao
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
			v_C[clients[i]] += costA[counter] * x_values[j][i]; // acrescentando Cj* = sum_{i \in F} (cij xij*)
			counter++;
		}

		// Colocando os v_C no vetor que sera ordenado
		sorted_vC[i] = v_C[clients[i]];
		sorted_vC_ID[i] = i;
	}

	// ordenando o vetor
	mergeSort(sorted_vC, sorted_vC_ID, 0, qty_clients-1);

	if (DEBUG >= DISPLAY_ACTIONS){
		cout << "Now we have the array sorted_vC sorted"<< endl;
	}

	// Salvando o index de cada cliente no vetor sorted_vC em seu node map
	for(int i=0;i<qty_clients;i++){
		index_vC[clients[sorted_vC_ID[i]]] = i;

		if (DEBUG >= DISPLAY_ACTIONS){
			cout << "Client " << sorted_vC_ID[i] << ": " << sorted_vC[i] << endl;  
		}
	}


	if (DEBUG >= DISPLAY_GRAPH){
		// Percorrendo por todos os nós A - clientes
		cout << "Scrolling through all clients" << endl;
		for(ListBpGraph::RedNodeIt n(g); n != INVALID; ++n){
			cout << "node id: " << g.id(n) << " - v_C: " << v_C[n];
			cout << " - index_vC: " << index_vC[n] << endl;
		}

		// Percorrendo por todos os nós B - instalacoes
		cout << "Scrolling through all facilities" << endl;
		for(ListBpGraph::BlueNodeIt n(g); n != INVALID; ++n){
			cout << "node id: " << g.id(n) << " - f: " << f[n] << " - open: " << open[n] << endl;
		}

		// Percorrendo por todos os arcos
		cout << "Scrolling through all edges" << endl;
		for(ListBpGraph::EdgeIt e(g); e!= INVALID; ++e){
			cout << "edge id: " << g.id(e) ;
			cout << " - client: " << g.id(g.u(e)) << " - facility: " << g.id(g.v(e));
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
    int cur_i_sorted_vC = 0; // indica qual o proximo indice do vetor sorted_vC devo tentar usar (que ainda nao foi usado)
	int cur_client = -1; // indica qual cliente escolhemos nessa iteracao atual
	int cur_facility = -1; // indica qual instalacao escolhemos nessa iteracao atual, vizinha de cur_client
	double rand_value; // indica o valor gerado randomicamente naquela iteracao 
	double sum_xij; // indica a soma dos xij ate encontrar a instalacao correspondente sorteada


	// ****** A partir daqui deve estar em um loop até todos os clientes se conectarem a alguma instalacao

	while(qty_active_clients > 0){

		if(DEBUG >= DISPLAY_ACTIONS){
			cout << endl << "------------------------------ WE STILL HAVE " << qty_active_clients << " ACTIVE CLIENTS ------------------------------" << endl << endl;
		}

		/*
		Escolha jk \in C que minimize vj* + Cj* dentre todos j \in C
		*/

		while((cur_i_sorted_vC < qty_clients ) && (sorted_vC[cur_i_sorted_vC] < 0)){ // segunda condicao indica que era -1, q significa que o cliente nao está ativo mais
			cur_i_sorted_vC += 1; // entao vamos tentar o proximo
		}

		// Se acabaram os clientes, devemos dar break (nao era pra entrar aqui nunca)
		if(cur_i_sorted_vC >= qty_clients){
			if(DEBUG >= DISPLAY_ACTIONS){
				cout << "Weird value of cur_i_sorted_vC, it's greater than qty_clients" << endl;
			}
			break;
		}

		// Pegando o primeiro cliente ok
		cur_client = sorted_vC_ID[cur_i_sorted_vC];

		if(DEBUG >= DISPLAY_ACTIONS){
			cout << "Current client: " << cur_client << endl;
		}


		/*
		Escolha ik \in N(jk) de acordo com a distribuição probabilística xij*
		*/

		rand_value = randZeroToOne();

		if(DEBUG >= DISPLAY_ACTIONS){
			cout << "rand value = " << rand_value << endl;
		}

		cur_facility = 0;
		sum_xij = 0;

		// Encontrando qual a instalacao correspondente ao numero sorteado
		while(cur_facility < qty_facilities){
			sum_xij += x_values[cur_facility][cur_client];
			if(sum_xij > rand_value){
				break;
			}
			cur_facility += 1;
		}

		if(DEBUG >= DISPLAY_ACTIONS){
			cout << "sum = " << sum_xij << endl;
			cout << "Chosen facility = " << cur_facility << endl;
		}

		/*
		F' <- F' U {ik}
		*/

		// Abrindo a instalacao escolhida
		if(!open[facilities[cur_facility]]){

			if(DEBUG >= DISPLAY_ACTIONS){
				cout << "Opening facility " << cur_facility << endl;
			}

			open[facilities[cur_facility]] = true;
			solution.finalTotalCost += f[facilities[cur_facility]];
		}

		/*
		Atribua jk e todos os clientes não atribuídos em N2(jk) a ik
		*/

		for(int j=0;j<qty_clients;j++){
			if(x_values[cur_facility][j] > 0){   // Se esse cliente for vizinho de cur_facility, vamos atribui-lo

				if(DEBUG >= DISPLAY_ACTIONS){
					cout << "Assigning client " << j << " to facility " << cur_facility << endl;
				}

				solution.assigned_facilities[j] = cur_facility;
				solution.finalTotalCost += assignment_cost[findEdge(g, clients[j], facilities[cur_facility])];

				/*
				C <- C - jk - N2(jk)
				*/

				// Desativando cliente
				sorted_vC[index_vC[clients[j]]] = -1;
				qty_active_clients -= 1;

				// Zerando todos os valores de x_values correspondentes a esse cliente
				for(int i=0;i<qty_facilities;i++){
					x_values[i][j] = 0;
				}
			}
		}
	}

	cout << "FINAL TOTAL COST: " << solution.finalTotalCost << endl;

	if(DEBUG >= DISPLAY_TIME){

		// Finalizando a contagem de tempo da funcao como um todo
		clock_gettime(CLOCK_REALTIME, &real_finish);

		// Calculando o tempo gasto total
		realTimeSpent =  (real_finish.tv_sec - real_start.tv_sec);
		realTimeSpent += (real_finish.tv_nsec - real_start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
		cout << "Final Total Function Time: " << realTimeSpent << " seconds" << endl;
	}

	free(clients);
	free(facilities);
	free(edges);
	free(sorted_vC);
	free(sorted_vC_ID);

	return(solution);
}
