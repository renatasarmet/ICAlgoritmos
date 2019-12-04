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


solutionType set_initial_sol_RANDOM(int qty_facilities, int qty_clients, double * costF, double * costA, int seed){ // type: 0 para greedy, 1 para LS_G, 2 para aleatorio
    
    // Semente do numero aleatorio
    srand(seed);

    // Struct que vai retornar a solução
    solutionType solution;

    int randNum;

    // Inicializando struct 
    solution.finalTotalCost = 0;

    // indicara as instalacoes atribuidas a cada cliente na resposta
    solution.assigned_facilities = (int*) malloc((qty_clients) * sizeof(int));
    if(!solution.assigned_facilities){
        cout << "Memory Allocation Failed";
        exit(1);
    }

    // Verificando quais instalacoes estarão abertas: 50% de chance de cada uma
    for(int i=0; i<qty_facilities; i++){
        randNum = rand() % 2; // Generate a random number between 0 and 1
        node.open_facilities[k] = randNum;
    }

    // Conectando cada cliente com a instalacao aberta mais proxima
    
    
    

    return(solution);



}

