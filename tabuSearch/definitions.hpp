#include <iostream>

struct solutionType {
	int * assigned_facilities;
	double finalTotalCost;
	bool local_optimum;
	double timeSpent;
};

solutionType tabuSearch(char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA, solutionType solution, int a1, int lc1, int lc2, int lo1, int lo2);
