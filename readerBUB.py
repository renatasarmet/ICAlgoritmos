import os
import csv
import sys

# Solution list is a list of tuple. Each tuple has the following format : input file name, solution cost, time spent and if it is optimal
def export_csv(output_file,solution_list):
	with open(output_file, 'w', newline='') as csvfile:
		w = csv.writer(csvfile, dialect='excel')
		w.writerow(["Input file name", "Solution cost BUB"])
		for sol in solution_list:
			if sol[0] == "":
				w.writerow([])
			else:
				w.writerow([sol[0],sol[1]])


def get_solutions(file_location):
	with open(file_location, 'r') as input_csv_data_file:
		input_csv_data = input_csv_data_file.read()

	files_test = input_csv_data.split("\n")
	del(files_test[0]) # deleting the row containing the column names

	solution_list = []

	for file in files_test:
		elements = file.split(",")
		if len(elements) > 1 : # not to count any empty lines you might have
			solution_list.append([elements[0],elements[1]])
			solution_list.append([elements[0],elements[1]]) # Repeat as many times as I want
			solution_list.append([elements[0],elements[1]])
			solution_list.append([elements[0],elements[1]])
			solution_list.append([elements[0],elements[1]])
			solution_list.append([elements[0],elements[1]])

			solution_list.append([""]) # write blank line

	return solution_list



if __name__ == '__main__':

	# Where all test cases are listed
	file_location = 'sol_csv/BUB-BUB_or_OPT.csv'

	solutions = get_solutions(file_location)

	# Exporting csv 
	export_csv("solBUB.csv",solutions)