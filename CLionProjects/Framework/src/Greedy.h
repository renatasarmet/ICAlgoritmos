#ifndef CLIONPROJECTS_GREEDY_H
#define CLIONPROJECTS_GREEDY_H


#include "Instance.h"
#include "Solution.h"

class Greedy {
private:
    int qty_facilities;
    int qty_clients;
    int * delete_clients;
    int qty_non_active_cli;
    int qty_non_active_cli_g;
    double **sorted_cij_g;
    int **sorted_cij_ID_g;
    double biggest_cij;
    double biggest_fi;

    // RedNode
    double * c_minX;
    bool * active;
public:
    virtual ~Greedy();

    void allocate(Solution * solution);

    void initialize(Solution * solution);

    void run(Solution * solution);

    static void mergeSort(double *vector, int *vectorID, int startPosition, int endPosition);

    void deletingNonActiveClients(int indexFac, int qtyCliDelete);

    void bestSubset(int &bestSize, double &bestCost, int indexFac, double fi, double gain, int startIndex);
};
#endif //CLIONPROJECTS_GREEDY_H
