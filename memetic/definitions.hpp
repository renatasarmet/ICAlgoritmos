#include <iostream>
#include "../global_definitions.hpp"

solutionType memetic(char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA);

void mergeSortID(double *vector, int *vectorID, int startPosition, int endPosition);

void connect_nearest(solutionType * node, int qty_clients, int ** sorted_cijID, double ** assignment_cost);

solutionType set_initial_sol_G(int qty_facilities, int qty_clients, double * costF, double * costA);

solutionType set_initial_sol_LS_G(char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA, solutionType initial_sol);

void set_initial_sol_RANDOM(solutionType * node, int qty_facilities, int qty_clients, double * costF, double ** assignment_cost, int seed, int ** sorted_cijID);
