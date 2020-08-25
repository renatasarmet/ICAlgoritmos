import os
import sys

EXE = './SUFLP' # Executavel gerado pela compilação dos arquivos que realmente resolvem o problema

DISPLAY_NAME = 1 # Exibe o nome da instancia a ser testada e o tipo das instancias

DEBUG = 1 # OPCOES DE DEBUG: 0 PARA NAO EXIBIR NADA, 1 PARA EXIBIR O NOME DA INSTANCIA

if __name__ == '__main__':

	# Parameters values
	number_of_scenarios = [5, 50] # numero de cenarios
	inflation_start = [1.1] # inicio intervalo da inflacao
	inflation_end = [1.5]  # fim intervalo da inflacao 
	chance_cli_scenario = [0.5] # porcentagem de chance de cada cliente aparecer em cada cenario

	file_location = 'testCases.txt'

	# Abertura para leitura de arquivo que contem uma lista com todos os arquivos de entrada
	with open(file_location, 'r') as input_list_data_file:
		input_list_data = input_list_data_file.read()

	files_test = input_list_data.split()

	# Enquanto houverem entradas nao testadas ainda
	for file_name in files_test:
		if file_name != ".DS_Store": # ignore this file
			if DEBUG >= DISPLAY_NAME:
				print()
				print("Input file:", file_name)

			file_name_replace = file_name.replace('.', '-')

			# Fazendo as combinacoes para esse file_name
			for n_scenario in number_of_scenarios:
				for i_start in inflation_start:
					for i_end in inflation_end:
						for chance in chance_cli_scenario:

							instance_name = file_name_replace + '_' + str(n_scenario) + 's_' + str(int(i_start*100)) + '-' + str(int(i_end*100)) + 'i_' + str(int(chance*100)) + 'c.txt'
							
							if DEBUG >= DISPLAY_NAME:
								print("Instance name:", instance_name)

							input_name = '../createInstance/generatedInstances/' + instance_name
							solution_name = 'solutions/' + instance_name + '.ilp'

							# Chamando o programa a ser testado
							os.system(EXE + " < " + input_name + " > " + solution_name)

