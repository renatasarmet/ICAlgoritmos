#include <iostream>

struct solutionType {
	int * assigned_facilities;
	double finalTotalCost;
};

solutionType localSearch(int qty_facilities, int qty_clients, double * costF, double * costA, solutionType solution);
