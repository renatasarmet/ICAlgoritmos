#include <iostream>

struct solutionType {
	int * assigned_facilities;
	double finalTotalCost;
};

solutionType primalDual(int qty_clients, int qty_facilities, double * costF, double * costA);
