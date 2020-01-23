
#ifndef CLIONPROJECTS_LATEACCEPTANCE_H
#define CLIONPROJECTS_LATEACCEPTANCE_H

#include <set>
#include "../global/Solution.h"

class LateAcceptance {
private:
    int qty_facilities;
    int qty_clients;
    double biggest_cij;
    int qty_moves;
    int qty_moves_done;
    int qty_inst_used;
    bool * flag;
    double * fa;
    int cur_index_extra;
    int old_index_extra;
    double **extra_cost;
    int n1;
    double aux_cij;
    set <int, greater <int> > open_facs; // conjunto de instalacoes que encontram-se abertas no momento
    set <int, greater <int> > :: iterator itr; // Iteratores para os conjuntos

    double a1;
    int lh;
    double limit_idle;
    bool best_fit;

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

    int idle_itr;
    int v; // virtual beggining


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
    void allocate(Solution * solution, bool _bestFit, double _a1, double _limitIdle, int _lh);

    void initialize(Solution * solution, bool _bestFit, double _a1, double _limitIdle, int _lh);

    void run(Solution * solution);

    virtual ~LateAcceptance();
};


#endif //CLIONPROJECTS_LATEACCEPTANCE_H
