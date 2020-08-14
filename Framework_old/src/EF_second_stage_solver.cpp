#include "auxi/MTRand.h"
#include "auxi/BRKGA.h"
#include <limits>
#include "EF_second_stage_solver.h"

namespace EF {

EF_second_stage_solver::EF_second_stage_solver(SP_instance &problem, int verbose, int time_limit):
										problem(problem),verbose(verbose),time_limit(time_limit){
}

EF_second_stage_solver::~EF_second_stage_solver() {
}

Brkga_Decoder::Brkga_Decoder(SP_instance &problem, int scenario, std::vector<bool>& selected_resources,double alpha):
						problem(problem),scenario(scenario),selected_resources(selected_resources),alpha(alpha) {
}

Brkga_Decoder::~Brkga_Decoder() {
}

double Brkga_Decoder::decode(std::vector< double >& chromosome) const {

	double total_cost = 0;
	std::vector<bool> resource_map(problem.get_number_of_resources());
	std::vector<double> allele_cost(problem.get_number_of_resources());
	std::vector<double> * costs_in_scenario = problem.get_vector_costs_in_scenario(scenario);

	for (int i = 0; i < problem.get_number_of_resources(); i++){
		if(selected_resources[i]) allele_cost[i] = 0;
		else{
			allele_cost[i] = EF_second_stage_solver::compute_perturbed_cost(alpha, chromosome[i], (*costs_in_scenario)[i]);
		}
	}

	problem.decode_Ap(problem, scenario, allele_cost, resource_map);

	for (int i = 0; i < problem.get_number_of_resources(); i++){
		if(resource_map[i] == true && selected_resources[i] == false){
			total_cost += (*costs_in_scenario)[i];
		}
	}
	return total_cost;
}

int EF_second_stage_solver::solve_brkga(int scenario,
		std::vector<bool>& selected_resources, std::vector<double>& chromosome,
		double alpha, int NUMBER_OF_GENERATIONS, int POPULATION_SIZE){

	Timer timer_global;
	timer_global.restart();

	const unsigned n = problem.get_number_of_resources();
	unsigned p = POPULATION_SIZE;
	double pe = 0.1;		// fraction of population to be the elite-set
	double pm = 0.2;		// fraction of population to be replaced by mutants
	double rhoe = 0.825;	// probability that offspring inherit an allele from elite parent

	unsigned K = 1;		    // number of independent populations
	const unsigned MAXT = 1;// number of threads for parallel decoding
	unsigned X_INTVL = 100;	// exchange best individuals at every 100 generations
	unsigned X_NUMBER = 2;	// exchange top X_NUMBER best

	Brkga_Decoder decoder(problem, scenario, selected_resources, alpha);

	std::vector < std::vector < double > > IniPopulation(p);
	for (unsigned i = 0; i < p; i++)	{
		IniPopulation[i].resize(n);
	}

	//Initialize first individual
	for (int i = 0; i < problem.get_number_of_resources(); i++)	{
		IniPopulation[0][i] = chromosome[i];
	}

	// initialize the BRKGA-based heuristic
	const long unsigned rngSeed = 0;	// seed to the random number generator
	MTRand rng(rngSeed);				// initialize the random number generator
	BRKGA< Brkga_Decoder, MTRand > algorithm(n, p, pe, pm, rhoe, decoder, rng, K, MAXT);

	algorithm.setInitialPopulation(IniPopulation, 1, 0);

	int current_generation = 0;

	//Brkga main loop
	do {
		algorithm.evolve();	// evolve the population for one current_generation
		if((++current_generation) % X_INTVL == 0) {
			algorithm.exchangeElite(X_NUMBER);	// exchange top individuals
		}
	} while (timer_global.elapsed() < time_limit && current_generation <= NUMBER_OF_GENERATIONS);

	chromosome.assign(algorithm.getBestChromosome().begin(), algorithm.getBestChromosome().end());

	time_used = timer_global.elapsed();
	solution_value = algorithm.getBestFitness();
	return solution_value;
}

int EF_second_stage_solver::solve(SP_solution& solution_final,
		std::vector<bool>& selected_resources, std::vector< std::vector <double> >& chromosomes,
		double alpha, int NUMBER_OF_GENERATIONS, int POPULATION_SIZE){

	Timer timer;
	timer.restart();

	double total_cost = 0;

	EF_second_stage_solver P_solver(problem, verbose, 36000);

	for (int s = 0; s < problem.get_number_of_scenarios(); s++){
		P_solver.solve_brkga(s, selected_resources, chromosomes[s], alpha, NUMBER_OF_GENERATIONS, POPULATION_SIZE);
		total_cost += P_solver.get_solution_value() * problem.get_probabilities_scenario(s);
	}

    for (int i = 0; i < problem.get_number_of_resources(); i++){
		if(selected_resources[i] == true) total_cost += problem.get_cost_first_stage(i);
	}

	solution_value = total_cost;
	time_used = timer.elapsed();
	return 0;
}


} /* namespace EF */
