//
// Created by Renata Sarmet Smiderle Mendes on 8/1/20.
//

#ifndef CLIONPROJECTS_MEMETIC_H
#define CLIONPROJECTS_MEMETIC_H





// RESOLVER SOBRE ESSA QUESTAO DOS DEFINES.. VAO FICAR ONDE? OLHA REPETIDO DA TREE

// RENOMEAR TODAS AS FUNCOES PARA CAMELCASE






#include "../global/Solution.h"
#include "Tree.h"
#include <cfloat>

#define QTY_NODES_TREE 13
#define QTY_SOLUTIONS_NODE 2 // inclui 1 pocket e 1 current
#define INDEX_POCKET 0
#define INDEX_CURRENT 1 // ocupa o segundo espaço de cada nó
#define QTY_CHILDREN 3 // quantos nós filhos cada nó pai tem

#define LA_INITIAL_POP false // indica se quer fazer LA na pop inicial gerada pelo random
#define OPEN_RANDOM_RATE 10 //1.5 // 10% // probabilidade de abrir uma instalacao na geração de solucoes random
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

    Tree tree;

    int QTY_INST_MUTATION;

    int qty_generations; // indica a quantidade de gerações já feitas
    int qty_gen_no_improv; // indica a quantidade de gerações sem melhora
    int qty_changes_root; // indica a quantidade vezes que mudou o root seguida
    int s1, s2, s3; // Auxiliares para identificação dos filhos no crossover
    double tempBestTotalCost; // indica o melhor custo encontrado com aquele root (se atualizou o root, o valor nao fica com o antigo)


public:
    virtual ~Memetic();

    void initialize(Solution * solution);

    void run(Solution * solution);

    void recombine(Solution * child, Solution *mother, Solution *father, int posNodeChild, int posIndividualChild);

    void crossover_mutation(Solution * child, Solution *mother, Solution *father);

    void uniform_crossover(Solution * child, Solution *mother, Solution *father);

    void one_point_crossover(Solution * child, Solution *mother, Solution *father);

    void union_crossover(Solution * child, Solution *mother, Solution *father);

    void groups_crossover(Solution * child, Solution *mother, Solution *father);

    void mutation(Solution * child);

};


#endif //CLIONPROJECTS_MEMETIC_H
