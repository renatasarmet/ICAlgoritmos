#include "Greedy.h"
#include <algorithm>
#include <iostream>
#include <ctime>

#define DISPLAY_ACTIONS 1 // corresponde a todos os cout quando uma acao é feita.
#define DISPLAY_TIME 2 // corresponde ao calculo de tempo

#define DEBUG 0 // OPCOES DE DEBUG: 1 PARA EXIBIR ACOES, 2 PARA EXIBIR TEMPO

using namespace std;

void Greedy::mergeSort(double *vector, int *vectorID, int startPosition, int endPosition) {

    int i, j, k, half_size, *temp_vector_ID;
    double *temp_vector;
    if(startPosition == endPosition) return;
    half_size = (startPosition + endPosition ) / 2;

    mergeSort(vector, vectorID, startPosition, half_size);
    mergeSort(vector, vectorID, half_size + 1, endPosition);

    i = startPosition;
    j = half_size + 1;
    k = 0;
    temp_vector = new double[endPosition - startPosition + 1];
    temp_vector_ID = new int[endPosition - startPosition + 1];

    while(i < half_size + 1 || j < endPosition + 1) {
        if (i == half_size + 1 ) {
            temp_vector[k] = vector[j];
            temp_vector_ID[k] = vectorID[j];
            j++;
            k++;
        }
        else {
            if (j == endPosition + 1) {
                temp_vector[k] = vector[i];
                temp_vector_ID[k] = vectorID[i];
                i++;
                k++;
            }
            else {
                if (vector[i] < vector[j]) {
                    temp_vector[k] = vector[i];
                    temp_vector_ID[k] = vectorID[i];
                    i++;
                    k++;
                }
                else {
                    temp_vector[k] = vector[j];
                    temp_vector_ID[k] = vectorID[j];
                    j++;
                    k++;
                }
            }
        }

    }
    for(i = startPosition; i <= endPosition; i++) {
        vector[i] = temp_vector[i - startPosition];
        vectorID[i] = temp_vector_ID[i - startPosition];
    }
    delete [] temp_vector;
    delete [] temp_vector_ID;
}

void Greedy::deletingNonActiveClients(int indexFac, int qtyCliDelete){
    int qty_remaining = qtyCliDelete;
    for(int i=0;i<qty_clients;i++){
        if(qty_remaining == 0){
            break;
        }

        for(int j=0; j < qtyCliDelete; j++){
            if(sorted_cij_ID_g[indexFac][i] == delete_clients[j]){
                sorted_cij_g[indexFac][i] = -1; // coloca cij = -1, indicando que ja foi atribuido
                qty_remaining -= 1;
                break;
            }
        }
    }

    // Colocando novamente em ordem
    mergeSort(sorted_cij_g[indexFac], sorted_cij_ID_g[indexFac], 0, (qty_clients - 1));
}


// Por referencia diz qual o melhor tamanho para Y e seu respectivo custo
void Greedy::bestSubset(int &bestSize, double &bestCost, int indexFac, double fi, double gain, int startIndex){
    int current_size = 1;
    double sum_current_cost = 0;
    double current_cost = 0;
    bestCost = fi + biggest_cij + 1; // limitante superior

    // inicializando só para nao ficar nulo se algo der errado
    bestSize = 1;

    // ira criar (vector_size - startIndex) subconjuntos
    for(int i=startIndex; i < qty_clients; i++){
        sum_current_cost = fi - gain;
        // vendo a soma dos custos desse subconjunto
        for(int j=startIndex; j <= i; j++){
            sum_current_cost += sorted_cij_g[indexFac][j];
        }

        current_cost = sum_current_cost / current_size;

        if(current_cost <= bestCost){
            bestCost = current_cost;
            bestSize = current_size;
        }
        current_size += 1;
    }
}


// Alocando memoria e inicializando valores
void Greedy::initialize(Solution *solution) {
    qty_facilities = solution->getQtyFacilities();
    qty_clients = solution->getQtyClients();

    // conjunto de clientes a serem removidos de g na iteração, pois deixaram de ser ativos
    delete_clients = new int[qty_clients];

    // Indica quantos clientes ja foram conectados a alguma instalacao
    qty_non_active_cli = 0;

    // sorted_cij_g - será os clientes ativos ordenados pelo cij àquela instalação
    sorted_cij_g = new double*[qty_facilities];

    // sorted_cij_ID_g - indica os ID dos clientes ativos ordenados pelo cij àquela instalação
    sorted_cij_ID_g = new int*[qty_facilities];

    for(int i = 0; i < qty_facilities; i++) {
        sorted_cij_g[i] = new double[qty_clients];
        sorted_cij_ID_g[i] = new int[qty_clients];
    }

    // c_minX - será correnpondente a c(j,X) = min_{i \in X} cij, sendo X as inst abertas
    c_minX = new double[qty_clients];

    // active - indica se o cliente está ativo ainda ou nao, isto é, se ele ainda nao foi conectado a nenhuma instalacao aberta. if j \in g
    active = new bool[qty_clients];

    qty_non_active_cli_g = qty_clients; // Indica a quantidade de clientes ainda em g, os clientes ativos.

    // Variavel que armazena o maior valor cij dado na entrada, para uso posterior
    biggest_cij = 0;

    // Variavel que armazena o maior valor fi dado na entrada, para uso posterior
    biggest_fi = 0;

    for(int i=0;i<qty_clients;++i){
        c_minX[i] = -1; // indica que nao está conectado com ninguém inicialmente //PROBLEMA: nao tenho ctz ainda qual a melhor inicializacao
        active[i] = true;
    }

    for(int j=0;j<qty_facilities;++j){
        solution->setOpenFacilityJ(j, false); // indica que a instalação não está aberta inicialmente

        // Salvando o valor do maior Fi da entrada
        if(solution->getCostFJ(j) > biggest_fi){
            biggest_fi = solution->getCostFJ(j);
        }

        for(int i=0;i<qty_clients;++i) {
            // Salvando o valor do maior Cij da entrada
            if (solution->getCostAIJ(i, j) > biggest_cij) {
                biggest_cij = solution->getCostAIJ(i, j);
            }
        }
    }

    if(DEBUG >= DISPLAY_ACTIONS){
        cout << "biggest_cij from input : " << biggest_cij << " biggest_fi from input : " << biggest_fi << endl;
    }

    // colocando para cada instalacao a ordem dos cij
    for(int j=0; j< qty_facilities; ++j){ // percorre as instalacoes
        // colocando no vetor que sera ordenado
        for(int i=0; i< qty_clients; ++i){ // percorre todos os clientes
            sorted_cij_ID_g[j][i] = i; // linha da instalacao, coluna do cliente
            sorted_cij_g[j][i] = solution->getCostAIJ(i, j); // linha da instalacao, coluna do cliente
        }

        // ordenando o vetor
        mergeSort(sorted_cij_g[j], sorted_cij_ID_g[j], 0, (qty_clients - 1));

        if (DEBUG >= DISPLAY_TIME){
            cout << "sorted to facility " << j << endl;
            for(int i=0;i<qty_clients;i++){
                cout << sorted_cij_g[j][i] << endl; // Adicionar o cliente j no conjunto de
            }
            cout << "-------" << endl;
        }
    }

    // Resetando o final total cost
    solution->setFinalTotalCost(0);

    // Executa o programa
    run(solution);
}


// Retornar o valor da solucao por referencia
void Greedy::run(Solution * solution) {

    cout << fixed;
    cout.precision(5);

    // Declaracao variaveis que indicam o tempo do programa
    struct timespec start, finish;

    int id_chosen_fac;
    double best_cost_chosen;
    int best_size_chosen;

    int current_Y_size;
    double current_Y_cost;

    double gain_cij;


    // INICIANDO A CONTAGEM DE TEMPO DA FUNCAO
    clock_gettime(CLOCK_REALTIME, &start);


    // ****** A partir daqui deve estar em um loop até nao ter mais clientes ativos:

    while(qty_non_active_cli_g > 0){

        if(DEBUG >= DISPLAY_ACTIONS){
            cout << endl << "------------------------------ STILL HAVE " << qty_non_active_cli_g << " ACTIVE CLIENTS ------------------------------" << endl << endl;
        }

        best_cost_chosen = biggest_cij * qty_clients + biggest_fi + 1; //limitante superior tranquilo
        best_size_chosen = -1;
        id_chosen_fac = -1;

//        for(int j=0; j< qty_facilities; ++j){ // percorre as instalacoes
            for(int j=qty_facilities-1; j>= 0; --j){ // percorre as instalacoes // colocando assim para manter os mesmos valores de quando eu usava grafo

                // Calculando gain de troca de atribuições
            // \sum_{j notin g} (c(j,X), cij)+
            gain_cij = 0;

            for(int i=0; i< qty_clients; ++i){ // percorre todos os clientes
                if(!active[i]){
                    if(c_minX[i] > solution->getCostAIJ(i,j)){
                        gain_cij += c_minX[i] - solution->getCostAIJ(i,j);
                    }
                }
            }
            // cout << "fac = " << j << " -- gain: " << gain_cij << endl;
            bestSubset(current_Y_size, current_Y_cost, j, solution->getCostFJ(j), gain_cij, qty_non_active_cli);

            // Atualizando valores do melhor custo
            if(current_Y_cost < best_cost_chosen){
                best_cost_chosen = current_Y_cost;
                best_size_chosen = current_Y_size;
                id_chosen_fac = j;
            }

        }

        // cout << "Let's open fac: " << id_chosen_fac << " and assign to " << best_size_chosen << " clients with cost: " << best_cost_chosen << endl;
        solution->incrementFinalTotalCost(solution->getCostFJ(id_chosen_fac)); // Somando os custos de abertura das instalacoes escolhidas para abrir

        // abrindo inst
        solution->setOpenFacilityJ(id_chosen_fac, true);

        // fi <- 0
        solution->setCostFJ(id_chosen_fac, 0);

        // g <- g - Y

        // criando o conjunto de clientes que serao apagados. inicia no primeiro ativo ate o tamanho escolhido na iteracao.
        for(int i = qty_non_active_cli; i < qty_non_active_cli + best_size_chosen; i++){
            delete_clients[i - qty_non_active_cli] = sorted_cij_ID_g[id_chosen_fac][i]; // delete_clients inicia na posicao 0.
        }

        qty_non_active_cli += best_size_chosen;
        qty_non_active_cli_g -= best_size_chosen;

        // Alterando o valor correspondente em sorted_cij_g e sorted_cij_ID_g para -1 em todas as instalacoes
        for(int j=0; j< qty_facilities; ++j){ // percorre as instalacoes
             deletingNonActiveClients(j, best_size_chosen);
        }

        // Atualizando o valor de c(j,X) para os clientes que ja nao eram ativos
        for(int i=0; i< qty_clients; ++i){ // percorre todos os clientes
            if(!active[i]){
                if(c_minX[i] > solution->getCostAIJ(i, id_chosen_fac)){
                    c_minX[i] = solution->getCostAIJ(i, id_chosen_fac);
                    solution->setAssignedFacilityJ(i, id_chosen_fac);
                    // cout << "Updating here: " << c_minX[i] << endl;
                }
            }
        }

        // Atualizando o valor de c(j,X) para todos os clientes agora atribuidos a i
        for(int i=0;i<best_size_chosen;i++){
            c_minX[delete_clients[i]] = solution->getCostAIJ(delete_clients[i], id_chosen_fac);
            solution->setAssignedFacilityJ(delete_clients[i],id_chosen_fac);
            active[delete_clients[i]] = false;
            // cout << "we have cij = " << solution->getCostAIJ(delete_clients[i], id_chosen_fac) << endl;
            // cout << "Updating here: " << c_minX[delete_clients[i]] << " and id= " <<  solution->getAssignedFacilityJ(delete_clients[i]) << endl;
        }
    }

    // Somando os custos de conexao dos clientes a instalacao mais proxima aberta
    for(int i=0; i< qty_clients; ++i){ // percorre todos os clientes
        solution->incrementFinalTotalCost(solution->getCostAIJ(i, solution->getAssignedFacilityJ(i)));
    }

    // FINALIZANDO A CONTAGEM DE TEMPO DA FUNCAO
    clock_gettime(CLOCK_REALTIME, &finish);

    if(DEBUG >= DISPLAY_ACTIONS){
        cout << "FINAL TOTAL COST: " << solution->getFinalTotalCost() << endl;
    }

    // Calculando o tempo gasto total
    solution->setTimeSpent((finish.tv_sec - start.tv_sec));
    solution->setTimeSpent(solution->getTimeSpent() + ((finish.tv_nsec - start.tv_nsec) / 1000000000.0)); // Necessario para obter uma precisao maior

    if(DEBUG >= DISPLAY_TIME){
        cout << "Final Total Function Time: " << solution->getTimeSpent() << " seconds" << endl;
    }
}

Greedy::~Greedy() {
    delete [] delete_clients;

    for (int i = 0; i < qty_facilities; ++i) {
        delete[] sorted_cij_g[i];
        delete[] sorted_cij_ID_g[i];
    }
    delete [] sorted_cij_g;
    delete [] sorted_cij_ID_g;

    delete [] c_minX;
    delete [] active;
}

