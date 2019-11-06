import matplotlib.pyplot as plt
import numpy
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
	y_values = [] # é uma matrix. O primeiro vetor contem len(solutions) elementos. Cada um desses elementos contem len(x_values) elementos

	for alg in range(len(solutions)): #percorre pelos tipos de algoritmo
		qty = 0
		y_values.append([])
		for x in x_values: # percorre pelos pontos do eixo x (% distancia da melhor solucao)
			while (qty < len(solutions[0])) and (distances[alg][qty] <= x): # enquanto nao tiver acabado o vetor e a distancia for menor que o x
				qty += 1

			y = (qty * 100)/len(solutions[0]) # calculando a porcentagem da quantidade
			y_values[-1].append(y)

	return y_values


if __name__ == '__main__':

	ZOOM = -1 # Usado caso queira dar um zoom no grafico, ignorando o maximo valor encontrado na solucao. obs: utilizar -1 caso nao queira dar zoom ou colocar o valor do maximo desejado

	solutions = [] #vai armazenar as soluces de cada algoritmo
	legend = [] #vai indicar a ordem da legenda no grafico

	# Pegando as informacoes das solucoes do csv de cada algoritmo
	solutions.append(get_solutions("gurobi/solutionsTrivial.csv", False))
	legend.append("Trivial")
	solutions.append(get_solutions("greedy/solutions.csv", False))
	legend.append("Greedy")
	solutions.append(get_solutions("randomizedRounding/solutions.csv", True))
	legend.append("Randomized Rounding")
	solutions.append(get_solutions("primalDual/solutions.csv",False))
	legend.append("Primal Dual")
	solutions.append(get_solutions("localSearch/solutionsTrivial.csv",False))
	legend.append("LS - Trivial")
	solutions.append(get_solutions("localSearch/solutionsGreedy.csv",False))
	legend.append("LS - Greedy")
	solutions.append(get_solutions("localSearch/solutionsRR.csv",True))
	legend.append("LS - RR")
	solutions.append(get_solutions("localSearch/solutionsDual.csv",False))
	legend.append("LS - PD")

	# Salvando a melhor solucao de cada instancia
	best_solutions = calculate_best(solutions)

	# Calculando a distancia da solucao de cada algoritmo com a melhor solucao de cada instancia
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
		while(alg[i]==0): # Garante que o minimum nao sera 0
			i+=1

		# Salvando a maior e a menor distancia encontrada
		if (i < len(alg)) and (alg[i] < minimum):
			minimum = alg[i]
		if (ZOOM < 0) and (alg[-1] > maximum):
			maximum = alg[-1]

	# caso tudo seja igual a 0, o passo sera 1
	if minimum == float("inf"):
		minimum = 1
	# caso o minimo seja muito pequeno, mudamos por razoes de capacidade de memoria
	elif minimum < 0.01:
		minimum = 0.01

	# Caso queira ignorar o maximum real e dar um zoom no grafico
	if ZOOM >= 0:
		maximum = ZOOM

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

	# Preparando os valores de y do grafico
	y_values = prepare_graphic(x_values,solutions,distances)

	# naming the x axis 
	plt.xlabel('x - % approximation') 
	# naming the y axis 
	plt.ylabel('y - % quantity') 
	  
	# giving a title to my graph 
	plt.title('Performance Profile UFL') 

	# Plotando as linhas de cada algoritmo
	for i in range(len(solutions)):
		plt.plot(x_values,y_values[i],label=legend[i])

	# show a legend on the plot 
	plt.legend() 

	plt.show()
