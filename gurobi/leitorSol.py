import os
import csv

# Solution list is a list of tuple. Each tuple has the following format : input file name, solution cost, time spent, if it is optimal or not
def export_csv(output_file,solution_list):
    with open(output_file, 'w', newline='') as csvfile:
        w = csv.writer(csvfile, dialect='excel')
        w.writerow(["Input file name", "Solution cost", "Time spent", "Optimal"])
        for sol in solution_list:
            w.writerow([sol[0],sol[1],sol[2],sol[3]])


files_test = os.listdir('solutions')
files_test.sort()

solution_list = []
for file_name in files_test:
	if file_name != ".DS_Store": # ignore this file
		print("Input file:", file_name)

		complete_file_name = 'solutions/' + file_name
		with open(complete_file_name, 'r') as input_data_file:
			input_data = input_data_file.read()

		if(input_data):
			# solution :: 0 - avgCost, 1 - avgTime, 2 - optimal
			solution = input_data.split(" ")

			if len(solution) >= 3: # checking if the solution is in a correct format
				solution_list.append((file_name[:-4],solution[0],solution[1],solution[2])) #deleting ".sol" from filename

# Exporting csv 
export_csv("solutions.csv",solution_list)