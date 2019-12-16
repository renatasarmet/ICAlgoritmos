import os
import sys


if __name__ == '__main__':

	# Where all test cases are listed
	file_location = 'testCasesGroups.txt'

	with open(file_location, 'r') as input_list_data_file:
		input_list_data = input_list_data_file.read()

	files_test = input_list_data.split()

	size = 500 #qtd de clientes e instalacoes


	for file_name in files_test:
		if file_name != ".DS_Store": # ignore this file
			print("Input file:", file_name)

			open_fac = [0] * size

			vector = []
			for i in range(size):
				vector.append([0] * size)

			complete_file_name = 'groups/' + file_name + ".sol"

			with open(complete_file_name, 'r') as input_data_file:
				input_data = input_data_file.read()

			if(input_data):
				# solution :: 0 - cost, 1 - timeSpent, 2 - optimal, 3....qty_clients - connected facility
				solution = input_data.split(" ")

				if len(solution) >= size+1: # checking if the solution is in a correct format
					for i in range(0,size):
						vector[i][int(solution[i+2])] = 1
						open_fac[int(solution[i+2])] = 1
				else:
					print("missing info in solution")

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
						if vector[j][i] == 1:
							print(j, end=" ")
							used = True
					if used:
						print("<-- fac: ", i, end="\n\n")






