import os
import sys

EXE = './lsr' # Executavel gerado pela compilação dos arquivos que realmente resolvem o problema

DISPLAY_NAME = 1 # Exibe o nome da instancia a ser testada e o tipo das instancias

DEBUG = 1 # OPCOES DE DEBUG: 0 PARA NAO EXIBIR NADA, 1 PARA EXIBIR O NOME DA INSTANCIA

if __name__ == '__main__':

	ok = True
	if len(sys.argv) > 2:
		input_type = sys.argv[1].strip()
		ls_type = sys.argv[2].strip()

		if(ls_type == '1'):
			if DEBUG >= DISPLAY_NAME:
				print("Complete LS type")

		elif(ls_type == '2'):
			if DEBUG >= DISPLAY_NAME:
				print("LS with neighborhood 1 (LS_N1)")

		elif(ls_type == '3'):
			if DEBUG >= DISPLAY_NAME:
				print("LS with neighborhood 2 (LS_N2)")

		else:
			print("ERROR: Invalid second parameter value")
			print("SOLUTION: Select 1 for LS, 2 for LS_N1 or 3 for LS_N2")
			ok = False


		n_tests = 1
		initial_sol_rr = False

		if len(sys.argv) > 3: 
			initial_sol_rr = True # indica que o teste será com soluções iniciais vindas do RR
			n_tests = int(sys.argv[3].strip()) # entao esse parametro indica a quantidade de testes que foram feitos (o N)

			if n_tests <= 0:
				print("ERROR: Invalid third parameter value")
				print("SOLUTION: The parameter must be greater than 0")
				ok = False

		if(input_type == '1'):
			if DEBUG >= DISPLAY_NAME:
				print("ORLIB type")
			file_location = 'testCases1.txt'

		elif(input_type == '2'):
			if DEBUG >= DISPLAY_NAME:
				print("SIMPLE FORMAT type")
			file_location = 'testCases2.txt'

		else:
			print("ERROR: Invalid first parameter value")
			print("SOLUTION: Select 1 for ORLIB inputs or 2 for SIMPLE FORMAT inputs")
			ok = False
			
		if ok:
			# Abertura para leitura de arquivo que contem uma lista com todos os arquivos de entrada
			with open(file_location, 'r') as input_list_data_file:
				input_list_data = input_list_data_file.read()

			files_test = input_list_data.split()

			# Enquanto houverem entradas nao testadas ainda
			for file_name in files_test:
				if DEBUG >= DISPLAY_NAME:
					print()
					print("Input file:", file_name)

				for seed in range(1,n_tests+1): # Repete n_tests vezes

					input_name = '../baseDeTestes/facilityTestCases/tests/' + file_name

					if initial_sol_rr:
						initialSolName = 'initialSolutions/' + file_name + "_" + str(seed) + '.sol'
						solutionName = 'solutions/' + file_name + "_" + str(seed) + '.sol'

					else:
						initialSolName = 'initialSolutions/' + file_name + '.sol'
						solutionName = 'solutions/' + file_name + '.sol'

					# Chamando o programa a ser testado
					os.system(EXE + " " + input_name + " " + initialSolName + " " + solutionName + " " + input_type + " " + ls_type)

	else:
		print('This test requires an input type and an LS type. \nFirst please select one: 1 for ORLIB inputs or 2 for SIMPLE FORMAT inputs.\nThen select: 1 for LS, 2 for LS_N1, 3 for LS_N2, 4 for LS_N1_TS.')



