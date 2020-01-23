//
// Created by Renata Sarmet Smiderle Mendes on 6/1/20.
//

#ifndef CLIONPROJECTS_TABUSEARCH_H
#define CLIONPROJECTS_TABUSEARCH_H


#include "../global/Solution.h"
#include <set>

class TabuSearch {
private:
    int qty_facilities;
    int qty_clients;
    double biggest_cij;
    int qty_moves;
    int * t;
    bool * flag;
    int cur_index_extra;
    int old_index_extra;
    double **extra_cost;
    int lc;
    int lo;
    int n1;
    double aux_cij;
    set <int, greater <int> > open_facs; // conjunto de instalacoes que encontram-se abertas no momento
    set <int, greater <int> > :: iterator itr; // Iteratores para os conjuntos

    bool best_fit;
    double a1;
    int lc1;
    int lc2;
    int lo1;
    int lo2;

    bool swap_done; // Variavel que indica se foi feito uma troca, se sim, devemos dar um break no for para voltar pro while
    double extra_cost2;  // Variavel que indica o custo extra de abrir ou fechar uma certa instalacao
    double complete_extra_cost2; // Variavel utilizada para calcular o custo completo extra do movimento de troca
    bool closed_nearest ; // Variavel auxiliar para ajudar quando houver substituicao de informacoes na troca

    double cur_cost;  // Total cost of the current solution
    double best_extra_cost; // Best delta in this iteration
    int fac_best_extra_cost; // Which facility corresponds to the best delta in this iteration
    int k_last_best; // The move number when solution.finalTotalCost is updated
    bool lets_move;  // Incates step 3.. if we will move in fact
    bool keep_searching; // Indica se continua ou nao o loop de busca
    bool update_near;  // Indica se vamos atualizar nearest 1 ou 2

    double c3_minX;
    int nearest3_open_fac;
    double aux_cij3;


    // RedNode
    double * c_minX;  // c_minX - será correnpondente a c(j,X) = min_{i \in X} cij, sendo X as inst abertas
    double * c2_minX;  // c2_minX - será correnpondente ao segundo menor c(j,X) = min_{i \in X} cij, sendo X as inst abertas exceto a nearest_open_fac
    int * nearest_open_fac; // nearest_open_fac - será correnpondente ao ID da instalacao i tal que min_{i \in X} cij, sendo X as inst abertas
    int * nearest2_open_fac;// nearest2_open_fac - será correnpondente ao ID da segunda instalacao i tal que min_{i \in X} cij, sendo X as inst abertas exceto a nearest_open_fac (segunda instalacao aberta mais proxima)
    int * temp_nearest_fac; // temp_nearest_fac - será correnpondente ao ID da instalacao i mais proxima que sera conectada (temporario)
    int * temp_nearest2_fac; // temp_nearest2_fac - será correnpondente ao ID da segunda instalacao i mais proxima que sera conectada (temporario)
    double * temp_c_minX; // temp_c_minX - será correnpondente ao custo c_min do temp nearest
    double * temp_c2_minX; // temp_c2_minX - será correnpondente ao custo c2_min do temp nearest2


public:
    virtual ~TabuSearch();

    void allocate(Solution * solution);

    void initialize(Solution * solution, bool _best_fit, double _a1, int _lc1, int _lc2, int _lo1, int _lo2, int seed);

    void run(Solution * solution);
};


#endif //CLIONPROJECTS_TABUSEARCH_H
