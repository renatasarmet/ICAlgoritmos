//#include <iostream>
//#include <fstream>
//#include <iomanip>
//
//#include "../global/Instance.h"
//#include "../global/Solution.h"
//#include "Greedy.h"
//
//#define DISPLAY_BASIC 1 // corresponde a exibicao do custo final da solucao
//
//#define DEBUG 1 // OPCOES DE DEBUG: 0 PARA NAO EXIBIR NADA, 1 PARA EXIBIR CUSTO FINAL
//
//using namespace std;
//
//int main(int argc, char *argv[]){
//
//    if(argc < 4){
//        cout << "Error in the parameters. You must enter 3 parameters" << endl;
//        return 0;
//    }
//
//    Instance instance(argv[1], argv[2]);
//
//    string input_type = argv[3];
//
//    // Arquivo para salvar a solucao
//    ofstream solutionTXT;
//
//    cout << fixed;
//    cout.precision(5);
//
//    // Lendo o arquivo de entrada
//    instance.readFileInstance(input_type);
//
//    Solution solution(instance);
//
//    Greedy greedy;
//
//    // Chamando a funcao que resolve o problema de fato
//    greedy.initialize(&solution);
//
//    if(DEBUG >= DISPLAY_BASIC){
//        cout << "Final total cost: " << solution.getFinalTotalCost() << endl;
//    }
//
//    //Abertura de arquivo para leitura e escrita do .sol .
//    solutionTXT.open(instance.getSolutionName(), std::ofstream::out | std::ofstream::trunc);
//
//    // Colocando no solutionTXT o valor do custo da solucao
//    solutionTXT << fixed << setprecision(5) << solution.getFinalTotalCost() << " ";
//
//    // Colocando no solutionTXT o tempo gasto
//    solutionTXT << solution.getTimeSpent();
//
//    // Colocando no solutionsTXT as instalacoes finais conectadas
//    for(int i=0; i < instance.getQtyClients(); i++){
//        solutionTXT << " " << solution.getAssignedFacilityJ(i);
//    }
//
//    // Fechando o arquivo
//    solutionTXT.close();
//
//    return 0;
//}
