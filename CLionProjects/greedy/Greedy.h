#ifndef CLIONPROJECTS_GREEDY_H
#define CLIONPROJECTS_GREEDY_H


#include "../global/Instance.h"
#include "../global/Solution.h"

class Greedy {
private:
    int qty_facilities;
    int qty_clients;
    int * delete_clients;
    int qty_non_active_cli;
    int qty_non_active_cli_g;
    double **sorted_cij;
    int **sorted_cijID;
    double biggest_cij;
    double biggest_fi;

    // RedNode
    double * c_minX;
    bool * active;
public:
    virtual ~Greedy();

    void initialize(Solution * solution);

    void run(Solution * solution);

    static void mergeSort(double *vector, int *vector_ID, int start_position, int end_position);

    void deletingNonActiveClients(int index_fac, int qty_cli_delete);

    void bestSubset(int &best_size, double &best_cost, int index_fac, double fi, double gain, int start_index);
};
#endif //CLIONPROJECTS_GREEDY_H
