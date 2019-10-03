#include <iostream>
#include <cstdlib>
#include <fstream>
#include "definitions.hpp"
#define EPS 0.001

using namespace std;

solutionType handlesInput(char inputName[], char primalSolName[], char dualSolName[], char inputType[]){

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

	int debug = 0; // OPCOES DE DEBUG: 0 PARA NAO EXIBIR NADA, 1 PARA EXIBIR AS INFORMACOES SENDO SALVAS

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

		cout << "QTY FACILITIES: " << qty_facilities << " E QTY CLIENTS: " << qty_clients << endl;

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

			if(debug > 0){
				cout << "Fi =  " << costF[i] << endl;  
			}  
		}


		// Lendo do arquivo os custos de atribuicao do clientes com as instalacoes e salvando no vetor costA
		counter = 0;
		for(int i=0;i<qty_clients;i++){
			inputFLP >> auxRead; // Descartando aqui a demanda do cliente

			if(debug > 0){
				cout << "Demand = " << auxRead << endl;
			}

			for(int j=0;j<qty_facilities;j++){
				inputFLP >> auxRead;
				costA[counter] = auxRead;

				if(debug > 0){
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

		cout << "QTY FACILITIES: " << qty_facilities << " E QTY CLIENTS: " << qty_clients << endl;

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

			if(debug > 0){
				cout << "Fi =  " << costF[i] << endl;  
			}  

			// Lendo do arquivo os custos de atribuicao do clientes com as instalacoes e salvando no vetor costA
			for(int j=0;j<qty_clients;j++){

				inputFLP >> auxRead;
				costA[i + j * qty_facilities] = auxRead;       // Esse indice estranho eh para deixa no mesmo formato que o caso ORLIB

				if(debug > 0){
					cout << "AC = " << costA[i + j * qty_facilities] << endl; 
				}
			}
		}
	}
	else{
		cout << "Invalid input type." << endl;
		return solution;
	}

  /*
	Fim ler o arquivo de entrada
	******************************************/


	/**************************************
	Lendo o arquivo com a solução LP primal
	*/

	if(debug > 0){
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

			if(debug > 0){
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

	if(debug > 0){
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

		if(debug > 0){
			cout << "client " << j << " =  " << v_values[j] << endl;  
		}  
	}

  /*
	Fim ler o arquivo com a solução LP dual
	******************************************/
	

	// Chamando a funcao que resolve o problema de fato
	solution = randRounding(qty_facilities, qty_clients, costF, costA, x_values, v_values);


	// Fechando os arquivos
	inputFLP.close();
	primalSol.close();
	dualSol.close();

	// Liberando memoria
	free(costF);
	free(costA);
	free(v_values);
	free(x_values);

	return solution;
}
