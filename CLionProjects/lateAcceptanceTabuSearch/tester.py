import os
import sys

EXE = './lats' # Executavel gerado pela compilação dos arquivos que realmente resolvem o problema

DISPLAY_NAME = 1 # Exibe o nome da instancia a ser testada e o tipo das instancias

DEBUG = 1 # OPCOES DE DEBUG: 0 PARA NAO EXIBIR NADA, 1 PARA EXIBIR O NOME DA INSTANCIA

if __name__ == '__main__':

	ok = True
	if len(sys.argv) > 1:
		input_type = sys.argv[1].strip()

		if len(sys.argv) > 9:
			a1 = sys.argv[2].strip()
			bc1 = sys.argv[3].strip()
			bc2 = sys.argv[4].strip()
			bo1 = sys.argv[5].strip()
			bo2 = sys.argv[6].strip()
			seed = sys.argv[7].strip()
			limit_idle = sys.argv[8].strip()
			lh = sys.argv[9].strip()
		else: # se eu nao passei por parametro, coloca os padroes
			a1 = "2.5"
			bc1 = bo1 = "0.01"
			bc2 = bo2 = "0.05" 
			seed = "0"
			limit_idle = "0.02"
			lh = "500"

		n_tests = 1
		initial_sol_rr = False

		if len(sys.argv) > 10: 
			initial_sol_rr = True # indica que o teste será com soluções iniciais vindas do RR
			n_tests = int(sys.argv[10].strip()) # entao esse parametro indica a quantidade de testes que foram feitos (o N)

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

				for _seed in range(1,n_tests+1): # Repete n_tests vezes

					input_name = '../baseDeTestes/facilityTestCases/tests/' + file_name

					if initial_sol_rr:
						initialSolName = 'initialSolutions/' + file_name + "_" + str(_seed) + '.sol'
						solutionName = 'solutions/' + file_name + "_" + str(_seed) + '.sol'

					else:
						initialSolName = 'initialSolutions/' + file_name + '.sol'
						solutionName = 'solutions/' + file_name + '__a1-' + a1 + '_l-' + bc1 + '-' + bc2 + '_idle-' + limit_idle + '_lh' + lh + '.sol'

					# Chamando o programa a ser testado
					os.system(EXE + " " + input_name + " " + initialSolName + " " + solutionName + " " + input_type + " " + a1 + " " + bc1 + " " + bc2 + " " + bo1 + " " + bo2 + " " + seed + " " + limit_idle + " " + lh)

	else:
		print('This test requires an input type. \nFirst please select one: 1 for ORLIB inputs or 2 for SIMPLE FORMAT inputs.')



