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

	- cada linha indica um cliente
	- cada coluna indica uma instalacao
	- Valor 1 na linha i, coluna j se o cliente i contribui pra instalacao j

	*/
	int matriz_adjacencia[qtd_clientes][qtd_instalacoes]; // indica se o cliente alcançou a instalação


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
	}


	// Criação de nós de instalações e atribuição de seus labels
	ListGraph::Node instalacoes[qtd_instalacoes];

	for(int i=0;i<qtd_instalacoes;i++){
		instalacoes[i] = g.addNode();
		label[instalacoes[i]] = 100 + 4+i/3.0; // fi = numero aleatorio
		tipo[instalacoes[i]] = 1; // indica que é instalação
		nome[instalacoes[i]] = qtd_clientes + i; // nomeia de acordo com a numeracao
	}


	// Inicializando a matriz de adjacencia, todos com valor 0
	for(int i=0;i<qtd_clientes;i++){
		for(int j=0;j<qtd_instalacoes;j++){
			matriz_adjacencia[i][j] = 0;
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
	cout << "Percorrendo por todos os nos" << endl;
	for(ListGraph::NodeIt v(S); v != INVALID; ++v){
		cout << "no id: " << S.id(v) << " - tipo: " << tipo[v] << " - nome: " << nome[v] << " - label: " << label[v] << endl;
	}


	// Percorrendo por todos os arcos
	cout << "Percorrendo por todos os arcos" << endl;
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
	// Entao, acionar a flag prontoContribuirW e colocar na matriz de adjacencia.
	for(ListGraph::EdgeIt e(S); e!= INVALID; ++e){
		if(custoAtribuicao[e] == custoAtribuicao[menor]){
			prontoContribuirW[e] = true;

			indice_inst = nome[S.u(e)] - qtd_clientes;
			matriz_adjacencia[nome[S.v(e)]][indice_inst] = 1;
			// PROBLEMA: ver se alguma instalacao ja estaria aberta (se ela tem custo zero).. fazer um if aqui

		}
	}


	// Percorrendo por todos os arcos
	cout << "Percorrendo por todos os arcos" << endl;
	for(ListGraph::EdgeIt e(S); e!= INVALID; ++e){
		cout << "arco id: " << S.id(e) ;
		cout << " - cliente: " << S.id(S.v(e)) << " - instalacao: " << S.id(S.u(e));
		cout<< " - ca: " << custoAtribuicao[e] << " w - " << w[e] << " PCW - " << prontoContribuirW[e] << endl;
	}



	// Percorrendo por todos os nós
	cout << "Percorrendo por todos os nos" << endl;
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

	// Para identificar cada nó
	ListGraph::NodeMap<int> nomeT(T);

	// Criação de nós de instalações em T
	ListGraph::Node instT[qtd_instalacoes];
	int qtd_instT = 0; // indica a quantidade de instalacoes ja em T



	int tag_inst_aberta = false; // tag para ver se a inst está aberta (se está em T)
	int menorAB; // receberá o menor valor entre o menor de A ou o menor de B

	float qtd_menorA;
	float qtd_atual;
	int nome_inst_menorA;
	int nome_cli_menorA;

	float qtd_menorB; 
	int nome_menorB;

	int somatorio_caso_b; // PROBLEMA: mudar o nome


	int id_cli;

	int qtd_contribui;

	bool teste = true; //PROBLEMA: mudar nome aqui 


	// ****** A partir daqui deve estar em um loop até nao ter mais clientes ativos:

	while(qtd_clientes_ativos_S > 0){

		cout << endl << "------------------------------ AINDA TEM "<< qtd_clientes_ativos_S <<" CLIENTES ATIVOS ------------------------------" << endl << endl;

		// 	A - fazer um for percorrendo todos os clientes e vendo o que resta de cij pra cada (achar o menor) (menor cij - vj)

		qtd_menorA = 10000; // PROBLEMA: seria bom pegar o valor da primeira, ao inves de 10000.. SOLUCAO: no começo salvar o maior cij existente dos dados da entrada
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

		qtd_menorB = 10000; // PROBLEMA: seria bom pegar o valor da primeira, ao inves de 10000.. SOLUCAO: mesma solucao do caso A.. colocar maior fi da entrada
		nome_menorB = -1;


		for(ListGraph::NodeIt v(S); v != INVALID; ++v){

			if (tipo[v]==1) { 			// se for instalacao

				indice_inst = nome[v] - qtd_clientes;

				qtd_contribui = 0;

				for(int i=0;i<qtd_clientes;i++){
					qtd_contribui += matriz_adjacencia[i][indice_inst]; // verificando quantos contribui pra ela // PROBLEMA: pensar melhor mas (talvez!!!!) pode salvar isso como node map, pra nao ter q ficar recalculando toda vez
				}

				if(qtd_contribui>0){ // SENAO, SE alguem ja esta pronto para contribuir pelo menos
					somatorio_caso_b = 0; 
					for (ListGraph::IncEdgeIt e(S, v); e != INVALID; ++e) { // Percorre todas arestas desse nó
						somatorio_caso_b += w[e]; // vai indicar somatorio de todos w dessa instalacao // PROBLEMA: pode salvar isso como node map, pra nao ter q ficar recalculando toda vez
					}

					qtd_atual = (label[v] - somatorio_caso_b)/qtd_contribui; //(fi - somatório de tudo que foi contribuído ate então para fi ) / (numero de clientes prontos para contribuir para fi) 

					if(qtd_atual < qtd_menorB){
						qtd_menorB = qtd_atual;
						nome_menorB = nome[v]; // PROBLEMA: E SE TIVER MAIS QUE UMA INSTALACAO?
					}
				}
				
			}
		}

		cout << "temos a qtd menor na parte B: " << qtd_menorB << " com a inst: " << nome_menorB << endl;


		/*


			PAREI DE PARAR POR AQUI


		*/


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
		// Entao, acionar a flag prontoContribuirW e colocar na matriz de adjacencia. 

		for(ListGraph::EdgeIt e(S); e!= INVALID; ++e){
			if(prontoContribuirW[e]){ // se está pronto para contribuir
				w[e] += menorAB; // PROBLEMA: será que não tem problema de aumentar esse valor varios vezes e estourar? Talvez dividir pelo numero de inst q ele contribui?
			}
			else if(custoAtribuicao[e] == label[S.v(e)]){ // SENAO SE: está pronto para contribuir (ja pagou o ca)
				prontoContribuirW[e] = true;

				indice_inst = nome[S.u(e)] - qtd_clientes;
				matriz_adjacencia[nome[S.v(e)]][indice_inst] = 1; // atribui esse novo cliente em sua lista
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
			for(int i=0;i<=qtd_clientes;i++){
				if(matriz_adjacencia[i][indice_inst]==1){
					cout<<"************* vamo remover cliente " << i << endl;


					//Apagando ele da lista de contribuintes das outras instalacoes

					for(int j=0;j<=qtd_instalacoes;j++){ // percorre por todas as instalacoes que o cliente contribui

						if(matriz_adjacencia[i][j]==1){ // se ele contribui pra instalacao j

							cout<<"removido cliente " << i << " da instalacao " << j << endl;


							if(j!=indice_inst){ // se nao estamos falando dessa inst atual

								matriz_adjacencia[i][j] = 0;
							}
						}
					}

					//Apagando ele dos clientes ativos (de S)
					S.erase(S.nodeFromId(i));
					qtd_clientes_ativos_S -=1;
				}
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
		cout << "Percorrendo por todos os arcos" << endl;
		for(ListGraph::EdgeIt e(S); e!= INVALID; ++e){
			cout << "arco id: " << S.id(e) ;
			cout << " - cliente: " << S.id(S.v(e)) << " - instalacao: " << S.id(S.u(e));
			cout<< " - ca: " << custoAtribuicao[e] << " w - " << w[e] << " PCW - " << prontoContribuirW[e] << endl;
		}


		// Percorrendo por todos os nós
		cout << "Percorrendo por todos os nos" << endl;
		for(ListGraph::NodeIt v(S); v != INVALID; ++v){
			cout << "no id: " << S.id(v) << " - tipo: " << tipo[v] << " - nome: " << nome[v] << " - label: " << label[v] << endl;
		}

	}


	/*

	CRIACAO DE Tlinha

	*/

	// grafo com as instalacoes abertas
	ListGraph Tlinha; 
	// Criação de nós de instalações em Tlinha
	ListGraph::Node instTlinha[qtd_instalacoes];
	int qtd_instTlinha = 0; // indica a quantidade de instalacoes ja em Tlinha

	// Para identificar cada nó
	ListGraph::NodeMap<int> nomeTlinha(Tlinha);

	cout << "Criando Tlinha" << endl;
	indice_inst = 0;
	int id_inst_apagar = 10;

	cout << "Exibindo matriz de adjacencia" << endl;
	for(int i=0;i<qtd_clientes;i++){
		for(int j=0;j<qtd_instalacoes;j++){
			cout<< matriz_adjacencia[i][j] << " ";
		}
		cout << endl;
	}

	// Enquanto T != 0
	while(qtd_instT > 0){

		for(ListGraph::NodeIt v(T); v != INVALID; ++v){
			// Escolha inst pertencente a T
			cout << "Escolhido instalacao " << nomeT[v] << endl;
			indice_inst = nomeT[v] - qtd_clientes;

			// Tlinha <- Tlinha U {i}
			instTlinha[qtd_instTlinha] = Tlinha.addNode();
			nomeTlinha[instTlinha[qtd_instTlinha]] = nomeT[v]; // pega o nome da instalacao

			qtd_instTlinha += 1;

			break;
		}


		//	Remove todas instalacoes i se algum cliente i contribuir a i e indice_inst
		for(int i=0;i<qtd_clientes;i++){

			if(matriz_adjacencia[i][indice_inst]==1){ // se o cliente i contribui para a inst indice_inst

				for(int j=0;j<qtd_instalacoes;j++){ // ve todas as instalacoes que esse cliente contribui tambem e remove

					if(matriz_adjacencia[i][j]){

						//PROBLEMA: falta verificar se a instalacao j está em T

						cout << "Removendo a instalacao " << (j + qtd_clientes) << " de T, pois cliente " << i << " contribui a ela" << endl;


						for(int k=0;k<qtd_clientes;k++){ // PROBLEMA: nao parece eficiente. Quero marcar que todas os clientes daquela instalacao agr nao contribuem mais pra ela
							matriz_adjacencia[k][j] = 0;
						}


						// PROBLEMA: nome é j, mas o ID nao sei...
						for(ListGraph::NodeIt v(T); v != INVALID; ++v){ // GAMBIARRA PRA DESCOBRIR O ID DA INSTALACAO J
							if(nomeT[v]==(j+qtd_clientes)){
								id_inst_apagar = T.id(v);
								break;
							}
						}

						T.erase(T.nodeFromId(id_inst_apagar));  
						qtd_instT -= 1;
					}
				}
			}
		}

	}


	// // Percorrendo por todos os nós de Tlinha
	cout<<"-- Tlinha --" << endl;
	for(ListGraph::NodeIt v(Tlinha); v != INVALID; ++v){
		cout << "no id: " << Tlinha.id(v) << " - nome: " << nomeTlinha[v] << endl;
	}
	cout<<"-------" << endl;

	// Depois abrir todas as instalacoes em Tlinha e atribuir cada cliente à instalacao mais próxima

	// PROBLEMA: qual melhor jeito? lembrando que tenho as informacoes iniciais em g.. mas percorrer g e ir verificando se está em Tlinha nao parece tao bom

}