import os
import csv
import sys

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


if __name__ == '__main__':

	# Parameters values

	# best_fit = ["0","1"]  # 1 para best fit 0 para first fit
	best_fit = ["1"]  # 1 para best fit 0 para first fit
	
	# a1 = ["0.1","0.5","1","2.5"]
	a1 = ["2.5"]

	limit_idle = ["0.02"] #??? #0.1 #0.02

	# lh = ["1","10"]
	# lh = ["1","10","50","250","500","1000","10000"] #????
	# lh = ["1","10","50","250","1000","10000"] #????


	# Where all test cases are listed
	file_location = 'testCasesReader.txt'

	with open(file_location, 'r') as input_list_data_file:
		input_list_data = input_list_data_file.read()

	files_test = input_list_data.split()

	solution_list = []

	for file_name in files_test:
		if file_name != ".DS_Store": # ignore this file
			print("Input file:", file_name)

			# Fazendo as combinacoes bf, lh, li e a1 para esse file_name
			for bf in best_fit:
				if bf == "1":
					a_lh = ["10"] # nesse caso sera o valor de lh mesmo
				else:
					a_lh = ["0.05","0.1","0.5","1"] # nesse caso sera multiplicado pelo numero de instalacoes
				for l in a_lh:
					for li in limit_idle:
						for a in a1:
							instance_name = file_name + '__bf-' + bf + '_a1-' + a + '_idle-' + li + '_lh-' + l
							complete_file_name = 'solutions/' + instance_name + '.sol'

							with open(complete_file_name, 'r') as input_data_file:
								input_data = input_data_file.read()

							if(input_data):
								# solution :: 0 - cost, 1 - timeSpent, 2 - optimal, 3....qty_clients - connected facility
								solution = input_data.split(" ")

								if len(solution) >= 3: # checking if the solution is in a correct format
									solution_list.append((instance_name,solution[0],solution[1],solution[2])) 


			# solution_list.append(("","","","")) 
			# solution_list.append(("","","","")) 

	# Exporting csv 
	export_csv("solutionsParameters.csv",solution_list)