#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>
#include <cfloat>
#include "definitions.hpp"

#define N_TESTS 10 //25 // Quantidade de vezes que sera testada a mesma entrada, para calculo de minimo, maximo e media

using namespace std;

int main(int argc, char *argv[]){

	if(argc != 2){
		cout << "Error in the parameters. You must enter the input type." << endl;
		return 0;
	}

	cout << fixed;
   	cout.precision(5);

	// Declaracao variavel auxiliar que tera o nome do arquivo a ser utilizado naquela iteracao
	char auxInputName[100]; 

	// Declaracao de variaveis auxiliares para a formacao do arquivo .sol
	char auxSolName[105];
	char dirName[45] = "solutions/";
	char completeAuxSolName[150];

	// Abertura de arquivo para leitura e escrita. 
	// Toda a escrita sera feita no final do arquivo, acrescentando ao conteudo ja existente (app)
	fstream timeLog;
	timeLog.open("timeLog.txt", std::fstream::in | std::fstream::out | std::fstream::app);


	// Arquivo para salvar a solucao
	ofstream solutionTXT;


	// Struct que vai conter a solução retornada
	solutionType currentSolution;

	// Struct que vai conter a melhor solução até agora
	solutionType bestSolution;
	

	// Declaracao variaveis que indicam o tempo no inicio e fim da execucao
	struct timespec start, finish;

	// Declaracao variavel que marcara o tempo calculado daquela execucao
	double timeSpent;

	// Declaracao variavel que indica o minimo de tempo gasto para executar aquela entrada entre os N_TESTS. 
	double minTime;

	// Declaracao variavel que indica o maximo de tempo gasto para executar aquela entrada entre os N_TESTS
	double maxTime;	

	// Declaracao variavel que indica a media de tempo gasto para executar aquela entrada entre os N_TESTS
	double avgTime;


	// Declaracao variavel que indica o minimo de custo da solucao retornado daquela entrada entre os N_TESTS. 
	double minCost;

	// Declaracao variavel que indica o maximo de custo da solucao retornado daquela entrada entre os N_TESTS. 
	double maxCost;	

	// Declaracao variavel que indica a media de custo da solucao retornado daquela entrada entre os N_TESTS. 
	double avgCost;


	// Declaracao variavel que indica o nome dos arquivos de teste
	char nameInput[50];

	char completeNameInput[150];

	char baseNameInput[50] = "../baseDeTestes/facilityTestCases/tests/";
	cout << "BASE: "<< baseNameInput << endl;


	// Declaracao variavel que indica o nome dos arquivos com a solução do LP primal
	char dirNamePrimalSol[45] = "primalLPSolutions/";
	char completeNamePrimalSol[150];

	// Declaracao variavel que indica o nome dos arquivos com a solução do LP dual
	char dirNameDualSol[45] = "dualLPSolutions/";
	char completeNameDualSol[150];


	if(strcmp(argv[1],"1") == 0){
		strcpy(nameInput,"testCases1.txt");
	}
	else if(strcmp(argv[1], "2") == 0){
		strcpy(nameInput,"testCases2.txt");
	}
	else{
		cout << "Error in the parameter indicating the input type." << endl;
		return 0;
	}

	// Abertura para leitura de arquivo que contem uma lista com todos os arquivos de entrada
	ifstream inputName(nameInput); 


	// Enquanto houverem entradas nao testadas ainda
	while(!inputName.eof()){

		// Composicao da string que contera a chamada para execucao do programa com o parametro da entrada correspondente
		inputName >> auxInputName;
		cout << endl << "Input to use: " <<  auxInputName << "\n" << endl;


		//Abertura de arquivo para leitura e escrita do .sol .
		strcpy(auxSolName,dirName);
		strcat(auxSolName,auxInputName);
		strcat(auxSolName,".sol");
		solutionTXT.open(auxSolName, std::ofstream::out | std::ofstream::trunc);

		// Criando a string do caminho completo do arquivo
		strcpy(completeNameInput,baseNameInput);
		strcat(completeNameInput,auxInputName);

		// Criando a string do caminho completo do arquivo com a solucao LP primal
		strcpy(completeNamePrimalSol,dirNamePrimalSol);
		strcat(completeNamePrimalSol,auxInputName);
		strcat(completeNamePrimalSol,".sol");

		// Criando a string do caminho completo do arquivo com a solucao LP dual
		strcpy(completeNameDualSol,dirNameDualSol);
		strcat(completeNameDualSol,auxInputName);
		strcat(completeNameDualSol,".sol");

		// Colocando no timeLog.txt o nome da proxima entrada a ser testada
		timeLog << auxInputName << endl;

		// Maior valor aceito por double colocado como inicio
		minTime = DBL_MAX;
		minCost = DBL_MAX; 

		// Inicia com 0, o minimo de tempo possivel
		maxTime = 0;
		maxCost = 0;

		// Inicia com 0, ja que sera incrementado a cada iteracao com seu valor gasto
		avgTime = 0;
		avgCost = 0;


		// Executa o programa com a mesma entrada N_TESTS vezes, para calculo de minimo, maximo e media de tempo
		for(int i = 0; i < N_TESTS; i++){

			//Iniciando a contagem do tempo
			clock_gettime(CLOCK_REALTIME, &start);

			// Chamando o programa a ser cronometrado
			currentSolution = handlesInput(completeNameInput, completeNamePrimalSol, completeNameDualSol, argv[1],i);

			// Finalizando a contagem do tempo
			clock_gettime(CLOCK_REALTIME, &finish);

			// Calculando o tempo gasto
			timeSpent =  (finish.tv_sec - start.tv_sec);
			timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
			cout << "Time spent: " << timeSpent << " seconds" << endl;

			// Atualizando valores de minimo e maximo do tempo
			if(timeSpent > maxTime)
				maxTime = timeSpent;
			if(timeSpent < minTime)
				minTime = timeSpent;

			// Atualizando valores de minimo e maximo do custo
			if(currentSolution.finalTotalCost > maxCost)
				maxCost = currentSolution.finalTotalCost;

			if(currentSolution.finalTotalCost < minCost){
				minCost = currentSolution.finalTotalCost;

				bestSolution = currentSolution; // atualizando a melhor solucao
				// bestSolution.timeSpent = timeSpent; // salvando o tempo gasto.. isso sera util se eu resolver colocar na tabela o tempo da solucao especifica, nao o tempo medio
			}

			// Acrescentando no timeLog.txt o tempo gasto nessa iteracao e o custo da solucao
			timeLog << i << " - Time spent: " << timeSpent << " - Solution cost: " << currentSolution.finalTotalCost << endl;

			// Incrementando valor que sera utilizado para calculo da media do tempo e do custo
			avgTime += timeSpent;
			avgCost += currentSolution.finalTotalCost;
		}

		// Calculando de fato a media do tempo gasto e do custo da solucao
		avgTime = avgTime/N_TESTS;
		avgCost = avgCost/N_TESTS;

		// Colocando no solutionTXT o valor minimo da solucao
		solutionTXT << fixed << setprecision(5) << minCost << " ";

		// Colocando no solutionTXT o valor medio da solucao
		solutionTXT << avgCost << " ";

		// Colocando no solutionTXT o valor maximo da solucao
		solutionTXT << maxCost << " ";
		
		// Colocando no solutionTXT o tempo medio gasto 
		solutionTXT << avgTime;

		// Colocando no solutionsTXT as instalacoes finais conectadas
		for(int i=0; i < bestSolution.qty_clients; i++){
			solutionTXT << " " << bestSolution.assigned_facilities[i];
		}

		// Colocando no timeLog.txt o valor minimo, maximo e medio de tempo gasto e do custo da solucao nas execucoes dessa entrada
		timeLog << "Time - Min: " << minTime << endl << "Max: " << maxTime << endl << "Avg: " << avgTime << endl << endl;
		timeLog << "Cost - Min: " << minCost << endl << "Max: " << maxCost << endl << "Avg: " << avgCost << endl << endl;

		cout << "\nBEST FINAL TOTAL COST: " << bestSolution.finalTotalCost << "\n"<< endl;

		solutionTXT.close();
	}

	timeLog.close();

	return 0;
}