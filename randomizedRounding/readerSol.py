import os
import csv
import sys

# Solution list is a list of tuple. Each tuple has the following format : 
# input file name, seed of the best solution, min_cost, avg_cost, max_cost, 
# cost standard deviation, avg_time, time standard deviation 
def export_csv(output_file,solution_list):
    with open(output_file, 'w', newline='') as csvfile:
        w = csv.writer(csvfile, dialect='excel')
        w.writerow(["file_name", "best_sol", "min_cost", "avg_cost", "max_cost", "sd_cost", "avg_time", "sd_time"])
        for sol in solution_list:
            w.writerow([sol[0],sol[1],sol[2],sol[3],sol[4],sol[5],sol[6],sol[7]])


# Where all test cases are listed
file_location = 'testCasesReader.txt'

with open(file_location, 'r') as input_list_data_file:
    input_list_data = input_list_data_file.read()

files_test = input_list_data.split()

# Number of runs of each instance
N_TESTS = 10

solution_list = []
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

		for seed in range(1, N_TESTS+1): # seed ranges from 1 to N_TESTS

			complete_file_name = 'solutions/' + file_name + '_' + str(seed) + '.sol'

			with open(complete_file_name, 'r') as input_data_file:
				input_data = input_data_file.read()

			if(input_data):
				# solution :: 0 - cost, 1 - timeSpent, 2....qtd_clientes - connected facility
				solution = input_data.split(" ")

				if len(solution) >= 2: # checking if the solution is in a correct format
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
export_csv("solutions.csv",solution_list)