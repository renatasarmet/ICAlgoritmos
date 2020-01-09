#include "Instance.h"
#include <fstream>

#define DISPLAY_BASIC 1
#define DISPLAY_ACTIONS 2 // corresponde a todos os cout quando uma acao é feita.

#define DEBUG 0 // OPCOES DE DEBUG: 1 PARA EXIBIR BASICO, 2 PARA EXIBIR ACOES


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

void Instance::setCostFJ(int j, double cost) {
    Instance::cost_f[j] = cost;
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

void Instance::setCostAIJ(int i, int j, double cost) {
    Instance::cost_a[i][j] = cost;
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

void Instance::setDestroyed(bool destroyed) {
    Instance::destroyed = destroyed;
}

Instance::Instance() {
    input_name = "";
    solution_name = "";
}

Instance::Instance(const string &inputName, const string &solutionName) : input_name(inputName),
                                                                          solution_name(solutionName) {}

Instance::Instance(int qtyFacilities, int qtyClients, const string &inputName, const string &solutionName)
        : qty_facilities(qtyFacilities), qty_clients(qtyClients), input_name(inputName), solution_name(solutionName) {
    allocateCosts();
}



Instance::~Instance() {
    if(!destroyed){ // utilizando isso pois estava dando problema de desalocar duas vezes
        if (qty_clients) {
            delete [] cost_a[0];
            cost_a[0] = NULL;
        }
        delete [] cost_a;
        cost_a = NULL;

        delete [] cost_f;
        cost_f = NULL;
    }

    destroyed = true;
}

void Instance::allocateCosts() {
    destroyed = false; // indica que terei que desalocar depois

    // Vetores que salvarao custos lidos no arquivo
    cost_f = new double[qty_facilities];

    cost_a = new double*[qty_clients];
    if (qty_clients)
    {
        cost_a[0] = new double[qty_clients * qty_facilities];
        for (int i = 1; i < qty_clients; ++i) {
            cost_a[i] = cost_a[0] + i * qty_facilities;
        }
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

void Instance::showInstance() const {
//    for(int i=0; i<qty_facilities; ++i){
//        cout << "Facility " << i << ": " << cost_f[i] << endl;
//
//        for(int j=0; j<qty_clients; ++j){
//            cout << "Client " << j << ": " << cost_a[j][i] << endl;
//        }
//        cout << endl;
//    }

    for(int i=qty_facilities-1; i>=0; --i){
        cout << "Facility " << i << ": " << cost_f[i] << endl;

        for(int j=qty_clients-1; j>=0; --j){
            cout << "Client " << j << ": " << cost_a[j][i] << endl;
        }
        cout << endl;
    }
}

// Copia os valores de uma instancia modelo para essa
void Instance::copyInstance(const Instance *model) {
    qty_facilities = model->qty_facilities;
    qty_clients = model->qty_clients;
    input_name = model->getInputName();
    solution_name = model->getSolutionName();

    allocateCosts();

    for(int j=0;j<qty_facilities;++j){
        cost_f[j] = model->getCostFJ(j);

        for(int i=0;i<qty_clients;i++){
            cost_a[i][j] = model->getCostAIJ(i,j);
        }
    }
}










