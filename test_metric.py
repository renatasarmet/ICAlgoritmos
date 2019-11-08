#!/usr/bin/python
# -*- coding: utf-8 -*-

# This code tests if the instance of the Facility Location Problem respect the triangular inequality

import sys
from collections import namedtuple

DEBUG = 0

Facility = namedtuple("Facility", ['index', 'setup_cost'])
Customer = namedtuple("Customer", ['index', 'attribution_cost'])

# memory limit in bytes
memory_limit = 10 ** 9 # 1GB

def test_metric(input_type, complete_file_name):

	with open(complete_file_name, 'r') as input_data_file:
		input_data = input_data_file.read()

	# parse the input
	parts = input_data.split()

	# ORLIB type input
	if (input_type == '1'):

		facility_count = int(parts[0])
		customer_count = int(parts[1])

		if DEBUG >= 1:
			print("Facilities amount: ", facility_count)
			print("Customers amount: ", customer_count)

		facilities = []
		cont = 0

		# Facilities loop
		for i in range(3, (facility_count+1)*2,2):
			facilities.append(Facility(
				cont, float(parts[i])))
			# print("for facility:",facilities[cont].index, "the setup cost is:", facilities[cont].setup_cost )
			cont += 1

		customers = []
		cust_number = 0
		cont = 0

		# Customers loop
		for i in range((facility_count+1)*2, ((facility_count+1)*2) + customer_count + (customer_count * facility_count)):
			if(cont <= 0):
				# Creating a new list of attribution cost to this new customer
				list_attr_cost = []
			else:
				# Putting together all attribution costs of this current customer
				list_attr_cost.append(float(parts[i]))
				   
				# Adding the customer when all costs are in his list
				if (cont == facility_count):
					cont = -1
					customers.append(Customer(
						cust_number, list_attr_cost))
					# print("for client", customers[-1].index, "the attr cost is:", customers[-1].attribution_cost)
					cust_number += 1

			cont+=1

	# SIMPLE FORMAT type input
	elif (input_type == '2'):

		#ignoring 0 and 1 that correspond to the file name

		facility_count = int(parts[2])
		customer_count = int(parts[3])

		if DEBUG >= 1:
			print("Facilities amount: ", facility_count)
			print("Customers amount: ", customer_count)

		facilities = []
		customers = []
		fac_number = 0
		cust_number = 0

		# Creating a new list of attribution cost. It will be a list of lists
		list_attr_cost = []

		cont = 0
		for i in range(5, 5 + (facility_count*2) + (facility_count * customer_count)):
			if(cont == 1):
				# Adding the facility
				facilities.append(Facility(
					fac_number, float(parts[i])))
				# print("for facility:",facilities[fac_number].index, "the setup cost is", facilities[fac_number].setup_cost )
				fac_number += 1
				# Creating a new list of attribution cost to this new facility
				list_attr_cost.append([])
			elif(cont > 1):
				# Putting together all attribution costs of this current customer
				list_attr_cost[fac_number-1].append(float(parts[i]))
				   
				# Going to the next facility
				if (cont == customer_count+1):
					cont = -1

			cont += 1

		for j in range(customer_count):
			list_to_add = []
			for i in range(facility_count):
				list_to_add.append(list_attr_cost[i][j])

			customers.append(Customer(
				cust_number, list_to_add))
			# print("for client", customers[j].index, "the attr cost is", customers[j].attribution_cost)
			cust_number += 1

	else: 
		return ("Weird type of input. You shouldn't be here")


	if facility_count * customer_count * facility_count <= memory_limit:
		# if 20 * facility_count * customer_count * facility_count <= memory_limit:
		# if (customer_count * facility_count) ** 2 <= memory_limit:

		for f_i in range(len(facilities)):
			print("hey ", f_i)
			for cli in customers:
				for f_i2 in range(len(facilities)):
					for cli_2 in customers:
						# if it does not respect the triangular inequality
						if (cli.attribution_cost[f_i] > cli.attribution_cost[f_i2] + cli_2.attribution_cost[f_i2] + cli_2.attribution_cost[f_i]):
							return False
	else:
		print("Exceeded Memory limit.")
		return False

	# respects the triangular inequality
	return True



if __name__ == '__main__':

	ok = False
	if len(sys.argv) > 1:
		input_type = sys.argv[1].strip()

		if(input_type == '1'):
			print("ORLIB type")
			file_location = 'gurobi/testCases1.txt'
			ok = True
		elif(input_type == '2'):
			print("SIMPLE FORMAT type")
			file_location = 'gurobi/testCases2.txt'
			ok = True
		else:
			print("ERROR: Invalid first parameter value")
			print("SOLUTION: Select 1 for ORLIB inputs or 2 for SIMPLE FORMAT inputs")

		if ok:
			with open(file_location, 'r') as input_list_data_file:
				input_list_data = input_list_data_file.read()

			files_test = input_list_data.split()

			for file_name in files_test:
				print()
				print("Input file:", file_name)

				complete_file_name = 'baseDeTestes/facilityTestCases/tests/' + file_name

				# Chamando a função que testa
				metric = test_metric(input_type, complete_file_name) 
				if(not metric):
					print("NOT METRIC!!!")
	else:
		print('This test requires an input type.\n Please select one: 1 for ORLIB inputs or 2 for SIMPLE FORMAT inputs.')

