#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>

#define N_TESTES 1//25 // Quantidade de vezes que sera testada a mesma entrada, para calculo de minimo, maximo e media

using namespace std;

int main(int argc, char *argv[]){

	// Abertura para leitura de arquivo que contem uma lista com todos os arquivos de entrada
	ifstream inputName("testCases.txt"); 

	// Declaracao variavel auxiliar que tera o nome do arquivo a ser utilizado naquela iteracao
	string auxInputName; 

	// Abertura de arquivo para leitura e escrita. 
	// Toda a escrita sera feita no final do arquivo, acrescentando ao conteudo ja existente (app)
	fstream timeLog;
	timeLog.open("timeLog.txt", std::fstream::in | std::fstream::out | std::fstream::app);


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


	// Enquanto houverem entradas nao testadas ainda
	while(!inputName.eof()){

		// Composicao da string que contera a chamada para execucao do programa com o parametro da entrada correspondente
		stringstream stringBuilder;
		inputName >> auxInputName;
		stringBuilder << "./main facilityTestCases/" << auxInputName;
		string completeString = stringBuilder.str();
		cout << endl << "Entrada a ser utilizada: " <<  auxInputName << endl;

		// Colocando no timeLog.txt o nome da proxima entrada a ser testada
		timeLog << auxInputName << endl;

		// Valor alto colocado como inicio, ideal que fosse algo que houvesse certeza que nunca ocorreria.
		minTime = 1000000;

		// Inicia com 0, o minimo de tempo possivel
		maxTime = 0;

		// Inicia com 0, ja que sera incrementado a cada iteracao com seu valor gasto
		avgTime = 0;


		// Executa o programa com a mesma entrada N_TESTES vezes, para calculo de minimo, maximo e media de tempo
		for(int i = 0; i < N_TESTES; i++){

			//Iniciando a contagem do tempo
			clock_gettime(CLOCK_REALTIME, &start);

			// Chamando o programa a ser cronometrado
			int retCode = system(completeString.c_str());

			// Finalizando a contagem do tempo
			clock_gettime(CLOCK_REALTIME, &finish);

			// Calculando o tempo gasto
			timeSpent =  (finish.tv_sec - start.tv_sec);
			timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 
			cout << "Time spent: " << timeSpent << " seconds" << endl;

			// Atualizando valores de minimo e maximo
			if(timeSpent > maxTime)
				maxTime = timeSpent;
			if(timeSpent < minTime)
				minTime = timeSpent;

			// Acrescentando no timeLog.txt o tempo gasto nessa iteracao
			timeLog << i << ": " << timeSpent << endl;

			// Incrementando valor que sera utilizado para calculo da media
			avgTime += timeSpent;
		}

		// Calculando de fato a media do tempo gasto
		avgTime = avgTime/N_TESTES;

		// Colocando no timeLog.txt o valor minimo, maximo e medio de tempo gasto nas execucoes dessa entrada
		timeLog << "Min: " << minTime << endl << "Max: " << maxTime << endl << "Avg: " << avgTime << endl << endl;

	}

	return 0;
}