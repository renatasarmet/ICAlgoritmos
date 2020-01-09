#ifndef CLIONPROJECTS_TREE_H
#define CLIONPROJECTS_TREE_H

#include "../global/Solution.h"
#include "../localSearch/LocalSearch.h"
#include "../lateAcceptance/LateAcceptance.h"
#include "../tabuSearch/TabuSearch.h"
#include "../localSearchCloseFac/LSCloseFac.h"
#include "../greedy/Greedy.h"

#define QTY_NODES_TREE 13
#define QTY_SOLUTIONS_NODE 2 // inclui 1 pocket e 1 current
#define INDEX_POCKET 0
#define INDEX_CURRENT 1 // ocupa o segundo espaço de cada nó
#define QTY_CHILDREN 3 // quantos nós filhos cada nó pai tem

#define LA_INITIAL_POP false // indica se quer fazer LA na pop inicial gerada pelo random
#define OPEN_RANDOM_RATE 10 //1.5 // 10% // probabilidade de abrir uma instalacao na geração de solucoes random
#define SHUFFLED_FACILITIES true // indica se vai gerar as solucoes iniciais garantindo que todas as instalacoes estarao presentes nas solucoes

#define DISPLAY_BASIC 1 // corresponde a exibicao da quantidade de movimentos
#define DISPLAY_MOVES 2 // corresponde a todos os cout quando um movimento é realizado de fato
#define DISPLAY_ACTIONS 3 // corresponde a todos os cout quando uma acao é feita.
#define DISPLAY_DETAILS 4 // corresponde a todos os cout mais detalhados quando uma acao é feita.
#define DISPLAY_TIME 5 // corresponde aos calculos de tempo

#define DEBUG 2 // OPCOES DE DEBUG: 1 - MOSTRAR A QTD DE MOVIMENTOS, 2 PARA EXIBIR OS MOVIMENTOS REALIZADOS, 3 PARA EXIBIR ACOES, 4 PARA EXIBIR DETALHES DAS ACOES, 5 PARA EXIBIR TEMPO, 6 PARA EXIBIR AS MUDANÇAS NO GRAFO

class Tree {
private:
    int qty_nodes; // indica quantos nodes tem na arvore
    int qty_solutions_node; // indica quantas solucoes/individuos tem em cada node
    Solution **nodes; // Cada nó é um agente, que possui 1 pocket e 1 current
    Instance * instance;

    Solution aux_sol;

    int * shuffled_facilities;
    int **sorted_cijID;

    int qty_shuf_made;
    int SLICE_SHUF;
    int MAX_QTY_SHUF_MADE;

    int * map;
    int * temp_open_facilities;
//    Instance * map_instance;

    int QTY_SUBS;  // Quantidade de pais que existem (quantidade de sub arvores)

public:

    explicit Tree(Instance * _instance);

    virtual ~Tree();

    int getQtyNodes() const;

    int getQtySolutionsNode() const;

    Solution **getNodes() const;

    Solution *getNodeJ(int j) const;

    Solution * getNodeJPosI(int j, int i) const;

    Solution * getPointerNodeJPosI(int j, int i);

    int **getSortedCijId() const;

    int getQtySubs() const;

    const Instance &getInstance() const;

    void initializePopulation();

    void copySolutions(int posNodeTarget, int posIndividualTarget, int posNodeModel, int posIndividualModel);

    void copyNodeToPointer(Solution * target, int posNodeModel, int posIndividualModel);

    void setInitialSolRandom(int posNode, int posIndividual);

    void setInitialSolShuffled(int posNode, int posIndividual);

    void checkShuffed(int *vet, int vetSize);

    static void createShuffledVector(int *vet, int vetSize);

    static void shuffle(int *vet, int vetSize);

    static void mergeSortID(double *vector, int *vectorID, int startPosition, int endPosition);

    void callGreedy(int posNode, int posIndividual);

    void mapAndCallG(int posNode, int posIndividual);

    void mapAndCallTS(int posNode, int posIndividual);

    int mapping(int posNode, int posIndividual);

    void unmapping(int posNode, int posIndividual, int cont_facilities);

    void printCountOpenFacilities();

    void printTreeComplete();

    void printSolutions();

    void updateSubPop(int idParent);

    void invertNodes(int posNode1, int posIndividual1, int posNode2, int posIndividual2);

    void updatePopulation();

    void printTree();

    void changeRoot(Solution * solution, int * qtyChangesRoot);

    void updatePopChangeRoot(int id_parent);

    void updateRefset();

    double comparePocketCurrent(int posNode);

    bool areDifferent(int posNode1, int posIndividual1, int posNode2, int posIndividual2);

    int qtyDiversity(bool * n1OpenFacilities, bool * n2OpenFacilities);

    void swapPocketCurrent(int posNode);

    void printTreeBest();

    void callLocalSearch(int posNode, int posIndividual, int lsType);

    void callLateAcceptance(int posNode, int posIndividual);

    void callTabuSearch(int posNode, int posIndividual);

    void callLocalSearchCloseFac(int posNode, int posIndividual);
};


#endif //CLIONPROJECTS_TREE_H
