#include <iostream>

struct solutionType {
	int * assigned_facilities;
	double finalTotalCost;
	bool local_optimum;
	double timeSpent;
};

solutionType lateAcceptance(char * solutionName, int qty_facilities, int qty_clients, double * costF, double * costA, solutionType solution, double a1, double limit_idle, int lh);
