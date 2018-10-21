#include <lemon/list_graph.h>
// #include <lemon/lgf_reader.h>
// #include <lemon/lgf_writer.h>
#include <iostream>
using namespace lemon;
using namespace std;


#define qtd_clientes 10
#define qtd_instalacoes 10


int main(){

	
	// Grafo que liga os clientes às instalações
	ListDigraph g;


	// Label do cliente será o quanto ele vai contribuir (v)
	// Label da instalação será o custo de instalação (fi)
	ListDigraph::NodeMap<float> label(g);


	// Tipo: 0 para cliente 1 para instalação
	ListDigraph::NodeMap<int> tipo(g);


	// Criação de nós clientes e atribuição de seus labels
	ListDigraph::Node clientes[qtd_clientes];

	for(int i=0;i<qtd_clientes;i++){
		clientes[i] = g.addNode();
		label[clientes[i]] = 0; // v = inicialmente nao contribui com nada
		tipo[clientes[i]] = 0; // indica que é cliente
	}


	// Criação de nós de instalações e atribuição de seus labels
	ListDigraph::Node instalacoes[qtd_instalacoes];

	for(int i=0;i<qtd_instalacoes;i++){
		instalacoes[i] = g.addNode();
		label[instalacoes[i]] = 4+i/3.0; // fi = numero aleatorio
		tipo[instalacoes[i]] = 1; // indica que é instalação
	}


	// Custo de atribuição é o custo de atribuir o cliente associado à instalação associada
	ListDigraph::ArcMap<int> custoAtribuicao(g);

	// W é a parte do custo da instalação associada que o cliente associado vai contribuir
	ListDigraph::ArcMap<int> w(g);

	// Criação de arcos e atribuição do custo e w
	ListDigraph::Arc arcos[qtd_clientes*qtd_instalacoes]; // conectando todos com todos

	int cont = 0;
	for(int i=0;i<qtd_clientes;i++){
		for(int j=0;j<qtd_instalacoes;j++){
			arcos[cont] = g.addArc(clientes[i],instalacoes[j]);
			cont++;
			custoAtribuicao[arcos[cont]] = 300 - i - j; // cij = numero aleatorio
			w[arcos[cont]] = 0; // inicialmente nao contribui com nada
		}
	}

	
	// // Percorrendo por todos os nós
	// for(ListDigraph::NodeIt v(g); v != INVALID; ++v){
	// 	cout << "no id: " << g.id(v) << " - tipo: " << tipo[v] << " - label: " << label[v] << endl;
	// }



	// PROBLEMA :  CUSTO DE ATRIBUICAO RECEBEU VALORES CORRETOS LA EM CIMA
	// POREM AQUI NO FOR EMBAIXO ELE PERDE O VALOR, APENAS ALGUNS INDICES PERMANECE
	// ISSO TAMBEM OCORRE COM O W, POREM COMO JA ERA 0 INICIALMENTE, NAO DA PRA NOTAR
	// FAÇA O TESTE MUDANDO O VALOR DE W LA EM CIMA


	// Percorrendo por todos os arcos
	cout << "NAO ERA PRA ESTAR COM VALOR 0 O CA! " << endl;
	for(ListDigraph::ArcIt a(g); a!= INVALID; ++a){
		cout << "arco id: " << g.id(a) ;
		cout << " - target: " << g.id(g.target(a)) << " - source: " << g.id(g.source(a));
		cout<< " - ca: " << custoAtribuicao[a] << " w - " << w[a] << endl;
	}
}