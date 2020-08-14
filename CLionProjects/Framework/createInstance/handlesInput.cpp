// OUTPUT FILE FORMAT:
// #Facilities = m #Clientes = n
// f1 f2 ... fm
// c11 c12 ... c1m # custo de conexão de cada cliente
// ..
// cn1 cn2 ... cnm
// #Scenarios = S
// p1 p2 ... pS
// s1d1 s1d2 ... s1dn # quais clientes estão em quais cenários (vetor binário 0 1)
// s2d1 s2d2 ... s2dn
// ..
// sSd1 sSd2 ... sSdn
// s1f1 s1f2 ... s1fm # qual custo de cada instalação em cada cenário
// s2f1 s2f2 ... s2fm
// ..
// sSf1 sSf2 ... sSfm


#include <iostream>
#include <fstream>
#include <iomanip>

#include "../src/Instance.h"

#define DISPLAY_BASIC 1 // corresponde a exibicao do custo final da solucao
#define DEBUG 1 // OPCOES DE DEBUG: 0 PARA NAO EXIBIR NADA, 1 PARA EXIBIR ALGUMAS INFORMACOES

using namespace std;

// generate a random number between min and max
double randMinToMax(double min, double max){
    return ((max - min) * ((double)rand() / (double)RAND_MAX) + min);
}


int main(int argc, char *argv[]){

    // PARAMETROS: 
    // instância base (da literatura)
    // nome do arquivo que ira salvar
    // tipo da instancia (1 ou 2)
    // numero de cenarios
    // intervalo da inflacao (inicio e fim)
    // porcentagem de chance de cada cliente aparecer em cada cenario

    if(argc < 8){
        cout << "Error in the parameters. You must enter 7 parameters" << endl;
        return 0;
    }

    Instance instance(argv[1], argv[2]);

    string input_type = argv[3];

    int number_of_scenarios = stoi(argv[4],nullptr,10); // convertendo argv[4] para inteiro
    double inflation_start = stod(argv[5],nullptr); // convertendo argv[5] para double
    double inflation_end = stod(argv[6],nullptr); // convertendo argv[6] para double
    double chance_cli_scenario = stod(argv[7],nullptr); // convertendo argv[7] para double

    if(DEBUG >= 1){
        cout << "Numero de cenarios:" << number_of_scenarios << endl;
        cout << "Inflacao: ["<< inflation_start << "," << inflation_end << "]" << endl;
        cout << "Chance de cada cliente aparecer em cenario: " << chance_cli_scenario << endl;
    }

    /* initialize random seed: */
    srand(time(NULL));

    // Arquivo para salvar a solucao
    ofstream solutionTXT;

    cout << fixed;
    cout.precision(5);

    // Lendo o arquivo de entrada
    instance.readFileInstance(input_type);

    //Abertura de arquivo para leitura e escrita do .sol .
    solutionTXT.open(instance.getSolutionName(), std::ofstream::out | std::ofstream::trunc);

    // Colocando no solutionTXT a qtd de instalações e de clientes
    // #Facilities = m #Clientes = n
    solutionTXT << fixed << setprecision(5) << instance.getQtyFacilities() << " " << instance.getQtyClients() << endl;

    // Colocando no solutionsTXT o preco de cada instalacao no primeiro estagio
    // f1 f2 ... fm
    for(int j=0; j < instance.getQtyFacilities(); j++){
        solutionTXT << instance.getCostFJ(j) << " ";
    }
    solutionTXT << endl;

    // Colocando no solutionsTXT a conexao de cada cliente
    // c11 c12 ... c1m # custo de conexão de cada cliente
    // ..
    // cn1 cn2 ... cnm
    for(int i=0; i < instance.getQtyClients(); i++){
        for(int j=0; j < instance.getQtyFacilities(); j++){
            solutionTXT << instance.getCostAIJ(i,j) << " ";
        }
        solutionTXT << endl;
    }

    // Colocando a qtd de cenarios
    // #Scenarios = S
    solutionTXT << number_of_scenarios << endl;

    // Colocando as probabilidades de cada cenario
    // p1 p2 ... pS
    double * probs = new double[number_of_scenarios];
    double total_probs = 0;
    // sortear número real Ni entre [0,1] para cada cenario i
    // somar todos os Ni = TOTAL
    for(int s=0; s < number_of_scenarios; s++){
        probs[s] = randMinToMax(0,1);
        total_probs += probs[s];
    }
    // pi = Ni/TOTAL para todo i
    for(int s=0; s < number_of_scenarios; s++){
        probs[s] /= total_probs;
        solutionTXT << probs[s] << " ";
    }
    solutionTXT << endl;
    
    // Colocando quais clientes estao em quais cenarios
    // s1d1 s1d2 ... s1dn # quais clientes estão em quais cenários (vetor binário 0 1)
    // s2d1 s2d2 ... s2dn
    // ..
    // sSd1 sSd2 ... sSdn
    double prob_cli = 0;
    int is_cli_in_scenario = 0;
    for(int s=0; s < number_of_scenarios; s++){
        for(int i=0; i < instance.getQtyClients(); i++){
            prob_cli = randMinToMax(0,1);
            is_cli_in_scenario = prob_cli < chance_cli_scenario ? 1 : 0;
            solutionTXT << is_cli_in_scenario << " ";
        }
        solutionTXT << endl;
    }

    // Colocando o custo de cada instalação em cada cenario
    // s1f1 s1f2 ... s1fm # qual custo de cada instalação em cada cenário
    // s2f1 s2f2 ... s2fm
    // ..
    // sSf1 sSf2 ... sSfm
    double inflation = 0;
    for(int s=0; s < number_of_scenarios; s++){
        for(int j=0; j < instance.getQtyFacilities(); j++){
            inflation = randMinToMax(inflation_start, inflation_end);
            solutionTXT << inflation * instance.getCostFJ(j) << " ";
        }
        solutionTXT << endl;
    }

    // Fechando o arquivo
    solutionTXT.close();

    // DEFINICOES DE UMA INSTANCIA:
    // EXEMPLO: instanciaBase_5s_110-150i_50c
    //      numero de cenarios: 5 
    //      inflacao: [1.1,1.5]
    //      chance de cada cliente em cada cenario: Cada cliente está em um cenário com 50% de chance
    if(DEBUG >= 1){
        cout << "Instancia gerada: " << instance.getSolutionName() << endl;
    }

    return 0;
}
