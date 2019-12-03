#include <iostream>

struct solutionType {
	int * assigned_facilities;
	double finalTotalCost;
	bool local_optimum;
	double timeSpent;
};

solutionType latabu(char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA, solutionType solution, double a1, int lc1, int lc2, int lo1, int lo2, int seed, double limit_idle, int lh);
