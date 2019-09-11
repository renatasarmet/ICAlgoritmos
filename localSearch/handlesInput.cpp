#include <iostream>
#include <cstdlib>
#include <fstream>
#include "definitions.hpp"
#define EPS 0.001

using namespace std;

solutionType handlesInput(char inputName[], char initialSolName[], char inputType[]){

	// Declaracoes iniciais
	int qty_clients, qty_facilities, counter;
	double auxRead;
	char auxCRead[30];
	ifstream inputFLP;

	double * custoA;
	double * custoF;

	ifstream initialSol;

	solutionType solution;

	int debug = 0; // OPCOES DE DEBUG: 0 PARA NAO EXIBIR NADA, 1 PARA EXIBIR AS INFORMACOES SENDO SALVAS



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
		custoF = (double*) malloc((qty_facilities) * sizeof(double));
		if(!custoF){
			cout << "Memory Allocation Failed";
			exit(1);
		}

		custoA = (double*) malloc((qty_clients*qty_facilities) * sizeof(double));
		if(!custoA){
			cout << "Memory Allocation Failed";
			exit(1);
		}


		// Lendo do arquivo os custos de abertura das instalacoes e salvando no vetor custoF
		for(int i=0;i<qty_facilities;i++){
			inputFLP >> auxCRead; // Descartando a capacidade da instalação
			// cout << "Discard: " << auxCRead << endl;
			inputFLP >> auxRead; // Pegando o valor do custo de abertura da instalação
			custoF[i] = auxRead;

			if(debug > 0){
				cout << "Fi =  " << custoF[i] << endl;  
			}  
		}


		// Lendo do arquivo os custos de atribuicao do clientes com as instalacoes e salvando no vetor custoA
		counter = 0;
		for(int i=0;i<qty_clients;i++){
			inputFLP >> auxRead; // Descartando aqui a demanda do cliente

			if(debug > 0){
				cout << "Demanda = " << auxRead << endl;
			}

			for(int j=0;j<qty_facilities;j++){
				inputFLP >> auxRead;                                 // PROBLEMA: Pq só está pegando 2 casas decimais???
				custoA[counter] = auxRead;

				if(debug > 0){
					cout << "CA = " << custoA[counter] << endl; 
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

		cout << "QTD INST: " << qty_facilities << " E QTD CLI: " << qty_clients << endl;

		// Vetores que salvarao custos lidos no arquivo
		custoF = (double*) malloc((qty_facilities + 1) * sizeof(double));
		if(!custoF){
			cout << "Memory Allocation Failed";
			exit(1);
		}

		custoA = (double*) malloc((qty_clients*qty_facilities + 1) * sizeof(double));
		if(!custoA){
			cout << "Memory Allocation Failed";
			exit(1);
		}


		// Lendo do arquivo os custos de abertura das instalacoes e salvando no vetor custoF
		for(int i=0;i<qty_facilities;i++){
			inputFLP >> auxRead; // Descartando o nome da instalação
			// cout << "DESCARTE:" << auxRead << endl;
			inputFLP >> auxRead; // Pegando o valor do custo de abertura da instalação
			custoF[i] = auxRead;

			if(debug > 0){
				cout << "Fi =  " << custoF[i] << endl;  
			}  

			// Lendo do arquivo os custos de atribuicao do clientes com as instalacoes e salvando no vetor custoA
			for(int j=0;j<qty_clients;j++){

				inputFLP >> auxRead;                                 // PROBLEMA: Pq só está pegando 2 casas decimais???
				custoA[i + j * qty_facilities] = auxRead;       // Esse indice estranho eh para deixa no mesmo formato que o caso ORLIB

				if(debug > 0){
					cout << "CA = " << custoA[i + j * qty_facilities] << endl; 
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
	Lendo o arquivo com a solução inicial
	*/

	// Abrindo arquivo de entrada
	initialSol.open(initialSolName); 

	if (!initialSol) {
		cout << "Unable to open file";
		exit(1); // terminate with error
	}


	initialSol >> auxCRead; // Descartando o custo minimo
	// cout << "Discard min: " << auxCRead << endl;

	// Lendo do arquivo o valor que indica o custo total
	initialSol >> solution.finalTotalCost; 
	cout << "Initial total cost: " << solution.finalTotalCost << endl;

	initialSol >> auxCRead; // Descartando o custo maximo
	// cout << "Discard max: " << auxCRead << endl;

	initialSol >> auxCRead; // Descartando o tempo gasto
	// cout << "Discard time: " << auxCRead << endl;


	// indica as instalacoes iniciais atribuidas a cada cliente
	solution.assigned_facilities = (int*) malloc((qty_clients) * sizeof(int));
	if(!solution.assigned_facilities){
		cout << "Memory Allocation Failed";
		exit(1);
	}


	// Lendo do arquivo as instalacoes iniciais atribuidas a cada cliente e salvando no vetor solution.assigned_facilities
	for(int i=0;i<qty_clients;i++){
		initialSol >> solution.assigned_facilities[i]; 

		if(debug > 0){
			cout << "cliente " << i << " =  " << solution.assigned_facilities[i] << endl;  
		}  
	}

  /*
	Fim ler o arquivo com a solução inicial
	******************************************/
	

	// salvando a quantidade de clientes em solution
	solution.qty_clients = qty_clients;

	// Chamando a funcao que resolve o problema de fato
	solution = localSearch(qty_facilities, custoF, custoA, solution);


	// Fechando o arquivo
	inputFLP.close();
	initialSol.close();
	free(custoF);
	free(custoA);
	return solution;
}
