//
// Created by Renata Sarmet Smiderle Mendes on 7/1/20.
//

#include "LocalSearch.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <ctime>
#include <fstream>
#include <cstring>
using namespace std;

#define DISPLAY_BASIC 1 // corresponde a exibicao da quantidade de movimentos
#define DISPLAY_MOVES 2 // corresponde a todos os cout quando um movimento é realizado de fato
#define DISPLAY_ACTIONS 3 // corresponde a todos os cout quando uma acao é feita.
#define DISPLAY_TIME 4 // corresponde aos calculos de tempo
#define DISPLAY_DETAILS 5 // corresponde a descricao dos clientes, instalacoes e arcos

#define DEBUG 1 // OPCOES DE DEBUG: 1 - MOSTRAR A QTD DE MOVIMENTOS, 2 PARA EXIBIR OS MOVIMENTOS REALIZADOS, 3 PARA EXIBIR ACOES, 4 PARA EXIBIR TEMPO, 5 PARA EXIBIR DETALHES

#define TIME_LIMIT 900 //15 minutos

#define TIME_COUNTER_STEP 5 // 5 segundos para testar as inst ga250a //30 // 30 segundos. Isso indicara que vai salvar a melhor solucao encontrada a cada minuto


// // Função para embaralhar o vetor com indices das facilities
// void shuffle(int *vet, int vetSize){
// 	int r, temp;
// 	for(int i=0; i<vetSize; i++){
// 		r = rand() % vetSize;
// 		temp = vet[i];
// 		vet[i] = vet[r];
// 		vet[r] = temp;
// 	}
// }


// Alocando memoria e inicializando valores
void LocalSearch::initialize(Solution *solution, int lsType) {

    qty_facilities = solution->getQtyFacilities();
    qty_clients = solution->getQtyClients();

    // Variavel que armazena o maior valor cij dado na entrada, para uso posterior
    biggest_cij = 0;

    // Variavel que marca quantas movimentacoes foram feitas de fato
    qty_moves = 0;

    c_minX = new double[qty_clients];
    c2_minX = new double[qty_clients];
    nearest_open_fac = new int[qty_clients];
    nearest2_open_fac = new int[qty_clients];
    temp_nearest_fac = new int[qty_clients];
    temp2_nearest_fac = new int[qty_clients];

    // Iniciando o conjunto de instalacoes abertas e fechadas
    for(int j=qty_facilities-1;j>=0;--j){
        if(solution->getOpenFacilityJ(j)){
            open_facs.insert(j);
        }
        else{
            closed_facs.insert(j);
        }
    }


    for(int i=qty_clients-1;i>=0;--i){
        temp_nearest_fac[i] = -1; // indica que nao há nenhuma inst temporaria ainda
        nearest_open_fac[i] = solution->getAssignedFacilityJ(i); // indica com qual inst está conectado inicialmente

        if(DEBUG >= DISPLAY_DETAILS){
            cout << "Client " << i << " - initial nearest open facility: " << nearest_open_fac[i] << endl;
        }

        for(int j=qty_facilities-1;j>=0;--j){
            // Salvando o valor do maior Cij da entrada
            if(solution->getCostAIJ(i,j) > biggest_cij){
                biggest_cij = solution->getCostAIJ(i,j);
            }
        }
    }


    // Atualizando valor c_minX para todos os clientes e iniciando c2_minX
    for(int i=qty_clients-1;i>=0;--i){
        c_minX[i] = solution->getCostAIJ(i, solution->getAssignedFacilityJ(i));
        c2_minX[i] = biggest_cij + 1; // limitante superior tranquilo
    }

    // Iniciando os labels correspondentes à segunda instalacao aberta mais proxima de cada cliente (c2_minX)
    for(int i=qty_clients-1;i>=0;--i){ // percorre todos os clientes
        for (itr = open_facs.begin(); itr != open_facs.end(); ++itr) { // percorrer todas as inst abertas
            aux_cij = solution->getCostAIJ(i, *itr);
            if((aux_cij < c2_minX[i]) && (*itr != nearest_open_fac[i])){ // se for melhor que o salvo ate agr, mas nao melhor que o melhor de todos
                nearest2_open_fac[i] = *itr;
                c2_minX[i] = aux_cij;
            }
        }
        if(c2_minX[i] > biggest_cij){ // se só tinha 1 inst aberta, entao colocamos -1 para indicar invalidez
            if(DEBUG >= DISPLAY_ACTIONS){
                cout << "There is just 1 open facility" << endl;
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

    swap_done = false;
    closed_nearest = false;

    run(solution, lsType);
}


// Retornar o valor da solucao por referencia
void LocalSearch::run(Solution *solution, int lsType) {

    cout << fixed;
    cout.precision(5);

    // Declaracao variaveis que indicam o tempo da funcao
    struct timespec start, finish, time_so_far;

    // Declaracao da variavel que vai indicar o contador para salvar a solucao atual
    double timeCounter = 0;

    // // Declaracao de variavel auxiliar para formacao do arquivo .log
    // char completeLogSolName[250] = "";

    // // Arquivo para salvar o log da solucao
    // ofstream solLog;
    // strcat(completeLogSolName,solutionName);
    // strcat(completeLogSolName,".log");
    // solLog.open(completeLogSolName, std::ofstream::out | std::ofstream::trunc);

    // //Salvando o cabecalho
    // solLog << "time spent so far, current solution cost, current qty moves" << endl;




   // MUDAR A ORDEM QUE PERCORRE AS INSTALACOES


    // INICIANDO A CONTAGEM DE TEMPO DA FUNCAO
    clock_gettime(CLOCK_REALTIME, &start);

    // ****** A partir daqui deve estar em um loop até nao ter mais melhoras possiveis, isto é, encontrar o otimo local

    // OBSERVACAO 1: tambem existe uma condicao de parada relacionada a tempo, feita logo no inicio do loop
    // OBSERVACAO 2: solution.local_optimum (variavel que indica se ja encontramos o otimo local)

    solution->setLocalOptimum(false);

    while(!solution->isLocalOptimum()){

        // CHECANDO A CONTAGEM DE TEMPO GASTO ATÉ AGORA
        clock_gettime(CLOCK_REALTIME, &time_so_far);

        // Calculando o tempo gasto até agora
        solution->setTimeSpent((time_so_far.tv_sec - start.tv_sec));
        solution->setTimeSpent(solution->getTimeSpent() + ((time_so_far.tv_nsec - start.tv_nsec) / 1000000000.0)); // Necessario para obter uma precisao maior

        if(DEBUG >= DISPLAY_TIME){
            cout << "Total time spent so far: " << solution->getTimeSpent() << " seconds" << endl;
        }

        // Se já passou o tempo limite, devemos parar
        if(solution->getTimeSpent() >= TIME_LIMIT){
            // // Acrescentando no solLog.txt o tempo final gasto nas iteracoes e o custo da solucao
            // solLog << solution->getTimeSpent() << "," << solution->getFinalTotalCost() << "," << qty_moves << endl;

            break;
        }
        else if(solution->getTimeSpent() >= timeCounter){
            // aumentando o contador de tempo
            timeCounter += TIME_COUNTER_STEP;

            // // Acrescentando no solLog.txt o tempo gasto nessa iteracao e o custo da solucao
            // solLog << solution->getTimeSpent() << "," << solution->getFinalTotalCost() << "," << qty_moves << endl;
        }

        if(DEBUG >= DISPLAY_ACTIONS){
            cout << endl << "------------------------------ NEXT MOVE ------------------------------" << endl << endl;
        }

        // Primeiramente vamos percorrer todas as instalacoes
        // Caso a instalacao esteja aberta, vamos tentar fechá-la e ver se há alguma melhora
        // Lembrando que temos que reatribuir todas os clientes a ela conectados, mudando para a prox inst mais proxima
        // Se houver melhora no custo total, efetuamos de fato a operacao
        // Da mesma forma, caso a instalacao esteja fechada, vamos tentar abri-la e ver se melhora
        // Lembrando que temos que buscar todos os clientes que possuem ela como inst aberta mais proxima para conecta-los
        // Esse teste sera sequencialmente feito com um for ate o final de percorrer as instalacoes
        // Apos isso, caso tenha encontrado alguma melhora, parte para a proxima iteracao do while e o for será repetido
        // Caso nenhuma melhora tenha sido encontrada, parte para o caso da troca

        // Apos testar todas as opcoes de add e delete e nada melhorar a solucao, vamos tentar a troca
        // Portanto havera um outro for, embaixo desse primeiro for, ainda dentro do while, no qual percorrera todas as inst
        // Caso a inst esteja aberta tenta fechar ela e haverá um for inteiro para tentar abrir as instalacoes SUBSEQUENTES (nao precisa repetir as anteriores, mas nao sei se consigo fazer isso no iterator) que estao fechadas atualmente
        // Caso esse movimento de troca resulte em uma melhora, será de fato feita a operacao
        // Do contrario, continuará percorrendo o for interno na busca por abrir a proxima inst fechada
        // Se nenhuma for encontrada, vai para a proxima iteracao do for externo e repete o processo
        // OBSERVAÇÃO IMPORTANTE: Quando encontrar o primeiro caso de troca bem sucedido, esse for para imediatamente, indo para a proxima iteracao do while
        // Assim serao testados novamente os movimentos de add ou delete, que sao mais simples
        // Só voltando novamente para a troca caso nenhum add nem delete melhore nada
        // OBSERVAÇÃO IMPORTANTE 2: Quando em nenhum caso de troca for encontrado melhora, encontramos de fato o minimo local!! E o while deve terminar


        // Podemos considerar que inicialmente estamos no otimo local ate que se prove o contrario.
        // Entao, vou percorrer todas as instalacoes e, caso eu encontre alguma melhora no percurso, modifico esse otimo local para falso
        // Na proxima iteracao volto a considerar que estamos no otimo local e repito o processo
        // No momento que eu percorrer todas as instalacoes e nenhuma conseguir melhorar nada, entao eu realmente estava no otimo local e finalizo o while
        // Encaixando a operação de troca, será um for logo depois desse primeiro for, que só ira entrar caso finalize o primeiro for com solution.local_optimum = true, pois indica que nada melhorou por ali
        solution->setLocalOptimum(true);

        if((lsType == 1) || (lsType == 2)){
            // VIZINHANCA 1: ABRIR OU FECHAR UMA INSTALACAO
            for(int j=qty_facilities-1;j>=0;--j){		// percorre as instalacoes

                if(DEBUG >= DISPLAY_ACTIONS){
                    cout << "Let's move facility " << j << endl;
                }

                if(solution->getOpenFacilityJ(j)){ // caso a inst esteja aberta

                    if(DEBUG >= DISPLAY_ACTIONS){
                        cout << "It is OPEN, let's close it" << endl;
                    }

                    if(open_facs.size() <= 1){
                        if(DEBUG >= DISPLAY_ACTIONS){
                            cout << "Sorry, you can't close it because it is the only one that is open." << endl;
                        }
                    }
                    else{
                        // Vamos fechar essa instalacao

                        extra_cost = -solution->getCostFJ(j); // subtrai o custo de abertura dessa inst

                        for(int i=qty_clients-1;i>=0;--i){	// percorre os clientes

                            if(nearest_open_fac[i] == j){ // caso esse cliente esteja conectado a essa instalacao

                                if(c2_minX[i] >= 0){ // CASO EXISTA A SEGUNDA INST MAIS PROXIMA SALVA

                                    // Atualizamos o custo extra, subtraindo o custo de atribuicao da antiga inst e somando o custo com a nova
                                    extra_cost += c2_minX[i] - c_minX[i];

                                    if(DEBUG >= DISPLAY_ACTIONS){
                                        cout << "client: " << i << " new nearest facility " << nearest2_open_fac[i] << endl;
                                    }
                                }
                                else{
                                    if(DEBUG >= DISPLAY_ACTIONS){
                                        cout << "You shouldn't be here. There is just 1 open facility and you want to close it." << endl;
                                    }
                                    continue; // vai para a proxima iteracao
                                }
                            }
                        }

                        if(DEBUG >= DISPLAY_ACTIONS){
                            cout << "--- EXTRA COST: " << extra_cost << endl;
                        }

                        // Caso melhore o custo total final, vamos efetuar de fato essa alteracao
                        if(extra_cost < 0){

                            if(DEBUG >= DISPLAY_ACTIONS){
                                cout << "YEEES THE COST IS BETTER, LET'S DO IT " << endl;
                            }

                            if(DEBUG >= DISPLAY_MOVES){
                                cout << "Move CLOSE facility: "<< j << endl;
                            }

                            qty_moves += 1;

                            for(int i=qty_clients-1;i>=0;--i){	// percorre os cliente

                                closed_nearest = false;

                                if(nearest_open_fac[i] == j){ // caso esse cliente esteja conectado a essa instalacao

                                    closed_nearest = true;

                                    if(DEBUG >= DISPLAY_ACTIONS){
                                        cout << "changing client: " << i << " nearest facility " << j << endl;
                                    }

                                    // ATUALIZANDO AQUI A INST MAIS PROXIMA
                                    nearest_open_fac[i] = nearest2_open_fac[i]; // reatribuindo com a nova inst mais proxima
                                    c_minX[i] = c2_minX[i]; // atualizando o menor cij desse cli

                                    // SALVANDO A SOLUCAO NOVA
                                    solution->setAssignedFacilityJ(i, nearest_open_fac[i]); // salvando alteracoes sobre a inst mais proxima na solucao final
                                }

                                if((closed_nearest)||(nearest2_open_fac[i] == j)){ // caso esse cliente tinha essa instalacao como primeira ou segunda mais proxima
                                    // ATUALIZANDO AQUI A SEGUNDA MAIS PROXIMA
                                    c2_minX[i] = biggest_cij + 1; // limitante superior tranquilo

                                    for (itr = open_facs.begin(); itr != open_facs.end(); ++itr) { // percorrer todas as inst abertas
                                        aux_cij = solution->getCostAIJ(i,*itr);
                                        if((aux_cij < c2_minX[i]) && (*itr != nearest_open_fac[i]) && (*itr != j)){ // se for melhor que o salvo ate agr, mas nao melhor que o melhor de todos nem da que vai fechar
                                            nearest2_open_fac[i] = *itr;
                                            c2_minX[i] = aux_cij;
                                        }
                                    }
                                    if(c2_minX[i] > biggest_cij){ // se só tinha 1 inst aberta depois do fechamento dessa iteracao, entao colocamos -1 para indicar invalidez
                                        if(DEBUG >= DISPLAY_ACTIONS){
                                            cout << "There is just 1 open facility" << endl;
                                        }
                                        c2_minX[i] = -1;
                                        nearest2_open_fac[i] = -1;
                                    }
                                }
                            }

                            solution->setOpenFacilityJ(j, false); // fechando de fato a instalacao
                            open_facs.erase(j);
                            closed_facs.insert(j);

                            solution->incrementFinalTotalCost(extra_cost); // Atualizando o custo total final

                            solution->setLocalOptimum(false); // Como encontrei uma melhora, entao eu nao estava no otimo local, portanto o while deve continuar
                        }
                        else {
                            if(DEBUG >= DISPLAY_ACTIONS){
                                cout << "NO! IT GOT WORSE" << endl;
                            }
                        }
                    }
                }

                else { // caso a inst esteja fechada

                    if(DEBUG >= DISPLAY_ACTIONS){
                        cout <<"It is CLOSED, let's open it" << endl;
                    }

                    // Vamos abrir essa instalacao

                    extra_cost = solution->getCostFJ(j); // soma o custo de abertura dessa inst

                    for(int i=qty_clients-1;i>=0;--i){	// percorre os clientes

                        if(solution->getCostAIJ(i,j) < c_minX[i]){ // caso a nova inst seja a mais proxima aberta desse cliente

                            if(DEBUG >= DISPLAY_ACTIONS){
                                cout << "client: " << i << " new nearest facility " << j << endl;
                            }

                            temp_nearest_fac[i] = j; // atualizando a inst mais perto temporaria

                            // Atualizamos o custo extra, subtraindo o custo de atribuicao da antiga inst e somando o custo com a nova
                            extra_cost += solution->getCostAIJ(i,j) - c_minX[i];
                        }
                        else { // senao, caso a nova inst nao seja mais proxima
                            temp_nearest_fac[i] = nearest_open_fac[i]; // colocando isso apenas para uso posterior na checagem nao haver lixo
                        }
                    }

                    if(DEBUG >= DISPLAY_ACTIONS){
                        cout << "--- EXTRA COST: " << extra_cost << endl;
                    }

                    // Caso melhore o custo total final, vamos efetuar de fato essa alteracao
                    if(extra_cost < 0){

                        if(DEBUG >= DISPLAY_ACTIONS){
                            cout << "YEEES THE COST IS BETTER, LET'S DO IT " << endl;
                        }

                        if(DEBUG >= DISPLAY_MOVES){
                            cout << "Move OPEN facility: "<< j << endl;
                        }

                        qty_moves += 1;

                        for(int i=qty_clients-1;i>=0;--i){	// percorre os clientes

                            aux_cij = solution->getCostAIJ(i,j);

                            if(nearest_open_fac[i] != temp_nearest_fac[i]) { // caso esse cliente seja mais proximo dessa nova instalacao
                                if(DEBUG >= DISPLAY_ACTIONS){
                                    cout << "changing client: " << i << " nearest facility " << j << endl;
                                }

                                // ATUALIZANDO A SEGUNDA INST MAIS PROXIMA
                                nearest2_open_fac[i] = nearest_open_fac[i];
                                c2_minX[i] = c_minX[i];

                                // ATUALIZANDO A INSTALACAO MAIS PROXIMA
                                nearest_open_fac[i] = j; // reatribuindo com a nova inst mais proxima
                                c_minX[i] = aux_cij; // atualizando o menor cij desse cli

                                solution->setAssignedFacilityJ(i, nearest_open_fac[i]); // salvando alteracoes na solucao final
                            }
                            else if((aux_cij < c2_minX[i])||(c2_minX[i] < 0)) { // senao, caso esse cliente tenha essa nova instalacao como segunda mais proxima (ou nao existia segunda mais proxima)
                                // ATUALIZANDO A SEGUNDA INST MAIS PROXIMA
                                nearest2_open_fac[i] = j;
                                c2_minX[i] = aux_cij;
                            }
                        }

                        solution->setOpenFacilityJ(j, true); // abrindo de fato a instalacao
                        closed_facs.erase(j);
                        open_facs.insert(j);

                        solution->incrementFinalTotalCost(extra_cost); // Atualizando o custo total final

                        solution->setLocalOptimum(false); // Como encontrei uma melhora, entao eu nao estava no otimo local, portanto o while deve continuar
                    }
                    else {
                        if(DEBUG >= DISPLAY_ACTIONS){
                            cout << "NO! IT GOT WORSE" << endl;
                        }
                    }
                }
            }
        }

        if((lsType == 1) || (lsType == 3)){
            // VIZINHANCA 2: CASO DE TROCA
            if(solution->isLocalOptimum()){ // Entra aqui se nao houve nenhuma melhora na tentativa de add ou delete

                if(DEBUG >= DISPLAY_ACTIONS){
                    cout << "Let's try to swap it. Now the total cost is " << solution->getFinalTotalCost() << endl;
                }

                for(int j=qty_facilities-1;j>=0;--j){ // percorre as instalacoes

                    if(DEBUG >= DISPLAY_ACTIONS){
                        cout << "Let's move facility " << j << endl;
                    }

                    if(solution->getOpenFacilityJ(j)){ // caso a inst esteja aberta

                        if(DEBUG >= DISPLAY_ACTIONS){
                            cout << "It is OPEN, let's swap it out" << endl;
                        }

                        if(open_facs.size() <= 1){
                            if(DEBUG >= DISPLAY_ACTIONS){
                                cout << "Sorry, you can't close it because it is the only one that is open." << endl;
                            }
                        }
                        else{

                            // Vamos fechar essa instalacao

                            extra_cost = -solution->getCostFJ(j); // subtrai o custo de abertura dessa inst

                            for(int i=qty_clients-1;i>=0;--i){	// percorre os clientes

                                if(nearest_open_fac[i] == j){ // caso esse cliente esteja conectado a essa instalacao

                                    if(c2_minX[i] >= 0){ // CASO EXISTA A SEGUNDA INST MAIS PROXIMA SALVA

                                        temp_nearest_fac[i] = nearest2_open_fac[i]; // atualizando a inst mais perto temporaria

                                        if(DEBUG >= DISPLAY_ACTIONS){
                                            cout << "client: " << i << " new nearest facility " << temp_nearest_fac[i] << endl;
                                        }

                                        // Atualizamos o custo extra, subtraindo o custo de atribuicao da antiga inst e somando o custo com a nova
                                        extra_cost += c2_minX[i] - c_minX[i];
                                    }
                                    else{
                                        if(DEBUG >= DISPLAY_ACTIONS){
                                            cout << "You shouldn't be here. There is just 1 open facility and you want to close it." << endl;
                                        }
                                        continue; // vai para a proxima iteracao
                                    }
                                }
                                else{
                                    temp_nearest_fac[i] = nearest_open_fac[i]; // colocando isso apenas para uso posterior na checagem nao haver lixo
                                }

                            }

                            if(DEBUG >= DISPLAY_ACTIONS){
                                cout << "--Partial extra cost:" << extra_cost << endl;
                            }

                            // Vamos abrir uma outra instalacao no lugar dessa que fechei

                            for (itr = closed_facs.begin(); itr != closed_facs.end(); ++itr) { // percorrer todas as inst fechadas

                                if(DEBUG >= DISPLAY_ACTIONS){
                                    cout << "Let's swap in the facility " << *itr << endl;
                                }

                                complete_extra_cost = extra_cost + solution->getCostFJ(*itr); // soma o custo de abertura dessa inst

                                for(int i=qty_clients-1;i>=0;--i){	// percorre os clientes

                                    aux_cij = solution->getCostAIJ(i, temp_nearest_fac[i]);

                                    if(solution->getCostAIJ(i, *itr) < aux_cij){ // caso a nova inst seja a mais proxima aberta desse cliente

                                        if(DEBUG >= DISPLAY_ACTIONS){
                                            cout << "client: " << i << " new nearest facility " << *itr << endl;
                                        }

                                        temp2_nearest_fac[i] = *itr; // atualizando a inst mais perto temporaria

                                        // Atualizamos o custo extra, subtraindo o custo de atribuicao da antiga inst e somando o custo com a nova
                                        complete_extra_cost += solution->getCostAIJ(i, *itr) - aux_cij;
                                    }
                                    else { // senao, caso a nova inst nao seja mais proxima
                                        temp2_nearest_fac[i] = temp_nearest_fac[i]; // colocando isso apenas para uso posterior na checagem nao haver lixo
                                    }
                                }

                                if(DEBUG >= DISPLAY_ACTIONS){
                                    cout << "--Complete extra cost: " << complete_extra_cost << endl;
                                }

                                // Caso melhore o custo total final, vamos efetuar de fato essa alteracao
                                if(complete_extra_cost < 0){

                                    if(DEBUG >= DISPLAY_ACTIONS){
                                        cout << "YEEES THE COST IS BETTER, LET'S DO IT " << endl;
                                    }

                                    if(DEBUG >= DISPLAY_MOVES){
                                        cout << "Move SWAP OUT facility: " << j << " and SWAP IN facility: " << *itr << endl;
                                    }

                                    qty_moves += 1;

                                    for(int i=qty_clients-1;i>=0;--i){	// percorre os clientes

                                        // SE A NOVA INST QUE ABRIU É A MAIS PROXIMA
                                        if(*itr == temp2_nearest_fac[i]){

                                            // SE A QUE FECHOU NÃO ERA A MAIS PROXIMA ANTES
                                            if(j != nearest_open_fac[i]){
                                                nearest2_open_fac[i] = nearest_open_fac[i]; // atualiza a segunda mais proxima com a mais proxima. obs: se tiver fechado a mais proxima, entao a segunda mais proxima continua igual
                                                c2_minX[i] = c_minX[i];
                                            }

                                            nearest_open_fac[i] = *itr; // atualiza a mais proxima com a que abriu
                                            c_minX[i] = solution->getCostAIJ(i, *itr); // atualizando o menor cij desse cli
                                        }
                                            // SENAO, SE A NOVA É A SEGUNDA MAIS PROXIMA
                                        else if((solution->getCostAIJ(i, *itr) < c2_minX[i])||(c2_minX[i] < 0)) { // senao, caso esse cliente tenha essa nova instalacao como segunda mais proxima ou nao exista segunda salva

                                            // SE A QUE FECHOU ERA A MAIS PROXIMA // eu sei que existe pelo menos mais outra, senao nao ia permitir fechar essa, entao nearest2 existe
                                            if(j == nearest_open_fac[i]){
                                                nearest_open_fac[i] = *itr; // a mais proxima recebe a nova que abriu
                                                c_minX[i] = solution->getCostAIJ(i, *itr);
                                            }
                                            else{
                                                nearest2_open_fac[i] = *itr; // a segunda mais proxima recebe a nova que abriu
                                                c2_minX[i] = solution->getCostAIJ(i, *itr);
                                            }
                                        }
                                            // SENAO, ELA NAO TEM NENHUMA RELACAO, NEM COMO PRIMEIRA NEM COMO SEGUNDA
                                        else { // eu sei que existe nearest2 pois tinha pelo menos 2 instalacoes ja la

                                            // Variavel auxiliar para nao ter perigo de perder informacao importante na substituicao
                                            closed_nearest = false;

                                            // SE A QUE FECHOU ERA A MAIS PROXIMA
                                            if(j == nearest_open_fac[i]){
                                                nearest_open_fac[i] = nearest2_open_fac[i]; // atualiza a instalacao mais proxima
                                                c_minX[i] = c2_minX[i];
                                                closed_nearest = true;
                                            }

                                            // SE A QUE FECHOU ERA A MAIS PROXIMA OU A SEGUNDA MAIS PROXIMA
                                            if((closed_nearest) || (j == nearest2_open_fac[i])){

                                                // Percorre para atualizar a segunda mais proxima
                                                c2_minX[i] = biggest_cij + 1; // limitante superior tranquilo

                                                for (itr2 = open_facs.begin(); itr2 != open_facs.end(); ++itr2) { // percorrer todas as inst abertas
                                                    aux_cij = solution->getCostAIJ(i, *itr2);
                                                    if((aux_cij < c2_minX[i]) && (*itr2 != nearest_open_fac[i]) && (*itr2 != j)){ // se for melhor que o salvo ate agr, mas nao melhor que o melhor de todos nem da que vai fechar
                                                        nearest2_open_fac[i] = *itr2;
                                                        c2_minX[i] = aux_cij;
                                                    }
                                                }

                                                // nao precisa conferir se a nova nao é a segunda mais proxima (ja q ela nao esta ainda no open_facs) pois senao entraria no else if anterior

                                                if(c2_minX[i] > biggest_cij){ // se só tinha 1 inst aberta, entao colocamos -1 para indicar invalidez
                                                    if(DEBUG >= DISPLAY_ACTIONS){
                                                        cout << "There is just 1 open facility" << endl;
                                                    }
                                                    c2_minX[i] = -1;
                                                    nearest2_open_fac[i] = -1;
                                                }
                                            }
                                        }

                                        solution->setAssignedFacilityJ(i, nearest_open_fac[i]); // salvando alteracoes sobre a inst mais proxima na solucao final
                                    }

                                    solution->setOpenFacilityJ(j, false); // fechando de fato a primeira instalacao
                                    open_facs.erase(j);
                                    closed_facs.insert(j);


                                    solution->setOpenFacilityJ(*itr, true); // abrindo de fato a segunda instalacao
                                    open_facs.insert(*itr);
                                    closed_facs.erase(*itr);

                                    solution->incrementFinalTotalCost(complete_extra_cost); // Atualizando o custo total final

                                    if(DEBUG >= DISPLAY_ACTIONS){
                                        cout << "*After the swap move we have total cost: " << solution->getFinalTotalCost() << endl;
                                    }

                                    solution->setLocalOptimum(false); // Como encontrei uma melhora, entao eu nao estava no otimo local, portanto o while deve continuar

                                    swap_done = true;
                                    break; // quero que volte a olhar os movimentos de add e delete, ao inves de ver outras possiveis trocas
                                }
                                else {
                                    if(DEBUG >= DISPLAY_ACTIONS){
                                        cout << "NO! IT GOT WORSE" << endl;
                                    }
                                }
                            }
                        }
                    }

                    else { // caso a inst esteja fechada

                        if(DEBUG >= DISPLAY_ACTIONS){
                            cout <<"It is CLOSED, let's open it and close another one." << endl;
                        }

                        // Vamos abrir essa instalacao

                        extra_cost = solution->getCostFJ(j); // soma o custo de abertura dessa inst

                        for(int i=qty_clients-1;i>=0;--i){	// percorre os clientes

                            if(solution->getCostAIJ(i,j) < c_minX[i]){ // caso a nova inst seja a mais proxima aberta desse cliente

                                if(DEBUG >= DISPLAY_ACTIONS){
                                    cout << "client: " << i << " new nearest facility " << j << endl;
                                }

                                temp_nearest_fac[i] = j; // atualizando a inst mais perto temporaria

                                // Atualizamos o custo extra, subtraindo o custo de atribuicao da antiga inst e somando o custo com a nova
                                extra_cost += solution->getCostAIJ(i,j) - c_minX[i];
                            }
                            else { // senao, caso a nova inst nao seja mais proxima
                                temp_nearest_fac[i] = nearest_open_fac[i]; // colocando isso apenas para uso posterior na checagem nao haver lixo
                            }
                        }

                        if(DEBUG >= DISPLAY_ACTIONS){
                            cout << "--Partial extra cost:" << extra_cost << endl;
                        }

                        // Vamos fechar uma outra instalacao no lugar dessa que abri

                        for (itr = open_facs.begin(); itr != open_facs.end(); ++itr) { // percorrer todas as inst abertas

                            if(DEBUG >= DISPLAY_ACTIONS){
                                cout << "Let's swap out the facility " << *itr << endl;
                            }

                            complete_extra_cost = extra_cost - solution->getCostFJ(*itr); // subtrai o custo de abertura dessa inst

                            for(int i=qty_clients-1;i>=0;--i){	// percorre os clientes

                                if(temp_nearest_fac[i] == *itr){ // caso esse cliente esteja conectado a essa instalacao

                                    // Devemos testar nearest2 com a instalacao que pretendo abrir e ver qual é a mais proxima
                                    aux_cij = solution->getCostAIJ(i,j);
                                    if((aux_cij < c2_minX[i])||(c2_minX[i] < 0)){ // caso a nova seja a inst mais perto ou nao exista nearest2
                                        temp2_nearest_fac[i] = j; // atualizando a inst mais perto temporaria
                                    }
                                    else{ // senao, entao a segunda mais proxima salva ser a mais proxima
                                        temp2_nearest_fac[i] = nearest2_open_fac[i]; // atualizando a inst mais perto temporaria
                                        aux_cij = c2_minX[i];
                                    }

                                    if(DEBUG >= DISPLAY_ACTIONS){
                                        cout << "client: " << i << " new nearest facility " << temp2_nearest_fac[i] << endl;
                                    }

                                    // Atualizamos o custo extra, subtraindo o custo de atribuicao da antiga inst e somando o custo com a nova
                                    complete_extra_cost += aux_cij - solution->getCostAIJ(i, temp_nearest_fac[i]);
                                }
                                else{
                                    temp2_nearest_fac[i] = temp_nearest_fac[i]; // colocando isso apenas para uso posterior na checagem nao haver lixo
                                }
                            }

                            if(DEBUG >= DISPLAY_ACTIONS){
                                cout << "--Complete extra cost: " << complete_extra_cost << endl;
                            }

                            // Caso melhore o custo total final, vamos efetuar de fato essa alteracao
                            if(complete_extra_cost < 0){

                                if(DEBUG >= DISPLAY_ACTIONS){
                                    cout << "YEEES THE COST IS BETTER, LET'S DO IT " << endl;
                                }

                                if(DEBUG >= DISPLAY_MOVES){
                                    cout << "Move SWAP IN facility: " << j << " and SWAP OUT facility: " << *itr << endl;
                                }

                                qty_moves += 1;

                                for(int i=qty_clients-1;i>=0;--i){	// percorre os clientes

                                    // SE A NOVA INST QUE ABRIU É A MAIS PROXIMA
                                    if(j == temp2_nearest_fac[i]){

                                        // SE A QUE FECHOU NÃO ERA A MAIS PROXIMA ANTES
                                        if(*itr != nearest_open_fac[i]){
                                            nearest2_open_fac[i] = nearest_open_fac[i]; // atualiza a segunda mais proxima com a mais proxima. obs: se tiver fechado a mais proxima, entao a segunda mais proxima continua igual
                                            c2_minX[i] = c_minX[i];
                                        }

                                        nearest_open_fac[i] = j; // atualiza a mais proxima com a que abriu
                                        c_minX[i] = solution->getCostAIJ(i,j); // atualizando o menor cij desse cli
                                    }

                                        // SENAO, SE A NOVA É A SEGUNDA MAIS PROXIMA
                                    else if((solution->getCostAIJ(i,j) < c2_minX[i])||(c2_minX[i] < 0)) { // senao, caso esse cliente tenha essa nova instalacao como segunda mais proxima ou nao exista segunda salva

                                        // SE A QUE FECHOU ERA A MAIS PROXIMA // nearest2 continuara sendo a mesma, mesmo se fosse inexistente
                                        if(*itr == nearest_open_fac[i]){
                                            nearest_open_fac[i] = j; // a mais proxima recebe a nova que abriu
                                            c_minX[i] = solution->getCostAIJ(i,j);
                                        }
                                        else{
                                            nearest2_open_fac[i] = j; // a segunda mais proxima recebe a nova que abriu
                                            c2_minX[i] = solution->getCostAIJ(i,j);
                                        }
                                    }
                                        // SENAO, ELA NAO TEM NENHUMA RELACAO, NEM COMO PRIMEIRA NEM COMO SEGUNDA
                                    else { // eu sei que existe nearest2 pois tinha pelo menos 2 instalacoes ja la

                                        // Variavel auxiliar para nao ter perigo de perder informacao importante na substituicao
                                        closed_nearest = false;

                                        // SE A QUE FECHOU ERA A MAIS PROXIMA
                                        if(*itr == nearest_open_fac[i]){
                                            nearest_open_fac[i] = nearest2_open_fac[i]; // atualiza a instalacao mais proxima
                                            c_minX[i] = c2_minX[i];
                                            closed_nearest = true;
                                        }

                                        // SE A QUE FECHOU ERA A MAIS PROXIMA OU A SEGUNDA MAIS PROXIMA
                                        if((closed_nearest) || (*itr == nearest2_open_fac[i])){

                                            // Percorre para atualizar a segunda mais proxima
                                            c2_minX[i] = biggest_cij + 1; // limitante superior tranquilo

                                            for (itr2 = open_facs.begin(); itr2 != open_facs.end(); ++itr2) { // percorrer todas as inst abertas
                                                aux_cij = solution->getCostAIJ(i,*itr2);
                                                if((aux_cij < c2_minX[i]) && (*itr2 != nearest_open_fac[i]) && (*itr2 != *itr)){ // se for melhor que o salvo ate agr, mas nao melhor que o melhor de todos nem da que vai fechar
                                                    nearest2_open_fac[i] = *itr2;
                                                    c2_minX[i] = aux_cij;
                                                }
                                            }

                                            // nao precisa conferir se a nova nao é a segunda mais proxima (ja q ela nao esta ainda no open_facs) pois senao entraria no else if anterior

                                            if(c2_minX[i] > biggest_cij){ // se só tinha 1 inst aberta, entao colocamos -1 para indicar invalidez
                                                if(DEBUG >= DISPLAY_ACTIONS){
                                                    cout << "There is just 1 open facility" << endl;
                                                }
                                                c2_minX[i] = -1;
                                                nearest2_open_fac[i] = -1;
                                            }
                                        }
                                    }

                                    solution->setAssignedFacilityJ(i, nearest_open_fac[i]); // salvando alteracoes sobre a inst mais proxima na solucao final
                                }

                                solution->setOpenFacilityJ(j, true); // abrindo de fato a primeira instalacao
                                closed_facs.erase(j);
                                open_facs.insert(j);


                                solution->setOpenFacilityJ(*itr, false); // fechando de fato a segunda instalacao
                                closed_facs.insert(*itr);
                                open_facs.erase(*itr);

                                solution->incrementFinalTotalCost(complete_extra_cost); // Atualizando o custo total final

                                if(DEBUG >= DISPLAY_ACTIONS){
                                    cout << "*After the swap move we have total cost: " << solution->getFinalTotalCost() << endl;
                                }

                                solution->setLocalOptimum(false); // Como encontrei uma melhora, entao eu nao estava no otimo local, portanto o while deve continuar

                                swap_done = true;
                                break; // quero que volte a olhar os movimentos de add e delete, ao inves de ver outras possiveis trocas
                            }
                            else {
                                if(DEBUG >= DISPLAY_ACTIONS){
                                    cout << "NO! IT GOT WORSE" << endl;
                                }
                            }
                        }
                    }

                    if(swap_done){
                        swap_done = false;
                        break;
                    }
                }
            }
        }
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

}

LocalSearch::~LocalSearch() {
    delete [] c_minX;
    delete []  c2_minX;
    delete [] nearest_open_fac;
    delete []  nearest2_open_fac;
    delete [] temp_nearest_fac;
    delete []  temp2_nearest_fac;
}
