#ifndef EF_SECOND_STAGE_SOLVER_H_
#define EF_SECOND_STAGE_SOLVER_H_

#include "auxi/myutils.h"
#include "auxi/timer.hpp"
#include <list>
#include <vector>
#include <algorithm>
#include <math.h>

#include "EF_generic_solver.h"
#include "SP_instance.h"
#include "SP_solution.h"

namespace EF {

class EF_second_stage_solver : public EF_generic_solver{
private:
	SP_instance &problem;
	int verbose;
	int time_limit;

public:
	EF_second_stage_solver(SP_instance &problem, int verbose, int time_limit);
	virtual ~EF_second_stage_solver();

	int solve_brkga(int scenario, std::vector<bool>& selected_resources, std::vector<double>& chromosome,
			double alpha, int NUMBER_OF_GENERATIONS, int POPULATION_SIZE);

	int solve(SP_solution& solution_final,
			std::vector<bool>& selected_resources, std::vector< std::vector <double> >& chromosomes,
			double alpha, int NUMBER_OF_GENERATIONS, int POPULATION_SIZE);

	double static compute_perturbed_cost(double alpha, double allele, double original_cost){
		return (1 - alpha + (2 * alpha * allele)) * original_cost;
	}

};

class Brkga_Decoder {
private:
	SP_instance &problem;
	int scenario;
	std::vector<bool> &selected_resources;
	double alpha;
public:
	Brkga_Decoder(SP_instance &problem, int scenario, std::vector<bool>& selected_resources, double alpha);
	~Brkga_Decoder();

	double decode(std::vector< double >& chromosome) const;

private:
};

} /* namespace EF */


#endif /* EF_SECOND_STAGE_SOLVER_H_ */

