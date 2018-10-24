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

	
	// Grafo que liga os clientes às instalações
	ListGraph g;


	// Label do cliente será o quanto ele vai contribuir (v)
	// Label da instalação será o custo de instalação (fi)
	ListGraph::NodeMap<float> label(g);


	// Tipo: 0 para cliente 1 para instalação
	ListGraph::NodeMap<int> tipo(g);


	// Criação de nós clientes e atribuição de seus labels
	ListGraph::Node clientes[qtd_clientes];

	for(int i=0;i<qtd_clientes;i++){
		clientes[i] = g.addNode();
		label[clientes[i]] = 0; // v = inicialmente nao contribui com nada
		tipo[clientes[i]] = 0; // indica que é cliente
	}


	// Criação de nós de instalações e atribuição de seus labels
	ListGraph::Node instalacoes[qtd_instalacoes];

	for(int i=0;i<qtd_instalacoes;i++){
		instalacoes[i] = g.addNode();
		label[instalacoes[i]] = 4+i/3.0; // fi = numero aleatorio
		tipo[instalacoes[i]] = 1; // indica que é instalação
	}


	// Custo de atribuição é o custo de atribuir o cliente associado à instalação associada
	ListGraph::EdgeMap<int> custoAtribuicao(g);

	// W é a parte do custo da instalação associada que o cliente associado vai contribuir
	ListGraph::EdgeMap<int> w(g);

	// Criação de arcos e atribuição do custo e w
	ListGraph::Edge arcos[qtd_clientes*qtd_instalacoes]; // conectando todos com todos

	int cont = 0;
	for(int i=0;i<qtd_clientes;i++){
		for(int j=0;j<qtd_instalacoes;j++){
			arcos[cont] = g.addEdge(clientes[i],instalacoes[j]);
			custoAtribuicao[arcos[cont]] = 300 - i - j; // cij = numero aleatorio
			w[arcos[cont]] = 0; // inicialmente nao contribui com nada
			cont++;
		}
	}

	
	// Percorrendo por todos os nós
	for(ListGraph::NodeIt v(g); v != INVALID; ++v){
		cout << "no id: " << g.id(v) << " - tipo: " << tipo[v] << " - label: " << label[v] << endl;
	}

	// Percorrendo por todos os arcos
	for(ListGraph::EdgeIt e(g); e!= INVALID; ++e){
		cout << "arco id: " << g.id(e) ;
		cout << " - target: " << g.id(g.v(e)) << " - source: " << g.id(g.u(e));
		cout<< " - ca: " << custoAtribuicao[e] << " w - " << w[e] << endl;
	}
	


	// v <- 0, w <- 0 ja acontece na inicializacao 


	// grafo com as instalacoes que possuem desigualdade justa
	ListGraph T; 
	// f será o custo de instalação (fi)
	ListGraph::NodeMap<float> f(g);


	ListGraph::Edge menor; 
	custoAtribuicao[menor] = 100000; // TODO: NAO SEI COMO FAZ PRA JA COLOCAR AQUI O VALOR DA PRIMEIRA

	// Percorrer todas as arestas para ver qual o menor cij
	for(ListGraph::EdgeIt e(g); e!= INVALID; ++e){
		if(custoAtribuicao[e] < custoAtribuicao[menor]){
			menor = e;
		}
	}

	cout << "menor : " << g.id(menor) << " com custo: " << custoAtribuicao[menor] << endl;


	// Percorrer todos os clientes para aumentar em todos esse valor
	for(ListGraph::NodeIt v(g); v != INVALID; ++v){
		if (tipo[v]==0) { 			// se for cliente
			label[v] += custoAtribuicao[menor];
		}
	}


	// Percorrer todas as arestas para ver quais bateram o custo de atribuicao
	// Entao, adicionar em T e tirando de g o cliente que bateu o custo de atribuicao
	for(ListGraph::EdgeIt e(g); e!= INVALID; ++e){
		if(custoAtribuicao[e] == custoAtribuicao[menor]){
			T.addNode();
			f[g.v(e)] = label[g.v(e)];
			//g.erase(g.u(e)); // ta travando na hora de remover
		}
	}
	// 
	

	// // Percorrendo por todos os nós
	for(ListGraph::NodeIt v(g); v != INVALID; ++v){
		cout << "no id: " << g.id(v) << " - tipo: " << tipo[v] << " - label: " << label[v] << endl;
	}

// Percorrendo por todos os arcos
	for(ListGraph::EdgeIt e(g); e!= INVALID; ++e){
		cout << "arco id: " << g.id(e) ;
		cout << " - target: " << g.id(g.v(e)) << " - source: " << g.id(g.u(e));
		cout<< " - ca: " << custoAtribuicao[e] << " w - " << w[e] << endl;
	}

}