#include <iostream>

struct solutionType {
	int * assigned_facilities;
	double finalTotalCost;
};

solutionType randRounding(int qty_facilities, int qty_clients, double * costF, double * costA, double ** x_values, double * v_values, int seed);