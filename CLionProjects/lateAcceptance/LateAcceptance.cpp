
#include "LateAcceptance.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <ctime>
#include <fstream>
#include <cstring>
#include <cfloat>

#define DISPLAY_BASIC 1 // corresponde a exibicao da quantidade de movimentos
#define DISPLAY_MOVES 2 // corresponde a todos os cout quando um movimento é realizado de fato
#define DISPLAY_ACTIONS 3 // corresponde a todos os cout quando uma acao é feita.
#define DISPLAY_DETAILS 4 // corresponde a todos os cout mais detalhados quando uma acao é feita.
#define DISPLAY_TIME 5 // corresponde aos calculos de tempo

#define DEBUG 1 // OPCOES DE DEBUG: 1 - MOSTRAR A QTD DE MOVIMENTOS, 2 PARA EXIBIR OS MOVIMENTOS REALIZADOS, 3 PARA EXIBIR ACOES, 4 PARA EXIBIR DETALHES DAS ACOES, 5 PARA EXIBIR TEMPO

// Alocando memoria e inicializando valores
void LateAcceptance::initialize(Solution *solution, bool _bestFit, double _a1, double _limitIdle, int _lh) {

    qty_facilities = solution->getQtyFacilities();
    qty_clients = solution->getQtyClients();

    // Variavel que armazena o maior valor cij dado na entrada, para uso posterior
    biggest_cij = 0;

    // Auxiliar para escolher a instalacao no first fir
    qty_inst_used = 0;

    // Variavel que marca quantas movimentacoes foram feitas de fato
    qty_moves = 0;

    // Variavel que marca quantas movimentacoes foram feitas de fato
    qty_moves_done = 0;

    a1 = _a1;
    lh = _lh;
    limit_idle = _limitIdle;
    best_fit = _bestFit;

    // Vetor que vai indicar se a instalacao está flagged
    flag = new bool[qty_facilities];

    // Vetor fitness array que representa as solucoes anteriores, que vou usar para comparacao (tamanho lh)
    fa = new double[lh];

    // Serao utilizados para acessar o vetor extra_cost, funciona como modulo 2
    cur_index_extra = 0;
    old_index_extra = 0;

    // extra_cost - será o delta z para cada iteracao, para cada instalacao
    extra_cost = new double*[2];
    for(int i = 0; i < 2; i++) {
        extra_cost[i] = new double[qty_facilities];
    }

    // Represents the amount of open facilities
    n1 = 0;

    c_minX = new double[qty_clients];
    c2_minX = new double[qty_clients];
    nearest_open_fac = new int[qty_clients];
    nearest2_open_fac = new int[qty_clients];
    temp_nearest_fac = new int[qty_clients];
    temp_nearest2_fac = new int[qty_clients];
    temp_c_minX = new double[qty_clients];
    temp_c2_minX = new double[qty_clients];


    for(int i=0;i<qty_clients;i++){
        temp_nearest_fac[i] = -1; // indica que nao há nenhuma inst temporaria ainda
        nearest_open_fac[i] = solution->getAssignedFacilityJ(i); // indica com qual inst está conectado inicialmente

        if(DEBUG >= DISPLAY_DETAILS){
            cout << "Client " << i << " - initial nearest open facility: " << nearest_open_fac[i] << endl;
        }

        for(int j=0;j<qty_facilities;j++){
            // Salvando o valor do maior Cij da entrada
            if(solution->getCostAIJ(i,j) > biggest_cij){
                biggest_cij = solution->getCostAIJ(i,j);
            }
        }
    }

    // Iniciando o conjunto de instalacoes abertas, alem dos vetores t e flag
    for(int i=0;i<qty_facilities;i++){
        if(solution->getOpenFacilityJ(i)){
            open_facs.insert(i);
            n1 += 1;
        }
        flag[i] = false;
    }

    // Atualizando valor c_minX para todos os clientes e iniciando c2_minX
    for(int i=0;i<qty_clients;i++){
        c_minX[i] = solution->getCostAIJ(i, solution->getAssignedFacilityJ(i));
        c2_minX[i] = biggest_cij + 1; // limitante superior tranquilo
    }


    // Iniciando os labels correspondentes à segunda instalacao aberta mais proxima de cada cliente (c2_minX)
    for(int i=0;i<qty_clients;i++){ // percorre todos os clientes
        for (itr = open_facs.begin(); itr != open_facs.end(); ++itr) { // percorrer todas as inst abertas
            aux_cij = solution->getCostAIJ(i, *itr);
            if((aux_cij < c2_minX[i]) && (*itr != nearest_open_fac[i])){ // se for melhor que o salvo ate agr, mas nao melhor que o melhor de todos
                nearest2_open_fac[i] = *itr;
                c2_minX[i] = aux_cij;
            }
        }
        if(c2_minX[i] > biggest_cij){ // se só tinha 1 inst aberta, entao colocamos -1 para indicar invalidez
            if(DEBUG >= DISPLAY_ACTIONS){
                cout << "Starting - There is just 1 open facility" << endl;
            }
            c2_minX[i] = -1;
            nearest2_open_fac[i] = -1;
        }

        if(DEBUG >= DISPLAY_DETAILS){
            cout << "Client " << i << " c1: " << c_minX[i] << " near: " << nearest_open_fac[i] << " c2: " << c2_minX[i] << " near2: " << nearest2_open_fac[i] << endl;
        }
    }

    if(DEBUG >= DISPLAY_DETAILS){
        cout << "biggest_cij from input : " << biggest_cij << endl;
    }

    cur_cost = solution->getFinalTotalCost();
    k_last_best = qty_moves;
    lets_move = false;
    keep_searching = true;
    update_near = false;

    // Auxiliares que indicam a terceira inst mais proxima (c3_minX = custo, nearest3_open_fac = id, aux_cij3 = auxiliar de custo)
    c3_minX = -1;
    nearest3_open_fac = -1;
    aux_cij3 = -1;

    idle_itr = 0;

    // Iniciando o vetor fa
    for(int i=0;i<lh;++i){
        fa[i] = solution->getFinalTotalCost();
    }

    run(solution);
}

// Retornar o valor da solucao por referencia
void LateAcceptance::run(Solution *solution) {
    cout << fixed;
    cout.precision(5);

    // Declaracao variaveis que indicam o tempo da funcao
    struct timespec start, finish, time_so_far;

    if(DEBUG >= DISPLAY_BASIC){
        if(best_fit)
            cout << "BEST FIT - ";
        else
            cout << "FIRST FIT - ";
        cout << "Lh: " << lh << endl;
        cout << "A1 criterion: " << int(a1 * qty_facilities) << " iterations without improvement best" << endl;
        cout << "Idle criterion: " << limit_idle << " * iterations without improvement current" << endl;
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
    for(int j=0;j<qty_facilities;++j){ // percorre as instalacoes
        if(solution->getOpenFacilityJ(j)){ // se a instalacao está aberta, vamos ver a mudanca se fechar
            extra_cost[0][j] = -solution->getCostFJ(j);
            for(int i=0;i<qty_clients;++i){ // percorre os clientes
                if(nearest_open_fac[i] == j){ // se essa inst for a mais proxima desse cli
                    extra_cost[0][j] += c2_minX[i] - solution->getCostAIJ(i,j);
                }
            }
        }
        else{ // se a instalacao está fechada, vamos ver a mudanca se abrir
            extra_cost[0][j] = solution->getCostFJ(j);
            for(int i=0;i<qty_clients;++i){ // percorre os clientes
                if(solution->getCostAIJ(i,j) < c_minX[i]){ // se essa inst for a mais proxima desse cli
                    extra_cost[0][j] += solution->getCostAIJ(i,j) - c_minX[i];
                }
            }
        }
    }

    int counter;

    // INICIANDO A CONTAGEM DE TEMPO DA FUNCAO
    clock_gettime(CLOCK_REALTIME, &start);


    // A partir daqui é loop até acabar a busca
    while(keep_searching){

        if(DEBUG >= DISPLAY_MOVES){
            cout << endl << "-------------------------- NEXT MOVE " << qty_moves << " ---------------------------" << endl << endl;
        }

        /*
        STEP 1
        */

        // Tecnica best fit
        if(best_fit){
            // Select a facility that has de minimum extra_cost and is not flagged (and extra_cost is not DBL_MAX ---> invalid)
            best_extra_cost = DBL_MAX; // limitante superior, maior double possivel
            fac_best_extra_cost = -1; // indica invalidez
            for(int i=0;i<qty_facilities;i++){
                if(!((solution->getOpenFacilityJ(i))&&(n1 == 1))){ // se ela nao for a unica instalacao aberta
                    if((extra_cost[cur_index_extra][i] < best_extra_cost) && ( !flag[i] )){ // se essa for menor do que a ja encontrada ate agr e nao estiver marcada, atualiza
                        best_extra_cost = extra_cost[cur_index_extra][i];
                        fac_best_extra_cost = i;
                    }
                }
            }
        }
        // Tecnica first fit
        else{
            fac_best_extra_cost = qty_inst_used % qty_facilities; // cada hora pega a proxima
            qty_inst_used += 1;
            counter = 0;
            while((solution->getOpenFacilityJ(fac_best_extra_cost))&&(n1 == 1)&&(counter < qty_facilities)){ // enquanto ela for a unica instalacao aberta
                fac_best_extra_cost = qty_inst_used % qty_facilities; // cada hora pega a proxima
                qty_inst_used += 1;
                counter += 1;
            }
            if((counter >= qty_facilities)&&(solution->getOpenFacilityJ(fac_best_extra_cost))&&(n1 == 1)){ // nao tem inst disponivel mais
                fac_best_extra_cost = -1;
            }
            else{
                best_extra_cost = extra_cost[cur_index_extra][fac_best_extra_cost];
            }
        }

        if(DEBUG >= DISPLAY_MOVES){
            cout << "Facility " << fac_best_extra_cost << " best delta extra cost: " << best_extra_cost << endl;
        }

        if(fac_best_extra_cost == -1){
            if(DEBUG >= DISPLAY_BASIC){
                cout << "Stop criterion: THERE ARE NO MOVES ANYMORE" << endl;
            }
            keep_searching = false;
        }
        else{
            if(DEBUG >= DISPLAY_MOVES){
                if(solution->getOpenFacilityJ(fac_best_extra_cost)){ // se a instalacao está aberta
                    cout << "We want to close it" << endl;
                }
                else{ // se a instalacao está fechada
                    cout << "We want to open it" << endl;
                }
            }

            // Check if the new cost is better than the old one
            if(best_extra_cost >= 0){
                idle_itr += 1;
            }
            else{
                idle_itr = 0;
            }

            // Calculating the virtual beggining
            v = qty_moves % lh;

            if(DEBUG >= DISPLAY_ACTIONS){
                cout << "V = " << v << endl;
                cout << "F[v]: " << fa[v] << "  old: " << cur_cost << "  new: " << cur_cost + best_extra_cost << endl;
            }

            if(((cur_cost + best_extra_cost) < fa[v]) || (best_extra_cost <= 0)){
                /*
                go to STEP 3
                */
                lets_move = true;
                if(DEBUG >= DISPLAY_ACTIONS){
                    cout << "It's better than the current cost or the f[v]!" << endl;
                }
            }
            else{
                // Mark facility as flagged
                flag[fac_best_extra_cost] = true;
                /*
                go back to STEP 1 -->> while
                */
                lets_move = false; // garantindo que o valor será falso
                keep_searching = true; // garantindo que o valor será verdadeiro

                if(DEBUG >= DISPLAY_ACTIONS){
                    cout << "It's NOT better than the current and the f[v]." << endl;
                }
            }

            // Aumentando a contagem de movimentos
            qty_moves += 1;

            /*
            STEP 3
            */

            if(lets_move){

                if(DEBUG >= DISPLAY_MOVES){
                    cout << "Lets move!" << endl;
                }

                // Aumentando a contagem de movimentos
                qty_moves_done += 1;

                // Restaurando
                lets_move = false;

                // Abrindo a inst se estiver fechada e fechando se estiver aberta
                solution->setOpenFacilityJ(fac_best_extra_cost, !solution->getOpenFacilityJ(fac_best_extra_cost));

                if(solution->getOpenFacilityJ(fac_best_extra_cost)){ // se a instalação estiver aberta agora
                    n1 += 1;
                    open_facs.insert(fac_best_extra_cost);
                }
                else { // senao, se a instalação estiver fechada agora
                    n1 -= 1;
                    open_facs.erase(fac_best_extra_cost);
                }

                // Atualizando o custo atual
                cur_cost += best_extra_cost;


                // Atualizando os indices para acessar o vetor extra_cost
                cur_index_extra = qty_moves_done % 2;
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

                // Atualizando a melhor solucao encontrada ate agr
                if(cur_cost < solution->getFinalTotalCost()){
                    if(DEBUG >= DISPLAY_MOVES){
                        cout << "Updating the best cost found so far: " << cur_cost << endl;
                    }
                    solution->setFinalTotalCost(cur_cost);
                    k_last_best = qty_moves;

                    // // Acrescentando no solLog.txt o tempo gasto nessa iteracao e o custo da solucao
                    // solLog << solution->getTimeSpent() << "," << solution->getFinalTotalCost() << "," << qty_moves << endl;
                }
                else{
                    if(DEBUG >= DISPLAY_ACTIONS){
                        cout << "IT IS NOT BETTER. current cost: " << cur_cost << endl;
                    }
                }


                /*
                STEP 4
                */

                /*
                Update extra_cost[cur_index_extra]
                */

                // If the facility was closed and we opened
                if(solution->getOpenFacilityJ(fac_best_extra_cost)){

                    if(DEBUG >= DISPLAY_ACTIONS){
                        cout << "The facility was closed and we opened. Lets update delta extra cost from other facilities" << endl;
                    }

                    for(int j2=0;j2 < qty_facilities; ++j2){ // percorre as instalacoes
                        if(j2!=fac_best_extra_cost){ // se nao for exatamente a inst que eu estou abrindo

                            // Open facilities after another facility opened
                            if(solution->getOpenFacilityJ(j2)){ // se a instalacao 2 está aberta

                                if(DEBUG >= DISPLAY_DETAILS){
                                    cout << "OPEN FACILITY: " << j2 << endl;
                                }
                                if(n1 > 2){ // se ela nao era a unica instalacao aberta (entao extra_cost[qty_moves - 1] está definido)
                                    extra_cost[cur_index_extra][j2] = extra_cost[old_index_extra][j2]; // inicia com o valor da anterior

                                    for(int i2=0;i2 < qty_clients; ++i2){ // percorre os clientes
                                        if(nearest_open_fac[i2] == j2){ // se esse cliente tinha a inst n2 como inst mais proxima

                                            //  Pegando o custo de atribuicao desse cliente com a inst que estou abrindo (cij')
                                            aux_cij = solution->getCostAIJ(i2, fac_best_extra_cost);

                                            // Tj1 = {i | di1 = j ^ cij' < cij}
                                            if(aux_cij < solution->getCostAIJ(i2, j2)){ // se a que abriu eh melhor que essa inst n2 (Tj1)
                                                extra_cost[cur_index_extra][j2] += solution->getCostAIJ(i2, j2) - c2_minX[i2];

                                                // Updating d2 --> dj1 (nearest)
                                                temp_nearest2_fac[i2] = nearest_open_fac[i2];
                                                temp_c2_minX[i2] = c_minX[i2];

                                                // Updating d1 --> j' (fac_best_extra_cost)
                                                temp_nearest_fac[i2] = fac_best_extra_cost;
                                                temp_c_minX[i2] = aux_cij;
                                            }

                                                // Tj2 = {i | di1 = j ^ cij <= cij' < cidi2}
                                            else if((solution->getCostAIJ(i2, j2) <= aux_cij) && (aux_cij < c2_minX[i2])){ // senao, se a que abriu eh melhor que a segunda inst mais proxima (Tj2)
                                                extra_cost[cur_index_extra][j2] += aux_cij - c2_minX[i2];

                                                // Saving d1 temp
                                                temp_nearest_fac[i2] = nearest_open_fac[i2];
                                                temp_c_minX[i2] = c_minX[i2];

                                                // Updating d2 --> j' (fac_best_extra_cost)
                                                temp_nearest2_fac[i2] = fac_best_extra_cost;
                                                temp_c2_minX[i2] = aux_cij;
                                            }
                                        }
                                    }
                                }
                                else { // se ela era a unica instalacao aberta
                                    if(DEBUG >= DISPLAY_ACTIONS){
                                        cout << "It was the only one open facility. We need to recompute the delta extra cost. Fac: " << j2 << endl;
                                    }
                                    /*
                                    Recomputing extra_cost
                                    */
                                    extra_cost[cur_index_extra][j2] = -solution->getCostFJ(j2); // inicia com o ganho ao fechar essa inst

                                    for(int i2=0;i2 < qty_clients; ++i2){ // percorre os clientes

                                        if(nearest_open_fac[i2] == j2){ // se essa inst for a mais proxima desse cli. OBS: isso deve sempre ser verdadeiro pois só tinha ela aberta

                                            //  Pegando o custo de atribuicao desse cliente com a inst que estou abrindo (cij')
                                            aux_cij = solution->getCostAIJ(i2, fac_best_extra_cost);;

                                            // Se a que abriu agora é a mais proxima
                                            if(aux_cij < c_minX[i2]){
                                                // Updating d2
                                                temp_nearest2_fac[i2] = nearest_open_fac[i2];
                                                temp_c2_minX[i2] = c_minX[i2];

                                                // Updating d1
                                                temp_nearest_fac[i2] = fac_best_extra_cost;
                                                temp_c_minX[i2] = aux_cij;
                                            }
                                                // Se nao, entao a que abriu é a segunda mais proxima
                                            else{

                                                // Saving d1 temp
                                                temp_nearest_fac[i2] = nearest_open_fac[i2];
                                                temp_c_minX[i2] = c_minX[i2];

                                                // Updating d2
                                                temp_nearest2_fac[i2] = fac_best_extra_cost;
                                                temp_c2_minX[i2] = aux_cij;

                                                // Complementa com o ganho de reatribuicao
                                                extra_cost[cur_index_extra][j2] += temp_c2_minX[i2] - solution->getCostAIJ(i2,j2);
                                            }
                                        }
                                    }
                                }
                            }

                                // Closed facilities after another facility opened
                            else{ // se a instalacao 2 está fechada
                                if(DEBUG >= DISPLAY_DETAILS){
                                    cout << "CLOSED FACILITY: " << j2<< endl;
                                }

                                extra_cost[cur_index_extra][j2] = extra_cost[old_index_extra][j2]; // inicia com o valor da anterior

                                for(int i2=0;i2 < qty_clients; ++i2){ // percorre os clientes

                                    update_near = false; // a principio nao sabemos se deveremos atualizar ou nao

                                    //  Pegando o custo de atribuicao desse cliente com a inst que estou abrindo (cij')
                                    aux_cij = solution->getCostAIJ(i2, fac_best_extra_cost);;

                                    // Sj1 = {i | cij < cij' < cidi1}
                                    if((solution->getCostAIJ(i2,j2) < aux_cij) && (aux_cij < c_minX[i2])){ // se a que abriu eh melhor que a nearest mas pior que essa inst n2 (Sj1)
                                        extra_cost[cur_index_extra][j2] += c_minX[i2] - aux_cij;
                                        update_near = true; // indica que vamos atualizar
                                    }
                                        // Sj2 = {i | cij' <= cij < cidi1}
                                    else if((aux_cij <= solution->getCostAIJ(i2,j2)) && (solution->getCostAIJ(i2,j2) < c_minX[i2])){ // senao, se a que abriu eh melhor que a nearest e essa n2, porem n2 melhor que nearest tbm (Sj2)
                                        extra_cost[cur_index_extra][j2] += c_minX[i2] - solution->getCostAIJ(i2,j2);
                                        update_near = true; // indica que vamos atualizar
                                    }

                                    // Updating d1 and d2
                                    if(update_near){

                                        // Updating d2 --> dj1 (nearest)
                                        temp_nearest2_fac[i2] = nearest_open_fac[i2];
                                        temp_c2_minX[i2] = c_minX[i2];

                                        // Updating d1 --> j' (fac_best_extra_cost)
                                        temp_nearest_fac[i2] = fac_best_extra_cost;
                                        temp_c_minX[i2] = aux_cij;
                                    }
                                }
                            }

                            // Mark each facility as unflagged
                            flag[j2] = false;
                        }
                    }

                    // Updating extra_cost for this chosen facility -> don't need to recalculate
                    extra_cost[cur_index_extra][fac_best_extra_cost] = - best_extra_cost;
                }

                    // Else, if the facility was opened and we closed
                else{

                    if(DEBUG >= DISPLAY_ACTIONS){
                        cout << "The facility was opened and we closed. Lets update delta extra cost from other facilities" << endl;
                    }
                    for(int j2=0;j2 < qty_facilities; ++j2){ 	// percorre as instalacoes

                        if(j2!=fac_best_extra_cost){ // se nao for exatamente a inst que eu estou abrindo

                            // Open facilities after another facility closed
                            if(solution->getOpenFacilityJ(j2)){ // se a instalacao 2 está aberta

                                if(DEBUG >= DISPLAY_DETAILS){
                                    cout << "OPEN FACILITY: " << j2 << endl;
                                }

                                if(n1 > 1){ // se ela nao eh a unica instalacao aberta

                                    extra_cost[cur_index_extra][j2] = extra_cost[old_index_extra][j2]; // inicia com o valor da anterior

                                    for(int i2=0;i2 < qty_clients; ++i2){ // percorre os clientes
                                        //  Pegando o custo de atribuicao desse cliente com a inst que estou fechando (cij')
                                        aux_cij = solution->getCostAIJ(i2, fac_best_extra_cost);

                                        // Se for Tj1 ou Tj2
                                        // Tj1 = {i | di1 = j' ^ di2 = j} 		// Tj2 = {i | di1 = j ^ di2 = j'}
                                        if(((nearest_open_fac[i2] == fac_best_extra_cost) && (nearest2_open_fac[i2] == j2))
                                           || ((nearest_open_fac[i2] == j2) && (nearest2_open_fac[i2] == fac_best_extra_cost))) { // se estou fechando a mais proxima e n2 é a segunda mais proxima (Tj1)
                                            /*
                                            Defining nearest3_open_fac e c3_minX
                                            */
                                            c3_minX = biggest_cij + 1; // limitante superior tranquilo
                                            for(itr = open_facs.begin(); itr != open_facs.end(); ++itr) { // percorrer todas as inst abertas
                                                aux_cij3 =  solution->getCostAIJ(i2, *itr);
                                                if((aux_cij3 < c3_minX) && (*itr != nearest_open_fac[i2]) && (*itr != nearest2_open_fac[i2])){ // se for melhor que o salvo ate agr, mas nao melhor que o melhor de todos nem o segundo melhor salvo
                                                    nearest3_open_fac = *itr;
                                                    c3_minX = aux_cij3;
                                                }
                                            }
                                            // incluindo a inst que fechamos na busca, ja q ela nao está mais em open_facs
                                            if((c3_minX > biggest_cij) || ((aux_cij < c3_minX) && (fac_best_extra_cost != nearest_open_fac[i2]) && (fac_best_extra_cost != nearest2_open_fac[i2]))) {
                                                nearest3_open_fac = fac_best_extra_cost;
                                                c3_minX = aux_cij;
                                            }

                                            // Se for Tj1
                                            // Tj1 = {i | di1 = j' ^ di2 = j}
                                            if((nearest_open_fac[i2] == fac_best_extra_cost) && (nearest2_open_fac[i2] ==j2)){ // se estou fechando a mais proxima e n2 é a segunda mais proxima (Tj1)
                                                extra_cost[cur_index_extra][j2] += c3_minX - solution->getCostAIJ(i2,j2);

                                                // Updating d1 --> dj2 (nearest2)
                                                temp_nearest_fac[i2] = nearest2_open_fac[i2];
                                                temp_c_minX[i2] = c2_minX[i2];
                                            }
                                                // Senao, se for Tj2
                                                // Tj2 = {i | di1 = j ^ di2 = j'}
                                            else {  // se estou fechando a segunda mais proxima e n2 é a mais proxima (Tj2)
                                                extra_cost[cur_index_extra][j2] += c3_minX - aux_cij;

                                                // saving temp d1
                                                temp_nearest_fac[i2] = nearest_open_fac[i2];
                                                temp_c_minX[i2] = c_minX[i2];

                                            }

                                            // Updating d2 --> d3 (nearest3), tanto para Tj1 quanto Tj2
                                            temp_nearest2_fac[i2] = nearest3_open_fac;
                                            temp_c2_minX[i2] = c3_minX;
                                        }
                                    }

                                }
                                else { // se ela eh a unica instalacao aberta
                                    // if(DEBUG >= DISPLAY_ACTIONS){
                                    cout << "It was the only one open facility. We cannot define a new delta extra cost. Fac: " << j2 << endl;
                                    // }
                                    /*
                                    Not possible to have extra_cost because this facility cannot be closed
                                    */

                                    extra_cost[cur_index_extra][j2] = DBL_MAX; // indica invalidez

                                    for(int i2=0;i2 < qty_clients; ++i2){ // percorre os clientes
                                        // colocando ela como inst mais proxima de todos os clientes
                                        temp_nearest_fac[i2] = j2;
                                        temp_c_minX[i2] = solution->getCostAIJ(i2,j2);

                                        // indicando invalidez
                                        temp_nearest2_fac[i2] = -1;
                                        temp_c2_minX[i2] = -1;
                                    }
                                }
                            }

                                // Closed facilities after another facility closed
                            else{ // se a instalacao 2 está fechada
                                if(DEBUG >= DISPLAY_DETAILS){
                                    cout << "CLOSED FACILITY: " << j2 << endl;
                                }

                                extra_cost[cur_index_extra][j2] = extra_cost[old_index_extra][j2]; // inicia com o valor da anterior

                                for(int i2=0;i2 < qty_clients; ++i2){ // percorre os clientes

                                    if(nearest_open_fac[i2] == fac_best_extra_cost){ // se esse cliente tinha a inst fac_best_extra_cost como inst mais proxima

                                        update_near = false; // a principio nao sabemos se deveremos atualizar ou nao

                                        // Pegando o custo de atribuicao desse cliente com a inst que estou fechando (cij')
                                        aux_cij = solution->getCostAIJ(i2, fac_best_extra_cost);

                                        // Sj1 = {i | di1 = j' ^ cij < cij'}
                                        if(solution->getCostAIJ(i2,j2) < aux_cij){ // se essa inst n2 eh melhor que a que fechou, que era a nearest (Sj1)
                                            extra_cost[cur_index_extra][j2] += aux_cij - c2_minX[i2];
                                            update_near = true; // indica que vamos atualizar
                                        }

                                            // Sj2 = {i | di1 = j' ^ cij' <= cij < cidi2}
                                        else if((aux_cij <= solution->getCostAIJ(i2,j2)) && (solution->getCostAIJ(i2,j2) < c2_minX[i2])){ // senao, se essa inst n2 é melhor que a nearest2 porem nao melhor que a nearest que fechou(Sj2)
                                            extra_cost[cur_index_extra][j2] += solution->getCostAIJ(i2,j2) - c2_minX[i2];
                                            update_near = true; // indica que vamos atualizar
                                        }

                                        // Updating d1 and d2
                                        if(update_near){

                                            // Updating d1 --> dj2 (nearest2)
                                            temp_nearest_fac[i2] = nearest2_open_fac[i2];
                                            temp_c_minX[i2] = c2_minX[i2];

                                            // Updating d2 --> search again

                                            temp_c2_minX[i2] = biggest_cij + 1; // limitante superior tranquilo

                                            for(itr = open_facs.begin(); itr != open_facs.end(); ++itr) { // percorrer todas as inst abertas
                                                aux_cij = solution->getCostAIJ(i2, *itr);
                                                if((aux_cij < temp_c2_minX[i2]) && (*itr != temp_nearest_fac[i2])){ // se for melhor que o salvo ate agr, mas nao melhor que o melhor de todos
                                                    temp_nearest2_fac[i2] = *itr;
                                                    temp_c2_minX[i2] = aux_cij;
                                                }
                                            }
                                            if(temp_c2_minX[i2] > biggest_cij){ // se só tinha 1 inst aberta, entao colocamos -1 para indicar invalidez
                                                if(DEBUG >= DISPLAY_ACTIONS){
                                                    cout << "There is just 1 open facility" << endl;
                                                }
                                                temp_c2_minX[i2] = -1;
                                                temp_nearest2_fac[i2] = -1;
                                            }
                                        }
                                    }
                                }
                            }

                            // Mark each facility as unflagged
                            flag[j2] = false;
                        }
                    }
                    // Updating extra_cost for this chosen facility -> don't need to recalculate
                    extra_cost[cur_index_extra][fac_best_extra_cost] = - best_extra_cost;
                }

                /*
                UPDATING NEAREST AND NEAREST 2
                */

                for(int i=0;i < qty_clients; ++i){ // percorre os clientes
                    if(temp_nearest_fac[i]!= -1){ // se houve alguma mudanca com esse cliente
                        nearest_open_fac[i] = temp_nearest_fac[i];
                        c_minX[i] = temp_c_minX[i];
                        nearest2_open_fac[i] = temp_nearest2_fac[i];
                        c2_minX[i] = temp_c2_minX[i];

                        temp_nearest_fac[i] = -1; // resetando
                    }

                    // Se a solucao final foi atualizada nessa iteracao
                    if(k_last_best == qty_moves){
                        // SALVANDO A SOLUCAO NOVA
                        solution->setAssignedFacilityJ(i, nearest_open_fac[i]); // salvando alteracoes sobre a inst mais proxima na solucao final
                    }
                }

                flag[fac_best_extra_cost] = true; // Flagging a facility escolhida para nao ficar repetindo o movimento
            }

            // Se ainda nao atingiu a condicao de parada respectivo ao idle itr
            if(idle_itr <= qty_moves * limit_idle){
                /*
                go back to STEP 1 -->> while
                */
                keep_searching = true; // garantindo que o valor será verdadeiro
            }

            else{
                if(DEBUG >= DISPLAY_MOVES){
                    cout << "Stop criterion idle: " << idle_itr << " > " <<  qty_moves * limit_idle << endl;
                }
                // Se ainda nao atingiu a condicao de parada respectivo ao a1
                if(qty_moves - k_last_best <= a1 * qty_facilities){
                    /*
                    go back to STEP 1 -->> while
                    */
                    keep_searching = true; // garantindo que o valor será verdadeiro
                }
                else{
                    /*
                    STOP
                    */
                    keep_searching = false;

                    if(DEBUG >= DISPLAY_BASIC){
                        cout << "Stop criterion a1 and idle itr" << endl;
                    }
                }
            }

            // Updating the fitness array
            if(cur_cost < fa[v]){
                fa[v] = cur_cost;
            }
        }
    }


    // ESSA PARTE DEVERIA SER FEITA MELHOR

    // Ajustando o vetor de instalacoes abertas de acordo com a solucao final
    for(int i=0;i<qty_facilities;i++){
        // Primeiro coloca todos como false
        solution->setOpenFacilityJ(i, false);
    }
    // Agora abre as que tem gente atribuida
    for(int i=0;i < qty_clients;i++){
        solution->setOpenFacilityJ(solution->getAssignedFacilityJ(i), true);
    }


    // FINALIZANDO A CONTAGEM DE TEMPO DA FUNCAO
    clock_gettime(CLOCK_REALTIME, &finish);

    if(DEBUG >= DISPLAY_ACTIONS){
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
        cout << "Last update final cost: " << k_last_best << endl;
    }

    // Calculando o tempo gasto total
    solution->setTimeSpent((finish.tv_sec - start.tv_sec));
    solution->setTimeSpent(solution->getTimeSpent() + ((finish.tv_nsec - start.tv_nsec) / 1000000000.0)); // Necessario para obter uma precisao maior

    if(DEBUG >= DISPLAY_TIME){
        cout << "Final Total Function Time: " << solution->getTimeSpent() << " seconds" << endl;
    }

    // // Acrescentando no solLog.txt o tempo gasto final da funcao e o custo final da solucao
    // solLog << solution->getTimeSpent() << "," << solution->getFinalTotalCost() << "," << qty_moves << endl;

    // solLog.close();
    // logDetail.close();
}


LateAcceptance::~LateAcceptance() {
    delete [] flag;
    delete [] fa;

    for (int i = 0; i < 2; ++i) {
        delete[] extra_cost[i];
    }
    delete [] extra_cost;

    delete [] c_minX;
    delete []  c2_minX;
    delete [] nearest_open_fac;
    delete []  nearest2_open_fac;
    delete [] temp_nearest_fac;
    delete []  temp_nearest2_fac;
    delete []  temp_c_minX;
    delete [] temp_c2_minX;
}
