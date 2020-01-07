//
// Created by Renata Sarmet Smiderle Mendes on 7/1/20.
//

#ifndef CLIONPROJECTS_LSCLOSEFAC_H
#define CLIONPROJECTS_LSCLOSEFAC_H

#include "../global/Solution.h"
#include <set>

class LSCloseFac {
private:
    int qty_facilities;
    int qty_clients;
    double biggest_cij;
    int qty_moves;
    double aux_cij;
    int n1;

    int cur_index_extra;
    int old_index_extra;
    double **extra_cost;

    double best_extra_cost; // Best delta in this iteration
    int fac_best_extra_cost; // Which facility corresponds to the best delta in this iteration

    set <int, greater <int> > open_facs; // conjunto de instalacoes que encontram-se abertas no momento

    set <int, greater <int> > :: iterator itr;  // Iteratores para os conjuntos
    set <int, greater <int> > :: iterator itr2;  // Iteratores para os conjuntos

    // RedNode
    int * nearest2_open_fac;// nearest2_open_fac - será correnpondente ao ID da segunda instalacao i tal que min_{i \in X} cij, sendo X as inst abertas exceto a nearest_open_fac (segunda instalacao aberta mais proxima)
    int * temp_nearest_fac; // temp_nearest_fac - será correnpondente ao ID da instalacao i mais proxima que sera conectada (temporario)
    int * temp_nearest2_fac; // temp_nearest2_fac - será correnpondente ao ID da segunda instalacao i mais proxima que sera conectada (temporario)

    double c3_minX;
    int nearest3_open_fac;
    double aux_cij3;

public:
    virtual ~LSCloseFac();

    void initialize(Solution * solution);

    void run(Solution * solution);
};


#endif //CLIONPROJECTS_LSCLOSEFAC_H
