#include <iostream>

struct solutionType {
	int * assigned_facilities;
	double finalTotalCost;
	double timeSpent;
	int qty_clients;
};

solutionType handlesInput(char inputName[], char primalSolName[], char dualSolName[], char inputType[], int seed);
solutionType randRounding(int qtyFac, int qtyCli, double * costF, double * costA, double ** x_values, double * v_values, int seed);
