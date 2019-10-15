import os
import sys

N_TESTS = 10 # Quantidade de vezes que sera testada a mesma entrada, para calculo de minimo, maximo e media
EXE = './rr' # Executavel gerado pela compilação dos arquivos que realmente resolvem o problema

DISPLAY_NAME = 1 # Exibe o nome da instancia a ser testada e o tipo das instancias
DISPLAY_SEED = 2 # Exibe qual semente sera usada

DEBUG = 1 # OPCOES DE DEBUG: 0 PARA NAO EXIBIR NADA, 1 PARA EXIBIR O NOME DA INSTANCIA, 2 PARA EXIBIR A SEMENTE

if __name__ == '__main__':

	ok = True
	if len(sys.argv) > 1:
		input_type = sys.argv[1].strip()

		if(input_type == '1'):
			if DEBUG >= DISPLAY_NAME:
				print("ORLIB type")
			file_location = 'testCases1.txt'

		elif(input_type == '2'):
			if DEBUG >= DISPLAY_NAME:
				print("SIMPLE FORMAT type")
			file_location = 'testCases2.txt'

		else:
			print("ERROR: Invalid parameter value")
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

				input_name = '../baseDeTestes/facilityTestCases/tests/' + file_name
				primalSolName = 'primalLPSolutions/' + file_name + '.sol'
				dualSolName = 'dualLPSolutions/' + file_name + '.sol'

				# Executa o programa com a mesma entrada N_TESTS vezes, para calculo de minimo, maximo e media de tempo
				for seed in range(1, N_TESTS+1): # semente varia de 1 até N_TESTS

					if DEBUG >= DISPLAY_SEED:
						print("SEED", seed)

					solutionName = 'solutions/' + file_name + '_' + str(seed) + '.sol'
					# Chamando o programa a ser testado
					os.system(EXE + " " + input_name + " " + primalSolName + " " + dualSolName + " " + solutionName + " " + input_type + " " + str(seed))

	else:
		print('This test requires an input type. \nFirst please select one: 1 for ORLIB inputs or 2 for SIMPLE FORMAT inputs.')



