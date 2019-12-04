#include <iostream>
#include "../global_definitions.hpp"

solutionType memetic(char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA);

void mergeSortID(double *vector, int *vectorID, int startPosition, int endPosition);

solutionType set_initial_sol(char * solutionName, int type, int qty_facilities, int qty_clients, double * costF, double * costA, solutionType initial_sol, int seed);