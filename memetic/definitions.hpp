#include <iostream>

struct solutionType {
	int * open_facilities; // size qty_facilities
	int * assigned_facilities; // size qty_clients
	double finalTotalCost;
	bool local_optimum;
	double timeSpent;
};

solutionType tabuSearch(char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA, solutionType solution, bool best_fit, double a1, int lc1, int lc2, int lo1, int lo2, int seed);
