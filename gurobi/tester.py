#!/usr/bin/python
# -*- coding: utf-8 -*-

# This example tests the Facility Location Problem

import sys
from gurobipy import *

# time limit in seconds
time_limit = 900 #15 minutes
# memory limit in bytes
memory_limit = 10 ** 9 # 1GB

DEBUG = 0
EXE = 'uflp.py' # File that really solves the problem

if __name__ == '__main__':

	ok = False
	if len(sys.argv) > 2:
		input_type = sys.argv[1].strip()
		formulation_type = sys.argv[2].strip()

		if(input_type == '1'):
			print("ORLIB type")
			file_location = 'testCases1.txt'
			ok = True
		elif(input_type == '2'):
			print("SIMPLE FORMAT type")
			file_location = 'testCases2.txt'
			ok = True
		else:
			print("ERROR: Invalid first parameter value")
			print("SOLUTION: Select 1 for ORLIB inputs or 2 for SIMPLE FORMAT inputs")

		if(formulation_type == '1'):
			f_type = "ILP"
		elif(formulation_type == '2'):
			f_type = "LP"
		elif(formulation_type == '3'):
			f_type = "MIP"
		elif(formulation_type == '4'):
			f_type = 'Dual'
		else:
			print("ERROR: Invalid second parameter value")
			print("SOLUTION: Select 1 for ILP, 2 for LP, 3 for MLP or 4 for Dual")
			ok = False

		if ok:
			with open(file_location, 'r') as input_list_data_file:
				input_list_data = input_list_data_file.read()

			files_test = input_list_data.split()

			for file_name in files_test:
				print()
				print("Input file:", file_name)

				complete_file_name = '../baseDeTestes/facilityTestCases/tests/' + file_name
				complete_sol_file_name = 'solutions' + f_type + '/' + file_name + '.sol'

				# Chamando o programa a ser testado
				os.system("python3 " + EXE + " " + input_type + " " + formulation_type + " " + complete_file_name + " " + complete_sol_file_name + " " + str(time_limit) + " " + str(memory_limit))

	else:
		print('This test requires an input type and the formulation type. \nFirst please select one: 1 for ORLIB inputs or 2 for SIMPLE FORMAT inputs. \nSecond please select one: 1 for ILP, 2 for LP, 3 for MIP or 4 for Dual')

