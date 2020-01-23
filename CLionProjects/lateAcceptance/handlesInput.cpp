#include <iostream>
#include <cstdlib>
#include <fstream>
#include <iomanip>

#include "../global/Instance.h"
#include "../global/Solution.h"
#include "LateAcceptance.h"

#define DISPLAY_BASIC 1 // corresponde a exibicao do custo inicial da solucao e do custo final
#define DISPLAY_SIZE 2 // corresponde a exibicao do tamanho da entrada (quantidade de clientes e de instalacoes)

#define DEBUG 1 // OPCOES DE DEBUG: 0 PARA NAO EXIBIR NADA, 1 PARA EXIBIR CUSTO INICIAL E FINAL, 2 PARA EXIBIR TEMPO E QTD CLI E INST

using namespace std;

int main(int argc, char *argv[]){

    if(argc < 9){
        cout << "Error in the parameters. You must enter 8 parameters" << endl;
        return 0;
    }

    int best_fit = stoi(argv[5],nullptr,10); // convertendo argv[5] para inteiro
    double a1 = stod(argv[6],nullptr); // convertendo argv[6] para double
    double limit_idle = stod(argv[7],nullptr); // convertendo argv[7] para double
    double a_lh = stod(argv[8],nullptr); // convertendo argv[8] para double

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


    int lh;
    if(best_fit)
        lh = a_lh;
    else
        lh = a_lh * instance.getQtyFacilities();

    LateAcceptance lateAcceptance;

    // Chamando a funcao que resolve o problema de fato
    lateAcceptance.allocate(&solution, best_fit, a1, limit_idle, lh);

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


