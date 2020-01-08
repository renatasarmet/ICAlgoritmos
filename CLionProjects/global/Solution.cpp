#include "Solution.h"
#include <fstream>


#define DISPLAY_BASIC 1 // corresponde a exibicao do custo inicial da solucao e do custo final
#define DISPLAY_SIZE 2 // corresponde a exibicao do tamanho da entrada (quantidade de clientes e de instalacoes)
#define DISPLAY_ACTIONS 3 // corresponde a todos os cout quando uma informacao eh salva

#define DEBUG 2 // OPCOES DE DEBUG: 0 PARA NAO EXIBIR NADA, 1 PARA EXIBIR CUSTO INICIAL E FINAL, 2 PARA EXIBIR TEMPO E QTD CLI E INST, 3 PARA EXIBIR AS INFORMACOES SENDO SALVAS

Solution::Solution() {}

// Alocando memoria para os vetores e inicializando as variaveis
Solution::Solution(const Instance &instance) : instance(instance) {
    resetSolution();
}

Solution::~Solution() {
    delete [] assigned_facilities;
    delete [] open_facilities;
}

void Solution::resetSolution() {
    final_total_cost = 0;
    time_spent = 0;
    local_optimum = false;
    allocateMemory();
    initializeArrays();
    instance.setDestroyed(true);
}

//void Solution::initializeInstance(const Instance& _instance) {
//    setInstance(_instance);
//    resetSolution();
//}

void Solution::allocateMemory() {
    assigned_facilities = new int[instance.getQtyClients()];
    open_facilities = new bool[instance.getQtyFacilities()];
}

void Solution::initializeArrays() {
    for(int i=0; i < instance.getQtyClients(); ++i){
        assigned_facilities[i] = -1; // inicialmente nao está conectado a ninguem
    }

    for(int i=0; i < instance.getQtyFacilities(); ++i){
        open_facilities[i] = false; // inicialmente nenhuma está aberta
    }
}

int *Solution::getAssignedFacilities() const {
    return assigned_facilities;
}

void Solution::setAssignedFacilities(int *assignedFacilities) {
    assigned_facilities = assignedFacilities;
}

int Solution::getAssignedFacilityJ(int j) const {
    return assigned_facilities[j];
}

void Solution::setAssignedFacilityJ(int j, int assignedFacility) {
    assigned_facilities[j] = assignedFacility;
}

double Solution::getFinalTotalCost() const {
    return final_total_cost;
}

void Solution::setFinalTotalCost(double finalTotalCost) {
    final_total_cost = finalTotalCost;
}

void Solution::incrementFinalTotalCost(double increment) {
    final_total_cost += increment;
}

double Solution::getTimeSpent() const {
    return time_spent;
}

void Solution::setTimeSpent(double timeSpent) {
    time_spent = timeSpent;
}

bool Solution::isLocalOptimum() const {
    return local_optimum;
}

void Solution::setLocalOptimum(bool localOptimum) {
    local_optimum = localOptimum;
}

bool *Solution::getOpenFacilities() const {
    return open_facilities;
}

void Solution::setOpenFacilities(bool *openFacilities) {
    open_facilities = openFacilities;
}

bool Solution::getOpenFacilityJ(int j) const {
    return open_facilities[j];
}

void Solution::setOpenFacilityJ(int j, bool openFacility) {
    open_facilities[j] = openFacility;
}

Instance &Solution::getInstance() {
    return instance;
}

void Solution::setInstance(const Instance &instance) {
    Solution::instance = instance;
}

int Solution::getQtyFacilities() const {
    return instance.getQtyFacilities();
}

int Solution::getQtyClients() const {
    return instance.getQtyClients();
}

double *Solution::getCostF() const {
    return instance.getCostF();
}

double Solution::getCostFJ(int j) const {
    return instance.getCostFJ(j);
}

void Solution::setCostFJ(int j, double costF) {
    instance.setCostFJ(j, costF);
}

double **Solution::getCostA() const {
    return instance.getCostA();
}

double Solution::getCostAIJ(int i, int j) const {
    return instance.getCostAIJ(i,j);
}


void Solution::readFileInitialSol(string initialSolName) {
    /**************************************
    Lendo o arquivo com a solução inicial
    */

    // Declaracoes iniciais
    string auxCRead;

    ifstream initialSol;

    // Abrindo arquivo de entrada
    initialSol.open(initialSolName);

    if (!initialSol) {
        cout << "Unable to open file";
        exit(1); // terminate with error
    }

    // Lendo do arquivo o valor que indica o custo total
    initialSol >> final_total_cost;
    if(DEBUG >= DISPLAY_BASIC){
        cout << "Initial total cost: " << final_total_cost << endl;
    }

    initialSol >> auxCRead; // Descartando o tempo gasto
    // cout << "Discard time: " << auxCRead << endl;


    // Lendo do arquivo as instalacoes iniciais atribuidas a cada cliente e salvando no vetor solution.assigned_facilities
    for(int i=0;i<getQtyClients();i++){
        initialSol >> assigned_facilities[i];

        open_facilities[assigned_facilities[i]] = true; // indica que a instalação está aberta agora

        if(DEBUG >= DISPLAY_ACTIONS){
            cout << "client " << i << " =  " << assigned_facilities[i] << endl;
        }
    }

    // Fechando arquivo
    initialSol.close();

    /***
      Fim ler o arquivo com a solução inicial
      ******************************************/
}

void Solution::showSolution() const {
    cout << "Final total cost: " << final_total_cost << endl;
    cout << "Open facilities: ";
    for(int i=0; i<getQtyFacilities(); ++i){
        if(open_facilities[i]){
            cout <<  i << " ";
        }
    }
    cout << endl;
    cout << "Assigned Facilities: ";
    for(int i=0;i<getQtyClients();++i){
        cout << assigned_facilities[i] << " ";
    }
    cout << endl;
}


// Recebe por parametro um ponteiro para uma solucao que vai usar como modelo, copiando os valores dela para si mesmo
void Solution::copySolution(Solution *model) {
    instance.copyInstance(&model->getInstance());
    final_total_cost = model->getFinalTotalCost();
//    time_spent = model->getTimeSpent();
//    local_optimum = model->isLocalOptimum();

    for(int i=0;i<instance.getQtyFacilities();i++){
        open_facilities[i] = model->getOpenFacilityJ(i);
    }
    for(int j=0;j<instance.getQtyClients();j++){
        assigned_facilities[j] = model->getAssignedFacilityJ(j);
    }
}

// Chama o local search nele mesmo
void Solution::callLocalSearch(int ls_type) {
    LocalSearch localSearch;

    localSearch.initialize(this, ls_type); // chamando local search do tipo ls_type
}

// Chama o late acceptance nele mesmo
void Solution::callLateAcceptance() {
    LateAcceptance lateAcceptance;

    lateAcceptance.initialize(this, true, 2.5, 0.02, 10); // best_fit = true, a1 = 2.5, limit_idle = 0.02, lh = 10
}

// Verifica quantas instalacoes estao abertas
int Solution::getQtyOpenFacilities() {
    int qty = 0;
    for(int i=0; i < getQtyFacilities(); i++){
        if(open_facilities[i]){
            qty += 1;
        }
    }
    return qty;
}

// Conecta os clientes à instalacao aberta mais proxima. É importante que sorted_cijID e costA sejam realmente compativeis
void Solution::connectNearest(int **sorted_cijID) {
    int cont;
    for(int i=0;i<instance.getQtyClients();i++){
        cont = 0;
        // Percorrendo pelo vetor ordenado de ID do cij, quando encontrar a primeira inst que estiver aberta, para
        while(!open_facilities[sorted_cijID[i][cont]]){
            cont+=1;
        }
        // Atribui essa inst como inst aberta mais proxima
        assigned_facilities[i] = sorted_cijID[i][cont];

        // Aumenta no custo total final
        final_total_cost += instance.getCostAIJ(i,sorted_cijID[i][cont]);
    }
}

// Chama a função que conecta os clientes à instalacao aberta mais proxima, fecha as instalacoes que nao tem ninguem conectado e atualiza o final_total_cost
void Solution::connectAndUpdateFacilities(int **sorted_cijID) {
    bool used;

    // Zera o custo total final
    final_total_cost = 0;

    // Conectando cada cliente com a instalacao aberta mais proxima
    connectNearest(sorted_cijID);

    // Fechar as instalacoes que nao foram conectadas a ninguem
    for(int i=0; i<instance.getQtyFacilities(); i++){
        used = false;
        if(open_facilities[i]){ // se essa instalacao estiver aberta
            for(int j=0;j<instance.getQtyClients();j++){ // percorre pelos clientes para ver se alguem está conectada nela
                if(assigned_facilities[j] == i){
                    used = true;
                    break; // encontrou alguem concetado, entao sai
                }
            }
            if(!used){ // se nao tem ninguem conectado nela, vamos fecha-la
                open_facilities[i] = false;

                // if(DEBUG >= DISPLAY_DETAILS){
                cout << "Closing facility " << i << " because no clients were assigned to it"<< endl;
                // }
            }
            else{ // Se essa instalacao esta sendo usada, vai continuar aberta e devemos somar o custo
                final_total_cost += instance.getCostFJ(i);
            }
        }
    }
}

void Solution::call_tabu_search() {
    int lc1 = 0.01 * instance.getQtyFacilities();
    int lc2 = 0.05 * instance.getQtyFacilities();

    TabuSearch tabuSearch;

    // Chamando a funcao que resolve o problema de fato
    tabuSearch.initialize(this, true, 0.5, lc1, lc2, lc1, lc2, 0);  // best_fit = true, a1 = 0.5, (lc1 = lo1 = 0.01 * qty_facilities), (lc2 = lo2 = 0.05 * qty_facilities), seed = 0
}

void Solution::print_individual() {
    cout << endl;

    for(int i=0;i<instance.getQtyFacilities();i++){
        cout << open_facilities[i] << " ";
    }
    cout << endl << endl;
}

void Solution::print_open_facilities() {
    cout << endl << "Open facilities: ";

    for(int i=0;i<instance.getQtyFacilities() ;i++){
        if(open_facilities[i]){
            cout << i << " ";
        }
    }
    cout << endl << endl;
}

void Solution::call_local_search_close_fac() {
    LSCloseFac lsCloseFac;

    lsCloseFac.initialize(this);
}
