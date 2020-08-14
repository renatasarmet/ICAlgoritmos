import os
import sys

EXE = './create' # Executavel gerado pela compilação dos arquivos que realmente resolvem o problema

DISPLAY_NAME = 1 # Exibe o nome da instancia a ser testada e o tipo das instancias

DEBUG = 1 # OPCOES DE DEBUG: 0 PARA NAO EXIBIR NADA, 1 PARA EXIBIR O NOME DA INSTANCIA

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


		if len(sys.argv) > 5:
			number_of_scenarios = int(sys.argv[2].strip()) # numero de cenarios
			inflation_start = float(sys.argv[3].strip()) # inicio intervalo da inflacao 
			inflation_end = float(sys.argv[4].strip()) # fim intervalo da inflacao 
			chance_cli_scenario = float(sys.argv[5].strip()) # porcentagem de chance de cada cliente aparecer em cada cenario

		else: # se eu nao passei por parametro, coloca os padroes
			number_of_scenarios = 5
			inflation_start = 1.1 
			inflation_end = 1.5
			chance_cli_scenario = 0.5
			

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

				file_name_replace = file_name.replace('.', '-')

				input_name = '../../baseDeTestes/facilityTestCases/tests/' + file_name
				solutionName = 'generatedInstances/' + file_name_replace + '_' + str(number_of_scenarios) + 's_' + str(int(inflation_start*100)) + '-' + str(int(inflation_end*100)) + 'i_' + str(int(chance_cli_scenario*100)) + 'c.txt'

				# Chamando o programa a ser testado
				os.system(EXE + " " + input_name + " " + solutionName + " " + input_type + " " + 
					str(number_of_scenarios) + " " + str(inflation_start) + " " + str(inflation_end) + " " + str(chance_cli_scenario))

	else:
		print('This test requires an input type. \nFirst please select one: 1 for ORLIB inputs or 2 for SIMPLE FORMAT inputs.')



