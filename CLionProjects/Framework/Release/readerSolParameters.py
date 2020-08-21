import os
import csv
import sys

# Solution list is a list of tuple. Each tuple has the following format : input file name, solution cost and time spent
def export_csv(output_file,solution_list):
	with open(output_file, 'w', newline='') as csvfile:
		w = csv.writer(csvfile, dialect='excel')
		w.writerow(["Input file name", "Solution cost", "Time spent"])
		for sol in solution_list:
			if sol[0] == "":
				w.writerow([])
			else:
				w.writerow([sol[0],sol[1],sol[2]])


if __name__ == '__main__':

	# Parameters values
	number_of_scenarios = [5, 50] # numero de cenarios
	inflation_start = [1.1] # inicio intervalo da inflacao
	inflation_end = [1.5]  # fim intervalo da inflacao 
	chance_cli_scenario = [0.5] # porcentagem de chance de cada cliente aparecer em cada cenario

	# Where all test cases are listed
	file_location = 'testCasesReader.txt'

	with open(file_location, 'r') as input_list_data_file:
		input_list_data = input_list_data_file.read()

	files_test = input_list_data.split()

	solution_list = []

	for file_name in files_test:
		if file_name != ".DS_Store": # ignore this file
			print("Input file:", file_name)

			file_name_replace = file_name.replace('.', '-')

			# Fazendo as combinacoes para esse file_name
			for n_scenario in number_of_scenarios:
				for i_start in inflation_start:
					for i_end in inflation_end:
						for chance in chance_cli_scenario:

							instance_name = file_name_replace + '_' + str(n_scenario) + 's_' + str(int(i_start*100)) + '-' + str(int(i_end*100)) + 'i_' + str(int(chance*100)) + 'c.txt'
							print("Instance name:", instance_name)

							complete_file_name = 'solutions/' + instance_name + '.sol'

							with open(complete_file_name, 'r') as input_data_file:
								input_data = input_data_file.read()

							if(input_data):
								# solution :: [0] - long debug, [1] - final time, [2] - final solution cost
								solution = input_data.split("@")

								if len(solution) == 3: # checking if the solution is in a correct format
									final_time = solution[1].split(" ")[1].replace('\n','')
									final_solution_cost = solution[2].split(" ")[1].replace('\n','')
									solution_list.append((instance_name, final_solution_cost, final_time)) 


			# solution_list.append(("","","","")) 

	# Exporting csv 
	export_csv("solutionsParameters.csv",solution_list)
