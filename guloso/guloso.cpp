#include <lemon/list_graph.h>
#include <algorithm>
#include <iostream>
#include <set>
#include <iterator>
#include <ctime>
#include "declaracoes.hpp"
#define EPSL 0.00000001

// Observacao importante: ao usar g.nodeFromId() eh necessario passar o ID geral, nao o blue ID nem red ID. 
// No caso do cliente, o red ID = ID geral, pois os clientes foram os primeiros a serem adicionados no grafo.
// No caso da instalacao, o ID geral = blue ID + qtd_clientes, pois as instalacoes foram adicionadas no grafo logo apos todos os clientes.

using namespace lemon;
using namespace std;

#define EXIBIR_ACOES 1 // corresponde a todos os cout quando uma acao é feita. 
#define EXIBIR_TEMPO 2 // corresponde aos calculos de tempo 
#define EXIBIR_GRAFO 3 // corresponde a descricao dos clientes, instalacoes e arcos
#define EXIBIR_MATRIZ_ADJACENCIA 4 // corresponde à parte final, na criacao de Tlinha

#define DEBUG 0 // OPCOES DE DEBUG: 1 PARA EXIBIR ACOES, 2 PARA EXIBIR TEMPO, 3 PARA EXIBIR AS MUDANÇAS NO GRAFO, 4 PARA EXIBIR AS MUDANCAS NA MATRIZ DE ADJACENCIA NA CRIACAO DE TLINHA

void mergeSort(int *vetor, int *vetorID, int posicaoInicio, int posicaoFim) {

    int i, j, k, metadeTamanho, *vetorTemp, *vetorTempID;
    if(posicaoInicio == posicaoFim) return;
    metadeTamanho = (posicaoInicio + posicaoFim ) / 2;

    mergeSort(vetor, vetorID, posicaoInicio, metadeTamanho);
    mergeSort(vetor, vetorID, metadeTamanho + 1, posicaoFim);

    i = posicaoInicio;
    j = metadeTamanho + 1;
    k = 0;
    vetorTemp = (int *) malloc(sizeof(int) * (posicaoFim - posicaoInicio + 1));
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


void excluindo_clientes_nao_ativos(int *vetor, int *vetorID, int tam_vetor, int *clientes_apagar, int quantidade_apagar){
	int qtd_restante = quantidade_apagar;
	for(int i=0;i<tam_vetor;i++){
		if(qtd_restante == 0){
			break;
		}
		
		for(int j=0;j<quantidade_apagar;j++){
			if(vetorID[i]==clientes_apagar[j]){
				// cout << "VAMOS APAGAR O CLINETE " << clientes_apagar[j] << endl;
				// vetorID[i] = -1; // ID -1 indica que nao esta ativo
				vetor[i] = -1; // coloca cij = -1, indicando que ja foi atribuido
				qtd_restante -= 1;
				break;
			}
		}
	}

	// Colocando novamente em ordem
	mergeSort(vetor, vetorID, 0, (tam_vetor-1));
}



// Por referencia diz qual o melhor tamanho para Y e seu respectivo custo
void melhor_subconjunto(int &melhor_tamanho, double &melhor_custo, int *vetor, double fi, double ganho, int index_inicio, int tam_vetor, double maior_cij){
	int tamanho_atual = 1;
	double soma_custo_atual = 0;
	double custo_atual = 0;
	melhor_custo = fi + maior_cij + 1; // limitante superior


	// inicializando só para nao ficar nulo se algo der errado
	melhor_tamanho = 1;

	// ira criar (tam_vetor - index_inicio) subconjuntos
	for(int i=index_inicio;i<tam_vetor;i++){
		soma_custo_atual = fi - ganho;
		// vendo a soma dos custos desse subconjunto
		for(int j=index_inicio;j<=i;j++){
			soma_custo_atual += vetor[j];
		}

		custo_atual = soma_custo_atual / tamanho_atual;

		if(custo_atual <= melhor_custo){
			melhor_custo = custo_atual;
			melhor_tamanho = tamanho_atual;
		}

		tamanho_atual += 1;
	}

}


bool igual(double i, double j){
    if((i > j-EPSL) && (i < j+EPSL))
        return true;
    return false;
}

bool maior_igual(double i, double j){
    if(i >= j * (1-EPSL))
        return true;
    return false;
}


// Retornar o valor da solucao
solutionType guloso(int qtdCli, int qtdInst, double * custoF, double * custoA){

	// Struct que vai retornar a solução
	solutionType solution;

	
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


	// indica as instalacoes finais atribuidas a cada cliente
	solution.instalacoes_conectadas = (int*) malloc((qtd_clientes) * sizeof(int));
    if(!solution.instalacoes_conectadas){
        cout << "Memory Allocation Failed";
        exit(1);
    }

    // Salvando a qtd de clientes para utilizar nos outros arquivos
    solution.qtd_clientes = qtd_clientes;

	// conjunto de clientes a serem removidos de g na iteração, pois deixaram de ser ativos
	int * apagar_clientes;
	apagar_clientes = (int*) malloc((qtd_clientes) * sizeof(int));
    if(!apagar_clientes){
        cout << "Memory Allocation Failed";
        exit(1);
    }


    int qtd_cli_nao_ativos = 0; // Indica quantos clientes ja foram conectados a alguma instalacao

	
	/* Sobre os grafos bipartidos, teremos
	
	Red para clientes

	Blue para instalações

	*/
	// Grafo que liga os clientes às instalações
	ListBpGraph g;

	// c_minX - será correnpondente a c(j,X) = min_{i \in X} cij, sendo X as inst abertas
	ListBpGraph::RedNodeMap<double> c_minX(g);

	// inst_aberta_prox - será correnpondente ao ID da instalacao i tal que min_{i \in X} cij, sendo X as inst abertas
	ListBpGraph::RedNodeMap<int> inst_aberta_prox(g);


	// ativo - indica se o cliente está ativo ainda ou nao, isto é, se ele ainda nao foi conectado a nenhuma instalacao aberta. if j \in g
	ListBpGraph::RedNodeMap<bool> ativo(g);

	// f - será o custo de instalação (fi)
	ListBpGraph::BlueNodeMap<double> f(g);


	// ordem_cij - será os clientes ativos ordenados pelo cij àquela instalação
	int **ordem_cij = (int**) malloc((qtd_instalacoes) * sizeof(int*));

	// ordem_cijID - indica os ID dos clientes ativos ordenados pelo cij àquela instalação
	int **ordem_cijID = (int**) malloc((qtd_instalacoes) * sizeof(int*));

	for(int i = 0; i < qtd_instalacoes; i++) {
		ordem_cij[i] = (int *)malloc(qtd_clientes * sizeof(int));
		ordem_cijID[i] = (int *)malloc(qtd_clientes * sizeof(int));
	}

    if((!ordem_cij)||(!ordem_cijID)){
        cout << "Memory Allocation Failed";
        exit(1);
    }

	// Indica se a instalação está aberta ou não
	ListBpGraph::BlueNodeMap<bool> aberta(g);

	// Para identificar cada nó
	ListBpGraph::NodeMap<int> nome(g);

	// Variavel que armazena o maior valor cij dado na entrada, para uso posterior
	double maiorCij = 0;

	// Variavel que armazena o maior valor fi dado na entrada, para uso posterior
	double maiorFi = 0;

	// usado como auxiliar para encontrar o index certo da instalacao (subtrai qtd_clientes)
	int indice_inst;

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
		c_minX[clientes[i]] = -1; // indica que nao está conectado com ninguém inicialmente //PROBLEMA: nao tenho ctz ainda qual a melhor inicializacao
		nome[clientes[i]] = i; // nomeia de acordo com a numeracao
		inst_aberta_prox[clientes[i]] = -1; // indica que nao está conectado com ninguém inicialmente
		ativo[clientes[i]] = true;
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
		aberta[instalacoes[i]] = false; // indica que a instalação não está aberta inicialmente
		nome[instalacoes[i]] = qtd_clientes + i; // nomeia de acordo com a numeracao

		// Salvando o valor do maior Fi da entrada
		if(f[instalacoes[i]]>maiorFi){
			maiorFi = f[instalacoes[i]];
		}
	}


	// Custo de atribuição é o custo de atribuir o cliente associado à instalação associada
	ListBpGraph::EdgeMap<double> custoAtribuicao(g);


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

	int qtd_clientes_ativos_g = qtd_clientes; // Indica a quantidade de clientes ainda em g, os clientes ativos.

	int qtd_inst_abertas = 0; // indica a quantidade de instalacoes abertas



	if(DEBUG >= EXIBIR_TEMPO){
		cout <<"[TEMPO] Finalizando contagem de tempo para criacao do grafo e seus maps" << endl;
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
		for(ListBpGraph::RedNodeIt n(g); n != INVALID; ++n){
			cout << "no id: " << g.id(n)  << " - nome: " << nome[n] << endl;
		}

		// Percorrendo por todos os nós B - instalacoes
		cout << "Percorrendo por todos as instalacoes" << endl;
		for(ListBpGraph::BlueNodeIt n(g); n != INVALID; ++n){
			cout << "no id: " << g.id(n)  << " - nome: " << nome[n] << " - f: " << f[n] << " - aberta: " << aberta[n] << endl;
		}


		// Percorrendo por todos os arcos
		cout << "Percorrendo por todos os arcos" << endl;
		for(ListBpGraph::EdgeIt e(g); e!= INVALID; ++e){
			cout << "arco id: " << g.id(e) ;
			cout << " - cliente: " << nome[g.u(e)] << " - instalacao: " << nome[g.v(e)];
			cout<< " - ca: " << custoAtribuicao[e] << endl;
		}
	}


	// colocando para cada instalacao a ordem dos cij
	for(ListBpGraph::BlueNodeIt n(g); n != INVALID; ++n){		// percorre as instalacoes
		cont = 0;
		// colocando no vetor que sera ordenado
		for (ListBpGraph::IncEdgeIt e(g, n); e != INVALID; ++e) { // Percorre todas arestas desse nó
			ordem_cijID[g.id(n)][cont] = g.id(g.u(e)); // linha da instalacao, coluna contando em cont
			ordem_cij[g.id(n)][cont] = custoAtribuicao[e]; // linha da instalacao, coluna contando em cont
			cont += 1;
		}

		// ordenando o vetor
		mergeSort(ordem_cij[g.id(n)], ordem_cijID[g.id(n)], 0, (cont-1));

		if (DEBUG >= EXIBIR_ACOES){
			cout << "ORDENADO PARA INSTALACAO " << g.id(n) << endl;
			for(int i=0;i<cont;i++){
				cout << ordem_cij[g.id(n)][i] << endl; // Adicionar o cliente j no conjunto de 
			}
			cout << "-------" << endl;
		}

	}


	int id_inst_escolhida;
	double melhor_custo_escolhido;
	int melhor_tam_escolhido;

	int tam_Y_atual;
	double custo_Y_atual;

	double ganho_cij;

	double gastoTotalFinal = 0;


	// ****** A partir daqui deve estar em um loop até nao ter mais clientes ativos:

	while(qtd_clientes_ativos_g > 0){

		if(DEBUG >= EXIBIR_ACOES){
			cout << endl << "------------------------------ AINDA TEM " << qtd_clientes_ativos_g << " CLIENTES ATIVOS ------------------------------" << endl << endl;
		}

		melhor_custo_escolhido = maiorCij * qtd_clientes + maiorFi + 1; //limitante superior tranquilo 
		melhor_tam_escolhido = -1;
		id_inst_escolhida = -1;
		
		for(ListBpGraph::BlueNodeIt n(g); n != INVALID; ++n){		// percorre as instalacoes
			// Calculando ganho de troca de atribuições
			// \sum_{j notin g} (c(j,X), cij)+
			ganho_cij = 0;
			
			for (ListBpGraph::IncEdgeIt e(g, n); e != INVALID; ++e) { // Percorre todas arestas desse nó
				if(!ativo[g.asRedNode(g.u(e))]){
					if(c_minX[g.asRedNode(g.u(e))] > custoAtribuicao[e]){
						ganho_cij += c_minX[g.asRedNode(g.u(e))] - custoAtribuicao[e];
					}
				}
			}
			// cout << "inst = " << g.id(n) << " -- GANHO: " << ganho_cij << endl;
			melhor_subconjunto(tam_Y_atual, custo_Y_atual, ordem_cij[g.id(n)], f[n], ganho_cij, qtd_cli_nao_ativos, qtd_clientes, maiorCij);

			// Atualizando valores do melhor custo
			if(custo_Y_atual < melhor_custo_escolhido){
				melhor_custo_escolhido = custo_Y_atual;
				melhor_tam_escolhido = tam_Y_atual;
				id_inst_escolhida = g.id(n);
			}

		}

		// cout << "Vamos abrir a instalacao " << id_inst_escolhida << " e conectar " << melhor_tam_escolhido << " clientes com custo " << melhor_custo_escolhido << endl;
		gastoTotalFinal += f[instalacoes[id_inst_escolhida]]; // Somando os custos de abertura das instalacoes escolhidas para abrir

		// fi <- 0
		f[instalacoes[id_inst_escolhida]] = 0;


		// g <- g - Y

		// criando o conjunto de clientes que serao apagados. inicia no primeiro ativo ate o tamanho escolhido na iteracao.
		for(int i = qtd_cli_nao_ativos; i < qtd_cli_nao_ativos + melhor_tam_escolhido; i++){
			apagar_clientes[i - qtd_cli_nao_ativos] = ordem_cijID[id_inst_escolhida][i]; // apagar_clientes inicia na posicao 0.
		}

		qtd_cli_nao_ativos += melhor_tam_escolhido;
		qtd_clientes_ativos_g -= melhor_tam_escolhido;

		// Alterando o valor correspondente em ordem_cij e ordem_cijID para -1 em todas as instalacoes
		for(ListBpGraph::BlueNodeIt n(g); n != INVALID; ++n){		// percorre as instalacoes
			excluindo_clientes_nao_ativos(ordem_cij[g.id(n)], ordem_cijID[g.id(n)], qtd_clientes, apagar_clientes, melhor_tam_escolhido);
		}

		// Atualizando o valor de c(j,X) para os clientes que ja nao eram ativos
		for(ListBpGraph::RedNodeIt n(g); n != INVALID; ++n){		// percorre os clientes
			if(!ativo[n]){
				if(c_minX[n] > custoAtribuicao[findEdge(g, n, instalacoes[id_inst_escolhida])]){
					c_minX[n] = custoAtribuicao[findEdge(g, n, instalacoes[id_inst_escolhida])];
					inst_aberta_prox[n] = id_inst_escolhida; // nao decidi ainda se aqui vai ser o ID ou o nome

					// cout << "Para ID = " << g.id(n) << " e inst: " << nome[instalacoes[id_inst_escolhida]] << endl;
					// cout << "atualizando aqui: " << c_minX[n] << endl;
				}
			}
		}

		// Atualizando o valor de c(j,X) para todos os clientes agora atribuidos a i
		for(int i=0;i<melhor_tam_escolhido;i++){
			c_minX[clientes[apagar_clientes[i]]] = custoAtribuicao[findEdge(g, clientes[apagar_clientes[i]], instalacoes[id_inst_escolhida])];
			inst_aberta_prox[clientes[apagar_clientes[i]]] = id_inst_escolhida; // nao decidi ainda se aqui vai ser o ID ou o nome
			ativo[clientes[apagar_clientes[i]]] = false;
			// cout << "Para ID = " << apagar_clientes[i] << " e inst: " << nome[instalacoes[id_inst_escolhida]] << endl;
			// cout << "temos cij = " << custoAtribuicao[findEdge(g, clientes[apagar_clientes[i]], instalacoes[id_inst_escolhida])] << endl;
			// cout << "atualizando aqui: " << c_minX[clientes[apagar_clientes[i]]] << " e id= " << inst_aberta_prox[clientes[apagar_clientes[i]]] << endl;
		}

	}

	// Somando os custos de conexao dos clientes a instalacao mais proxima aberta
	cont = 0;
	for(ListBpGraph::RedNodeIt n(g); n != INVALID; ++n){		// percorre os clientes
		gastoTotalFinal += custoAtribuicao[findEdge(g, n, instalacoes[inst_aberta_prox[n]])];

		// Colocando as instalacoes abertas mais proximas em um vetor pra retornar na solucao
		solution.instalacoes_conectadas[cont] = inst_aberta_prox[clientes[cont]]; // esse loop percorre todos os clientes, então posso usar o cont normalmente aqui
		// cout << "cliente " << cont << " com inst " << solution.instalacoes_conectadas[cont] << endl;

		cont += 1;
	}
	cout << "GASTO TOTAL FINAL: " << gastoTotalFinal << endl;


	if(DEBUG >= EXIBIR_TEMPO){

		// FINALIZANDO A CONTAGEM DE TEMPO DA FUNCAO COMO UM TODO
		clock_gettime(CLOCK_REALTIME, &real_finish);

		// Calculando o tempo gasto total
		realTimeSpent =  (real_finish.tv_sec - real_start.tv_sec);
		realTimeSpent += (real_finish.tv_nsec - real_start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
		cout << "Tempo total final da funcao: " << realTimeSpent << " segundos" << endl;
	}

	// Colocando valores na struct para retornar
	solution.gastoTotalFinal = gastoTotalFinal;
	solution.timeSpent = 0; // só para não deixar lixo, isso vai ser preenchido melhor no outro arquivo


	free(clientes);
	free(instalacoes);
	free(arcos);

	return(solution);
}
