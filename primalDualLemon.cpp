#include <lemon/list_graph.h>
#include <algorithm>
// #include <lemon/lgf_reader.h>
// #include <lemon/lgf_writer.h>
#include <iostream>
using namespace lemon;
using namespace std;


#define qtd_clientes 10
#define qtd_instalacoes 10


// for (ListGraph::IncEdgeIt e(g, n); e != INVALID; ++e) {

int main(){


	/*

	ESTRUTURA QUE INDICA QUANTOS E QUAIS CLIENTES ESTAO CONTRIBUINDO (AUMENTANDO O W) 
	PARA UMA CERTA INSTALACAO

	- cada linha indica uma instalacao
	- a primeira coluna indica a quantidade N de clientes contribuindo
	- as próximas N colunas indicam o nome de cada cliente que está contribuindo


	obs: O ideal seria uma lista normal talvez

	*/
	int cli_contrib_inst[qtd_instalacoes][qtd_clientes + 1];


	
	// Grafo que liga os clientes às instalações
	ListGraph g;

	// Label do cliente será o quanto ele vai contribuir (v)
	// Label da instalação será o custo de instalação (fi)
	ListGraph::NodeMap<float> label(g);


	// Tipo: 0 para cliente 1 para instalação
	ListGraph::NodeMap<int> tipo(g);

	// Para identificar cada nó
	ListGraph::NodeMap<int> nome(g);


	// Criação de nós clientes e atribuição de seus labels
	ListGraph::Node clientes[qtd_clientes];

	for(int i=0;i<qtd_clientes;i++){
		clientes[i] = g.addNode();
		label[clientes[i]] = 0; // v = inicialmente nao contribui com nada
		tipo[clientes[i]] = 0; // indica que é cliente
		nome[clientes[i]] = i; // nomeia de acordo com a numeracao
	}


	// Criação de nós de instalações e atribuição de seus labels
	ListGraph::Node instalacoes[qtd_instalacoes];

	for(int i=0;i<qtd_instalacoes;i++){
		instalacoes[i] = g.addNode();
		label[instalacoes[i]] = 100 + 4+i/3.0; // fi = numero aleatorio
		tipo[instalacoes[i]] = 1; // indica que é instalação
		nome[instalacoes[i]] = qtd_clientes + i; // nomeia de acordo com a numeracao

		// Inicializando o cli_contrib_inst, primeira coluna com 0 todas outras com -1
		cli_contrib_inst[i][0] = 0;
		for(int j=1;j<=qtd_clientes;j++){
			cli_contrib_inst[i][j] = -1;
		}
	}


	// Custo de atribuição é o custo de atribuir o cliente associado à instalação associada
	ListGraph::EdgeMap<int> custoAtribuicao(g);

	// W é a parte do custo da instalação associada que o cliente associado vai contribuir
	ListGraph::EdgeMap<int> w(g);

	// Indica se aquele cliente ja está pagando todo seu custo de atribuicao àquela instalação, e esta pronto para começar a aumentar o w
	ListGraph::EdgeMap<bool> prontoContribuirW(g);

	// Criação de arcos e atribuição do custo e w
	ListGraph::Edge arcos[qtd_clientes*qtd_instalacoes]; // conectando todos com todos

	int cont = 0;
	for(int i=0;i<qtd_clientes;i++){
		for(int j=0;j<qtd_instalacoes;j++){
			arcos[cont] = g.addEdge(clientes[i],instalacoes[j]);
			custoAtribuicao[arcos[cont]] = 300 - i - j; // cij = numero aleatorio
			w[arcos[cont]] = 0; // inicialmente nao contribui com nada
			prontoContribuirW[arcos[cont]] = false; // inicia com falso em todos
			cont++;
		}
	}

	ListGraph S; // Grafo que contem os clientes os quais estamos aumentando as variaveis duais
	graphCopy(g,S).run();
	
	// Percorrendo por todos os nós
	for(ListGraph::NodeIt v(S); v != INVALID; ++v){
		cout << "no id: " << S.id(v) << " - tipo: " << tipo[v] << " - nome: " << nome[v] << " - label: " << label[v] << endl;
	}

	// Percorrendo por todos os arcos
	for(ListGraph::EdgeIt e(S); e!= INVALID; ++e){
		cout << "arco id: " << S.id(e) ;
		cout << " - cliente: " << S.id(S.v(e)) << " - instalacao: " << S.id(S.u(e));
		cout<< " - ca: " << custoAtribuicao[e] << " w - " << w[e] << " PCW - " << prontoContribuirW[e] << endl;
	}
	

	// v <- 0, w <- 0 ja acontece na inicializacao 



	// Aresta auxiliar, inicia com o valor da primeira aresta
	ListGraph::Edge menor = arcos[0]; 

	// Percorrer todas as arestas para ver qual o menor cij
	for(ListGraph::EdgeIt e(S); e!= INVALID; ++e){
		if(custoAtribuicao[e] < custoAtribuicao[menor]){
			menor = e;
		}
	}

	cout << "menor : " << S.id(menor) << " com custo: " << custoAtribuicao[menor] << endl;



	// Percorrer todos os clientes para aumentar em todos esse valor
	for(ListGraph::NodeIt v(S); v != INVALID; ++v){
		if (tipo[v]==0) { 			// se for cliente
			label[v] += custoAtribuicao[menor];
		}
	}


	int indice_inst;
	// Percorrer todas as arestas para ver quais bateram o custo de atribuicao
	// Entao, acionar a flag prontoContribuirW e colocar no cli_contrib_inst
	for(ListGraph::EdgeIt e(S); e!= INVALID; ++e){
		if(custoAtribuicao[e] == custoAtribuicao[menor]){
			prontoContribuirW[e] = true;
			indice_inst = nome[S.u(e)] - qtd_clientes;
			cli_contrib_inst[indice_inst][0] = 1;
			cli_contrib_inst[indice_inst][1] = nome[S.v(e)];
		}
	}


	// Percorrendo por todos os arcos
	for(ListGraph::EdgeIt e(S); e!= INVALID; ++e){
		cout << "arco id: " << S.id(e) ;
		cout << " - cliente: " << S.id(S.v(e)) << " - instalacao: " << S.id(S.u(e));
		cout<< " - ca: " << custoAtribuicao[e] << " w - " << w[e] << " PCW - " << prontoContribuirW[e] << endl;
	}


	// Percorrendo por todos os nós
	for(ListGraph::NodeIt v(S); v != INVALID; ++v){
		cout << "no id: " << S.id(v) << " - tipo: " << tipo[v] << " - nome: " << nome[v] << " - label: " << label[v] << endl;
	}

	

	// ****** A partir daqui deve estar em um loop até nao ter mais clientes ativos:


	

	// 	A - fazer um for percorrendo todos os clientes e vendo o que resta de cij pra cada (achar o menor) (menor cij - vj)

	float qtd_menorA = 10000; // PROBLEMA: seria bom pegar o valor da primeira, ao inves de 10000
	float qtd_atual;

	for(ListGraph::NodeIt v(S); v != INVALID; ++v){
		if (tipo[v]==0) { 			// se for cliente
	
			for (ListGraph::IncEdgeIt e(S, v); e != INVALID; ++e) { // Percorre todas arestas desse nó
				if(!prontoContribuirW[e]){
					qtd_atual = custoAtribuicao[e] - label[v];
					if(qtd_atual < qtd_menorA){
						qtd_menorA = qtd_atual;
					}
				}
			}
		}
	}

	cout << "temos a qtd menor na parte A: " << qtd_menorA << endl;



	// B - fazer um for percorrendo todas as instalações para ver (fi - somatório de tudo que foi contribuído ate então para fi ) / numero de clientes prontos para contribuir para fi … os que ja alcançaram cij e ainda estão ativos

	float qtd_menorB = 10000; // PROBLEMA: seria bom pegar o valor da primeira, ao inves de 10000

	for(ListGraph::NodeIt v(S); v != INVALID; ++v){
		if (tipo[v]==1) { 			// se for instalacao

			indice_inst = nome[v] - qtd_clientes;

			if(cli_contrib_inst[indice_inst][0]==0){
				qtd_atual = label[v]; // Se ninguem comecou a contribuir para ela ainda, entao a conta é o valor de fi mesmo
			}

			else{ // SENAO, SE alguem ja esta pronto para contribuir pelo menos

				qtd_atual = 0; 
				for (ListGraph::IncEdgeIt e(S, v); e != INVALID; ++e) { // Percorre todas arestas desse nó
					qtd_atual += w[e]; // vai indicar somatorio de todos w dessa instalacao
				}

				qtd_atual = (label[v] - qtd_atual)/cli_contrib_inst[indice_inst][0];

			}

			cout << "qtd atualL " << qtd_atual << endl;
			if(qtd_atual < qtd_menorB){
				qtd_menorB = qtd_atual;
			}
		}
	}

	cout << "temos a qtd menor na parte B: " << qtd_menorB << endl;


	// // SE FOR O CASO A
	// if(qtd_menorA < qtd_menorB){

	// }



	/*

	CRIACAO DE T

	*/

	// // grafo com as instalacoes que possuem desigualdade justa
	// ListGraph T; 
	// // f será o custo de instalação (fi)
	// ListGraph::NodeMap<float> f(T);
	// // Criação de nós de instalações em T
	// ListGraph::Node instT[qtd_instalacoes];
	// int qtd_instT = 0; // indica a quantidade de instalacoes ja em T

	// // Para identificar cada nó
	// ListGraph::NodeMap<int> nomeT(T);


	// // Percorrer todas as arestas para ver quais bateram o custo de atribuicao
	// // Entao, adicionar em T e tirando de g o cliente que bateu o custo de atribuicao
	// for(ListGraph::EdgeIt e(S); e!= INVALID; ++e){
	// 	if(custoAtribuicao[e] == custoAtribuicao[menor]){
	// 		instT[qtd_instT] = T.addNode();
	// 		f[instT[qtd_instT]] = label[S.u(e)]; // pega o valor da instalacao
	// 		nomeT[instT[qtd_instT]] = nome[S.u(e)]; // pega o nome da instalacao
	// 		qtd_instT += 1;
	// 		S.erase(S.v(e)); 
	// 	}
	// }

	// // // Percorrendo por todos os nós de T
	// for(ListGraph::NodeIt v(T); v != INVALID; ++v){
	// 	cout << "no id: " << T.id(v) << " - nome: " << nomeT[v] << " - f: " << f[v] << endl;
	// }



}