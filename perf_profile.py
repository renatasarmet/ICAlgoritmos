import matplotlib.pyplot
import numpy

#https://www.geeksforgeeks.org/graph-plotting-in-python-set-1/

# meses = ['Janeiro', 'Fevereiro', 'Março', 'Abril', 'Maio', 'Junho']
# valores = [105235, 107697, 110256, 109236, 108859, 109986]

# matplotlib.pyplot.plot(meses, valores)

# matplotlib.pyplot.show()

import os
import sys

def get_solutions(file_location,initial_sol_rr):
	with open(file_location, 'r') as input_csv_data_file:
		input_csv_data = input_csv_data_file.read()

	files_test = input_csv_data.split("\n")
	del(files_test[0]) #deletando a linha que contem os nomes das colunas

	solution_list = []

	if not initial_sol_rr: # se a solução inicial não vier do RR
		for file in files_test:
			elements = file.split(",")

			if len(elements) > 1 : # para nao contar alguma linha vazia que possa ter
				solution_list.append(elements[1]) #pega o custo da solução

			
	else: # se a solução inicial vier do RR
		for file in files_test:
			elements = file.split(",")

			if len(elements) > 1 : # para nao contar alguma linha vazia que possa ter
				solution_list.append(elements[3]) #pega a média do custo da solução

	return solution_list


def calculate_best(solutions):
	best = [] #armazena o melhor valor pra cada instancia

	for i in range(len(solutions[0])): #percorre por cada instancia
		best.append(float("inf")) # inicia com um valor infinito
		
		for alg in solutions: #percorre pelos tipos de algoritmo
			if float(alg[i]) < best[-1]: # atualiza o valor da melhor solucao dessa instancia
				best[-1] = float(alg[i])

	return best


def calculate_distances(solutions,best_solutions):
	distances = [] #  é uma matrix.. O primeiro vetor contem len(solutions) elementos. Cada um desses elelementos contem len(solutions[0]) elementos e armazena a distancia de cada instancia da melhor solucao (em %)

	for alg in solutions: #percorre pelos tipos de algoritmo
		distances.append([])

		for i in range(len(solutions[0])): #percorre por cada instancia
			sub = float(alg[i]) - best_solutions[i]
			d = (sub * 100) / best_solutions[i] # calcula a porcentagem da distancia
			distances[-1].append(d)

		distances[-1].sort() # ordena as distancias

	return distances


def prepare_graphic(x_values,solutions, distances):
	y_values = [] # é uma matrix. O primeiro vetor contem len(solutions) elementos. Cada um desses elementos contem len(x_values) elementos)

	for alg in range(len(solutions)): #percorre pelos tipos de algoritmo
		qty = 0
		i = 0
		y_values.append([])
		for x in x_values: # percorre pelos pontos do eixo x (% distancia da melhor solucao)
			while (i < len(solutions[0])) and (distances[alg][i] <= x):
				qty += 1
				i+=1

			y = (qty * 100)/len(solutions[0]) # calculando a porcentagem da quantidade
			y_values[-1].append(y)

	return y_values


if __name__ == '__main__':

	# definindo os indices de cada algoritmo no vetor solutions
	T = 0 # trivial
	G = 1 # greedy
	RR = 2 # randomized rounding
	LS_T = 3 # local search with trivial as initial solution
	LS_G = 4 # local search with greedy as initial solution
	LS_RR = 5 # local search with randomized rounding as initial solution
	# PD = 6 # primal dual
	# LS_PD = 7 # local search with primal dual as initial solution

	solutions = []

	solutions.append(get_solutions("gurobi/solutionsTrivial.csv", False))
	solutions.append(get_solutions("greedy/solutions.csv", False))
	solutions.append(get_solutions("randomizedRounding/solutions.csv", True))
	solutions.append(get_solutions("localSearch/solutionsTrivial.csv",False))
	solutions.append(get_solutions("localSearch/solutionsGreedy.csv",False))
	solutions.append(get_solutions("localSearch/solutionsRR.csv",True))

	best_solutions = calculate_best(solutions)

	distances = calculate_distances(solutions,best_solutions)

	"""
	Verificar a menor e a maior distancia
	"""

	# Iniciam com valores invalidos
	minimum = float("inf")
	maximum = -1

	# percorre pelos tipos de algoritmo
	for alg in distances: 
		i = 0
		while(alg[i]==0):
			i+=1

		if (i < len(alg)) and (alg[i] < minimum):
			minimum = alg[i]
		if alg[-1] > maximum:
			maximum = alg[-1]

	# caso tudo seja igual a 0
	if minimum == float("inf"):
		minimum = 1
	elif minimum < 0.1:
		minimum = 0.1

	# IGNORANDO O MAXIMO REAL, APENAS PARA FICAR MAIS VISIVEL
	maximum = 8

	""" Fim verificar menor distancia """

	"""
	Construir o eixo x:
	- inicio = 0
	- fim = maximum
	- passo = minimum
	"""
	x_values = []
	for i in numpy.arange(0,maximum+minimum,minimum):
		x_values.append(i)

	""" Fim construir o eixo x """

	y_values = prepare_graphic(x_values,solutions,distances)

	for i in range(len(solutions)):
		matplotlib.pyplot.plot(x_values,y_values[i])

	matplotlib.pyplot.show()

