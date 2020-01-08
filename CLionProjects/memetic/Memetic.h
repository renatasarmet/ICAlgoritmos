#ifndef CLIONPROJECTS_MEMETIC_H
#define CLIONPROJECTS_MEMETIC_H

#include "../global/Solution.h"
#include "Tree.h"
#include <cfloat>

#define INDEX_POCKET 0
#define INDEX_CURRENT 1 // ocupa o segundo espaço de cada nó

#define MUTATION_RATE 0.01 // 1% 0.1// 10% //0.05 // 5% * qty_facilities
#define CROSSOVER_TYPE 4 // 1 para uniform, 2 para one-point, 3 para union, 4 para groups 0 para aleatorio cada vez
#define PROB_LA_RATE 50 // 50%
#define MAX_GEN_NO_IMPROVEMENT 5 // quantidade de geracoes sem melhora, para atualizar o root
#define MAX_CHANGES_ROOT 2 // indica a quantidade de vezes seguidas que pode atualizar o root sem melhorar a best salva

#define DISPLAY_BASIC 1 // corresponde a exibicao da quantidade de movimentos
#define DISPLAY_MOVES 2 // corresponde a todos os cout quando um movimento é realizado de fato
#define DISPLAY_ACTIONS 3 // corresponde a todos os cout quando uma acao é feita.
#define DISPLAY_DETAILS 4 // corresponde a todos os cout mais detalhados quando uma acao é feita.
#define DISPLAY_TIME 5 // corresponde aos calculos de tempo

#define DEBUG 2 // OPCOES DE DEBUG: 1 - MOSTRAR A QTD DE MOVIMENTOS, 2 PARA EXIBIR OS MOVIMENTOS REALIZADOS, 3 PARA EXIBIR ACOES, 4 PARA EXIBIR DETALHES DAS ACOES, 5 PARA EXIBIR TEMPO, 6 PARA EXIBIR AS MUDANÇAS NO GRAFO

class Memetic {
private:
    int qty_facilities;
    int qty_clients;

    Instance instance;

    int **cli_cli;

    Tree * tree;

    int QTY_INST_MUTATION;

    int qty_generations; // indica a quantidade de gerações já feitas
    int qty_gen_no_improv; // indica a quantidade de gerações sem melhora
    int qty_changes_root; // indica a quantidade vezes que mudou o root seguida
    int s1, s2, s3; // Auxiliares para identificação dos filhos no crossover
    double temp_best_total_cost; // indica o melhor custo encontrado com aquele root (se atualizou o root, o valor nao fica com o antigo)


public:
    virtual ~Memetic();

    void initialize(Solution * solution);

    void run(Solution * solution);

    void recombine(Solution * child, Solution *mother, Solution *father, int posNodeChild, int posIndividualChild);

    void crossoverMutation(Solution * child, Solution *mother, Solution *father);

    void uniformCrossover(Solution * child, Solution *mother, Solution *father);

    void onePointCrossover(Solution * child, Solution *mother, Solution *father);

    void unionCrossover(Solution * child, Solution *mother, Solution *father);

    void groupsCrossover(Solution * child, Solution *mother, Solution *father);

    void mutation(Solution * child);

};


#endif //CLIONPROJECTS_MEMETIC_H
