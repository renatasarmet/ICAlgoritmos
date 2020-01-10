#include <iostream>
#include <cstdlib>
#include <fstream>
#include <iomanip>

#include "../global/Instance.h"
#include "../global/Solution.h"
#include "TabuSearch.h"

#define DISPLAY_BASIC 1 // corresponde a exibicao do custo inicial da solucao e do custo final
#define DISPLAY_SIZE 2 // corresponde a exibicao do tamanho da entrada (quantidade de clientes e de instalacoes)

#define DEBUG 2 // OPCOES DE DEBUG: 0 PARA NAO EXIBIR NADA, 1 PARA EXIBIR CUSTO INICIAL E FINAL, 2 PARA EXIBIR TEMPO E QTD CLI E INST

using namespace std;

int main(int argc, char *argv[]){

    if(argc < 12){
        cout << "Error in the parameters. You must enter 10 parameters" << endl;
        return 0;
    }

    int best_fit = stoi(argv[5],nullptr,10); // convertendo argv[5] para inteiro
    double a1 = stod(argv[6],nullptr); // convertendo argv[6] para double
    double bc1 = stod(argv[7],nullptr); // convertendo argv[7] para double
    double bc2 = stod(argv[8],nullptr); // convertendo argv[8] para double
    double bo1 = stod(argv[9],nullptr); // convertendo argv[9] para double
    double bo2 = stod(argv[10],nullptr); // convertendo argv[10] para double
    int seed = stoi(argv[11],nullptr,10); // convertendo argv[11] para inteiro

    Instance instance(argv[1], argv[3]);

    string input_type = argv[4];

    // Arquivo para salvar a solucao
    ofstream solutionTXT;

    cout << fixed;
    cout.precision(5);

    // Lendo o arquivo de entrada
    instance.readFileInstance(input_type);

    Solution solution(instance);

    // Lendo solucao inicial
    solution.readFileInitialSol(argv[2]);

//    cout << "Initial solution:" << endl;
//    solution.showSolution();

    // Preparando intervalo lc e lo (por enquanto os intervalos serao iguais)
    int lc1, lc2, lo1, lo2;

    lc1 = lo1 = bc1 * instance.getQtyFacilities();
    lc2 = lo2 = bc2 * instance.getQtyFacilities();

    if(lo1==0){
        lc1 = lo1 = 1;
    }
    if(lo2==0){
        lc2 = lo2 = 2;
    }

    if(DEBUG >= DISPLAY_SIZE){
        cout << "Tabu tenure -> start: " << lc1 << "  end: " << lc2 << endl;
    }

    TabuSearch tabuSearch;

    // Chamando a funcao que resolve o problema de fato
    tabuSearch.initialize(&solution, best_fit, a1, lc1, lc2, lo1, lo2, seed);

    if(DEBUG >= DISPLAY_BASIC){
        cout << "Final total cost: " << solution.getFinalTotalCost() << endl;
    }

    //Abertura de arquivo para leitura e escrita do .sol .
    solutionTXT.open(instance.getSolutionName(), std::ofstream::out | std::ofstream::trunc);

    // Colocando no solutionTXT o valor do custo da solucao
    solutionTXT << fixed << setprecision(5) << solution.getFinalTotalCost() << " ";

    // Colocando no solutionTXT o tempo gasto
    solutionTXT << solution.getTimeSpent() << " ";

    // Colocando no solutionTXT se o otimo local foi encontrado
    solutionTXT << solution.isLocalOptimum();

    // Colocando no solutionsTXT as instalacoes finais conectadas
    for(int i=0; i < instance.getQtyClients(); i++){
        solutionTXT << " " << solution.getAssignedFacilityJ(i);
    }

    // Fechando o arquivo
    solutionTXT.close();

    return 0;
}
