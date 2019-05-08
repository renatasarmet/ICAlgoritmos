#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>
#include "declaracoes.hpp"

#define N_TESTES 1//25 // Quantidade de vezes que sera testada a mesma entrada, para calculo de minimo, maximo e media

using namespace std;

int main(int argc, char *argv[]){

	if(argc != 2){
		cout << "Erro nos parametros. Necessario passar o tipo de entrada." << endl;
		return 0;
	}

	// Declaracao variavel auxiliar que tera o nome do arquivo a ser utilizado naquela iteracao
	char auxInputName[100]; 

	// Abertura de arquivo para leitura e escrita. 
	// Toda a escrita sera feita no final do arquivo, acrescentando ao conteudo ja existente (app)
	fstream timeLog;
	timeLog.open("timeLog.txt", std::fstream::in | std::fstream::out | std::fstream::app);

	// Arquivo para salvar as solucoes 
	ofstream solutionsCSV;
	


	// Declaracao variaveis que indicam o tempo no inicio e fim da execucao
	struct timespec start, finish;

	// Declaracao variavel que marcara o tempo calculado daquela execucao
	double timeSpent;

	// Declaracao variavel que indica o minimo de tempo gasto para executar aquela entrada entre os N_TESTES. 
	double minTime;

	// Declaracao variavel que indica o maximo de tempo gasto para executar aquela entrada entre os N_TESTES
	double maxTime;	

	// Declaracao variavel que indica a media de tempo gasto para executar aquela entrada entre os N_TESTES
	double avgTime;


	// Declaracao variavel que guardara o custo da solucao retornado naquela execucao
	double cost;

	// Declaracao variavel que indica o minimo de custo da solucao retornado daquela entrada entre os N_TESTES. 
	double minCost;

	// Declaracao variavel que indica o maximo de custo da solucao retornado daquela entrada entre os N_TESTES. 
	double maxCost;	

	// Declaracao variavel que indica a media de custo da solucao retornado daquela entrada entre os N_TESTES. 
	double avgCost;


	// Declaracao variavel que indica o nome dos arquivos de teste
	char nameInput[50];

	char completeNameInput[150];

	char baseNameInput[50] = "facilityTestCases/tests/";
	cout << "BASE: "<< baseNameInput << endl;

	if(strcmp(argv[1],"1") == 0){
		strcpy(nameInput,"testCases1.txt");
		//Abertura de arquivo para leitura e escrita.
		solutionsCSV.open("solutions1.csv", std::ofstream::out | std::ofstream::trunc);
	}
	else if(strcmp(argv[1], "2") == 0){
		strcpy(nameInput,"testCases2.txt");
		//Abertura de arquivo para leitura e escrita.
		solutionsCSV.open("solutions2.csv", std::ofstream::out | std::ofstream::trunc);
	}
	else{
		cout << "Erro no parametro que indica o tipo de entrada." << endl;
		return 0;
	}

	// Abertura para leitura de arquivo que contem uma lista com todos os arquivos de entrada
	ifstream inputName(nameInput); 


	// Enquanto houverem entradas nao testadas ainda
	while(!inputName.eof()){

		// Composicao da string que contera a chamada para execucao do programa com o parametro da entrada correspondente
		inputName >> auxInputName;
		cout << endl << "Entrada a ser utilizada: " <<  auxInputName << endl;

		// Criando a string do caminho completo do arquivo
		strcpy(completeNameInput,baseNameInput);
		strcat(completeNameInput,auxInputName);

		// Colocando no timeLog.txt o nome da proxima entrada a ser testada
		timeLog << auxInputName << endl;

		// Colocando no solutionsCSV o nome da proxima entrada a ser testada
		solutionsCSV << auxInputName << ",";

		// Valor alto colocado como inicio, ideal que fosse algo que houvesse certeza que nunca ocorreria.
		minTime = 1000000000000000000;
		minCost = 1000000000000000000; //maior valor aceito por double (?)

		// Inicia com 0, o minimo de tempo possivel
		maxTime = 0;
		maxCost = 0;

		// Inicia com 0, ja que sera incrementado a cada iteracao com seu valor gasto
		avgTime = 0;
		avgCost = 0;


		// Executa o programa com a mesma entrada N_TESTES vezes, para calculo de minimo, maximo e media de tempo
		for(int i = 0; i < N_TESTES; i++){

			//Iniciando a contagem do tempo
			clock_gettime(CLOCK_REALTIME, &start);

			// Chamando o programa a ser cronometrado
			cost = trataInput(completeNameInput, argv[1]);

			cout <<"EH"<< cost << endl;

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
			if(cost > maxCost)
				maxCost = cost;
			if(cost < minCost)
				minCost = cost;

			// Acrescentando no timeLog.txt o tempo gasto nessa iteracao e o custo da solucao
			timeLog << i << " - Time spent: " << timeSpent << " - Solution cost: " << cost << endl;

			// Incrementando valor que sera utilizado para calculo da media do tempo e do custo
			avgTime += timeSpent;
			avgCost += cost;
		}

		// Calculando de fato a media do tempo gasto e do custo da solucao
		avgTime = avgTime/N_TESTES;
		avgCost = avgCost/N_TESTES;

		// Colocando no solutionsCSV o valor medio da solucao
		solutionsCSV << avgCost << ",";

		// Colocando no solutionsCSV o tempo medio gasto 
		solutionsCSV << avgTime << endl;
		
		// Colocando no timeLog.txt o valor minimo, maximo e medio de tempo gasto e do custo da solucao nas execucoes dessa entrada
		timeLog << "Time - Min: " << minTime << endl << "Max: " << maxTime << endl << "Avg: " << avgTime << endl << endl;
		timeLog << "Cost - Min: " << minCost << endl << "Max: " << maxCost << endl << "Avg: " << avgCost << endl << endl;

	}

	return 0;
}