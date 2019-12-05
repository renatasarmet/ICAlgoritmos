#include <iostream>
#include <ctime>
#include <fstream>
#include "definitions.hpp"
#include "../greedy/definitions.hpp"
#include "../localSearch/definitions.hpp"

using namespace std;

void mergeSortID(double *vector, int *vectorID, int startPosition, int endPosition) {

    int i, j, k, halfSize, *tempVectorID;
    double *tempVector;
    if(startPosition == endPosition) return;
    halfSize = (startPosition + endPosition ) / 2;

    mergeSortID(vector, vectorID, startPosition, halfSize);
    mergeSortID(vector, vectorID, halfSize + 1, endPosition);

    i = startPosition;
    j = halfSize + 1;
    k = 0;
    tempVector = (double *) malloc(sizeof(double) * (endPosition - startPosition + 1));
    tempVectorID = (int *) malloc(sizeof(int) * (endPosition - startPosition + 1));

    while(i < halfSize + 1 || j  < endPosition + 1) {
        if (i == halfSize + 1 ) { 
            tempVector[k] = vector[vectorID[j]];
            tempVectorID[k] = vectorID[j];
            j++;
            k++;
        }
        else {
            if (j == endPosition + 1) {
                tempVector[k] = vector[vectorID[i]];
                tempVectorID[k] = vectorID[i];
                i++;
                k++;
            }
            else {
                if (vector[vectorID[i]] < vector[vectorID[j]]) {
                    tempVector[k] = vector[vectorID[i]];
                    tempVectorID[k] = vectorID[i];
                    i++;
                    k++;
                }
                else {
                    tempVector[k] = vector[vectorID[j]];
                    tempVectorID[k] = vectorID[j];
                    j++;
                    k++;
                }
            }
        }
    }
    for(i = startPosition; i <= endPosition; i++) {
        vectorID[i] = tempVectorID[i - startPosition];
    }
    free(tempVector);
    free(tempVectorID);
}


// Conecta os clientes à instalacao aberta mais proxima. Parametro por referencia.
void connect_nearest(solutionType * node, int qty_clients, int ** sorted_cijID, double ** assignment_cost){
    int cont;
    for(int i=0;i<qty_clients;i++){
        cont = 0;
        // Percorrendo pelo vetor ordenado de ID do cij, quando encontrar a primeira inst que estiver aberta, para
        while(!node->open_facilities[sorted_cijID[i][cont]]){
            cont+=1;
        }
        // Atribui essa inst como inst aberta mais proxima
        node->assigned_facilities[i] = sorted_cijID[i][cont];

        // Aumenta no custo total final
        node->finalTotalCost += assignment_cost[i][sorted_cijID[i][cont]];
    }
}


solutionType set_initial_sol_G(int qty_facilities, int qty_clients, double * costF, double * costA){ 
    solutionType node;

    node = greedy(qty_clients, qty_facilities, costF, costA);

    // Inicializando todas as inst fechadas
    for(int k=0;k<qty_facilities;k++){
        node.open_facilities[k] = false;
    }

    // Atualizando as instalacoes abertas
    for(int i=0;i<qty_clients;i++){
        node.open_facilities[node.assigned_facilities[i]] = true;
    }        

    return node;
}


solutionType set_initial_sol_LS_G(char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA, solutionType initial_sol){
    solutionType node;

    node = localSearch(solutionName, qty_facilities, qty_clients, costF, costA, initial_sol, 1); // tipo LS completo

    // Inicializando todas as inst fechadas
    for(int k=0;k<qty_facilities;k++){
        node.open_facilities[k] = false;
    }

    // Atualizando as instalacoes abertas
    for(int i=0;i<qty_clients;i++){
        node.open_facilities[node.assigned_facilities[i]] = true;
    }        

    return node;
}


// Recebe node por referencia. Modificacoes feitas aqui refletem diretamente la
void set_initial_sol_RANDOM(solutionType * node, int qty_facilities, int qty_clients, double * costF, double ** assignment_cost, int seed, int ** sorted_cijID){ // type: 0 para greedy, 1 para LS_G, 2 para aleatorio

    // Semente do numero aleatorio
    srand(seed);

    int randNum;
    bool used;

    // Inicializando struct 
    node->finalTotalCost = 0;

    // Verificando quais instalacoes estarão abertas: 50% de chance de cada uma
    for(int i=0; i<qty_facilities; i++){
        randNum = rand() % 2; // Generate a random number between 0 and 1
        node->open_facilities[i] = randNum;
    }

    // Conectando cada cliente com a instalacao aberta mais proxima
    connect_nearest(node, qty_clients, sorted_cijID, assignment_cost);

    // Fechar as instalacoes que nao foram conectadas a ninguem
    for(int i=0; i<qty_facilities; i++){
        used = false;
        if(node->open_facilities[i]){ // se essa instalacao estiver aberta
            for(int j=0;j<qty_clients;j++){ // percorre pelos clientes para ver se alguem está conectada nela
                if(node->assigned_facilities[j] == i){
                    used = true;
                    break; // encontrou alguem concetado, entao sai
                }
            }
            if(!used){ // se nao tem ninguem conectado nela, vamos fecha-la
                node->open_facilities[i] = false;
            }
            else{ // Se essa instalacao esta sendo usada, vai continuar aberta e devemos somar o custo
                node->finalTotalCost += costF[i];
            }
        }
    }
}

