#include <iostream>
#include <cstdlib>
#include <fstream>
#include "definitions.hpp"

#define EPS 0.001

#define DISPLAY_TIME_SIZE 1 // corresponde a exibicao do tempo total gasto e do tamanho da entrada (quantidade de clientes e de instalacoes)
#define DISPLAY_ACTIONS 2 // corresponde a todos os cout quando uma informacao eh salva 

#define DEBUG 0 // OPCOES DE DEBUG: 0 PARA NAO EXIBIR NADA, 1 PARA EXIBIR TEMPO E QTD CLI E INST, 2 PARA EXIBIR AS INFORMACOES SENDO SALVAS

using namespace std;

int main(int argc, char *argv[]){

	if(argc < 6){
		cout << "Error in the parameters. You must enter at least 6 parameters" << endl;
		return 0;
	}

	char * inputName = argv[1];
	char * primalSolName = argv[2];
	char * dualSolName = argv[3];
	char * solutionName = argv[4];
	char * inputType = argv[5];
	int seed;

	if(argc >= 7)
		seed = stoi(argv[6],nullptr,10); // convertendo argv[6] para inteiro
	else
		seed = ((unsigned)time(NULL)); // seed nao foi passada por parametro, entao vai aleatoria

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

	double * costA;
	double * costF;

	ifstream inputFLP;
	ifstream primalSol;
	ifstream dualSol;

	solutionType solution;

	double ** x_values;
	double * v_values;

	//Iniciando a contagem do tempo
	clock_gettime(CLOCK_REALTIME, &start);

	cout << fixed;
   	cout.precision(5);

	/****************************
	Lendo o arquivo de entrada
	*/

	// Abrindo arquivo de entrada
	inputFLP.open(inputName); 

	if (!inputFLP) {
		cout << "Unable to open file";
		exit(1); // terminate with error
	}


	if(strcmp(inputType,"1")==0){
		// Lendo do arquivo os valores que indicam a quantidade de instalacoes e clientes
		inputFLP >> qty_facilities >> qty_clients; 

		if(DEBUG >= DISPLAY_TIME_SIZE){
			cout << "QTY FACILITIES: " << qty_facilities << " E QTY CLIENTS: " << qty_clients << endl;
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
			cout << "QTY FACILITIES: " << qty_facilities << " E QTY CLIENTS: " << qty_clients << endl;
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
			// cout << "DESCARTE:" << auxRead << endl;
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


	/**************************************
	Lendo o arquivo com a solução LP primal
	*/

	if(DEBUG >= DISPLAY_ACTIONS){
		cout << "primal solution" << endl;  
	}  

	// Abrindo arquivo de entrada
	primalSol.open(primalSolName); 

	if (!primalSol) {
		cout << "Unable to open file";
		exit(1); // terminate with error
	}


	primalSol >> auxCRead; // Descartando o custo 
	// cout << "Discard cost: " << auxCRead << endl;

	primalSol >> auxCRead; // Descartando o tempo gasto 
	// cout << "Discard time: " << auxCRead << endl;

	primalSol >> auxCRead; // Descartando a tag de otimalidade 
	// cout << "Discard optimal: " << auxCRead << endl;


	// Descartando do arquivo os yi de cada instalacao do LP primal
	for(int i=0;i<qty_facilities;i++){
		primalSol >> auxCRead; // Descartando o yi 
		// cout << "Discard y[" << i << "]: " << auxCRead << endl;
	}


	// indica o valor de x de cada par instalacao/cliente
	x_values = (double**) malloc((qty_facilities) * sizeof(double*));

	for(int i = 0; i < qty_facilities; i++) {
		x_values[i] = (double *)malloc(qty_clients * sizeof(double));
	}

	if(!x_values){
		cout << "Memory Allocation Failed";
		exit(1);
	}

	// Lendo do arquivo os xij de cada par instalacao/cliente no LP primal e salvando no vetor x_values
	for(int i=0;i<qty_facilities;i++){
		for(int j=0;j<qty_clients;j++){
			primalSol >> x_values[i][j]; 

			if(DEBUG >= DISPLAY_ACTIONS){
				cout << "facility " << i << " client " << j << " =  " << x_values[i][j] << endl;  
			}  
		}
	}

  /*
	Fim ler o arquivo com a solução LP primal
	******************************************/


	/**************************************
	Lendo o arquivo com a solução LP dual
	*/

	if(DEBUG >= DISPLAY_ACTIONS){
		cout << "dual solution" << endl;  
	}  

	// Abrindo arquivo de entrada
	dualSol.open(dualSolName); 

	if (!dualSol) {
		cout << "Unable to open file";
		exit(1); // terminate with error
	}


	dualSol >> auxCRead; // Descartando o custo 
	// cout << "Discard cost: " << auxCRead << endl;

	dualSol >> auxCRead; // Descartando o tempo gasto 
	// cout << "Discard time: " << auxCRead << endl;

	dualSol >> auxCRead; // Descartando a tag de otimalidade 
	// cout << "Discard optimal: " << auxCRead << endl;


	// indica o valor de v de cada cliente
	v_values = (double*) malloc((qty_clients) * sizeof(double));
	if(!v_values){
		cout << "Memory Allocation Failed";
		exit(1);
	}

	// Lendo do arquivo os vj de cada cliente no LP primal e salvando no vetor v_values
	for(int j=0;j<qty_clients;j++){
		dualSol >> v_values[j]; 

		if(DEBUG >= DISPLAY_ACTIONS){
			cout << "client " << j << " =  " << v_values[j] << endl;  
		}  
	}

  /*
	Fim ler o arquivo com a solução LP dual
	******************************************/
	

	// Chamando a funcao que resolve o problema de fato
	solution = randRounding(qty_facilities, qty_clients, costF, costA, x_values, v_values, seed);


	// Finalizando a contagem do tempo
	clock_gettime(CLOCK_REALTIME, &finish);

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

	// Fechando os arquivos
	inputFLP.close();
	primalSol.close();
	dualSol.close();
	solutionTXT.close();

	// Liberando memoria
	free(costF);
	free(costA);
	free(v_values);
	free(x_values);

	// return solution;
	return 0;
}
