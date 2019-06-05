#include <lemon/list_graph.h>
#include <algorithm>
#include <iostream>
#include <set>
#include <iterator>
#include <ctime>
#include "declaracoes.hpp"
#define EPSL 0.00000001


// Link para visualizacao relatorio: https://www.overleaf.com/read/ptfjyjssxwkc

// Casos de testes: http://resources.mpi-inf.mpg.de/departments/d1/projects/benchmarks/UflLib/index.html

// TODO: seria legal se tivesse uma fila de prioridades para ver o caso A (menor cij) 


// Observacao importante: ao usar S.nodeFromId() eh necessario passar o ID geral, nao o blue ID nem red ID. 
// No caso do cliente, o red ID = ID geral, pois os clientes foram os primeiros a serem adicionados no grafo.
// No caso da instalacao, o ID geral = blue ID + qtdClientes, pois as instalacoes foram adicionadas no grafo logo apos todos os clientes.

using namespace lemon;
using namespace std;

#define EXIBIR_ACOES 1 // corresponde a todos os cout quando uma acao é feita. 
#define EXIBIR_TEMPO 2 // corresponde aos calculos de tempo 
#define EXIBIR_GRAFO 3 // corresponde a descricao dos clientes, instalacoes e arcos
#define EXIBIR_MATRIZ_ADJACENCIA 4 // corresponde à parte final, na criacao de Tlinha

#define DEBUG 1 // OPCOES DE DEBUG: 1 PARA EXIBIR ACOES, 2 PARA EXIBIR TEMPO, 3 PARA EXIBIR AS MUDANÇAS NO GRAFO, 4 PARA EXIBIR AS MUDANCAS NA MATRIZ DE ADJACENCIA NA CRIACAO DE TLINHA



bool igual(double i, double j){
    if((i > j-EPSL) && (i < j+EPSL))
        return true;
    return false;
}

bool maior_igual(double i, double j){
    // if(i >= j * (1-EPSL))
    if(i >= j - EPSL)
        return true;
    return false;
}


// Retornar o valor da solucao
double primalDual(int qtdCli, int qtdInst, double * custoF, double * custoA){
	
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


	if(DEBUG >= EXIBIR_TEMPO){
		// INICIANDO A CONTAGEM DE TEMPO DA FUNCAO COMO UM TODO
		clock_gettime(CLOCK_REALTIME, &real_start);
	}


	int qtd_clientes = qtdCli; // Indica quantidade de clientes
	int qtd_instalacoes = qtdInst; // Indica quantidade de instalacoes


	// conjunto de clientes a serem removidos de S na iteração, pois deixaram de ser ativos
    set <int, greater <int> > apagar_clientes; 
    // Iterator para o conjunto 
    set <int, greater <int> > :: iterator itr; 

	/*

	ESTRUTURA QUE INDICA QUANTOS E QUAIS CLIENTES ESTAO CONTRIBUINDO (AUMENTANDO O W) 
	PARA UMA CERTA INSTALACAO. valor fixo, nao muda durante o algoritmo, só no final, na criaçao de Tlinha.
	Essa estrutura é importante pois perdemos os valores do grafo S.

	- cada linha indica um cliente
	- cada coluna indica uma instalacao
	- Valor 1 na linha i, coluna j se o cliente i contribui pra instalacao j

	*/
	
	// indica se o cliente alcançou a instalação
	int **matriz_adjacencia = (int**) malloc((qtd_clientes) * sizeof(int*));
	for(int i = 0; i < qtd_clientes; i++) matriz_adjacencia[i] = (int *)malloc(qtd_instalacoes * sizeof(int));

    if(!matriz_adjacencia){
        cout << "Memory Allocation Failed";
        exit(1);
    }

	/* Sobre os grafos bipartidos, teremos
	
	Red para clientes

	Blue para instalações

	*/
	// Grafo que liga os clientes às instalações
	ListBpGraph g;

	// v - será o quanto o cliente vai contribuir (v)
	ListBpGraph::RedNodeMap<double> v(g);

	// f - será o custo de instalação (fi)
	ListBpGraph::BlueNodeMap<double> f(g);

	// somatorioW - indica o quanto ja foi pago do custo de abrir a instalacao i... somatorio de todos os w correspondente a essa i
	ListBpGraph::BlueNodeMap<double> somatorioW(g);

	// qtdContribuintes - será a quantidade de clientes ativos que estao prontos para contribuir com a inst i
	ListBpGraph::BlueNodeMap<int> qtdContribuintes(g);

	// Indica se a instalação está aberta ou não
	ListBpGraph::BlueNodeMap<bool> aberta(g);

	// Indica se a instalação está em Tlinha (util para o final)
	ListBpGraph::BlueNodeMap<bool> estaEmTlinha(g);

	// Para identificar cada nó
	ListBpGraph::NodeMap<int> nome(g);

	// Variavel que armazena o maior valor cij dado na entrada, para uso posterior
	int maiorCij = 0;

	// Variavel que armazena o maior valor fi dado na entrada, para uso posterior
	int maiorFi = 0;

	if(DEBUG >= EXIBIR_TEMPO){
		cout <<"[TEMPO] Iniciando contagem de tempo para criacao do grafo e seus maps" << endl;
		//Iniciando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &start);
	}

	// Criação de nós clientes e atribuição de seus labels
	ListBpGraph::RedNode * clientes;
	clientes = (ListBpGraph::RedNode*) malloc((qtd_clientes) * sizeof(ListBpGraph::RedNode));
    if(!clientes){
        cout << "Memory Allocation Failed";
        exit(1);
    }

	for(int i=0;i<qtd_clientes;i++){
		clientes[i] = g.addRedNode();
		v[clientes[i]] = 0; // v = inicialmente nao contribui com nada
		nome[clientes[i]] = i; // nomeia de acordo com a numeracao
	}


	// Criação de nós de instalações e atribuição de seus labels
	ListBpGraph::BlueNode * instalacoes;
	instalacoes = (ListBpGraph::BlueNode*) malloc((qtd_instalacoes) * sizeof(ListBpGraph::BlueNode));
    if(!instalacoes){
        cout << "Memory Allocation Failed";
        exit(1);
    }


	for(int i=0;i<qtd_instalacoes;i++){
		instalacoes[i] = g.addBlueNode();
		f[instalacoes[i]] = custoF[i]; // pegando valor vindo por parametro
		somatorioW[instalacoes[i]] = 0; // no começo nada foi pago dessa instalacao
		qtdContribuintes[instalacoes[i]] = 0; // no começo ninguem contribui pra ninguem
		aberta[instalacoes[i]] = false; // indica que a instalação não está aberta inicialmente
		estaEmTlinha[instalacoes[i]] = false; // no começo ninguem esta em Tlinha
		nome[instalacoes[i]] = qtd_clientes + i; // nomeia de acordo com a numeracao

		// Salvando o valor do maior Fi da entrada
		if(f[instalacoes[i]]>maiorFi){
			maiorFi = f[instalacoes[i]];
		}
	}


	// Custo de atribuição é o custo de atribuir o cliente associado à instalação associada
	ListBpGraph::EdgeMap<double> custoAtribuicao(g);

	// W é a parte do custo da instalação associada que o cliente associado vai contribuir
	ListBpGraph::EdgeMap<double> w(g);

	// Indica se aquele cliente ja está pagando todo seu custo de atribuicao àquela instalação, e esta pronto para começar a aumentar o w
	ListBpGraph::EdgeMap<bool> prontoContribuirW(g);


	// Criação de arcos e atribuição de seus labels
	ListBpGraph::Edge * arcos; // conectando todos com todos
	arcos = (ListBpGraph::Edge*) malloc((qtd_clientes*qtd_instalacoes) * sizeof(ListBpGraph::Edge));
    if(!arcos){
        cout << "Memory Allocation Failed";
        exit(1);
    }

	int cont = 0;
	for(int i=0;i<qtd_clientes;i++){
		for(int j=0;j<qtd_instalacoes;j++){
			arcos[cont] = g.addEdge(clientes[i],instalacoes[j]);
			custoAtribuicao[arcos[cont]] = custoA[cont]; // pegando valor vindo por parametro
			w[arcos[cont]] = 0; // inicialmente nao contribui com nada
			prontoContribuirW[arcos[cont]] = false; // inicia com falso em todos

			// Salvando o valor do maior Cij da entrada
			if(custoAtribuicao[arcos[cont]]>maiorCij){
				maiorCij = custoAtribuicao[arcos[cont]];
			}

			cont++;
		}
	}


	if(DEBUG >= EXIBIR_ACOES){
		cout << "maiorCij da entrada : " << maiorCij << endl << "maiorFi da entrada: " << maiorFi << endl;
	}

	int qtd_clientes_ativos_S = qtd_clientes; // Indica a quantidade de clientes ainda em S, os clientes ativos.

	int qtd_inst_abertas = 0; // indica a quantidade de instalacoes abertas


	// Inicializando a matriz de adjacencia, todos com valor 0
	for(int i=0;i<qtd_clientes;i++){
		for(int j=0;j<qtd_instalacoes;j++){
			matriz_adjacencia[i][j] = 0;
		}
	}

	if(DEBUG >= EXIBIR_TEMPO){
		cout <<"[TEMPO] Finalizando contagem de tempo para criacao do grafo e seus maps" << endl;
		//Finalizando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &finish);

		// Calculando o tempo gasto
		timeSpent =  (finish.tv_sec - start.tv_sec);
		timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
		cout << "[TEMPO] Time spent: " << timeSpent << " seconds" << endl;
	}

	ListBpGraph S; // Grafo que contem os clientes os quais estamos aumentando as variaveis duais
	bpGraphCopy(g,S).run(); // copiando todas as informacoes de g para S
	

	if (DEBUG >= EXIBIR_GRAFO){
		// Percorrendo por todos os nós A - clientes
		cout << "Percorrendo por todos os clientes" << endl;
		for(ListBpGraph::RedNodeIt n(S); n != INVALID; ++n){
			cout << "no id: " << S.id(n)  << " - nome: " << nome[n] << " - v: " << v[n] << endl;
		}

		// Percorrendo por todos os nós B - instalacoes
		cout << "Percorrendo por todos as instalacoes" << endl;
		for(ListBpGraph::BlueNodeIt n(S); n != INVALID; ++n){
			cout << "no id: " << S.id(n)  << " - nome: " << nome[n] << " - f: " << f[n] << " - aberta: " << aberta[n] << " - qtdContribuintes: " << qtdContribuintes[n]<< endl;
		}


		// Percorrendo por todos os arcos
		cout << "Percorrendo por todos os arcos" << endl;
		for(ListBpGraph::EdgeIt e(S); e!= INVALID; ++e){
			cout << "arco id: " << S.id(e) ;
			cout << " - cliente: " << nome[S.u(e)] << " - instalacao: " << nome[S.v(e)];
			cout<< " - ca: " << custoAtribuicao[e] << " w - " << w[e] << " PCW - " << prontoContribuirW[e] << endl;
		}

		// Exibindo a matriz de adjacencia
		cout << "Exibindo matriz de adjacencia" << endl;
		for(int i=0;i<qtd_clientes;i++){
			for(int j=0;j<qtd_instalacoes;j++){
				cout<< matriz_adjacencia[i][j] << " ";
			}
			cout << endl;
		}
	}
	

	// v <- 0, w <- 0 ja acontece na inicializacao 


	if(DEBUG >= EXIBIR_TEMPO){
		cout <<"[TEMPO] Iniciando contagem de tempo para execucao parte inicial, antes do loop" << endl;
		//Iniciando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &start);
	}


	// Aresta auxiliar, inicia com o valor da primeira aresta
	ListBpGraph::Edge menor = arcos[0]; 

	// Percorrer todas as arestas para ver qual o menor cij
	for(ListBpGraph::EdgeIt e(S); e!= INVALID; ++e){
		if(custoAtribuicao[e] < custoAtribuicao[menor]){
			menor = e;
		}
	}

	if(DEBUG >= EXIBIR_ACOES){
		cout << "menor : " << S.id(menor) << " com custo: " << custoAtribuicao[menor] << endl;
	}

	// Percorrer todos os clientes para aumentar em todos esse valor
	for(ListBpGraph::RedNodeIt n(S); n != INVALID; ++n){
		v[n] += custoAtribuicao[menor];
	}


	int indice_inst;

	// Percorrer todas as arestas para ver quais bateram o custo de atribuicao
	// Entao, acionar a flag prontoContribuirW e colocar na matriz de adjacencia.
	for(ListBpGraph::EdgeIt e(S); e!= INVALID; ++e){
		if(igual(custoAtribuicao[e],custoAtribuicao[menor])){ 
			prontoContribuirW[e] = true;
			qtdContribuintes[S.asBlueNode(S.v(e))] += 1;

			// indice_inst = nome[S.v(e)] - qtd_clientes;
			// matriz_adjacencia[nome[S.u(e)]][indice_inst] = 1;
			
		}
	}


	if(DEBUG >= EXIBIR_TEMPO){
		cout <<"[TEMPO] Finalizando contagem de tempo para execucao parte inicial, antes do loop" << endl;
		//Finalizando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &finish);

		// Calculando o tempo gasto
		timeSpent =  (finish.tv_sec - start.tv_sec);
		timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
		cout << "[TEMPO] Time spent: " << timeSpent << " seconds" << endl;
	}


	if (DEBUG >= EXIBIR_GRAFO){
		// Percorrendo por todos os nós A - clientes
		cout << "Percorrendo por todos os clientes" << endl;
		for(ListBpGraph::RedNodeIt n(S); n != INVALID; ++n){
			cout << "no id: " << S.id(n)  << " - nome: " << nome[n] << " - v: " << v[n] << endl;
		}

		// Percorrendo por todos os nós B - instalacoes
		cout << "Percorrendo por todos as instalacoes" << endl;
		for(ListBpGraph::BlueNodeIt n(S); n != INVALID; ++n){
			cout << "no id: " << S.id(n)  << " - nome: " << nome[n] << " - f: " << f[n] << " - aberta: " << aberta[n] << " - qtdContribuintes: " << qtdContribuintes[n]<< endl;
		}


		// Percorrendo por todos os arcos
		cout << "Percorrendo por todos os arcos" << endl;
		for(ListBpGraph::EdgeIt e(S); e!= INVALID; ++e){
			cout << "arco id: " << S.id(e) ;
			cout << " - cliente: " << nome[S.u(e)] << " - instalacao: " << nome[S.v(e)];
			cout<< " - ca: " << custoAtribuicao[e] << " w - " << w[e] << " PCW - " << prontoContribuirW[e] << endl;
		}

		// Exibindo a matriz de adjacencia
		cout << "Exibindo matriz de adjacencia" << endl;
		for(int i=0;i<qtd_clientes;i++){
			for(int j=0;j<qtd_instalacoes;j++){
				cout<< matriz_adjacencia[i][j] << " ";
			}
			cout << endl;
		}
	}


	double menorAB; // receberá o menor valor entre o menor de A ou o menor de B

	double qtd_menorA;
	double qtd_atual;

	double qtd_menorB; 

	int contador_iteracoes = 0; // Variavel auxiliar, que, em caso de DEBUG >= EXIBIR_ACOES indica quantas iteracoes houveram no while

	// ****** A partir daqui deve estar em um loop até nao ter mais clientes ativos:

	while(qtd_clientes_ativos_S > 0){
		
		if(DEBUG >= EXIBIR_ACOES){
			cout << endl << "------------------------------ AINDA TEM " << qtd_clientes_ativos_S << " CLIENTES ATIVOS ------------------------------" << endl << endl;
		}


		if(DEBUG >= EXIBIR_TEMPO){
			contador_iteracoes += 1;
			cout <<"[TEMPO] Iniciando contagem de tempo para execucao do caso A, iteracao: " << contador_iteracoes << endl;
			//Iniciando a contagem do tempo
			clock_gettime(CLOCK_REALTIME, &start);
		}

		// 	A - fazer um for percorrendo todos os clientes e vendo o que resta de cij pra cada (achar o menor) (menor cij - vj)

		qtd_menorA = maiorCij + maiorFi; // inicia com o maior valor de cij + maior valor de fi dados na entrada

		for(ListBpGraph::RedNodeIt n(S); n != INVALID; ++n){		// percorre os clientes
	
			for (ListBpGraph::IncEdgeIt e(S, n); e != INVALID; ++e) { // Percorre todas arestas desse nó
				if(!prontoContribuirW[e]){
					qtd_atual = custoAtribuicao[e] - v[n];
					if(qtd_atual < qtd_menorA){
						qtd_menorA = qtd_atual;
					}
				}
			}
		}
		
		if(DEBUG >= EXIBIR_ACOES){
			cout << "temos a qtd menor na parte A: " << qtd_menorA << endl;
		}


		if(DEBUG >= EXIBIR_TEMPO){
			cout <<"[TEMPO] Finalizando contagem de tempo para execucao do caso A, iteracao: " << contador_iteracoes << endl;
			//Finalizando a contagem do tempo
			clock_gettime(CLOCK_REALTIME, &finish);

			// Calculando o tempo gasto
			timeSpent =  (finish.tv_sec - start.tv_sec);
			timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
			cout << "[TEMPO] Time spent: " << timeSpent << " seconds" << endl;
		


			cout <<"[TEMPO] Iniciando contagem de tempo para execucao do caso B, iteracao: " << contador_iteracoes << endl;
			//Iniciando a contagem do tempo
			clock_gettime(CLOCK_REALTIME, &start);
		}

		// B - fazer um for percorrendo todas as instalações para ver (fi - somatório de tudo que foi contribuído ate então para fi ) / numero de clientes prontos para contribuir para fi … os que ja alcançaram cij e ainda estão ativos

		qtd_menorB = maiorCij + maiorFi; // inicia com o maior valor de cij + maior valor de fi dados na entrada

		for(ListBpGraph::BlueNodeIt n(S); n != INVALID; ++n){ // percorre as instalacoes

			if(!aberta[n]){ // se a instalacao ainda nao estava aberta

				if(qtdContribuintes[n] > 0){ // SE alguem ja esta pronto para contribuir pelo menos

					qtd_atual = (f[n] - somatorioW[n])/qtdContribuintes[n]; //(fi - somatório de tudo que foi contribuído ate então para fi ) / (numero de clientes prontos para contribuir para fi) 

					if(qtd_atual < qtd_menorB){
						qtd_menorB = qtd_atual;
					}
				}
			}
		}
		
		if(DEBUG >= EXIBIR_ACOES){
			cout << "temos a qtd menor na parte B: " << qtd_menorB  << endl;
		}


		if(DEBUG >= EXIBIR_TEMPO){
			cout <<"[TEMPO] Finalizando contagem de tempo para execucao do caso B, iteracao: " << contador_iteracoes << endl;
			//Finalizando a contagem do tempo
			clock_gettime(CLOCK_REALTIME, &finish);

			// Calculando o tempo gasto
			timeSpent =  (finish.tv_sec - start.tv_sec);
			timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
			cout << "[TEMPO] Time spent: " << timeSpent << " seconds" << endl;
		}

		//	ver o menor entre A e B e aumentar esse valor em todos os clientes (tanto no cij se ainda faltar quanto no wij se ja estiver contribuindo)

		if(qtd_menorA < qtd_menorB){
			menorAB = qtd_menorA;
		}
		else { // inclui caso de empate
			menorAB = qtd_menorB;
		}

		if(DEBUG >= EXIBIR_TEMPO){
			cout <<"[TEMPO] Iniciando contagem de tempo para atualizar valores como v, w, somatorioW, prontoContribuirW, qtdContribuintes e matriz_adjacencia. Iteracao: " << contador_iteracoes << endl;
			//Iniciando a contagem do tempo
			clock_gettime(CLOCK_REALTIME, &start);
		}


		// Percorrer todos os clientes para aumentar em todos esse valor em v
		for(ListBpGraph::RedNodeIt n(S); n != INVALID; ++n){
			v[n] += menorAB;
		}


		// Percorrer todas as arestas para aumentar em w, caso o cliente estivesse pronto para contribuir
		// Aproveitar e ver quais bateram o custo de atribuicao
		// Entao, acionar a flag prontoContribuirW e colocar na matriz de adjacencia. 
		for(ListBpGraph::EdgeIt e(S); e!= INVALID; ++e){
			if(prontoContribuirW[e]){ // se está pronto para contribuir
				w[e] += menorAB; 
				somatorioW[S.asBlueNode(S.v(e))] += menorAB; // aumenta esse valor no somatorio da instalacao correspondente

				indice_inst = nome[S.v(e)] - qtd_clientes;
				matriz_adjacencia[nome[S.u(e)]][indice_inst] = 1; // atribui esse novo cliente em sua lista
			}
			else if(igual(custoAtribuicao[e],v[S.asRedNode(S.u(e))])){ // SENAO SE: acabou de ficar pronto para contribuir (pagou o c.a.)
				prontoContribuirW[e] = true;
				qtdContribuintes[S.asBlueNode(S.v(e))] += 1;
				// indice_inst = nome[S.v(e)] - qtd_clientes;
				// matriz_adjacencia[nome[S.u(e)]][indice_inst] = 1; // atribui esse novo cliente em sua lista
			}
		}

		if(DEBUG >= EXIBIR_TEMPO){
			cout <<"[TEMPO] Finalizando contagem de tempo para atualizar valores como v, w, somatorioW, prontoContribuirW, qtdContribuintes e matriz_adjacencia. Iteracao: " << contador_iteracoes << endl;
			//Finalizando a contagem do tempo
			clock_gettime(CLOCK_REALTIME, &finish);

			// Calculando o tempo gasto
			timeSpent =  (finish.tv_sec - start.tv_sec);
			timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
			cout << "[TEMPO] Time spent: " << timeSpent << " seconds" << endl;
		}

		// Repetir IF, tratar dos detalhes agora

		// SE FOR O CASO A ou empate: verificar se a instalação que aquele cliente alcançou ja estava aberta, se sim, remover ele dos ativos
		// if(qtd_menorA <= qtd_menorB){
		if(maior_igual(qtd_menorB,qtd_menorA)){

			if(DEBUG >= EXIBIR_TEMPO){
				cout <<"[TEMPO] Iniciando contagem de tempo para detalhes caso A. Iteracao: " << contador_iteracoes << endl;
				//Iniciando a contagem do tempo
				clock_gettime(CLOCK_REALTIME, &start);
			}


			if(DEBUG >= EXIBIR_ACOES){
				cout << "Caso A!" << endl;
			}

			for(ListBpGraph::EdgeIt e(S); e!= INVALID; ++e){ // percorrer todas as arestas
				if(prontoContribuirW[e]){ // verificar se o cliente j alcancou a inst i (se ta pronto para contribuir)
					if(aberta[S.asBlueNode(S.v(e))]){ // verificar se inst i ta aberta
						apagar_clientes.insert(S.id(S.u(e))); // Adicionar o cliente j no conjunto de futuros a apagar

						// Tirando ele dos contribuintes dessa instalacao
						prontoContribuirW[e] = false;
						qtdContribuintes[S.asBlueNode(S.v(e))] -= 1;

						if(DEBUG >= EXIBIR_ACOES){
							cout<<"removido cliente " << nome[S.u(e)] << " do pronto para contribuir da instalacao " << nome[S.v(e)] << endl;
						}

						//Apagando ele da lista de contribuintes das outras instalacoes
						for (ListBpGraph::IncEdgeIt e2(S, S.u(e)); e2 != INVALID; ++e2) { // Percorre todas arestas desse nó cliente (ligam a instalacoes)
							if(prontoContribuirW[e2]){ // se o cliente estava pronto para contribuir com essa instalacao
								prontoContribuirW[e2] = false;
								qtdContribuintes[S.asBlueNode(S.v(e2))] -= 1;

								if(DEBUG >= EXIBIR_ACOES){
									cout<<"removido cliente " << nome[S.u(e2)] << " do pronto para contribuir da instalacao " << nome[S.v(e2)] << endl;
								}
							}
						}
					}
				}
			}

			if(DEBUG >= EXIBIR_ACOES){
				cout << "No caso A, tamanho do conjunto apagar_clientes: " << apagar_clientes.size() << endl;
			}

			if(DEBUG >= EXIBIR_TEMPO){
				cout <<"[TEMPO] Finalizando contagem de tempo para detalhes caso A. Iteracao: " << contador_iteracoes << endl;
				//Finalizando a contagem do tempo
				clock_gettime(CLOCK_REALTIME, &finish);

				// Calculando o tempo gasto
				timeSpent =  (finish.tv_sec - start.tv_sec);
				timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
				cout << "[TEMPO] Time spent: " << timeSpent << " seconds" << endl;
			}

		}

		// SE FOR O CASO B ou empate: caso B seja o menor valor: abrir a instalação i e remover os seus contribuintes dos clientes ativos (lembrando de remover eles das listas de contribuintes das outras instalações)
		// if(qtd_menorA >= qtd_menorB){ 
		if(maior_igual(qtd_menorA,qtd_menorB)){ 

			if(DEBUG >= EXIBIR_TEMPO){
				cout <<"[TEMPO] Iniciando contagem de tempo para detalhes caso B. Iteracao: " << contador_iteracoes << endl;
				//Iniciando a contagem do tempo
				clock_gettime(CLOCK_REALTIME, &start);
			}


			if(DEBUG >= EXIBIR_ACOES){
				cout << "Caso B!" << endl;
			}

			for(ListBpGraph::BlueNodeIt n(S); n != INVALID; ++n){ // percorrer todas as instalacoes
				if(!aberta[n]){ // se a instalacao ainda nao estava aberta
					cout.precision(10);
					if(maior_igual(somatorioW[n],f[n])){ // se a soma das partes completou o custo de abrir a instalacao, vamos abrir!
						aberta[n] = true;
						qtd_inst_abertas += 1;

						if(DEBUG >= EXIBIR_ACOES){
							cout << "Instalacao " << nome[n] << " deve ser aberta!!!!" << endl;
							cout << "to aumentando aqui o qtd de insts abertas: " << qtd_inst_abertas << endl;
						}

						//Remover os seus contribuintes dos clientes ativos

						for (ListBpGraph::IncEdgeIt e(S, n); e != INVALID; ++e) { // Percorre todas arestas desse nó ( ligam a clientes )

							if(prontoContribuirW[e]){ // se esse cliente está pronto para contribuir


								apagar_clientes.insert(S.id(S.u(e))); // Adicionar o cliente j no conjunto de futuros a apagar

								if(DEBUG >= EXIBIR_ACOES){
									cout<<"************* vamos remover cliente " << nome[S.u(e)] << endl;
									cout << "No caso B, tamanho do conjunto apagar_clientes: " << apagar_clientes.size() << endl;
								}

								//Apagando ele da lista de contribuintes das outras instalacoes
								for (ListBpGraph::IncEdgeIt e2(S, S.u(e)); e2 != INVALID; ++e2) { // Percorre todas arestas desse nó cliente (ligam a instalacoes)
									if(prontoContribuirW[e2]){ // se o cliente estava pronto para contribuir com essa instalacao
										prontoContribuirW[e2] = false;
										qtdContribuintes[S.asBlueNode(S.v(e2))] -= 1;

										if(DEBUG >= EXIBIR_ACOES){
											cout<<"removido cliente " << nome[S.u(e2)] << " do pronto para contribuir da instalacao " << nome[S.v(e2)] << " agora ela tem qtdContribuintes = " << qtdContribuintes[S.asBlueNode(S.v(e2))] << endl;
										}
									}
								}
							}
						}
					}
				}
			}

			if(DEBUG >= EXIBIR_TEMPO){
				cout <<"[TEMPO] Finalizando contagem de tempo para detalhes caso B. Iteracao: " << contador_iteracoes << endl;
				//Finalizando a contagem do tempo
				clock_gettime(CLOCK_REALTIME, &finish);

				// Calculando o tempo gasto
				timeSpent =  (finish.tv_sec - start.tv_sec);
				timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
				cout << "[TEMPO] Time spent: " << timeSpent << " seconds" << endl;
			}

		}


		if(DEBUG >= EXIBIR_TEMPO){
			cout <<"[TEMPO] Iniciando contagem de tempo para apagar clientes congelados. Iteracao: " << contador_iteracoes << endl;
			//Iniciando a contagem do tempo
			clock_gettime(CLOCK_REALTIME, &start);
		}


		for (itr = apagar_clientes.begin(); itr != apagar_clientes.end(); ++itr) { // percorrer todos os elementos do conjunto 
			if(DEBUG >= EXIBIR_ACOES){
				cout <<"Instalacao foi aberta. Removendo o cliente " << *itr << " dos ativos "<< endl;
			}

	        S.erase(S.nodeFromId(*itr));  // apagando dos clientes ativos
			qtd_clientes_ativos_S -= 1;
	    }

	    //Apagar tudo do conjunto (limpar conjunto)
	    apagar_clientes.erase(apagar_clientes.begin(), apagar_clientes.end()); 


	    if(DEBUG >= EXIBIR_TEMPO){
			cout <<"[TEMPO] Finalizando contagem de tempo para apagar clientes congelados. Iteracao: " << contador_iteracoes << endl;
			//Finalizando a contagem do tempo
			clock_gettime(CLOCK_REALTIME, &finish);

			// Calculando o tempo gasto
			timeSpent =  (finish.tv_sec - start.tv_sec);
			timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
			cout << "[TEMPO] Time spent: " << timeSpent << " seconds" << endl;
		}


		if (DEBUG >= EXIBIR_GRAFO){
			// Percorrendo por todos os nós A - clientes
			cout << "Percorrendo por todos os clientes" << endl;
			for(ListBpGraph::RedNodeIt n(S); n != INVALID; ++n){
				cout << "no id: " << S.id(n)  << " - nome: " << nome[n] << " - v: " << v[n] << endl;
			}

			// Percorrendo por todos os nós B - instalacoes
			cout << "Percorrendo por todos as instalacoes" << endl;
			for(ListBpGraph::BlueNodeIt n(S); n != INVALID; ++n){
				cout << "no id: " << S.id(n)  << " - nome: " << nome[n] << " - f: " << f[n] << " - aberta: " << aberta[n] << " - qtdContribuintes: " << qtdContribuintes[n]<< endl;
			}

			// Percorrendo por todos os arcos
			cout << "Percorrendo por todos os arcos" << endl;
			for(ListBpGraph::EdgeIt e(S); e!= INVALID; ++e){
				cout << "arco id: " << S.id(e) ;
				cout << " - cliente: " << nome[S.u(e)] << " - instalacao: " << nome[S.v(e)];
				cout<< " - ca: " << custoAtribuicao[e] << " w - " << w[e] << " PCW - " << prontoContribuirW[e] << endl;
			}

			// Exibindo a matriz de adjacencia
			cout << "Exibindo matriz de adjacencia" << endl;
			for(int i=0;i<qtd_clientes;i++){
				for(int j=0;j<qtd_instalacoes;j++){
					cout<< matriz_adjacencia[i][j] << " ";
				}
				cout << endl;
			}
		}
	}

	/*

	CRIACAO DE Tlinha

	*/

	// grafo com as instalacoes abertas (no futuro quero que tenha os clientes conectados a suas instalacoes correspondentes)
	ListBpGraph Tlinha; 

	// Criação de nós de instalações em Tlinha
	ListBpGraph::BlueNode * instTlinha;
	instTlinha = (ListBpGraph::BlueNode*) malloc((qtd_instalacoes) * sizeof(ListBpGraph::BlueNode));
    if(!instTlinha){
        cout << "Memory Allocation Failed";
        exit(1);
    }

	int qtd_instTlinha = 0; // indica a quantidade de instalacoes ja em Tlinha

	// Para identificar cada nó
	ListBpGraph::NodeMap<int> nomeTlinha(Tlinha);

	// fTlinha - será o custo de instalação das insts em Tlinha (fi)
	ListBpGraph::BlueNodeMap<double> fTlinha(g);

	if(DEBUG >= EXIBIR_ACOES){
		cout << endl<< "Criando Tlinha" << endl;
	}

	indice_inst = 0;


	if(DEBUG >= EXIBIR_MATRIZ_ADJACENCIA){
		cout << "Exibindo matriz de adjacencia" << endl;
		for(int i=0;i<qtd_clientes;i++){
			for(int j=0;j<qtd_instalacoes;j++){
				cout<< matriz_adjacencia[i][j] << " ";
			}
			cout << endl;
		}
	}


	int * vetor;
	vetor = (int *) malloc(qtd_clientes * sizeof(int));

	int cont2 = 0;

	cout << "INSTALACOES ABERTAS: " << endl;

	for(ListBpGraph::BlueNodeIt n(S); n != INVALID; ++n){
		if(aberta[n]){
			cout << "no id: " << S.id(n)  << " - nome: " << nome[n] << " - aberta: " << aberta[n] << endl;
		}
	}

	cout << "LINHA: " << endl;
	for(int i=0;i<qtd_clientes;i++){
		if(matriz_adjacencia[i][49]){
			cout << i << " ";
			vetor[cont2++] = i;
		}
	}
	cout << endl;

	for(int i=0;i<cont2;i++){
		cout <<"cliente: " << vetor[i] << " - ";
		for(int j=0;j<qtd_instalacoes;j++){
			if(matriz_adjacencia[vetor[i]][j]){
				cout << j << " "; 
			}
		}
		cout << endl;
	}


	if(DEBUG >= EXIBIR_TEMPO){
		cout <<"[TEMPO] Iniciando contagem de tempo para modelar Tlinha" << endl;
		//Iniciando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &start);
	}

	// Enquanto houverem instalacoes abertas em S ( no algoritmo: while T != vazio )
	ListBpGraph::BlueNodeIt inst_atual(S);
	while(qtd_inst_abertas > 0){
		while(inst_atual != INVALID){
			if(aberta[inst_atual]){ 
				// Escolha inst aberta de S
				if(DEBUG >= EXIBIR_ACOES){
					cout << "Escolhido instalacao " << nome[inst_atual] << endl;
				}

				indice_inst = nome[inst_atual] - qtd_clientes;

				// Tlinha <- Tlinha U {i}
				instTlinha[qtd_instTlinha] = Tlinha.addBlueNode();
				nomeTlinha[instTlinha[qtd_instTlinha]] = nome[inst_atual]; // pega o nome da instalacao
				fTlinha[instTlinha[qtd_instTlinha]] = f[inst_atual]; // pega o custo de abrir a instalacao


				// indica em g que ele está em Tlinha
				estaEmTlinha[inst_atual] = true;

				qtd_instTlinha += 1;

				++inst_atual;
				break;
			}
			++inst_atual;
		}


		if (DEBUG >= EXIBIR_GRAFO){
			cout << "SOBRE GRAFO g" << endl;
			// Percorrendo por todos os nós A - clientes
			cout << "Percorrendo por todos os clientes" << endl;
			for(ListBpGraph::RedNodeIt n(g); n != INVALID; ++n){
				cout << "no id: " << g.id(n)  << " - nome: " << nome[n] << endl;
			}

			// Percorrendo por todos os nós B - instalacoes
			cout << "Percorrendo por todos as instalacoes" << endl;
			for(ListBpGraph::BlueNodeIt n(S); n != INVALID; ++n){
				cout << "no id: " << S.id(n)  << " - nome: " << nome[n] << " - f: " << f[n] << 
				" - aberta: " << aberta[n] << " - estaEmTlinhaL " << estaEmTlinha[n] << endl;
			}

			// Percorrendo por todos os arcos
			cout << "Percorrendo por todos os arcos" << endl;
			for(ListBpGraph::EdgeIt e(g); e!= INVALID; ++e){
				cout << "arco id: " << g.id(e) ;
				cout << " - cliente: " << nome[g.u(e)] << " - instalacao: " << nome[g.v(e)];
				cout<< " - ca: " << custoAtribuicao[e] << endl;
			}
		}


		// TODO: pensar em usar um conjunto aqui: para cada instalacao ter um conjunto dos clientes que contribuem e para cada cliente ter um conjunto de instalacoes elas contribuem
		// talvez eliminar o for mais interno com a adicao de um novo vetor, apenas para indicar se ele ainda esta em T ou nao. Dentro do for ao inves de zerar a coluna, muda o valor desse vetor pra false

		//	Remove todas instalacoes i se algum cliente j contribuir a i e indice_inst
		for(int i=0;i<qtd_clientes;i++){

			if(matriz_adjacencia[i][indice_inst]==1){ // se o cliente i contribui para a inst indice_inst			

				matriz_adjacencia[i][indice_inst] = 0; 

				for(int j=0;j<qtd_instalacoes;j++){ // ve todas as instalacoes que esse cliente contribui tambem e remove


					if(matriz_adjacencia[i][j]){ // nao tem perigo de pegar a inst inicial pois ja zeramos o seu valor correspondente

						// Limpando a matriz de adjacencia naquela coluna
						for(int k=0;k<qtd_clientes;k++){ // marcar que todas os clientes daquela instalacao agr nao contribuem mais pra ela
							matriz_adjacencia[k][j] = 0;

							if (DEBUG >= EXIBIR_MATRIZ_ADJACENCIA){

								// Exibindo a matriz de adjacencia
								cout << "Exibindo matriz de adjacencia" << endl;
								for(int i2=0;i2<qtd_clientes;i2++){
									for(int j2=0;j2<qtd_instalacoes;j2++){
										cout<< matriz_adjacencia[i2][j2] << " ";
									}
									cout << endl;
								}
							}

						}

						if(aberta[S.asBlueNode(S.nodeFromId(j + qtd_clientes))]){ // Se a instalacao j está aberta

							if(DEBUG >= EXIBIR_ACOES){
								cout << "Removendo a instalacao " << j << " das abertas, pois cliente " << i << " contribui a ela" << endl;
							}

							aberta[S.asBlueNode(S.nodeFromId(j + qtd_clientes))] = false;
							qtd_inst_abertas -= 1;
						}
					}
				}
			}
		}

		if(DEBUG >= EXIBIR_ACOES){
			cout << "Removendo a instalacao escolhida " << indice_inst + qtd_clientes << " das abertas" << endl;
		}

		aberta[S.asBlueNode(S.nodeFromId(indice_inst + qtd_clientes))] = false;
		qtd_inst_abertas -= 1;
	}


	if(DEBUG >= EXIBIR_TEMPO){
		cout <<"[TEMPO] Finalizando contagem de tempo para modelar Tlinha" << endl;
		//Finalizando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &finish);

		// Calculando o tempo gasto
		timeSpent =  (finish.tv_sec - start.tv_sec);
		timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
		cout << "[TEMPO] Time spent: " << timeSpent << " seconds" << endl;
	}



	if(DEBUG >= EXIBIR_MATRIZ_ADJACENCIA){
		cout << "Exibindo matriz de adjacencia" << endl;
		for(int i=0;i<qtd_clientes;i++){
			for(int j=0;j<qtd_instalacoes;j++){
				cout<< matriz_adjacencia[i][j] << " ";
			}
			cout << endl;
		}
	}

	// // Percorrendo por todos os nós de Tlinha que sao instalacoes
	int qtd_inst_Tlinha = 0;

	if(DEBUG >= EXIBIR_ACOES){

		cout<<"-- Tlinha instalacoes --" << endl;
		for(ListBpGraph::BlueNodeIt n(Tlinha); n != INVALID; ++n){
			qtd_inst_Tlinha += 1;
			cout << "no id: " << Tlinha.id(n) << " - nome: " << nomeTlinha[n] << endl;
		}
		cout<<"-------" << endl;
	}
	else if(DEBUG == 0){ // preciso fazer essa contagem de qualquer forma
		for(ListBpGraph::BlueNodeIt n(Tlinha); n != INVALID; ++n){
			qtd_inst_Tlinha += 1;
		}
	}


	if(DEBUG >= EXIBIR_TEMPO){
		cout <<"[TEMPO] Iniciando contagem de tempo para abrir tudo de Tlinha e associar os clientes" << endl;
		//Iniciando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &start);
	}

	// Abrir todas as instalacoes em Tlinha e atribuir cada cliente à instalacao mais próxima

	// Criação de nós de clientes em Tlinha
	ListBpGraph::RedNode * cliTlinha;
	cliTlinha = (ListBpGraph::RedNode*) malloc((qtd_clientes) * sizeof(ListBpGraph::RedNode));
    if(!cliTlinha){
        cout << "Memory Allocation Failed";
        exit(1);
    }


	int qtd_cliTlinha = 0; // indica a quantidade de clientes ja em Tlinha


	// caTlinha - em Tlinha indica o Custo de atribuição: é o custo de atribuir o cliente associado à instalação associada
	ListBpGraph::EdgeMap<double> caTlinha(g);

	// Criação de arcos e atribuição de seus labels. Obs: criando no tamanho maximo necessario, limitado pelo numero de inst ja em Tlinha
	ListBpGraph::Edge * arcosTlinha;
	arcosTlinha = (ListBpGraph::Edge*) malloc((qtd_clientes*qtd_inst_Tlinha) * sizeof(ListBpGraph::Edge));
    if(!arcosTlinha){
        cout << "Memory Allocation Failed";
        exit(1);
    }

	int qtd_arcosTlinha = 0; // indica a quantidade de arcos ja criados em Tlinha


	int menorDistancia = maiorCij;
	int nomeCliMenorDist = -1;
	int nomeInstMenorDist = -1;

	int idInstMenorDist = -1;


	//TODO: Talvez mudar aqui o jeito de fazer pra percorrer menos gente
	// Percorrer todos os clientes de g
	for(ListBpGraph::RedNodeIt n(g); n != INVALID; ++n){
		for (ListBpGraph::IncEdgeIt e(g, n); e != INVALID; ++e) { // Percorre todas arestas desse nó (ligam a instalacoes)
			if(estaEmTlinha[g.asBlueNode(g.v(e))]){ // se a instalacao correspondente está em Tlinha
				if(custoAtribuicao[e] <= menorDistancia){ // encontra a instalacao (que está em Tlinha) que possui a menor distancia
					menorDistancia = custoAtribuicao[e];
					nomeCliMenorDist = nome[g.u(e)];
					nomeInstMenorDist = nome[g.v(e)];
				}
			}
		}

		// Adiciona esse cliente em Tlinha
		cliTlinha[qtd_cliTlinha] = Tlinha.addRedNode();
		nomeTlinha[cliTlinha[qtd_cliTlinha]] = nomeCliMenorDist; // pega o nome do cliente
		qtd_cliTlinha += 1;


		//SOLUÇÃO: TALVEZ criar um vetor que dado o nome da inst, tem o valor do ID de Tlinha.. criar isso quando cria o Tlinha
		//PROBLEMA: Gambiarra 
		// Descobrir o ID em Tlinha da instalacao correspondente a nomeInstMenorDist
		for(int i=0;i<qtd_instTlinha;i++){
			if(nomeTlinha[instTlinha[i]] == nomeInstMenorDist){
				idInstMenorDist = i;
				break;
			}
		}

		// Cria aresta para associar esse cliente com a instalacao que possui a menor distancia dele
		arcos[qtd_arcosTlinha] = Tlinha.addEdge(cliTlinha[qtd_cliTlinha-1],instTlinha[idInstMenorDist]);
		caTlinha[arcos[qtd_arcosTlinha]] = menorDistancia; // pega o custo de atribuicao

		qtd_arcosTlinha += 1;

		// Zerando valores
		menorDistancia = maiorCij;
		nomeCliMenorDist = -1;
		nomeInstMenorDist = -1;
		idInstMenorDist = -1;
	}

	if(DEBUG >= EXIBIR_TEMPO){
		cout <<"[TEMPO] Finalizando contagem de tempo para abrir tudo de Tlinha e associar os clientes" << endl;
		//Finalizando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &finish);

		// Calculando o tempo gasto
		timeSpent =  (finish.tv_sec - start.tv_sec);
		timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
		cout << "[TEMPO] Time spent: " << timeSpent << " seconds" << endl;
	


		cout <<"[TEMPO] Iniciando contagem de tempo para exibir resposta final" << endl;
		//Iniciando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &start);
	}

	// Indica a soma dos custos de instalacoes + custos de atribuicao
	double gastoTotalFinal = 0;


	//Resposta final: Grafo Tlinha
	if(DEBUG >= EXIBIR_ACOES){
		cout << endl <<  "Resposta final: GRAFO TLINHA" << endl;

		// Percorrendo por todos os nós A - clientes
		cout << endl << "Percorrendo por todos os clientes" << endl;
		for(ListBpGraph::RedNodeIt n(Tlinha); n != INVALID; ++n){
			cout << "no id: " << Tlinha.id(n)  << " - nome: " << nomeTlinha[n] << endl;
		}
	}


	// Percorrendo por todos os nós B - instalacoes
	if(DEBUG >= EXIBIR_ACOES){
		cout << endl << "Percorrendo por todos as instalacoes" << endl;
	}

	for(ListBpGraph::BlueNodeIt n(Tlinha); n != INVALID; ++n){
		if(DEBUG >= EXIBIR_ACOES){
			cout << "no id: " << Tlinha.id(n)  << " - nome: " << nomeTlinha[n] << " - f: " << fTlinha[n] << endl;
		}

		gastoTotalFinal += fTlinha[n]; // acrescentando o valor do custo de abrir essa instalacao
	}


	// Percorrendo por todos os arcos
	if(DEBUG >= EXIBIR_ACOES){
		cout << endl << "Percorrendo por todos os arcos" << endl;
	}

	for(ListBpGraph::EdgeIt e(Tlinha); e!= INVALID; ++e){
		if(DEBUG >= EXIBIR_ACOES){
			cout << "arco id: " << Tlinha.id(e) ;
			cout << " - cliente: " << nomeTlinha[Tlinha.u(e)] << " - instalacao: " << nomeTlinha[Tlinha.v(e)];
			cout<< " - ca: " << caTlinha[e] << endl;
		}

		gastoTotalFinal += caTlinha[e]; // acrescentando o valor de atribuicao desse cliente a essa instalacao
	}

	cout << "Gasto total final: " << gastoTotalFinal << endl;

	if(DEBUG >= EXIBIR_TEMPO){
		cout <<"[TEMPO] Finalizando contagem de tempo para exibir resposta final" << endl;
		//Finalizando a contagem do tempo
		clock_gettime(CLOCK_REALTIME, &finish);

		// Calculando o tempo gasto
		timeSpent =  (finish.tv_sec - start.tv_sec);
		timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
		cout << "[TEMPO] Time spent: " << timeSpent << " seconds" << endl;
	

		// FINALIZANDO A CONTAGEM DE TEMPO DA FUNCAO COMO UM TODO
		clock_gettime(CLOCK_REALTIME, &real_finish);

		// Calculando o tempo gasto total
		realTimeSpent =  (real_finish.tv_sec - real_start.tv_sec);
		realTimeSpent += (real_finish.tv_nsec - real_start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
		cout << "Tempo total final da funcao: " << realTimeSpent << " segundos" << endl;
	}
	
	free(matriz_adjacencia);
	free(clientes);
	free(instalacoes);
	free(arcos);
	free(instTlinha);
	free(arcosTlinha);
	return(gastoTotalFinal);
}
