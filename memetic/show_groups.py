import os
import sys


if __name__ == '__main__':

	# Where all test cases are listed
	file_location = 'testCasesGroups.txt'

	with open(file_location, 'r') as input_list_data_file:
		input_list_data = input_list_data_file.read()

	files_test = input_list_data.split()

	size = 500 #qtd de clientes e instalacoes


	cli_cli = [] #matriz cliente x cliente
	for i in range(size):
		cli_cli.append([0] * size)


	cont = 0
	vector = [] #[file_test][clients][facilities]
	vector.append([]) # cont = 0
	vector.append([]) # cont = 1

	nearest_fac = [] #vetor de clientes (cont 0 e 1)
	nearest_fac.append([])
	nearest_fac.append([])

	for file_name in files_test:
		if file_name != ".DS_Store": # ignore this file
			print("Input file:", file_name)

			open_fac = [0] * size

			for i in range(size):
				vector[cont].append([0] * size)

			complete_file_name = 'groups/' + file_name + ".sol"

			with open(complete_file_name, 'r') as input_data_file:
				input_data = input_data_file.read()

			if(input_data):
				# solution :: 0 - cost, 1 - timeSpent, 2 - optimal, 3....qty_clients - connected facility
				solution = input_data.split(" ")

				if len(solution) >= size+1: # checking if the solution is in a correct format
					for i in range(0,size):
						vector[cont][i][int(solution[i+2])] = 1
						open_fac[int(solution[i+2])] = 1
						nearest_fac[cont].append(int(solution[i+2]))
				else:
					print("missing info in solution")

				print("Solution value: ", solution[0])
				# mostrar instalacoes abertas
				print("open fac:", end=" ")
				for i in range(size):
					if open_fac[i] == 1:
						print(i, end=" ")

				print(end="\n\n")
				# mostrar grupos
				for i in range(size):
					used = False
					for j in range(size):
						if vector[cont][j][i] == 1:
							print(j, end=" ")
							used = True
					if used:
						print("<-- fac: ", i, end="\n\n")

			cont += 1


	# Verificando a semelhanca
	for i in range(size): #para cada cliente
		for j in range(size): #para cada outro cliente
			if vector[0][j][nearest_fac[0][i]] and vector[1][j][nearest_fac[1][i]]: # se o cliente estava atribuído à mesma instalacao em ambos os casos
				if nearest_fac[0][i] == nearest_fac[1][i]: # se era exatamente a mesma instalacao
					cli_cli[i][j] = 2
				else:
					cli_cli[i][j] = 1


	# exibindo detalhes
	# for i in range(size): #para cada cliente
	# 	print("cli ", i, end=": ")
	# 	for j in range(size): #para cada outro cliente
	# 		if i != j:
	# 			if cli_cli[i][j] == 1:
	# 				print(j, end=" ")
	# 			if cli_cli[i][j] == 2:
	# 				print("(",j,"->",nearest_fac[0][i],")", end=" ")
	# 	print()


	# lista de todos os clientes
	list_cli = []
	for i in range(size):
		list_cli.append(i)

	print()
	print("Groups:")
	# exibindo os grupos
	for i in range(size): #para cada cliente
		used = False
		same_fac = False
		for j in range(size): #para cada outro cliente
			if j in list_cli:
				if cli_cli[i][j] >= 1:
					print(j, end=" ")
					list_cli.remove(j)
					used = True
					if cli_cli[i][j] == 2:
						same_fac = True
					elif same_fac:
						print("*****ERROR")
		if same_fac:
			print(" ***** always facility -> ",nearest_fac[0][i], " ******** ")
		elif used:
			print()









