#include <lemon/list_graph.h>
#include <algorithm>
// #include <lemon/lgf_reader.h>
// #include <lemon/lgf_writer.h>
#include <iostream>
using namespace lemon;
using namespace std;


#define qtd_clientes 10
#define qtd_instalacoes 10


// aqui so para salvar: for (ListGraph::IncEdgeIt e(g, n); e != INVALID; ++e) {

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

	/*

	ESTRUTURA QUE INDICA QUANTOS E QUAIS INSTALACOES CADA CLIENTE ESTA CONTRIBUINDO (AUMENTANDO O W) 
	PARA UM CERTO CLIENTE

	- cada linha indica um cliente
	- a primeira coluna indica a quantidade N de instalacoes que este cliente está contribuindo
	- as próximas N colunas indicam o nome de cada instalacao que ele está contribuindo


	obs: O ideal seria uma lista normal talvez

	*/
	int insts_que_cli_contrib[qtd_clientes][qtd_instalacoes + 1];


	int qtd_clientes_ativos_S = qtd_clientes; // Indica a quantidade de clientes ainda em S, os clientes ativos.
	
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

		// Inicializando o insts_que_cli_contrib, primeira coluna com 0 todas outras com -1
		insts_que_cli_contrib[i][0] = 0;
		for(int j=1;j<=qtd_instalacoes;j++){
			insts_que_cli_contrib[i][j] = -1;
		}
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
	// Entao, acionar a flag prontoContribuirW e colocar no cli_contrib_inst. Tambem anotar no insts_que_cli_contrib
	for(ListGraph::EdgeIt e(S); e!= INVALID; ++e){
		if(custoAtribuicao[e] == custoAtribuicao[menor]){
			prontoContribuirW[e] = true;

			indice_inst = nome[S.u(e)] - qtd_clientes;
			cli_contrib_inst[indice_inst][0] = 1;
			cli_contrib_inst[indice_inst][1] = nome[S.v(e)];

			insts_que_cli_contrib[nome[S.v(e)]][0] = 1;
			insts_que_cli_contrib[nome[S.v(e)]][1] = nome[S.u(e)];
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

	


	/*

	CRIACAO DE T

	*/

	// grafo com as instalacoes que possuem desigualdade justa
	ListGraph T; 
	// f será o custo de instalação (fi)
	ListGraph::NodeMap<float> f(T);
	// Criação de nós de instalações em T
	ListGraph::Node instT[qtd_instalacoes];
	int qtd_instT = 0; // indica a quantidade de instalacoes ja em T

	// Para identificar cada nó
	ListGraph::NodeMap<int> nomeT(T);



	int tag_inst_aberta = false; // tag para ver se a inst está aberta (se está em T)
	int menorAB; // receberá o menor valor entre o menor de A ou o menor de B

	float qtd_menorA;
	float qtd_atual;
	int nome_inst_menorA;
	int nome_cli_menorA;

	float qtd_menorB; 
	int nome_menorB;


	int id_cli;

	// ****** A partir daqui deve estar em um loop até nao ter mais clientes ativos:

	while(qtd_clientes_ativos_S > 0){

		cout << endl << "------------------------------ AINDA TEM "<< qtd_clientes_ativos_S <<" CLIENTES ATIVOS ------------------------------" << endl << endl;

		// 	A - fazer um for percorrendo todos os clientes e vendo o que resta de cij pra cada (achar o menor) (menor cij - vj)

		qtd_menorA = 10000; // PROBLEMA: seria bom pegar o valor da primeira, ao inves de 10000
		nome_inst_menorA = -1;
		nome_cli_menorA = -1;

		for(ListGraph::NodeIt v(S); v != INVALID; ++v){
			if (tipo[v]==0) { 			// se for cliente
		
				for (ListGraph::IncEdgeIt e(S, v); e != INVALID; ++e) { // Percorre todas arestas desse nó
					if(!prontoContribuirW[e]){
						qtd_atual = custoAtribuicao[e] - label[v];
						if(qtd_atual < qtd_menorA){
							qtd_menorA = qtd_atual;
							nome_inst_menorA = nome[S.u(e)]; // PROBLEMA: E SE TIVER MAIS QUE UMA INSTALACAO?
							nome_cli_menorA = nome[S.v(e)]; //  PROBLEMA: E SE TIVER MAIS QUE UM CLIENTE?
						}
					}
				}
			}
		}

		cout << "temos a qtd menor na parte A: " << qtd_menorA << endl;



		// B - fazer um for percorrendo todas as instalações para ver (fi - somatório de tudo que foi contribuído ate então para fi ) / numero de clientes prontos para contribuir para fi … os que ja alcançaram cij e ainda estão ativos

		qtd_menorB = 10000; // PROBLEMA: seria bom pegar o valor da primeira, ao inves de 10000
		nome_menorB = -1;

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

					qtd_atual = (label[v] - qtd_atual)/cli_contrib_inst[indice_inst][0]; //(fi - somatório de tudo que foi contribuído ate então para fi ) / (numero de clientes prontos para contribuir para fi) 

				}

				if(qtd_atual < qtd_menorB){
					qtd_menorB = qtd_atual;
					nome_menorB = nome[v]; // PROBLEMA: E SE TIVER MAIS QUE UMA INSTALACAO?
				}
			}
		}

		cout << "temos a qtd menor na parte B: " << qtd_menorB << " com a inst: " << nome_menorB << endl;


		//	ver o menor entre A e B e aumentar esse valor em todos os clientes (tanto no cij se ainda faltar quanto no wij se ja estiver contribuindo)

		if(qtd_menorA < qtd_menorB){
			menorAB = qtd_menorA;
		}
		else if(qtd_menorB < qtd_menorA){
			menorAB = qtd_menorB;
		}
		// SENAO: EMPATE
		else{
			cout << "Empatou! Ver o que fazer" << endl;
			menorAB = qtd_menorA; // PROBLEMA: E SE DEU EMPATE? por enquanto ta assim
		}

		// Percorrer todos os clientes para aumentar em todos esse valor em v
		for(ListGraph::NodeIt v(S); v != INVALID; ++v){
			if (tipo[v]==0) { 			// se for cliente
				label[v] += menorAB;
			}
		}

		// Percorrer todas as arestas para aumentar em w, caso o cliente estivesse pronto para contribuir
		// Aproveitar e ver quais bateram o custo de atribuicao
		// Entao, acionar a flag prontoContribuirW e colocar no cli_contrib_inst. Tambem anotar no insts_que_cli_contrib
		for(ListGraph::EdgeIt e(S); e!= INVALID; ++e){
			if(prontoContribuirW[e]){ // se está pronto para contribuir
				w[e] += menorAB; // PROBLEMA: será que não tem problema de aumentar esse valor varios vezes e estourar? Talvez dividir pelo numero de inst q ele contribui?
			}
			else if(custoAtribuicao[e] == label[S.v(e)]){ // SENAO SE: está pronto para contribuir (ja pagou o ca)
				prontoContribuirW[e] = true;

				indice_inst = nome[S.u(e)] - qtd_clientes;
				cli_contrib_inst[indice_inst][0] += 1; // aumenta em 1 o numero de cliente que contribui para aquela inst
				cli_contrib_inst[indice_inst][cli_contrib_inst[indice_inst][0]] = nome[S.v(e)]; // atribui esse novo cliente em sua lista

				insts_que_cli_contrib[nome[S.v(e)]][0] += 1; // aumenta em 1 o numero de instalacoes que aquele cliente contribui
				insts_que_cli_contrib[nome[S.v(e)]][insts_que_cli_contrib[nome[S.v(e)]][0]] = nome[S.u(e)]; // atribui essa nova inst em sua lista
			}
		}



		// REPETINDO O IF, PARA TRATAR DOS DETALHES AGORA

		// SE FOR O CASO A: verificar se a instalação que aquele cliente alcançou ja estava aberta, se sim, remover ele dos ativos
		if(qtd_menorA < qtd_menorB){
			cout << "Caso A!" << endl;
			tag_inst_aberta = false;
			for(ListGraph::NodeIt v(T); v != INVALID; ++v){ // Percorrer por T para ver se o nome_inst_menorA está la (se a inst ja está aberta)
				if(nomeT[v] == nome_inst_menorA){
					tag_inst_aberta = true;
					break;
				}
			}
			if(tag_inst_aberta){ // se a instalacao ja estava aberta
				cout <<"Instalacao ja estava aberta. Removendo o cliente em questao dos ativos "<< endl;
				S.erase(S.nodeFromId(nome_cli_menorA)); 
				qtd_clientes_ativos_S -= 1;
			}
		}
		// SE FOR O CASO B: caso B seja o menor valor: abrir a instalação i e remover os seus contribuintes dos clientes ativos (lembrando de remover eles das listas de contribuintes das outras instalações (talvez só fazer uma tag se está ativo ou não)
		else if(qtd_menorA > qtd_menorB){ 
			cout << "Caso B!" << endl;
			instT[qtd_instT] = T.addNode();
			f[instT[qtd_instT]] = label[S.nodeFromId(nome_menorB)]; // pega o valor fi da instalacao
			nomeT[instT[qtd_instT]] = nome_menorB; // pega o nome da instalacao
			qtd_instT += 1;

			//Remover os seus contribuintes dos clientes ativos
			indice_inst = nome_menorB - qtd_clientes;
			cout<<"*-* aqui "<< indice_inst <<" contribuem: " << cli_contrib_inst[indice_inst][0] << " clientes" << endl;
			for(int i=1;i<=cli_contrib_inst[indice_inst][0];i++){

				id_cli = cli_contrib_inst[indice_inst][i];
				
				cout<<"************* vamo remover cliente " << id_cli << endl;



				// //Apagando ele da lista de contribuintes das outras instalacoes

				/*

				PROBLEMA GRAVE: como apagar da lista? Precisaria "arrastar" todos de tras pra frente, sem deixar vacuo no meio

				*/


				// for(int j=1;j<=insts_que_cli_contrib[id_cli][0];j++){ // percorre por todas as instalacoes que o cliente contribui

				// 	if(insts_que_cli_contrib[id_cli][j]!=nome_menorB){ // se nao estamos falando dessa inst atual

				// 		for(int k=1;k<=cli_contrib_inst[insts_que_cli_contrib[id_cli][j]][0];k++){ // percorre por todos os clientes que contibuem pra essa instalacao j

				// 			if(cli_contrib_inst[insts_que_cli_contrib[id_cli][j]][k] == id_cli){ // se encontrou o cliente em questao
				// 				cli_contrib_inst[insts_que_cli_contrib[id_cli][j]][k] = -1;      // PROBLEMA GRAVE: apaga colocando -1. Na verdade tem que locomover todos para frente
				// 				cli_contrib_inst[insts_que_cli_contrib[id_cli][j]][0] -= 1;      // diminui a quantidade de clientes que contribuem para ela
				// 				break;
				// 			}
				// 		}
				// 	}
				// }

				//Apagando ele dos clientes ativos (de S)
				S.erase(S.nodeFromId(id_cli));
				qtd_clientes_ativos_S -=1;

				
			}

		} 
		// SENAO: EMPATE
		else{
			cout << "Empatou! Ver o que fazer" << endl;
		}


		// // Percorrendo por todos os nós de T
		cout<<"-- T --" << endl;
		for(ListGraph::NodeIt v(T); v != INVALID; ++v){
			cout << "no id: " << T.id(v) << " - nome: " << nomeT[v] << " - f: " << f[v] << endl;
		}
		cout<<"-------" << endl;



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


	}


}