//
// Created by Renata Sarmet Smiderle Mendes on 5/1/20.
//

#ifndef CLIONPROJECTS_GREEDY_H
#define CLIONPROJECTS_GREEDY_H


#include "../Instance.h"
#include "../Solution.h"

class Greedy {
private:
    int qty_facilities;
    int qty_clients;
    int * delete_clients;
    int qty_non_active_cli;
    int qty_non_active_cli_g;
    double **sorted_cij;
    int **sorted_cijID;
    double biggestCij;
    double biggestFi;

    // RedNode
    int * nearest_open_fac;
    double * c_minX;
    bool * active;
public:
    virtual ~Greedy();

public:
    void initialize(Solution * solution);

    void run(Solution * solution);

    static void mergeSort(double *vector, int *vectorID, int startPosition, int endPosition);

    void deletingNonActiveClients(double *vector, int *vectorID, int vector_size, int *clients_delete,
                                  int qty_cli_delete);

    void bestSubset(int &best_size, double &best_cost, double *vector, double fi, double gain, int start_index,
                    int vector_size, double biggest_cij);
};
#endif //CLIONPROJECTS_GREEDY_H
