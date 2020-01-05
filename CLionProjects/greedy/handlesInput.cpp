#include <iostream>
#include <cstdlib>
#include <fstream>
#include <iomanip>

#include "../Instance.h"
#include "../Solution.h"
#include "Greedy.h"

#define DISPLAY_BASIC 1 // corresponde a exibicao do custo final da solucao
#define DISPLAY_SIZE 2 // corresponde a exibicao do tempo total gasto e do tamanho da entrada (quantidade de clientes e de instalacoes)
#define DISPLAY_ACTIONS 3 // corresponde a todos os cout quando uma informacao eh salva

#define DEBUG 1 // OPCOES DE DEBUG: 0 PARA NAO EXIBIR NADA, 1 PARA EXIBIR CUSTO FINAL, 2 PARA EXIBIR TEMPO E QTD CLI E INST, 3 PARA EXIBIR AS INFORMACOES SENDO SALVAS
using namespace std;

int main(int argc, char *argv[]){
//    if(argc < 4){
//        cout << "Error in the parameters. You must enter 3 parameters" << endl;
//        return 0;
//    }
//
//    Instance instance(argv[1], argv[2]);
//
//    string inputType = argv[3];

    Instance instance("../baseDeTestes/facilityTestCases/tests/cap71.txt", "solutions/cap71.txt.sol");

    string inputType = "1";

    // Arquivo para salvar a solucao
    ofstream solutionTXT;

    cout << fixed;
    cout.precision(5);

    // Lendo o arquivo de entrada
    instance.readFileInstance(inputType);

    Solution solution(instance);

    Greedy greedy;

    // Chamando a funcao que resolve o problema de fato
    greedy.initialize(&solution);

    if(DEBUG >= DISPLAY_BASIC){
        cout << "Final total cost: " << solution.getFinalTotalCost() << endl;
    }

    //Abertura de arquivo para leitura e escrita do .sol .
    solutionTXT.open(instance.getSolutionName(), std::ofstream::out | std::ofstream::trunc);

    // Colocando no solutionTXT o valor do custo da solucao
    solutionTXT << fixed << setprecision(5) << solution.getFinalTotalCost() << " ";

    // Colocando no solutionTXT o tempo gasto
    solutionTXT << solution.getTimeSpent();

    // Colocando no solutionsTXT as instalacoes finais conectadas
    for(int i=0; i < instance.getQtyClients(); i++){
        solutionTXT << " " << solution.getAssignedFacilityJ(i);
    }

    return 0;
}
