#include <algorithm>
#include <iostream>
#include <set>
#include <iterator>
#include <ctime>
#include <fstream>
#include <cstring>
#include <cfloat>
#include "definitions.hpp"

using namespace std;

#define DISPLAY_BASIC 1 // corresponde a exibicao da quantidade de movimentos
#define DISPLAY_MOVES 2 // corresponde a todos os cout quando um movimento é realizado de fato
#define DISPLAY_ACTIONS 3 // corresponde a todos os cout quando uma acao é feita. 
#define DISPLAY_DETAILS 4 // corresponde a todos os cout mais detalhados quando uma acao é feita. 
#define DISPLAY_TIME 5 // corresponde aos calculos de tempo 

#define DEBUG 2 // OPCOES DE DEBUG: 1 - MOSTRAR A QTD DE MOVIMENTOS, 2 PARA EXIBIR OS MOVIMENTOS REALIZADOS, 3 PARA EXIBIR ACOES, 4 PARA EXIBIR DETALHES DAS ACOES, 5 PARA EXIBIR TEMPO

// Retornar o valor da solucao
solutionType LSCloseFac(char * solutionName, int qty_facilities, int qty_clients, double * costF, double ** assignment_cost, solutionType solution){

	cout << fixed;
   	cout.precision(5);

	// conjunto de instalacoes que encontram-se abertas no momento
	set <int, greater <int> > open_facilities; 

	// Represents the amount of open facilities
	int n1 = 0;

	// Iniciando o conjunto de instalacoes abertas
	for(int i=0;i<qty_facilities;i++){
		if(solution.open_facilities[i]){
			open_facilities.insert(i);
			n1 += 1;
		}
	}

	// Se só tiver 1 instalação aberta, então podemos encerrar porque nossos movimentos são apenas de fechar, e devemos ter ao menos 1 inst aberta
	if(n1 > 1){ // entao realmente executaremos o programa


		/* Inicio declaracoes variaveis para calculo de tempo */

		// Declaracao variaveis que indicam o tempo da funcao
		struct timespec start, finish, time_so_far;

		/* Fim declaracoes para calculo de tempo */


		// Variavel que armazena o maior valor cij dado na entrada, para uso posterior
		double biggestCij = 0;
		

		// Variavel que marca quantas movimentacoes foram tentadas
		int qty_moves = 0;


		// nearest2_open_fac - será correnpondente ao ID da segunda instalacao i tal que min_{i \in X} cij, sendo X as inst abertas exceto a nearest_open_fac (segunda instalacao aberta mais proxima)
		int * nearest2_open_fac = (int *)malloc(qty_clients * sizeof(int));

		// temp_nearest_fac - será correnpondente ao ID da instalacao i mais proxima que sera conectada (temporario)
		int * temp_nearest_fac = (int *)malloc(qty_clients * sizeof(int));

		// temp_nearest2_fac - será correnpondente ao ID da segunda instalacao i mais proxima que sera conectada (temporario)
		int * temp_nearest2_fac = (int *)malloc(qty_clients * sizeof(int));

		if((!nearest2_open_fac)||(!temp_nearest_fac)||(!temp_nearest2_fac)){
	        cout << "Memory Allocation Failed";
	        exit(1);
	    }

		
		// Iteratores para os conjuntos 
		set <int, greater <int> > :: iterator itr;
		// Iteratores para os conjuntos 
		set <int, greater <int> > :: iterator itr2;


		// Serao utilizados para acessar o vetor extra_cost, funciona como modulo 2
		int cur_index_extra = 0;
		int old_index_extra = 0;

		// extra_cost - será o delta z para cada iteracao, para cada instalacao
		double **extra_cost = (double**) malloc((2) * sizeof(double*));

		for(int i = 0; i < 2; i++) {
			extra_cost[i] = (double *)malloc(qty_facilities * sizeof(double));
		}

	    if(!extra_cost){
	        cout << "Memory Allocation Failed";
	        exit(1);
	    }

		if(DEBUG >= DISPLAY_BASIC){
			cout << "Local Search Close Facilities - BEST FIT" << endl;
		}




		// // Declaracao de variavel auxiliar para formacao do arquivo .log
		// char completeLogSolName[250] = "";

		// // Declaracao de variavel auxiliar para formacao do arquivo .log
		// char completeLogDetailName[300] = "";

		// // Arquivo para salvar o log da solucao
		// ofstream solLog;
		// strcat(completeLogSolName,solutionName);
		// strcat(completeLogSolName,".log");
		// solLog.open(completeLogSolName, std::ofstream::out | std::ofstream::trunc);

		// // Arquivo para salvar o log detail da solucao
		// ofstream logDetail;
		// strcat(completeLogDetailName,solutionName);
		// strcat(completeLogDetailName,".log_detail");
		// logDetail.open(completeLogDetailName, std::ofstream::out | std::ofstream::trunc);

		// //Salvando o cabecalho
		// solLog << "time spent so far, current solution cost, current qty moves" << endl;

		// //Salvando o cabecalho
		// logDetail << "time spent so far, current solution cost, current qty moves" << endl;



		for(int i=0;i<qty_clients;i++){
			temp_nearest_fac[i] = -1; // indica que nao há nenhuma inst temporaria ainda
		}


		// Salvando o valor do maior Cij da entrada e iniciando o nearest2_open_fac
		for(int i=0;i<qty_clients;i++){
			nearest2_open_fac[i] = -1;
			for(int j=0;j<qty_facilities;j++){
				if(assignment_cost[i][j] > biggestCij){
					biggestCij = assignment_cost[i][j];
				}
			}
		}

		if(DEBUG >= DISPLAY_DETAILS){
			cout << "biggestCij from input : " << biggestCij << endl;
		}

		double aux_cij;

		// Iniciando os labels correspondentes à segunda instalacao aberta mais proxima de cada cliente (nearest2_open_fac)
		for(int i=0;i<qty_clients;i++){ // percorre todos os clientes
			for (itr = open_facilities.begin(); itr != open_facilities.end(); ++itr) { // percorrer todas as inst abertas
				aux_cij = assignment_cost[i][*itr];

				// se nao for o melhor de todos
				if(*itr != solution.assigned_facilities[i]){

					// Se nao tiver ninguem lá salvo ainda, entao coloca
					if(nearest2_open_fac[i] == -1){
						nearest2_open_fac[i] = *itr;
					}
					// senao, se for melhor que o salvo ate agr, atualiza
					else if(aux_cij < assignment_cost[i][nearest2_open_fac[i]]){
						nearest2_open_fac[i] = *itr;
					}
				}
			}

			if(DEBUG >= DISPLAY_DETAILS){
				cout << "Client " << i << " c1: " << assignment_cost[i][solution.assigned_facilities[i]] << " near: " << solution.assigned_facilities[i] << " c2: " << assignment_cost[i][nearest2_open_fac[i]] << " near2: " << nearest2_open_fac[i] << endl;
			}
		}


		// Best delta in this iteration
		double best_extra_cost;

		// Which facility corresponds to the best delta in this iteration
		int fac_best_extra_cost;

		// Indica se continua ou nao o loop de busca (continua até achar o local optimum)
		solution.local_optimum = false;


		// Auxiliares que indicam a terceira inst mais proxima (c3_minX = custo, nearest3_open_fac = id, aux_cij3 = auxiliar de custo)
		double c3_minX = -1;
		int nearest3_open_fac = -1;
		double aux_cij3 = -1;


		/* 
		STEP 0 - algumas coisas ja foram feitas antes
		*/

		// Inicializando o vetor extra_cost[0] correspondente a delta z
		for(int i=0;i<qty_facilities;i++){		// percorre as instalacoes
			if(solution.open_facilities[i]){ // se a instalacao está aberta, vamos ver a mudanca se fechar
				extra_cost[0][i] = -costF[i];
				for (int j=0;j<qty_clients;j++) { // Percorre todos os clientes)
					if(solution.assigned_facilities[j] == i){ // se essa inst for a mais proxima desse cli
						extra_cost[0][i] += assignment_cost[j][nearest2_open_fac[j]] - assignment_cost[j][i];
					}
				}
			}
			else{ // se a instalacao está fechada, entao a mudança deve ser infinito, pois não queremos mudar isso nunca
				extra_cost[0][i] = DBL_MAX;
			}
		}


		// CHECANDO A CONTAGEM DE TEMPO GASTO ATÉ AGORA
		clock_gettime(CLOCK_REALTIME, &time_so_far);

		// Calculando o tempo gasto até agora
		solution.timeSpent =  (time_so_far.tv_sec - start.tv_sec);
		solution.timeSpent += (time_so_far.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 


		// // Acrescentando no solLog.txt o tempo e o custo inicial
		// solLog << solution.timeSpent << "," << solution.finalTotalCost << "," << qty_moves << endl;

		// // Acrescentando no logDetail.txt o tempo e o custo inicial
		// logDetail << solution.timeSpent << "," << solution.finalTotalCost << "," << qty_moves << endl;

		// INICIANDO A CONTAGEM DE TEMPO DA FUNCAO
		clock_gettime(CLOCK_REALTIME, &start);

		// A partir daqui é loop até acabar a busca
		while(!solution.local_optimum){

			if(DEBUG >= DISPLAY_MOVES){
				cout << endl << "-------------------------- NEXT MOVE " << qty_moves << " ---------------------------" << endl << endl;
			}

			/* 
			STEP 1
			*/

			// Select a facility that has de minimum extra_cost (and extra_cost is not DBL_MAX ---> invalid)
			best_extra_cost = DBL_MAX; // limitante superior, maior double possivel
			fac_best_extra_cost = -1; // indica invalidez
			if(n1 > 1){ // se nao tiver só uma instalacao aberta
				for(itr = open_facilities.begin(); itr != open_facilities.end(); ++itr) { // percorrer todas as inst abertas
					if(extra_cost[cur_index_extra][*itr] < best_extra_cost){ // se essa for menor do que a ja encontrada ate agr, atualiza
						best_extra_cost = extra_cost[cur_index_extra][*itr];
						fac_best_extra_cost = *itr;
					}
				}
			}
			
			if(DEBUG >= DISPLAY_MOVES){
				cout << "Facility " << fac_best_extra_cost << " best delta extra cost: " << best_extra_cost << endl;
			}

			if(best_extra_cost >= 0){ // siginifica que estamos em um otimo local dessa vizinhança, nenhum movimento de close fac melhor a solucao
				if(DEBUG >= DISPLAY_BASIC){
					cout << "Stop criterion: Local optimum! THERE ARE NO MOVES ANYMORE" << endl;
				}
				solution.local_optimum = true;
			}
			else{
				if(DEBUG >= DISPLAY_MOVES){
					if(solution.open_facilities[fac_best_extra_cost]){ // se a instalacao está aberta
						cout << "We want to close it" << endl;
					}
					else{ // se a instalacao está fechada
						cout << "We should't be here. We would like to open it, but we won't" << endl;
						break;
					}
				}

				// Aumentando a contagem de movimentos
				qty_moves += 1;

				// Fechando a instalação
				solution.open_facilities[fac_best_extra_cost] = false;

				n1 -= 1;
				open_facilities.erase(fac_best_extra_cost);


				// Atualizando o custo atual
				solution.finalTotalCost += best_extra_cost;

				// Atualizando os indices para acessar o vetor extra_cost
				cur_index_extra = qty_moves % 2;
				old_index_extra = !cur_index_extra;

				// CHECANDO A CONTAGEM DE TEMPO GASTO ATÉ AGORA
				clock_gettime(CLOCK_REALTIME, &time_so_far);

				// Calculando o tempo gasto até agora
				solution.timeSpent =  (time_so_far.tv_sec - start.tv_sec);
				solution.timeSpent += (time_so_far.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 

				if(DEBUG >= DISPLAY_TIME){
					cout << "Total time spent so far: " << solution.timeSpent << " seconds" << endl;
				}

				// // Acrescentando no logDetail.txt o tempo gasto nessa iteracao e o custo da solucao
				// logDetail << solution.timeSpent << "," << solution.finalTotalCost << "," << qty_moves << endl;


				/* 
				Update extra_cost[cur_index_extra]
				*/


				// The facility was opened and we closed

				if(DEBUG >= DISPLAY_ACTIONS){
					cout << "The facility was opened and we closed. Lets update delta extra cost from other facilities" << endl;
				}

				// Open facilities after another facility closed
				for(itr = open_facilities.begin(); itr != open_facilities.end(); ++itr) { // percorrer todas as inst abertas

					if(*itr != fac_best_extra_cost){ // se nao for exatamente a inst que eu estou fechando

						if(DEBUG >= DISPLAY_DETAILS){
							cout << "OPEN FACILITY: " << *itr << endl;
						}

						if(n1 > 1){ // se ela nao eh a unica instalacao aberta 

							extra_cost[cur_index_extra][*itr] = extra_cost[old_index_extra][*itr]; // inicia com o valor da anterior

							for(int j=0;j<qty_clients;j++){ // percorre os clientes

								//  Pegando o custo de atribuicao desse cliente com a inst que estou fechando (cij')
								aux_cij = assignment_cost[j][fac_best_extra_cost];

								// Se for Tj1 ou Tj2
								// Tj1 = {i | di1 = j' ^ di2 = j} 		// Tj2 = {i | di1 = j ^ di2 = j'}
								if(((solution.assigned_facilities[j] == fac_best_extra_cost) && (nearest2_open_fac[j] == *itr)) || ((solution.assigned_facilities[j] == *itr) && (nearest2_open_fac[j] == fac_best_extra_cost))) { // se estou fechando a mais proxima e *itr é a segunda mais proxima (Tj1)
									/*
									Defining nearest3_open_fac e c3_minX
									*/
									c3_minX = biggestCij + 1; // limitante superior tranquilo
									for(itr2 = open_facilities.begin(); itr2 != open_facilities.end(); ++itr2) { // percorrer todas as inst abertas
										aux_cij3 = assignment_cost[j][*itr2];
										if((aux_cij3 < c3_minX) && (*itr2 != solution.assigned_facilities[j]) && (*itr2 != nearest2_open_fac[j])){ // se for melhor que o salvo ate agr, mas nao melhor que o melhor de todos nem o segundo melhor salvo
											nearest3_open_fac = *itr2;
											c3_minX = aux_cij3;
										}
									}
									// incluindo a inst que fechamos na busca, ja q ela nao está mais em open_facilities
									if((c3_minX > biggestCij) || ((aux_cij < c3_minX) && (fac_best_extra_cost != solution.assigned_facilities[j]) && (fac_best_extra_cost != nearest2_open_fac[j]))) {
										nearest3_open_fac = fac_best_extra_cost;
										c3_minX = aux_cij;
									}

									// Se for Tj1
									// Tj1 = {i | di1 = j' ^ di2 = j}
									if((solution.assigned_facilities[j] == fac_best_extra_cost) && (nearest2_open_fac[j] == *itr)){ // se estou fechando a mais proxima e *itr é a segunda mais proxima (Tj1)
										extra_cost[cur_index_extra][*itr] += c3_minX - assignment_cost[j][*itr];

										// Updating d1 --> dj2 (nearest2)
										temp_nearest_fac[j] = nearest2_open_fac[j];
									}
									// Senao, se for Tj2
									// Tj2 = {i | di1 = j ^ di2 = j'}
									else {  // se estou fechando a segunda mais proxima e n2 é a mais proxima (Tj2)
										extra_cost[cur_index_extra][*itr] += c3_minX - aux_cij;

										// saving temp d1
										temp_nearest_fac[j] = solution.assigned_facilities[j];
									}

									// Updating d2 --> d3 (nearest3), tanto para Tj1 quanto Tj2
									temp_nearest2_fac[j] = nearest3_open_fac;
								}
							}
						}

						else { // se ela eh a unica instalacao aberta
							// if(DEBUG >= DISPLAY_ACTIONS){
								cout << "It was the only one open facility. We cannot define a new delta extra cost. Fac: " << *itr << endl;
							// }
							/* 
							Not possible to have extra_cost because this facility cannot be closed
							*/

							extra_cost[cur_index_extra][*itr] = DBL_MAX; // indica invalidez

							// Entao chegamos em um minimo local, ja que só tenho movimentos de fechar instalacao e nao posso fazer aqui
							solution.local_optimum = true;

							for(int j=0;j<qty_clients;j++) { // Percorre todos os clientes)
								// colocando ela como inst mais proxima de todos os clientes
								temp_nearest_fac[j] = *itr;

								// indicando invalidez
								temp_nearest2_fac[j] = -1;
							}
						}
					}
				}

				// Updating extra_cost for this chosen facility -> INF
				extra_cost[cur_index_extra][fac_best_extra_cost] = DBL_MAX;

				/*
				UPDATING NEAREST AND NEAREST 2
				*/

				for(int j=0; j<qty_clients; j++){ // percorre os clientes
					if(temp_nearest_fac[j]!= -1){ // se houve alguma mudanca com esse cliente
	 					solution.assigned_facilities[j] = temp_nearest_fac[j]; // salvando alteracoes sobre a inst mais proxima na solucao final

	 					nearest2_open_fac[j] = temp_nearest2_fac[j];

	 					temp_nearest_fac[j] = -1; // resetando
					}
				}
			}
		}

		// FINALIZANDO A CONTAGEM DE TEMPO DA FUNCAO
		clock_gettime(CLOCK_REALTIME, &finish);

		if(DEBUG >= DISPLAY_MOVES){
			cout << "FINAL TOTAL COST: " << solution.finalTotalCost << endl;
		}

		// Exibir quais instalacoes foram abertas
		if(DEBUG >= DISPLAY_ACTIONS){
			cout << "OPEN FACILITIES: " << endl;
			for (itr = open_facilities.begin(); itr != open_facilities.end(); ++itr) { // percorrer todas as inst abertas
				cout << *itr << " ";
			}
			cout << endl;
		}

		if(DEBUG >= DISPLAY_BASIC){
			cout << "Total moves: " << qty_moves << endl;
		}


		// Calculando o tempo gasto da funcao
		solution.timeSpent =  (finish.tv_sec - start.tv_sec);
		solution.timeSpent += (finish.tv_nsec - start.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior 

		if(DEBUG >= DISPLAY_TIME){
			cout << "Final Total Function Time: " << solution.timeSpent << " seconds" << endl;
		}

		// // Acrescentando no solLog.txt o tempo gasto final da funcao e o custo final da solucao
		// solLog << solution.timeSpent << "," << solution.finalTotalCost << "," << qty_moves << endl;

		// // Acrescentando no logDetail.txt o tempo gasto nessa iteracao e o custo da solucao
		// logDetail << solution.timeSpent << "," << solution.finalTotalCost << "," << qty_moves << endl;

		// solLog.close();
		// logDetail.close();

		free(extra_cost);
	}

	return(solution);
}
