#include <iostream>
#include <cstdlib>
#include <fstream>
#include "definitions.hpp"
#define EPS 0.001

#define DISPLAY_BASIC 1 // corresponde a exibicao do custo final da solucao
#define DISPLAY_TIME_SIZE 2 // corresponde a exibicao do tempo total gasto e do tamanho da entrada (quantidade de clientes e de instalacoes)
#define DISPLAY_ACTIONS 3 // corresponde a todos os cout quando uma informacao eh salva 

#define DEBUG 1 // OPCOES DE DEBUG: 0 PARA NAO EXIBIR NADA, 1 PARA EXIBIR CUSTO FINAL, 2 PARA EXIBIR TEMPO E QTD CLI E INST, 3 PARA EXIBIR AS INFORMACOES SENDO SALVAS

using namespace std;

int main(int argc, char *argv[]){

	if(argc < 4){
		cout << "Error in the parameters. You must enter 3 parameters" << endl;
		return 0;
	}

	char * inputName = argv[1];
	char * solutionName = argv[2];
	char * inputType = argv[3];

	// Arquivo para salvar a solucao
	ofstream solutionTXT;

	// Declaracao variaveis que indicam o tempo no inicio e fim da execucao
	struct timespec start, finish;

	// Declaracao variavel que marcara o tempo calculado daquela execucao
	double timeSpent;

	// Declaracao de variaveis auxiliares para a formacao do arquivo .sol
	char auxSolName[105];

	// Declaracoes iniciais
	int qty_clients, qty_facilities, counter;
	double auxRead;
	char auxCRead[30];

	ifstream inputFLP;

	double * costA;
	double * costF;

	solutionType solution;

	//Iniciando a contagem do tempo
	clock_gettime(CLOCK_REALTIME, &start);

	cout << fixed;
	cout.precision(5);

	/****************************
	Lendo o arquivo de entrada
	*/

	// Abrindo arquivo 
	inputFLP.open(inputName); 

	if (!inputFLP) {
		cout << "Unable to open file";
		exit(1); // terminate with error
	}

	if(strcmp(inputType,"1")==0){
		// Lendo do arquivo os valores que indicam a quantidade de instalacoes e clientes
		inputFLP >> qty_facilities >> qty_clients; 

		if(DEBUG >= DISPLAY_TIME_SIZE){
			cout << "QTY FACILITIES: " << qty_facilities << " AND QTY CLIENTS: " << qty_clients << endl;
		}

		// Vetores que salvarao custos lidos no arquivo
		costF = (double*) malloc((qty_facilities) * sizeof(double));
		if(!costF){
			cout << "Memory Allocation Failed";
			exit(1);
		}

		costA = (double*) malloc((qty_clients*qty_facilities) * sizeof(double));
		if(!costA){
			cout << "Memory Allocation Failed";
			exit(1);
		}


		// Lendo do arquivo os custos de abertura das instalacoes e salvando no vetor costF
		for(int i=0;i<qty_facilities;i++){
			inputFLP >> auxCRead; // Descartando a capacidade da instalação
			// cout << "Discard: " << auxCRead << endl;
			inputFLP >> auxRead; // Pegando o valor do custo de abertura da instalação
			costF[i] = auxRead;

			if(DEBUG >= DISPLAY_ACTIONS){
				cout << "Fi =  " << costF[i] << endl;  
			}   
		}


		// Lendo do arquivo os custos de atribuicao do clientes com as instalacoes e salvando no vetor costA
		counter = 0;
		for(int i=0;i<qty_clients;i++){
			inputFLP >> auxRead; // Descartando aqui a demanda do cliente

			if(DEBUG >= DISPLAY_ACTIONS){
				cout << "Demand = " << auxRead << endl;
			}

			for(int j=0;j<qty_facilities;j++){
				inputFLP >> auxRead;
				costA[counter] = auxRead;

				if(DEBUG >= DISPLAY_ACTIONS){
					cout << "AC = " << costA[counter] << endl; 
				}

				counter+=1;
			}
		}

	}
	else if(strcmp(inputType,"2")==0){
		inputFLP >> auxCRead; // Descartando a palavra FILE
		inputFLP >> auxCRead; // Descartando o nome do arquivo

		// Lendo do arquivo os valores que indicam a quantidade de instalacoes e clientes. E descartando o 0
		inputFLP >> qty_facilities >> qty_clients >> auxRead; 

		if(DEBUG >= DISPLAY_TIME_SIZE){
			cout << "QTY FACILITIES: " << qty_facilities << " AND QTY CLIENTS: " << qty_clients << endl;
		}
		// Vetores que salvarao custos lidos no arquivo
		costF = (double*) malloc((qty_facilities + 1) * sizeof(double));
		if(!costF){
			cout << "Memory Allocation Failed";
			exit(1);
		}

		costA = (double*) malloc((qty_clients*qty_facilities + 1) * sizeof(double));
		if(!costA){
			cout << "Memory Allocation Failed";
			exit(1);
		}


		// Lendo do arquivo os custos de abertura das instalacoes e salvando no vetor costF
		for(int i=0;i<qty_facilities;i++){
			inputFLP >> auxRead; // Descartando o nome da instalação
			// cout << "Discard: " << auxRead << endl;
			inputFLP >> auxRead; // Pegando o valor do custo de abertura da instalação
			costF[i] = auxRead;

			if(DEBUG >= DISPLAY_ACTIONS){
				cout << "Fi =  " << costF[i] << endl;  
			}    

			// Lendo do arquivo os custos de atribuicao do clientes com as instalacoes e salvando no vetor costA
			for(int j=0;j<qty_clients;j++){

				inputFLP >> auxRead;
				costA[i + j * qty_facilities] = auxRead;       // Esse indice estranho eh para deixa no mesmo formato que o caso ORLIB

				if(DEBUG >= DISPLAY_ACTIONS){
					cout << "AC = " << costA[i + j * qty_facilities] << endl; 
				}
			}
		}
	}
	else{
		cout << "Invalid input type." << endl;
		// return solution;
		return 0;
	}

	/*
	Fim ler o arquivo de entrada
	******************************************/

	// Chamando a funcao que resolve o problema de fato
	solution = greedy(qty_clients, qty_facilities, costF, costA);

	// Finalizando a contagem do tempo
	clock_gettime(CLOCK_REALTIME, &finish);

	if(DEBUG >= DISPLAY_BASIC){
		cout << "Final total cost: " << solution.finalTotalCost << endl;
	}

	// Calculando o tempo gasto
	timeSpent =  (finish.tv_sec - start.tv_sec);
	timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
	if(DEBUG >= DISPLAY_TIME_SIZE){
		cout << "Time spent: " << timeSpent << " seconds" << endl;
	}

	//Abertura de arquivo para leitura e escrita do .sol .
	solutionTXT.open(solutionName, std::ofstream::out | std::ofstream::trunc);

	// Colocando no solutionTXT o valor do custo da solucao
	solutionTXT << fixed << setprecision(5) << solution.finalTotalCost << " ";
	
	// Colocando no solutionTXT o tempo gasto 
	solutionTXT << timeSpent;

	// Colocando no solutionsTXT as instalacoes finais conectadas
	for(int i=0; i < qty_clients; i++){
		solutionTXT << " " << solution.assigned_facilities[i];
	}

	// Fechando o arquivo
	inputFLP.close();

	// Liberando memoria
	free(costF);
	free(costA);

	// return solution;
	return 0;
}
