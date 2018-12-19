#include <lemon/list_graph.h>
#include <algorithm>
// #include <lemon/lgf_reader.h>
// #include <lemon/lgf_writer.h>
#include <iostream>
#include <set>
#include <iterator>
#define EPSL 0.001

using namespace lemon;
using namespace std;

#define qtd_clientes 10
#define qtd_instalacoes 10

#define EXIBIR_ACOES 1 // corresponde a todos os cout quando uma acao é feita
#define EXIBIR_GRAFO 2 // corresponde a descricao dos clientes, instalacoes e arcos
#define EXIBIR_MATRIZ_ADJACENCIA 3 // corresponde à parte final, na criacao de Tlinha

// Documentacao grafo bipartido: http://lemon.cs.elte.hu/pub/doc/latest-svn/a00352.html#004394d647a7b4b4016097ba61413b50

int main(){

	int debug = 0; // OPCOES DE DEBUG: 1 PARA EXIBIR ACOES, 2 PARA EXIBIR AS MUDANÇAS NO GRAFO, 3 PARA EXIBIR AS MUDANCAS NA MATRIZ DE ADJACENCIA NA CRIACAO DE TLINHA

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
	int matriz_adjacencia[qtd_clientes][qtd_instalacoes]; // indica se o cliente alcançou a instalação


	int qtd_clientes_ativos_S = qtd_clientes; // Indica a quantidade de clientes ainda em S, os clientes ativos.
	

	/* Sobre os grafos bipartidos, teremos
	
	Red para clientes

	Blue para instalações

	*/
	// Grafo que liga os clientes às instalações
	ListBpGraph g;

	// v - será o quanto o cliente vai contribuir (v)
	ListBpGraph::RedNodeMap<float> v(g);

	// f - será o custo de instalação (fi)
	ListBpGraph::BlueNodeMap<float> f(g);

	// somatorioW - indica o quanto ja foi pago do custo de abrir a instalacao i... somatorio de todos os w correspondente a essa i
	ListBpGraph::BlueNodeMap<float> somatorioW(g);

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

	int qtd_inst_abertas = 0; // indica a quantidade de instalacoes abertas



	// Criação de nós clientes e atribuição de seus labels
	ListBpGraph::RedNode clientes[qtd_clientes];

	for(int i=0;i<qtd_clientes;i++){
		clientes[i] = g.addRedNode();
		v[clientes[i]] = 0; // v = inicialmente nao contribui com nada
		nome[clientes[i]] = i; // nomeia de acordo com a numeracao
	}



	// Criação de nós de instalações e atribuição de seus labels
	ListBpGraph::BlueNode instalacoes[qtd_instalacoes];


	for(int i=0;i<qtd_instalacoes;i++){
		instalacoes[i] = g.addBlueNode();
		f[instalacoes[i]] = 100 + 4+i/3.0; // fi = numero aleatorio
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


	// Inicializando a matriz de adjacencia, todos com valor 0
	for(int i=0;i<qtd_clientes;i++){
		for(int j=0;j<qtd_instalacoes;j++){
			matriz_adjacencia[i][j] = 0;
		}
	}



	// Custo de atribuição é o custo de atribuir o cliente associado à instalação associada
	ListBpGraph::EdgeMap<float> custoAtribuicao(g);

	// W é a parte do custo da instalação associada que o cliente associado vai contribuir
	ListBpGraph::EdgeMap<float> w(g);

	// Indica se aquele cliente ja está pagando todo seu custo de atribuicao àquela instalação, e esta pronto para começar a aumentar o w
	ListBpGraph::EdgeMap<bool> prontoContribuirW(g);


	// Criação de arcos e atribuição de seus labels
	ListBpGraph::Edge arcos[qtd_clientes*qtd_instalacoes]; // conectando todos com todos

	int cont = 0;
	for(int i=0;i<qtd_clientes;i++){
		for(int j=0;j<qtd_instalacoes;j++){
			arcos[cont] = g.addEdge(clientes[i],instalacoes[j]);
			custoAtribuicao[arcos[cont]] = 300 - i - j; // cij = numero aleatorio
			w[arcos[cont]] = 0; // inicialmente nao contribui com nada
			prontoContribuirW[arcos[cont]] = false; // inicia com falso em todos

			// Salvando o valor do maior Cij da entrada
			if(custoAtribuicao[arcos[cont]]>maiorCij){
				maiorCij = custoAtribuicao[arcos[cont]];
			}

			cont++;
		}
	}



	ListBpGraph S; // Grafo que contem os clientes os quais estamos aumentando as variaveis duais
	bpGraphCopy(g,S).run(); // copiando todas as informacoes de g para S
	

	if (debug >= EXIBIR_GRAFO){
		// Percorrendo por todos os nós A - clientes
		cout << "Percorrendo por todos os clientes" << endl;
		for(ListBpGraph::RedNodeIt n(S); n != INVALID; ++n){
			cout << "no id: " << S.id(n)  << " - nome: " << nome[n] << " - v: " << v[n] << endl;
		}

		// Percorrendo por todos os nós B - instalacoes
		cout << "Percorrendo por todos as instalacoes" << endl;
		for(ListBpGraph::BlueNodeIt n(S); n != INVALID; ++n){
			cout << "no id: " << S.id(n)  << " - nome: " << nome[n] << " - f: " << f[n] << " - aberta: " << aberta[n] << endl;
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



	// Aresta auxiliar, inicia com o valor da primeira aresta
	ListBpGraph::Edge menor = arcos[0]; 

	// Percorrer todas as arestas para ver qual o menor cij
	for(ListBpGraph::EdgeIt e(S); e!= INVALID; ++e){
		if(custoAtribuicao[e] < custoAtribuicao[menor]){
			menor = e;
		}
	}

	if(debug >= EXIBIR_ACOES){
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
		if(custoAtribuicao[e] == custoAtribuicao[menor]){
			prontoContribuirW[e] = true;
			qtdContribuintes[S.asBlueNode(S.v(e))] += 1;

			indice_inst = nome[S.v(e)] - qtd_clientes;
			matriz_adjacencia[nome[S.u(e)]][indice_inst] = 1;
			
			//verificar se alguma instalacao ja estaria aberta (se ela tem custo zero)
			if(f[S.asBlueNode(S.v(e))] == 0){

				if(aberta[S.asBlueNode(S.v(e))]){ // se ela ja estivesse aberta (abriu em uma iteracao anterior)
					apagar_clientes.insert(S.id(S.u(e))); // Adicionar o cliente j no conjunto de futuros a apagar	
				}

				else{ // se aquela instalacao nao estava aberta ainda
					
					aberta[S.asBlueNode(S.v(e))] = true;
					qtd_inst_abertas += 1;

					if(debug >= EXIBIR_ACOES){
						cout << "Instalacao " << nome[S.v(e)] << " deve ser aberta!!!!" << endl;
						cout << "to aumentando aqui o qtd de insts abertas: " << qtd_inst_abertas << endl;
					}

					//Remover os seus contribuintes dos clientes ativos

					indice_inst = nome[S.v(e)] - qtd_clientes;

					for (ListBpGraph::IncEdgeIt e1(S, S.v(e)); e1 != INVALID; ++e1) { // Percorre todas arestas desse nó ( ligam a clientes )

						if(prontoContribuirW[e1]){ // se esse cliente está pronto para contribuir
							
							apagar_clientes.insert(S.id(S.u(e1))); // Adicionar o cliente j no conjunto de futuros a apagar

							if(debug >= EXIBIR_ACOES){
								cout<<"************* vamos remover cliente " << nome[S.u(e1)] << endl;
								cout << "No inicio, tamanho do conjunto apagar_clientes: " << apagar_clientes.size() << endl;
							}

							//Apagando ele da lista de contribuintes das outras instalacoes
							for (ListBpGraph::IncEdgeIt e2(S, S.u(e1)); e2 != INVALID; ++e2) { // Percorre todas arestas desse nó cliente (ligam a instalacoes)
								if(prontoContribuirW[e2]){ // se o cliente estava pronto para contribuir com essa instalacao
									prontoContribuirW[e2] = false;
									qtdContribuintes[S.asBlueNode(S.v(e2))] -= 1;

									if(debug >= EXIBIR_ACOES){
										cout<<"removido cliente " << nome[S.u(e2)] << " do pronto para contribuir da instalacao " << nome[S.v(e2)] << endl;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// Apagar os clientes que deixaram de ser ativos logo agora no começo (que alcançaram uma inst de custo fi = 0)
	for (itr = apagar_clientes.begin(); itr != apagar_clientes.end(); ++itr) { // percorrer todos os elementos do conjunto 
		if(debug >= EXIBIR_ACOES){
			cout <<"Instalacao foi aberta. Removendo o cliente " << *itr << " dos ativos "<< endl;
		}
        S.erase(S.nodeFromId(*itr));  // apagando dos clientes ativos

		qtd_clientes_ativos_S -= 1;
    }

    //Apagar tudo do conjunto (limpar conjunto)
    apagar_clientes.erase(apagar_clientes.begin(), apagar_clientes.end()); 


	if (debug >= EXIBIR_GRAFO){
		// Percorrendo por todos os nós A - clientes
		cout << "Percorrendo por todos os clientes" << endl;
		for(ListBpGraph::RedNodeIt n(S); n != INVALID; ++n){
			cout << "no id: " << S.id(n)  << " - nome: " << nome[n] << " - v: " << v[n] << endl;
		}

		// Percorrendo por todos os nós B - instalacoes
		cout << "Percorrendo por todos as instalacoes" << endl;
		for(ListBpGraph::BlueNodeIt n(S); n != INVALID; ++n){
			cout << "no id: " << S.id(n)  << " - nome: " << nome[n] << " - f: " << f[n] << " - aberta: " << aberta[n] << endl;
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

	float menorAB; // receberá o menor valor entre o menor de A ou o menor de B

	float qtd_menorA;
	float qtd_atual;

	float qtd_menorB; 

	// ****** A partir daqui deve estar em um loop até nao ter mais clientes ativos:

	while(qtd_clientes_ativos_S > 0){
		
		if(debug >= EXIBIR_ACOES){
			cout << endl << "------------------------------ AINDA TEM " << qtd_clientes_ativos_S << " CLIENTES ATIVOS ------------------------------" << endl << endl;
		}

		// 	A - fazer um for percorrendo todos os clientes e vendo o que resta de cij pra cada (achar o menor) (menor cij - vj)

		qtd_menorA = maiorCij; // inicia com o maior valor de cij dado na entrada

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
		
		if(debug >= EXIBIR_ACOES){
			cout << "temos a qtd menor na parte A: " << qtd_menorA << endl;
		}


		// B - fazer um for percorrendo todas as instalações para ver (fi - somatório de tudo que foi contribuído ate então para fi ) / numero de clientes prontos para contribuir para fi … os que ja alcançaram cij e ainda estão ativos

		qtd_menorB = maiorFi; // inicia com o maior valor de fi dado na entrada

		for(ListBpGraph::BlueNodeIt n(S); n != INVALID; ++n){ // percorre as instalacoes

			indice_inst = nome[n] - qtd_clientes;

			if(qtdContribuintes[n] > 0){ // SE alguem ja esta pronto para contribuir pelo menos

				qtd_atual = (f[n] - somatorioW[n])/qtdContribuintes[n]; //(fi - somatório de tudo que foi contribuído ate então para fi ) / (numero de clientes prontos para contribuir para fi) 

				if(qtd_atual < qtd_menorB){
					qtd_menorB = qtd_atual;
				}
			}
		}
		
		if(debug >= EXIBIR_ACOES){
			cout << "temos a qtd menor na parte B: " << qtd_menorB  << endl;
		}


		//	ver o menor entre A e B e aumentar esse valor em todos os clientes (tanto no cij se ainda faltar quanto no wij se ja estiver contribuindo)

		if(qtd_menorA < qtd_menorB){
			menorAB = qtd_menorA;
		}
		else if(qtd_menorB <= qtd_menorA){ // inclui caso de empate
			menorAB = qtd_menorB;
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
			}
			else if(custoAtribuicao[e] == v[S.asRedNode(S.u(e))]){ // SENAO SE: acabou de ficar pronto para contribuir (pagou o c.a.)
				prontoContribuirW[e] = true;
				qtdContribuintes[S.asBlueNode(S.v(e))] += 1;
				indice_inst = nome[S.v(e)] - qtd_clientes;
				matriz_adjacencia[nome[S.u(e)]][indice_inst] = 1; // atribui esse novo cliente em sua lista
			}
		}

		// Repetir IF, tratar dos detalhes agora

		// SE FOR O CASO A ou empate: verificar se a instalação que aquele cliente alcançou ja estava aberta, se sim, remover ele dos ativos
		if(qtd_menorA <= qtd_menorB){
			if(debug >= EXIBIR_ACOES){
				cout << "Caso A!" << endl;
			}

			for(ListBpGraph::EdgeIt e(S); e!= INVALID; ++e){ // percorrer todas as arestas
				if(prontoContribuirW[e]){ // verificar se o cliente j alcancou a inst i (se ta pronto para contribuir)
					if(aberta[S.asBlueNode(S.v(e))]){ // verificar se inst i ta aberta
						apagar_clientes.insert(S.id(S.u(e))); // Adicionar o cliente j no conjunto de futuros a apagar
					}
				}
			}

			if(debug >= EXIBIR_ACOES){
				cout << "No caso A, tamanho do conjunto apagar_clientes: " << apagar_clientes.size() << endl;
			}

			for (itr = apagar_clientes.begin(); itr != apagar_clientes.end(); ++itr) { // percorrer todos os elementos do conjunto 
				if(debug >= EXIBIR_ACOES){
					cout <<"Instalacao ja estava aberta. Removendo o cliente " << *itr << " dos ativos "<< endl;
				}

		        S.erase(S.nodeFromId(*itr));  // apagando dos clientes ativos
				qtd_clientes_ativos_S -= 1;
		    }

		    //Apagar tudo do conjunto (limpar conjunto)
		    apagar_clientes.erase(apagar_clientes.begin(), apagar_clientes.end()); 

		}

		// SE FOR O CASO B ou empate: caso B seja o menor valor: abrir a instalação i e remover os seus contribuintes dos clientes ativos (lembrando de remover eles das listas de contribuintes das outras instalações
		if(qtd_menorA >= qtd_menorB){ 
			if(debug >= EXIBIR_ACOES){
				cout << "Caso B!" << endl;
			}

			for(ListBpGraph::BlueNodeIt n(S); n != INVALID; ++n){ // percorrer todas as instalacoes
				if(!aberta[n]){ // se a instalacao ainda nao estava aberta
					if((somatorioW[n] > f[n] - EPSL)&&(somatorioW[n] < f[n] + EPSL)){ // se a soma das partes completou o custo de abrir a instalacao, vamos abrir! (isso é a msm coisa q somatorioW[n] == f[n] mas garante erros minusculos)
						aberta[n] = true;
						qtd_inst_abertas += 1;

						if(debug >= EXIBIR_ACOES){
							cout << "Instalacao " << nome[n] << " deve ser aberta!!!!" << endl;
							cout << "to aumentando aqui o qtd de insts abertas: " << qtd_inst_abertas << endl;
						}

						//Remover os seus contribuintes dos clientes ativos

						indice_inst = nome[n] - qtd_clientes;

						for (ListBpGraph::IncEdgeIt e(S, n); e != INVALID; ++e) { // Percorre todas arestas desse nó ( ligam a clientes )

							if(prontoContribuirW[e]){ // se esse cliente está pronto para contribuir


								apagar_clientes.insert(S.id(S.u(e))); // Adicionar o cliente j no conjunto de futuros a apagar

								if(debug >= EXIBIR_ACOES){
									cout<<"************* vamos remover cliente " << nome[S.u(e)] << endl;
									cout << "No caso B, tamanho do conjunto apagar_clientes: " << apagar_clientes.size() << endl;
								}

								//Apagando ele da lista de contribuintes das outras instalacoes
								for (ListBpGraph::IncEdgeIt e2(S, S.u(e)); e2 != INVALID; ++e2) { // Percorre todas arestas desse nó cliente (ligam a instalacoes)
									if(prontoContribuirW[e2]){ // se o cliente estava pronto para contribuir com essa instalacao
										prontoContribuirW[e2] = false;
										qtdContribuintes[S.asBlueNode(S.v(e2))] -= 1;

										if(debug >= EXIBIR_ACOES){
											cout<<"removido cliente " << nome[S.u(e2)] << " do pronto para contribuir da instalacao " << nome[S.v(e2)] << endl;
										}
									}
								}
							}
						}
					}
				
					for (itr = apagar_clientes.begin(); itr != apagar_clientes.end(); ++itr) { // percorrer todos os elementos do conjunto 
						if(debug >= EXIBIR_ACOES){
							cout <<"Instalacao foi aberta. Removendo o cliente " << *itr << " dos ativos "<< endl;
						}

				        S.erase(S.nodeFromId(*itr));  // apagando dos clientes ativos
						qtd_clientes_ativos_S -= 1;
				    }

				    //Apagar tudo do conjunto (limpar conjunto)
				    apagar_clientes.erase(apagar_clientes.begin(), apagar_clientes.end()); 
				}
			}
		}


	if (debug >= EXIBIR_GRAFO){
			// Percorrendo por todos os nós A - clientes
			cout << "Percorrendo por todos os clientes" << endl;
			for(ListBpGraph::RedNodeIt n(S); n != INVALID; ++n){
				cout << "no id: " << S.id(n)  << " - nome: " << nome[n] << " - v: " << v[n] << endl;
			}

			// Percorrendo por todos os nós B - instalacoes
			cout << "Percorrendo por todos as instalacoes" << endl;
			for(ListBpGraph::BlueNodeIt n(S); n != INVALID; ++n){
				cout << "no id: " << S.id(n)  << " - nome: " << nome[n] << " - f: " << f[n] << " - aberta: " << aberta[n] << endl;
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
	ListBpGraph::BlueNode instTlinha[qtd_instalacoes];

	int qtd_instTlinha = 0; // indica a quantidade de instalacoes ja em Tlinha

	// Para identificar cada nó
	ListBpGraph::NodeMap<int> nomeTlinha(Tlinha);

	// fTlinha - será o custo de instalação das insts em Tlinha (fi)
	ListBpGraph::BlueNodeMap<float> fTlinha(g);

	if(debug >= EXIBIR_ACOES){
		cout << endl<< "Criando Tlinha" << endl;
	}

	indice_inst = 0;

	if(debug >= EXIBIR_MATRIZ_ADJACENCIA){
		cout << "Exibindo matriz de adjacencia" << endl;
		for(int i=0;i<qtd_clientes;i++){
			for(int j=0;j<qtd_instalacoes;j++){
				cout<< matriz_adjacencia[i][j] << " ";
			}
			cout << endl;
		}
	}

	// Enquanto houverem instalacoes abertas em S ( no algoritmo: while T != vazio )
	while(qtd_inst_abertas > 0){

		for(ListBpGraph::BlueNodeIt n(S); n != INVALID; ++n){
			if(aberta[n]){ 
				// Escolha inst aberta de S
				if(debug >= EXIBIR_ACOES){
					cout << "Escolhido instalacao " << nome[n] << endl;
				}

				indice_inst = nome[n] - qtd_clientes;

				// Tlinha <- Tlinha U {i}
				instTlinha[qtd_instTlinha] = Tlinha.addBlueNode();
				nomeTlinha[instTlinha[qtd_instTlinha]] = nome[n]; // pega o nome da instalacao
				fTlinha[instTlinha[qtd_instTlinha]] = f[n]; // pega o custo de abrir a instalacao


				// indica em g que ele está em Tlinha
				estaEmTlinha[n] = true;

				qtd_instTlinha += 1;

				break;
			}
		}


		if (debug >= EXIBIR_GRAFO){
			cout << "SOBRE GRAFO g" << endl;
			// Percorrendo por todos os nós A - clientes
			cout << "Percorrendo por todos os clientes" << endl;
			for(ListBpGraph::RedNodeIt n(g); n != INVALID; ++n){
				cout << "no id: " << g.id(n)  << " - nome: " << nome[n] << endl;
			}

			// Percorrendo por todos os nós B - instalacoes
			cout << "Percorrendo por todos as instalacoes" << endl;
			for(ListBpGraph::BlueNodeIt n(g); n != INVALID; ++n){
				cout << "no id: " << g.id(n)  << " - nome: " << nome[n] << " - f: " << f[n] << 
				" - aberta: " << aberta[n] << " - estaEmTlinhaL " << estaEmTlinha[n] << endl;
			}

			// Percorrendo por todos os arcos
			cout << "Percorrendo por todos os arcos" << endl;
			for(ListBpGraph::EdgeIt e(S); e!= INVALID; ++e){
				cout << "arco id: " << g.id(e) ;
				cout << " - cliente: " << nome[g.u(e)] << " - instalacao: " << nome[g.v(e)];
				cout<< " - ca: " << custoAtribuicao[e] << endl;
			}
		}


		//	Remove todas instalacoes i se algum cliente j contribuir a i e indice_inst
		for(int i=0;i<qtd_clientes;i++){

			if(matriz_adjacencia[i][indice_inst]==1){ // se o cliente i contribui para a inst indice_inst			

				matriz_adjacencia[i][indice_inst] = 0; 

				for(int j=0;j<qtd_instalacoes;j++){ // ve todas as instalacoes que esse cliente contribui tambem e remove


					if(matriz_adjacencia[i][j]){ // nao tem perigo de pegar a inst inicial pois ja zeramos o seu valor correspondente

						// Limpando a matriz de adjacencia naquela coluna
						for(int k=0;k<qtd_clientes;k++){ // marcar que todas os clientes daquela instalacao agr nao contribuem mais pra ela
							matriz_adjacencia[k][j] = 0;

							if (debug >= EXIBIR_MATRIZ_ADJACENCIA){

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


						if(aberta[S.asBlueNode(S.nodeFromId(j+qtd_clientes))]){ // Se a instalacao j está aberta
							if(debug >= EXIBIR_ACOES){
								cout << "Removendo a instalacao " << j << " das abertas, pois cliente " << i << " contribui a ela" << endl;
							}

							aberta[S.asBlueNode(S.nodeFromId(j))] = false;
							qtd_inst_abertas -= 1;
						}
					}
				}
			}
		}

		if(debug >= EXIBIR_ACOES){
			cout << "Removendo a instalacao escolhida " << indice_inst + qtd_clientes << " das abertas" << endl;
		}

		aberta[S.asBlueNode(S.nodeFromId(indice_inst))] = false;
		qtd_inst_abertas -= 1;

	}



	if(debug >= EXIBIR_MATRIZ_ADJACENCIA){
		cout << "Exibindo matriz de adjacencia" << endl;
		for(int i=0;i<qtd_clientes;i++){
			for(int j=0;j<qtd_instalacoes;j++){
				cout<< matriz_adjacencia[i][j] << " ";
			}
			cout << endl;
		}
	}

	// // Percorrendo por todos os nós de Tlinha que sao instalacoes
	if(debug >= EXIBIR_ACOES){

		cout<<"-- Tlinha instalacoes --" << endl;
		for(ListBpGraph::BlueNodeIt n(Tlinha); n != INVALID; ++n){
			cout << "no id: " << Tlinha.id(n) << " - nome: " << nomeTlinha[n] << endl;
		}
		cout<<"-------" << endl;
	}



	// Abrir todas as instalacoes em Tlinha e atribuir cada cliente à instalacao mais próxima


	// Criação de nós de clientes em Tlinha
	ListBpGraph::RedNode cliTlinha[qtd_clientes];

	int qtd_cliTlinha = 0; // indica a quantidade de clientes ja em Tlinha


	// caTlinha - em Tlinha indica o Custo de atribuição: é o custo de atribuir o cliente associado à instalação associada
	ListBpGraph::EdgeMap<float> caTlinha(g);


	// Criação de arcos e atribuição de seus labels
	ListBpGraph::Edge arcosTlinha[qtd_clientes*qtd_instalacoes];

	int qtd_arcosTlinha = 0; // indica a quantidade de arcos ja criados em Tlinha


	int menorDistancia = maiorCij;
	int nomeCliMenorDist = -1;
	int nomeInstMenorDist = -1;

	int idInstMenorDist = -1;

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


	// Indica a soma dos custos de instalacoes + custos de atribuicao
	float gastoTotalFinal = 0;

	// Resposta final: Grafo Tlinha
	cout << endl <<  "Resposta final: GRAFO TLINHA" << endl;

	// Percorrendo por todos os nós A - clientes
	cout << endl << "Percorrendo por todos os clientes" << endl;
	for(ListBpGraph::RedNodeIt n(Tlinha); n != INVALID; ++n){
		cout << "no id: " << Tlinha.id(n)  << " - nome: " << nomeTlinha[n] << endl;
	}

	// Percorrendo por todos os nós B - instalacoes
	cout << endl << "Percorrendo por todos as instalacoes" << endl;
	for(ListBpGraph::BlueNodeIt n(Tlinha); n != INVALID; ++n){
		cout << "no id: " << Tlinha.id(n)  << " - nome: " << nomeTlinha[n] << " - f: " << fTlinha[n] << endl;
		gastoTotalFinal += fTlinha[n]; // acrescentando o valor do custo de abrir essa instalacao
	}


	// Percorrendo por todos os arcos
	cout << endl << "Percorrendo por todos os arcos" << endl;
	for(ListBpGraph::EdgeIt e(Tlinha); e!= INVALID; ++e){
		cout << "arco id: " << Tlinha.id(e) ;
		cout << " - cliente: " << nomeTlinha[Tlinha.u(e)] << " - instalacao: " << nomeTlinha[Tlinha.v(e)];
		cout<< " - ca: " << caTlinha[e] << endl;
		gastoTotalFinal += caTlinha[e]; // acrescentando o valor de atribuicao desse cliente a essa instalacao
	}

	cout << endl << "Gasto total final: " << gastoTotalFinal << endl << endl;

}