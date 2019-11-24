import numpy as np
import seaborn as sns
import matplotlib.pyplot as plt
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

	ZOOM = -1 # Used if you want to zoom the graph, ignoring the maximum value found in the solution. Note: use -1 if you do not want to zoom or set the desired maximum value

	# Parameters values

	best_fit = ["0","1"]  # 1 para best fit 0 para first fit
	
	# a1 = ["0.1","0.5","1","2.5"]
	a1 = ["1"]

	limit_idle = ["0.02"] #??? #0.1 #0.02

	# lh = ["5","50","150"] #????
	lh = ["1","10","1000","10000"] #????

	initialPath = "solutions/"

	# Where all test cases are listed
	file_location = 'testCasesPlot.txt'

	with open(file_location, 'r') as input_list_data_file:
		input_list_data = input_list_data_file.read()

	files_test = input_list_data.split()

	for file_name in files_test:
		if file_name != ".DS_Store": # ignore this file
			print("Input file:", file_name)

			solutions = []
			legends = []

			# Fazendo as combinacoes para esse file_name
			for bf in best_fit:
				for l in lh:
					for li in limit_idle:
						for a in a1:
							instance_name = file_name + '__bf-' + bf + '_a1-' + a1 + '_idle-' + li + '_lh-' + l
							instance_file = instance_name + '.sol.log'
							path = initialPath + instance_file
							print(instance_name)
							solutions.append(get_solution(path))
							legends.append(instance_name)

			# Pegando o maior e menor valor do custo (para eixo y) e o maior numero de iteracoes (para eixo x)
			max_itr = -1
			max_cost = -1
			min_cost = float("inf")
			for s in solutions:
				aux_max_itr = len(s)
				aux_max_cost = max(s)
				aux_min_cost = min(s)
				if aux_max_itr > max_itr:
					max_itr = aux_max_itr
				if aux_max_cost > max_cost:
					max_cost = aux_max_cost
				if aux_min_cost < min_cost:
					min_cost = aux_min_cost

			print("MAX ITR: ", max_itr)
			print("MAX COST: ", max_cost)
			print("MIN COST: ", min_cost)


			# If you want to ignore the real maximum and zoom the graph
			if ZOOM >= 0:
				max_itr = ZOOM
				for i in range(len(solutions)):
					if(len(solutions[i]) > ZOOM):
						solutions[i] = solutions[i][:ZOOM]


			x_values = []
			# Definindo cada x_value
			for sol in solutions:
				x_values.append([])
				for i in np.arange(1,len(sol)+1):
					x_values[-1].append(i)


			# Aumentando o limite de cores
			sns.set()
			sns.set_palette(sns.color_palette("Paired",len(solutions)))


			step_y = (max_cost - min_cost) / 6
			# print("max: ", maxi)
			# print("min: ", mini)
			# print("step_y: ", step_y)
			plt.yticks(np.arange(min_cost,max_cost + 1,step_y))

			step_x = int(max_itr / 10)
			if step_x == 0:
				step_x = 1
			# print("len: ", max_itr)
			# print("step_x: ", step_x)	
			plt.xticks(np.arange(0,max_itr+1,step_x))


			# naming the x axis 
			plt.xlabel('x - iterations') 
			# naming the y axis 
			plt.ylabel('y - best cost') 
			
			# giving a title to my graph 
			title = 'Best cost UFL - ' + file_name
			if ZOOM >= 0:
				title += ' - zoom ' + str(ZOOM)
			plt.title(title) 

			# Plotting the lines of each algorithm
			for i in range(len(solutions)):
				sns.lineplot(x_values[i],solutions[i],label=legends[i])

			# show a legend on the plot 
			# plt.legend(loc='upper left', bbox_to_anchor=(0.1, 1), ncol=2, fancybox=True, shadow=True)
			plt.legend() 

			plt.show()
