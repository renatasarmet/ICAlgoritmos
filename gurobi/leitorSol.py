import os
import csv
import sys

# Solution list is a list of tuple. Each tuple has the following format : input file name, solution cost, time spent, if it is optimal or not
def export_csv(output_file,solution_list):
	with open(output_file, 'w', newline='') as csvfile:
		w = csv.writer(csvfile, dialect='excel')
		w.writerow(["Input file name", "Solution cost", "Time spent", "Optimal"])
		for sol in solution_list:
			w.writerow([sol[0],sol[1],sol[2],sol[3]])




if __name__ == '__main__':

	ok = 1
	if len(sys.argv) > 1:
		formulation_type = sys.argv[1].strip()

		if(formulation_type == '1'):
			print("Integer Linear Program")
		elif(formulation_type == '2'):
			print("Linear Program")
		elif(formulation_type == '3'):
			print("Mixed Integer Linear Program")
		elif(formulation_type == '4'):
			print("Dual Linear Program")
		else:
			print("ERROR: Invalid parameter value")
			print("SOLUTION: Select 1 for ILP or 2 for LP or 3 for MLP or 4 for Dual")
			ok = 0


		if ok:

			file_location = 'testCasesReader.txt'

			with open(file_location, 'r') as input_list_data_file:
			    input_list_data = input_list_data_file.read()

			files_test = input_list_data.split()

			solution_list = []
			for file_name in files_test:
				if file_name != ".DS_Store": # ignore this file
					print("Input file:", file_name)

					if formulation_type == '4':
						formulation_type = 'Dual'

					complete_file_name = 'solutions' + formulation_type + '/' + file_name + ".sol"
					with open(complete_file_name, 'r') as input_data_file:
						input_data = input_data_file.read()

					if(input_data):
						# solution :: 0 - avgCost, 1 - avgTime, 2 - optimal, 3....qty_clients - connected facility
						solution = input_data.split(" ")

						if len(solution) >= 3: # checking if the solution is in a correct format
							solution_list.append((file_name,solution[0],solution[1],solution[2]))

			# Exporting csv 
			export_csv("solutions" + formulation_type + ".csv",solution_list)
	else:
		print("ERROR: You need to enter a parameter")
		print("SOLUTION: Select 1 for ILP or 2 for LP or 3 for MLP or 4 for Dual")
		ok = 0