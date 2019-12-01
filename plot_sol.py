import matplotlib.pyplot as plt
import numpy
import os
import sys

def get_solution(file_location):
	with open(file_location, 'r') as input_data_file:
		input_log_data = input_data_file.read()

	cur_sol = input_log_data.split("\n")
	del(cur_sol[0]) # deleting the row containing the column names

	solution_list = []
	counter = 0

	for sol in cur_sol:
		elements = sol.split(",")

		if len(elements) > 1 : # not to count any empty lines you might have
			while(int(elements[2])>counter):
				# print(counter, " old: ", old)
				solution_list.append(old) # get the cost of the old solution
				counter += 1

			solution_list.append(float(elements[1])) # get the cost of the solution
			old = float(elements[1])
			# print(counter, " new: ", old)
			counter += 1

	return solution_list



if __name__ == '__main__':

	if len(sys.argv) > 1:
		path = sys.argv[1].strip()
	else:
		path = "lateAcceptance/solOfast/ga250a-3__bf-1_a1-2.5_idle-0.02_lh-10.sol.log_detail"


	# testing just ga250a instances
	solution = get_solution(path)
	# solution = [5,6,7,8,9,10]

	x_values = []
	for i in numpy.arange(1,len(solution)+1):
		x_values.append(i)

	maxi = max(solution)
	mini = min(solution) 
	step = (maxi - mini) / 6
	# print("max: ", maxi)
	# print("min: ", mini)
	# print("step: ", step)
	plt.yticks(numpy.arange(mini,maxi + 1,step))

	step = int(len(solution) / 10)
	if step == 0:
		step = 1
	# print("len: ", len(solution))
	# print("step: ", step)
	plt.xticks(numpy.arange(0,len(solution),step))

	# naming the x axis 
	plt.xlabel('x - iterations') 
	# naming the y axis 
	# plt.ylabel('y - current cost') 
	
	# giving a title to my graph 
	plt.title('Cost UFL') 

	# Plotting the lines of each algorithm
	plt.plot(x_values,solution)


	plt.show()
