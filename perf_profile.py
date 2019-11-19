import matplotlib.pyplot as plt
import numpy
import os
import sys

def get_solutions(file_location, initial_sol_rr, pp_type):
	with open(file_location, 'r') as input_csv_data_file:
		input_csv_data = input_csv_data_file.read()

	files_test = input_csv_data.split("\n")
	del(files_test[0]) # deleting the row containing the column names

	solution_list = []

	if not initial_sol_rr: # if the initial solution does not come from RR
		if pp_type == 1: # represents cost
			index = 1
		else: # represents time
			index = 2 
		for file in files_test:
			elements = file.split(",")

			if len(elements) > 1 : # not to count any empty lines you might have
				solution_list.append(elements[index]) # get the cost of the solution

	else: # if the initial solution comes from RR
		if pp_type == 1: # represents cost
			index = 3
		else: # represents time
			index = 6
		for file in files_test:
			elements = file.split(",")

			if len(elements) > 1 : # not to count any empty lines you might have
				solution_list.append(elements[index]) # get the average solution cost

	return solution_list


def calculate_best(solutions):
	best = [] # stores the best value for each instance

	for i in range(len(solutions[0])): # scroll through each instance
		best.append(float("inf")) # starts with an infinite value
		for alg in solutions: # scroll through algorithm types
			if float(alg[i]) < best[-1]: # updates the value of the best solution for this instance
				best[-1] = float(alg[i])

	return best


def calculate_distances(solutions,best_solutions):
	distances = [] # It is a matrix. The first vector contains len(solutions) elements. Each of these elements contains len(solutions[0]) elements and stores the distance of each instance of the best solution (in %).
	for alg in solutions: # scroll through algorithm types
		distances.append([])

		for i in range(len(solutions[0])): # scroll through each instance
			sub = float(alg[i]) - best_solutions[i]
			d = (sub * 100) / best_solutions[i] # calculate percentage of distance
			distances[-1].append(d)

		distances[-1].sort() # sort the distances

	return distances


def prepare_graphic(x_values,solutions, distances):
	y_values = [] # It is a matrix. The first vector contains len(solutions) elements. Each of these elements contains len(x_values) elements

	for alg in range(len(solutions)): # scroll through algorithm types
		qty = 0
		y_values.append([])
		for x in x_values: # scroll through x-axis points (% distance from best solution)
			while (qty < len(solutions[0])) and (distances[alg][qty] <= x): # until the vector is finished and the distance is less than x
				qty += 1

			y = (qty * 100)/len(solutions[0]) # calculating the percentage of quantity
			y_values[-1].append(y)

	return y_values


if __name__ == '__main__':

	if len(sys.argv) > 1:
		pp_type = int(sys.argv[1].strip()) # 1 - cost, 2 - time
		if(pp_type == 1):
			print("Performance Profile - Cost")
		else:
			print("Performance Profile - Time")
	else:
		print("Performance Profile - Cost")
		pp_type = 1; # default is cost 

	ZOOM = -1 # Used if you want to zoom the graph, ignoring the maximum value found in the solution. Note: use -1 if you do not want to zoom or set the desired maximum value

	solutions = [] # it will store the solutions of each algorithm
	legend = [] # it will indicate the order of the caption on the graph

	# Getting csv solution information from each algorithm
	# solutions.append(get_solutions("gurobi/solutionsTrivial.csv", False, pp_type))
	# legend.append("Trivial")
	# solutions.append(get_solutions("greedy/solutions.csv", False, pp_type))
	# legend.append("Greedy")
	# solutions.append(get_solutions("randomizedRounding/solutions.csv", True, pp_type))
	# legend.append("Randomized Rounding")
	# solutions.append(get_solutions("primalDual/solutions.csv",False, pp_type))
	# legend.append("Primal Dual")
	# solutions.append(get_solutions("localSearch/solutionsTrivial.csv",False, pp_type))
	# legend.append("LS - Trivial")
	# solutions.append(get_solutions("localSearch/solutionsGreedy.csv",False, pp_type))
	# legend.append("LS - Greedy")
	# solutions.append(get_solutions("localSearch/solutionsRR.csv",True, pp_type))
	# legend.append("LS - RR")
	# solutions.append(get_solutions("localSearch/solutionsDual.csv",False, pp_type))
	# legend.append("LS - PD")

	# testing just ga250a instances
	solutions.append(get_solutions("greedy/solutions.csv", False, pp_type))
	legend.append("Greedy")
	solutions.append(get_solutions("localSearch/solutions_LS_G_N1.csv",False, pp_type))
	legend.append("LS_G_N1")
	solutions.append(get_solutions("localSearch/solutions_LS_G_N2.csv",False, pp_type))
	legend.append("LS_G_N2")
	solutions.append(get_solutions("localSearch/solutions_LS_G.csv",False, pp_type))
	legend.append("LS_G")
	solutions.append(get_solutions("tabuSearch/solutionsGreedy.csv",False, pp_type))
	legend.append("TS_G")
	solutions.append(get_solutions("tabuSearch/solutionsTrivial.csv",False, pp_type))
	legend.append("TS_T")

	# Saving the best solution for each instance
	best_solutions = calculate_best(solutions)

	# Calculating the distance of each algorithm solution with the best solution of each instance
	distances = calculate_distances(solutions,best_solutions)

	"""
	Check the smallest and the biggest distance
	"""

	# Start with invalid values
	minimum = float("inf")
	maximum = -1

	# scroll through algorithm types
	for alg in distances: 
		i = 0
		while ((i < len(alg)) and (alg[i]==0)): # Guarantees that the minimum will not be 0
			i+=1

		# Saving the smallest and the biggest distance found
		if (i < len(alg)) and (alg[i] < minimum):
			minimum = alg[i]
		if (ZOOM < 0) and (alg[-1] > maximum):
			maximum = alg[-1]

	print("minimum: ", minimum)
	print("maximum: ", maximum)
	print("max / min: ", (maximum+minimum)/minimum)
	# if everything equals 0, the step will be 1
	if minimum == float("inf"):
		minimum = 1
	# # if the minimum is too small, we change for reasons of memory capacity
	# elif minimum < 0.0001:
	# 	minimum = 0.0001

	# we change for reasons of memory capacity (check which values to put here)
	if ((maximum+minimum)/minimum) > 1000:
		print("Changing minimum for reasons of memory capacity")
		minimum = 0.1

	# If you want to ignore the real maximum and zoom the graph
	if ZOOM >= 0:
		maximum = ZOOM

	""" End of checking the smallest and the biggest distance """

	"""
	build the x axis:
	- start = 0
	- end = maximum
	- step = minimum
	"""
	x_values = []
	for i in numpy.arange(0,maximum+minimum,minimum):
		x_values.append(i)
		
	""" End of building the x axis """

	# Preparing the graph's y values
	y_values = prepare_graphic(x_values,solutions,distances)

	# naming the x axis 
	plt.xlabel('x - % approximation') 
	# naming the y axis 
	plt.ylabel('y - % quantity') 
	
	# giving a title to my graph 
	if pp_type == 1: # if it's cost
		plt.title('Performance Profile UFL - Cost') 
	else: # if it's time
		plt.title('Performance Profile UFL - Time') 

	# Plotting the lines of each algorithm
	for i in range(len(solutions)):
		plt.plot(x_values,y_values[i],label=legend[i])

	# show a legend on the plot 
	plt.legend() 

	plt.show()
