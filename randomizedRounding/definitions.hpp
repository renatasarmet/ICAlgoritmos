#include <iostream>

struct solutionType {
	int * assigned_facilities;
	double finalTotalCost;
	double timeSpent;
	int qty_clients;
};

solutionType handlesInput(char inputName[], char initialSolName[], char inputType[]);
solutionType localSearch(int qtyFac, double * costF, double * costA, solutionType solution);
