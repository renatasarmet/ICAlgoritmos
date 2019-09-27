import os
import csv

# Solution list is a list of tuple. Each tuple has the following format : input file name, solution cost and time spent
def export_csv(output_file,solution_list):
    with open(output_file, 'w', newline='') as csvfile:
        w = csv.writer(csvfile, dialect='excel')
        w.writerow(["Input file name", "Solution cost", "Time spent"])
        for sol in solution_list:
            w.writerow([sol[0],sol[1],sol[2]])


# If you want to list your test cases
file_location = 'testCasesReader.txt'

with open(file_location, 'r') as input_list_data_file:
    input_list_data = input_list_data_file.read()

files_test = input_list_data.split()
# ---- end if ----- #

# Else if you want to test everything you have and sort the result
# files_test = os.listdir('solutions')
# files_test.sort()
# IMPORTANT: IF YOU CHOOSE THIS CASE, YOU NEED TO CHANGE THE complete_file_name AND THE file_name
# ---- end else if ----- #

solution_list = []
for file_name in files_test:
	if file_name != ".DS_Store": # ignore this file
		print("Input file:", file_name)

		complete_file_name = 'solutions/' + file_name + ".sol"
		# complete_file_name = 'solutions/' + file_name
		with open(complete_file_name, 'r') as input_data_file:
			input_data = input_data_file.read()

		if(input_data):
			# solution :: 0 - minCost, 1 - avgCost, 2 - maxCost, 3 - avgTime, 4....qty_clients - connected facility
			solution = input_data.split(" ")

			if len(solution) >= 4: # checking if the solution is in a correct format
				solution_list.append((file_name,solution[0],solution[3])) 
				# solution_list.append((file_name[:-4],solution[0],solution[3])) #deleting ".sol" from filename

# Exporting csv 
export_csv("solutions.csv",solution_list)