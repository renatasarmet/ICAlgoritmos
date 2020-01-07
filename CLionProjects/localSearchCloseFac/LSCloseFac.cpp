#include "LSCloseFac.h"

#include <algorithm>
#include <iostream>
#include <set>
#include <iterator>
#include <ctime>
#include <fstream>
#include <cstring>
#include <cfloat>

using namespace std;

#define DISPLAY_BASIC 1 // corresponde a exibicao da quantidade de movimentos
#define DISPLAY_MOVES 2 // corresponde a todos os cout quando um movimento é realizado de fato
#define DISPLAY_ACTIONS 3 // corresponde a todos os cout quando uma acao é feita.
#define DISPLAY_TIME 4 // corresponde aos calculos de tempo
#define DISPLAY_DETAILS 5 // corresponde a descricao dos clientes, instalacoes e arcos

#define DEBUG 0 // OPCOES DE DEBUG: 1 - MOSTRAR A QTD DE MOVIMENTOS, 2 PARA EXIBIR OS MOVIMENTOS REALIZADOS, 3 PARA EXIBIR ACOES, 4 PARA EXIBIR TEMPO, 5 PARA EXIBIR DETALHES

#define TIME_LIMIT 900 //15 minutos

#define TIME_COUNTER_STEP 5 // 5 segundos para testar as inst ga250a //30 // 30 segundos. Isso indicara que vai salvar a melhor solucao encontrada a cada minuto


// Alocando memoria e inicializando valores
void LSCloseFac::initialize(Solution *solution) {

    qty_facilities = solution->getQtyFacilities();
    qty_clients = solution->getQtyClients();

    // Represents the amount of open facilities
    n1 = 0;

    // Variavel que armazena o maior valor cij dado na entrada, para uso posterior
    biggest_cij = 0;

    // Variavel que marca quantas movimentacoes foram feitas de fato
    qty_moves = 0;

    nearest2_open_fac = new int[qty_clients];
    temp_nearest_fac = new int[qty_clients];
    temp_nearest2_fac = new int[qty_clients];

    // Iniciando o conjunto de instalacoes abertas
    for(int j=qty_facilities-1;j>=0;--j){
        if(solution->getOpenFacilityJ(j)){
            open_facs.insert(j);
            n1 += 1;
        }
    }

    // Serao utilizados para acessar o vetor extra_cost, funciona como modulo 2
    cur_index_extra = 0;
    old_index_extra = 0;

    // extra_cost - será o delta z para cada iteracao, para cada instalacao
    extra_cost = new double*[2];
    for(int i = 0; i < 2; i++) {
        extra_cost[i] = new double[qty_facilities];
    }

    for(int i=qty_clients-1;i>=0;--i){
        temp_nearest_fac[i] = -1; // indica que nao há nenhuma inst temporaria ainda
        nearest2_open_fac[i] = -1;

        for(int j=qty_facilities-1;j>=0;--j){
            // Salvando o valor do maior Cij da entrada
            if(solution->getCostAIJ(i,j) > biggest_cij){
                biggest_cij = solution->getCostAIJ(i,j);
            }
        }
    }

    // Iniciando os labels correspondentes à segunda instalacao aberta mais proxima de cada cliente (c2_minX)
    for(int i=qty_clients-1;i>=0;--i){ // percorre todos os clientes
        for (itr = open_facs.begin(); itr != open_facs.end(); ++itr) { // percorrer todas as inst abertas
            aux_cij = solution->getCostAIJ(i, *itr);
            if(*itr != solution->getAssignedFacilityJ(i)){ // se for melhor que o salvo ate agr, mas nao melhor que o melhor de todos
                // Se nao tiver ninguem lá salvo ainda, entao coloca
                if(nearest2_open_fac[i] == -1){
                    nearest2_open_fac[i] = *itr;
                }
                // senao, se for melhor que o salvo ate agr, atualiza
                else if(aux_cij < solution->getCostAIJ(i, nearest2_open_fac[i])){
                    nearest2_open_fac[i] = *itr;
                }
            }
        }

        if(DEBUG >= DISPLAY_DETAILS){
            cout << "Client " << i << " c1: " << solution->getCostAIJ(i,solution->getAssignedFacilityJ(i)) << " near: " << solution->getAssignedFacilityJ(i) << " c2: " << solution->getCostAIJ(i,nearest2_open_fac[i]) << " near2: " << nearest2_open_fac[i] << endl;
        }
    }

    if(DEBUG >= DISPLAY_DETAILS){
        cout << "biggest_cij from input : " << biggest_cij << endl;
    }

    // Auxiliares que indicam a terceira inst mais proxima (c3_minX = custo, nearest3_open_fac = id, aux_cij3 = auxiliar de custo)
    c3_minX = -1;
    nearest3_open_fac = -1;
    aux_cij3 = -1;

    run(solution);
}


// Retornar o valor da solucao por referencia
void LSCloseFac::run(Solution *solution) {

    cout << fixed;
    cout.precision(5);

    // Se só tiver 1 instalação aberta, então podemos encerrar porque nossos movimentos são apenas de fechar, e devemos ter ao menos 1 inst aberta
    if(n1 > 1){ // entao realmente executaremos o programa

        // Declaracao variaveis que indicam o tempo da funcao
        struct timespec start, finish, time_so_far;


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



        /*
        STEP 0 - algumas coisas ja foram feitas antes
        */

        // Inicializando o vetor extra_cost[0] correspondente a delta z
        for(int i=0;i<qty_facilities;i++){		// percorre as instalacoes
            if(solution->getOpenFacilityJ(i)){ // se a instalacao está aberta, vamos ver a mudanca se fechar
                extra_cost[0][i] = -solution->getCostFJ(i);
                for (int j=0;j<qty_clients;j++) { // Percorre todos os clientes)
                    if(solution->getAssignedFacilityJ(j) == i){ // se essa inst for a mais proxima desse cli
                        extra_cost[0][i] += solution->getCostAIJ(j, nearest2_open_fac[j]) - solution->getCostAIJ(j,i);
                    }
                }
            }
            else{ // se a instalacao está fechada, entao a mudança deve ser infinito, pois não queremos mudar isso nunca
                extra_cost[0][i] = DBL_MAX;
            }
        }

        // INICIANDO A CONTAGEM DE TEMPO DA FUNCAO
        clock_gettime(CLOCK_REALTIME, &start);


        // Indica se continua ou nao o loop de busca (continua até achar o local optimum)
        solution->setLocalOptimum(false);

        // A partir daqui é loop até acabar a busca
        while(!solution->isLocalOptimum()){

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
                for(itr = open_facs.begin(); itr != open_facs.end(); ++itr) { // percorrer todas as inst abertas
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
                solution->setLocalOptimum(true);
            }
            else{
                if(DEBUG >= DISPLAY_MOVES){
                    if(solution->getOpenFacilityJ(fac_best_extra_cost)){ // se a instalacao está aberta
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
                solution->setOpenFacilityJ(fac_best_extra_cost, false);

                n1 -= 1;
                open_facs.erase(fac_best_extra_cost);


                // Atualizando o custo atual
                solution->incrementFinalTotalCost(best_extra_cost);

                // Atualizando os indices para acessar o vetor extra_cost
                cur_index_extra = qty_moves % 2;
                old_index_extra = !cur_index_extra;

                // CHECANDO A CONTAGEM DE TEMPO GASTO ATÉ AGORA
                clock_gettime(CLOCK_REALTIME, &time_so_far);

                // Calculando o tempo gasto até agora
                solution->setTimeSpent((time_so_far.tv_sec - start.tv_sec));
                solution->setTimeSpent(solution->getTimeSpent() + ((time_so_far.tv_nsec - start.tv_nsec) / 1000000000.0)); // Necessario para obter uma precisao maior

                if(DEBUG >= DISPLAY_TIME){
                    cout << "Total time spent so far: " << solution->getTimeSpent() << " seconds" << endl;
                }

                // // Acrescentando no logDetail.txt o tempo gasto nessa iteracao e o custo da solucao
                // logDetail << solution->getTimeSpent() << "," << cur_cost << "," << qty_moves << endl;


                /*
                Update extra_cost[cur_index_extra]
                */


                // The facility was opened and we closed

                if(DEBUG >= DISPLAY_ACTIONS){
                    cout << "The facility was opened and we closed. Lets update delta extra cost from other facilities" << endl;
                }

                // Open facilities after another facility closed
                for(itr = open_facs.begin(); itr != open_facs.end(); ++itr) { // percorrer todas as inst abertas

                    if(*itr != fac_best_extra_cost){ // se nao for exatamente a inst que eu estou fechando

                        if(DEBUG >= DISPLAY_DETAILS){
                            cout << "OPEN FACILITY: " << *itr << endl;
                        }

                        if(n1 > 1){ // se ela nao eh a unica instalacao aberta

                            extra_cost[cur_index_extra][*itr] = extra_cost[old_index_extra][*itr]; // inicia com o valor da anterior

                            for(int j=0;j<qty_clients;j++){ // percorre os clientes

                                //  Pegando o custo de atribuicao desse cliente com a inst que estou fechando (cij')
                                aux_cij = solution->getCostAIJ(j,fac_best_extra_cost);

                                // Se for Tj1 ou Tj2
                                // Tj1 = {i | di1 = j' ^ di2 = j} 		// Tj2 = {i | di1 = j ^ di2 = j'}
                                if(((solution->getAssignedFacilityJ(j) == fac_best_extra_cost) && (nearest2_open_fac[j] == *itr))
                                    || ((solution->getAssignedFacilityJ(j) == *itr) && (nearest2_open_fac[j] == fac_best_extra_cost))) { // se estou fechando a mais proxima e *itr é a segunda mais proxima (Tj1)
                                    /*
                                    Defining nearest3_open_fac e c3_minX
                                    */
                                    c3_minX = biggest_cij + 1; // limitante superior tranquilo
                                    for(itr2 = open_facs.begin(); itr2 != open_facs.end(); ++itr2) { // percorrer todas as inst abertas
                                        aux_cij3 = solution->getCostAIJ(j,*itr2);
                                        if((aux_cij3 < c3_minX) && (*itr2 != solution->getAssignedFacilityJ(j)) && (*itr2 != nearest2_open_fac[j])){ // se for melhor que o salvo ate agr, mas nao melhor que o melhor de todos nem o segundo melhor salvo
                                            nearest3_open_fac = *itr2;
                                            c3_minX = aux_cij3;
                                        }
                                    }
                                    // incluindo a inst que fechamos na busca, ja q ela nao está mais em open_facs
                                    if((c3_minX > biggest_cij) || ((aux_cij < c3_minX) && (fac_best_extra_cost != solution->getAssignedFacilityJ(j)) && (fac_best_extra_cost != nearest2_open_fac[j]))) {
                                        nearest3_open_fac = fac_best_extra_cost;
                                        c3_minX = aux_cij;
                                    }

                                    // Se for Tj1
                                    // Tj1 = {i | di1 = j' ^ di2 = j}
                                    if((solution->getAssignedFacilityJ(j) == fac_best_extra_cost) && (nearest2_open_fac[j] == *itr)){ // se estou fechando a mais proxima e *itr é a segunda mais proxima (Tj1)
                                        extra_cost[cur_index_extra][*itr] += c3_minX - solution->getCostAIJ(j,*itr);

                                        // Updating d1 --> dj2 (nearest2)
                                        temp_nearest_fac[j] = nearest2_open_fac[j];
                                    }
                                        // Senao, se for Tj2
                                        // Tj2 = {i | di1 = j ^ di2 = j'}
                                    else {  // se estou fechando a segunda mais proxima e n2 é a mais proxima (Tj2)
                                        extra_cost[cur_index_extra][*itr] += c3_minX - aux_cij;

                                        // saving temp d1
                                        temp_nearest_fac[j] = solution->getAssignedFacilityJ(j);
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
                            solution->setLocalOptimum(true);

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
                        solution->setAssignedFacilityJ(j, temp_nearest_fac[j]); // salvando alteracoes sobre a inst mais proxima na solucao final

                        nearest2_open_fac[j] = temp_nearest2_fac[j];

                        temp_nearest_fac[j] = -1; // resetando
                    }
                }
            }
        }

        // FINALIZANDO A CONTAGEM DE TEMPO DA FUNCAO
        clock_gettime(CLOCK_REALTIME, &finish);

        if(DEBUG >= DISPLAY_MOVES){
            cout << "FINAL TOTAL COST: " << solution->getFinalTotalCost() << endl;
        }

        // Exibir quais instalacoes foram abertas
        if(DEBUG >= DISPLAY_ACTIONS){
            cout << "OPEN FACILITIES: " << endl;
            for (itr = open_facs.begin(); itr != open_facs.end(); ++itr) { // percorrer todas as inst abertas
                cout << *itr << " ";
            }
            cout << endl;
        }

        if(DEBUG >= DISPLAY_BASIC){
            cout << "Total moves: " << qty_moves << endl;
        }


        // Calculando o tempo gasto total
        solution->setTimeSpent((finish.tv_sec - start.tv_sec));
        solution->setTimeSpent(solution->getTimeSpent() + ((finish.tv_nsec - start.tv_nsec) / 1000000000.0)); // Necessario para obter uma precisao maior

        if(DEBUG >= DISPLAY_TIME){
            cout << "Final Total Function Time: " << solution->getTimeSpent() << " seconds" << endl;
        }

        // // Acrescentando no solLog.txt o tempo gasto final da funcao e o custo final da solucao
        // solLog << solution->getTimeSpent() << "," << solution->getFinalTotalCost() << "," << qty_moves << endl;

        // // Acrescentando no logDetail.txt o tempo gasto nessa iteracao e o custo da solucao
        // logDetail << solution->getTimeSpent() << "," << solution->getFinalTotalCost() << "," << qty_moves << endl;

        // solLog.close();
        // logDetail.close();

    }
}

LSCloseFac::~LSCloseFac() {
    delete []  nearest2_open_fac;
    delete [] temp_nearest_fac;
    delete []  temp_nearest2_fac;

    for (int i = 0; i < 2; ++i) {
        delete[] extra_cost[i];
    }
    delete [] extra_cost;
}
