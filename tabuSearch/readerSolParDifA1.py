import os
import csv
import sys


# Codigo para ver qual seria o resultado do custo e do tempo caso o parametro a1 fosse menor (new_a1)


# Solution list is a list of tuple. Each tuple has the following format : input file name, solution cost, time spent and if it is optimal
def export_csv(output_file,solution_list):
	with open(output_file, 'w', newline='') as csvfile:
		w = csv.writer(csvfile, dialect='excel')
		w.writerow(["Input file name", "Solution cost", "Time spent", "Optimal"])
		for sol in solution_list:
			if sol[0] == "":
				w.writerow([])
			else:
				w.writerow([sol[0],sol[1],sol[2],sol[3]])


def get_sol(file_location, qty_itr, prop):
	with open(file_location, 'r') as input_csv_data_file:
		input_csv_data = input_csv_data_file.read()

	lines = input_csv_data.split("\n")
	del(lines[0]) # deleting the row containing the column names

	for line in lines:
		elements = line.split(",")
		if len(elements) > 2: # not to count any empty lines you might have
			if int(elements[2]) == 0: 
				last_itr = 0
				sol_value = elements[1]
				if float(elements[0]) < 0:
					sol_time = 0
			else:
				if int(elements[2]) <= last_itr + qty_itr:
					last_itr = int(elements[2])
					sol_value = elements[1]
					sol_time = elements[0]

	if(lines[-1] == ""):
		end_time = float(lines[-2].split(",")[0])
		start_time = float(lines[-3].split(",")[0])
	else:
		end_time = float(lines[-1].split(",")[0])
		start_time = float(lines[-2].split(",")[0])

	if start_time < 0:
		start_time = 0
	if end_time < 0:
		end_time = 0
	time_check_end = (end_time - start_time) / prop
	sol_time = float(sol_time) + time_check_end 			

	return (sol_value, sol_time, last_itr)


if __name__ == '__main__':

	# Parameters values

	best_fit = ["0","1"]  # 1 para best fit 0 para first fit

	bc1 = ["0.01","0.04","0.01"] # equal to bo1
	bc2 = ["0.05","0.08","0.08"] # equal to bo2

	a1 = "2.5" # a1 original
	new_a1 = 0.5

	prop = float(a1) / 0.5

	qtd = len(bc1)

	# Where all test cases are listed
	file_location = 'testCasesReader.txt'

	with open(file_location, 'r') as input_list_data_file:
		input_list_data = input_list_data_file.read()

	files_test = input_list_data.split()

	solution_list = []

	for file_name in files_test:
		if file_name != ".DS_Store": # ignore this file
			print("Input file:", file_name)

			input_name = '../baseDeTestes/facilityTestCases/tests/' + file_name

			# é input_type = 2 que estou testand só, enta nao preciso fazer esse if
			with open(input_name, 'r') as input_name_data_file:
				input_data = input_name_data_file.read()

			if(input_data):
				file = input_data.split(" ")
				qty_fac = int(file[2])
				qty_itr = new_a1 * qty_fac

				# Fazendo as combinacoes a1 com bc para esse file_name
				for bf in best_fit:
					for i in range(qtd):
						instance_name = file_name + '__bf-' + bf + '_a1-' + a1 + '_l-' + bc1[i] + '-' + bc2[i]
						new_instance_name = file_name + '__bf-' + bf + '_a1-' + str(new_a1) + '_l-' + bc1[i] + '-' + bc2[i]

						complete_file_name = 'solutions/' + instance_name + '.sol.log'

						solution = get_sol(complete_file_name,qty_itr, prop)

						solution_list.append((new_instance_name,solution[0],solution[1],"0")) 

				solution_list.append(("","","","")) 

	# Exporting csv 
	export_csv("solutionsParDifA1-" + str(new_a1) + ".csv",solution_list)