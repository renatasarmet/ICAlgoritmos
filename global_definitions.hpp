#include <iostream>

#ifndef GLOBAL_DEFINITIONS_HPP
#define GLOBAL_DEFINITIONS_HPP

struct solutionType {
	int * assigned_facilities; // size qty_clients
	double finalTotalCost;
	double timeSpent;
	bool local_optimum;
	int * open_facilities; // size qty_facilities
};

#endif /* GLOBAL_DEFINITIONS_HPP */