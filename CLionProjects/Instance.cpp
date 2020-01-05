//
// Created by Renata Sarmet Smiderle Mendes on 5/1/20.
//

#include "Instance.h"
#include <fstream>

#define DISPLAY_BASIC 1
#define DISPLAY_ACTIONS 2 // corresponde a todos os cout quando uma acao é feita.

#define DEBUG 0 // OPCOES DE DEBUG: 1 PARA EXIBIR ACOES, 2 PARA EXIBIR TEMPO


int Instance::getQtyFacilities() const {
    return qty_facilities;
}

void Instance::setQtyFacilities(int qtyFacilities) {
    qty_facilities = qtyFacilities;
}

int Instance::getQtyClients() const {
    return qty_clients;
}

void Instance::setQtyClients(int qtyClients) {
    qty_clients = qtyClients;
}

double *Instance::getCostF() const {
    return cost_f;
}

void Instance::setCostF(double *costF) {
    Instance::cost_f = costF;
}

double Instance::getCostFJ(int j) const {
    return cost_f[j];
}

void Instance::setCostFJ(int j, double costF) {
    Instance::cost_f[j] = costF;
}

double **Instance::getCostA() const {
    return cost_a;
}

void Instance::setCostA(double **costA) {
    Instance::cost_a = costA;
}

double Instance::getCostAIJ(int i, int j) const {
    return cost_a[i][j];
}

const string &Instance::getInputName() const {
    return input_name;
}

void Instance::setInputName(const string &inputName) {
    Instance::input_name = inputName;
}

const string &Instance::getSolutionName() const {
    return solution_name;
}

void Instance::setSolutionName(const string &solutionName) {
    Instance::solution_name = solutionName;
}

Instance::Instance(const string &inputName, const string &solutionName) : input_name(inputName),
                                                                          solution_name(solutionName) {}

Instance::Instance(int qtyFacilities, int qtyClients, const string &inputName, const string &solutionName)
        : qty_facilities(qtyFacilities), qty_clients(qtyClients), input_name(inputName), solution_name(solutionName) {
    allocateCosts();
}



Instance::~Instance() {
    if(cost_a){
        for (int i = 0; i < qty_clients; ++i) {
            delete [] cost_a[i];
        }
        delete [] cost_a;
    }
    if(cost_f){
        delete [] cost_f;
    }
}

void Instance::allocateCosts() {
    // Vetores que salvarao custos lidos no arquivo
    cost_f = new double[qty_facilities];

    cost_a = new double*[qty_clients];
    for (int i = 0; i < qty_clients; ++i) {
        cost_a[i] = new double[qty_facilities];
    }
}


void Instance::readFileInstance(string inputType) {

    // Declaracoes iniciais
    double auxRead;
    string auxCRead;

    ifstream inputFLP;

    /****************************
    Lendo o arquivo de entrada
    */

    // Abrindo arquivo
    inputFLP.open(input_name);

    if (!inputFLP) {
        cout << "Unable to open file";
        exit(1); // terminate with error
    }

    if(inputType.compare("1")==0){
        // Lendo do arquivo os valores que indicam a quantidade de instalacoes e clientes
        inputFLP >> qty_facilities >> qty_clients;

        if(DEBUG >= DISPLAY_BASIC){
            cout << "QTY FACILITIES: " << qty_facilities << " AND QTY CLIENTS: " << qty_clients << endl;
        }

        allocateCosts();

        // Lendo do arquivo os custos de abertura das instalacoes e salvando no vetor cost_f
        for(int i=0;i<qty_facilities;++i){
            inputFLP >> auxCRead; // Descartando a capacidade da instalação
            // cout << "Discard: " << auxCRead << endl;
            inputFLP >> auxRead; // Pegando o valor do custo de abertura da instalação
            cost_f[i] = auxRead;

            if(DEBUG >= DISPLAY_ACTIONS){
                cout << "Fi =  " << cost_f[i] << endl;
            }
        }

        // Lendo do arquivo os custos de atribuicao do clientes com as instalacoes e salvando no vetor cost_a
        for(int i=0;i<qty_clients;++i){
            inputFLP >> auxRead; // Descartando aqui a demanda do cliente

            if(DEBUG >= DISPLAY_ACTIONS){
                cout << "Demand = " << auxRead << endl;
            }

            for(int j=0;j<qty_facilities;++j){
                inputFLP >> auxRead;
                cost_a[i][j] = auxRead;

                if(DEBUG >= DISPLAY_ACTIONS){
                    cout << "AC = " << cost_a[i][j] << endl;
                }
            }
        }
    }
    else if(inputType.compare("2")==0){
        inputFLP >> auxCRead; // Descartando a palavra FILE
        inputFLP >> auxCRead; // Descartando o nome do arquivo

        // Lendo do arquivo os valores que indicam a quantidade de instalacoes e clientes. E descartando o 0
        inputFLP >> qty_facilities >> qty_clients >> auxRead;

        if(DEBUG >= DISPLAY_BASIC){
            cout << "QTY FACILITIES: " << qty_facilities << " AND QTY CLIENTS: " << qty_clients << endl;
        }

        allocateCosts();

        // Lendo do arquivo os custos de abertura das instalacoes e salvando no vetor cost_f
        for(int i=0;i<qty_facilities;++i){
            inputFLP >> auxRead; // Descartando o nome da instalação
            // cout << "Discard: " << auxRead << endl;
            inputFLP >> auxRead; // Pegando o valor do custo de abertura da instalação
            cost_f[i] = auxRead;

            if(DEBUG >= DISPLAY_ACTIONS){
                cout << "Fi =  " << cost_f[i] << endl;
            }

            // Lendo do arquivo os custos de atribuicao do clientes com as instalacoes e salvando no vetor cost_a
            for(int j=0;j<qty_clients;++j){

                inputFLP >> auxRead;
                cost_a[j][i] = auxRead;

                if(DEBUG >= DISPLAY_ACTIONS){
                    cout << "AC = " << cost_a[j][i] << endl;
                }
            }
        }
    }
    else{
        cout << "Invalid input type." << endl;
    }

    // Fechando o arquivo
    inputFLP.close();

    /*
    Fim ler o arquivo de entrada
    ******************************************/
}







