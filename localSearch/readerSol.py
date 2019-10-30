import os
import csv

# Solution list is a list of tuple. Each tuple has the following format : input file name, solution cost, time spent and if it is optimal
def export_csv(output_file,solution_list):
	with open(output_file, 'w', newline='') as csvfile:
		w = csv.writer(csvfile, dialect='excel')
		w.writerow(["Input file name", "Solution cost", "Time spent", "Optimal"])
		for sol in solution_list:
			w.writerow([sol[0],sol[1],sol[2],sol[3]])


# Solution list is a list of tuple. Each tuple has the following format : 
# input file name, seed of the best solution, min_cost, avg_cost, max_cost, 
# cost standard deviation, avg_time, time standard deviation 
def export_csv_rr(output_file,solution_list):
    with open(output_file, 'w', newline='') as csvfile:
        w = csv.writer(csvfile, dialect='excel')
        w.writerow(["file_name", "best_sol", "min_cost", "avg_cost", "max_cost", "sd_cost", "avg_time", "sd_time"])
        for sol in solution_list:
            w.writerow([sol[0],sol[1],sol[2],sol[3],sol[4],sol[5],sol[6],sol[7]])



if __name__ == '__main__':

	n_tests = 1 # Number of runs of each instance
	initial_sol_rr = False

	if len(sys.argv) > 1:
		initial_sol_rr = True # indica que o teste será com soluções iniciais vindas do RR
		n_tests = sys.argv[1].strip()

		if n_tests <= 0:
			print("ERROR: Invalid parameter value")
			print("SOLUTION: The parameter must be greater than 0")
			ok = False

	# Where all test cases are listed
	file_location = 'testCasesReader.txt'

	with open(file_location, 'r') as input_list_data_file:
		input_list_data = input_list_data_file.read()

	files_test = input_list_data.split()

	solution_list = []

	if not initial_sol_rr: # se a solução inicial não vier do RR
		for file_name in files_test:
			if file_name != ".DS_Store": # ignore this file
				print("Input file:", file_name)

				complete_file_name = 'solutions/' + file_name + ".sol"

				with open(complete_file_name, 'r') as input_data_file:
					input_data = input_data_file.read()

				if(input_data):
					# solution :: 0 - cost, 1 - timeSpent, 2 - optimal, 3....qty_clients - connected facility
					solution = input_data.split(" ")

					if len(solution) >= 3: # checking if the solution is in a correct format
						solution_list.append((file_name,solution[0],solution[1],solution[2])) 

		# Exporting csv 
		export_csv("solutions.csv",solution_list)

	else: # se a solução inicial vier do RR
		for file_name in files_test:
			if file_name != ".DS_Store": # ignore this file
				print("Input file:", file_name)

				min_cost = sys.maxsize
				max_cost = 0
				avg_cost = 0
				costs = []

				avg_time = 0
				times = []

				best_sol = -1 # seed of the solution with minimum cost

				for seed in range(1, n_tests+1): # seed ranges from 1 to n_tests

					complete_file_name = 'solutions/' + file_name + '_' + str(seed) + '.sol'

					with open(complete_file_name, 'r') as input_data_file:
						input_data = input_data_file.read()

					if(input_data):
						# solution :: 0 - cost, 1 - timeSpent, 2 - optimal, 3....qty_clients - connected facility
						solution = input_data.split(" ")

						if len(solution) >= 3: # checking if the solution is in a correct format
							costs.append(float(solution[0]))
							times.append(float(solution[1]))

							# first adding everything
							avg_cost += costs[-1]
							avg_time += times[-1]

							if costs[-1] > max_cost:
								max_cost = costs[-1]
							if costs[-1] < min_cost:
								min_cost = costs[-1]
								best_sol = seed

						else:
							print("PROBLEM WITH SEED", seed, ": the solution format is not right.")
							exit()
					else:
						print("PROBLEM WITH FILE:", file_name)
						exit()
					
				# Averaging
				avg_cost = avg_cost / N_TESTS
				avg_time = avg_time / N_TESTS

				# Standard Deviation Calculation for Cost and Time
				c_dividend = 0
				t_dividend = 0
				for i in range(N_TESTS):
					c_dividend += (costs[i] - avg_cost) ** 2
					t_dividend += (times[i] - avg_time) ** 2

				# sd_cost = (c_dividend / N_TESTS) ** (1/2) # population sd
				sd_cost = (c_dividend / (N_TESTS - 1)) ** (1/2) # sample sd
				# sd_time = (t_dividend / N_TESTS) ** (1/2) # population sd
				sd_time = (t_dividend / (N_TESTS - 1)) ** (1/2) # sample sd

				# Saving information for future csv
				solution_list.append((file_name, best_sol, min_cost, avg_cost, max_cost, sd_cost, avg_time, sd_time)) 

		# Exporting csv 
		export_csv_rr("solutions.csv",solution_list)