#!/usr/bin/python
# -*- coding: utf-8 -*-

# This example solves the Facility Location Problem

import time
import sys
from collections import namedtuple
from gurobipy import *
import csv

from dual import dualLP
from ILP import facilityILP
from LP import facilityLP
from MIP import facilityMIP


Facility = namedtuple("Facility", ['index', 'setup_cost'])
Customer = namedtuple("Customer", ['index', 'attribution_cost'])

DEBUG = 0

def solve_it(input_type, input_data, formulation_type):

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

	bound = 0 # just because I dont know yet what to do with this

	pair_best = 0
	pair_new = 0

	if facility_count * customer_count * facility_count <= memory_limit:
		# if 20 * facility_count * customer_count * facility_count <= memory_limit:
		# if (customer_count * facility_count) ** 2 <= memory_limit:

		if(formulation_type == 'ILP'):
			pair_new = facilityILP(facilities, customers, bound, time_limit, DEBUG)

			if DEBUG >= 2:
				print(f"Facility ILP solution value = {pair_new[0]}")

		elif(formulation_type == 'LP'):
			pair_new = facilityLP(facilities, customers, time_limit, DEBUG)

			if DEBUG >= 2:
				print(f"Facility LP solution value = {pair_new[0]}")

		elif(formulation_type == 'MIP'):
			pair_new = facilityMIP(facilities, customers, bound, time_limit, DEBUG)

			if DEBUG >= 2:
				print(f"Facility MIP solution value = {pair_new[0]}")

		elif(formulation_type == 'Dual'):
			pair_new = dualLP(facilities, customers, time_limit, DEBUG)

			if DEBUG >= 2:
				print(f"Dual LP solution value = {pair_new[0]}")

		else:
			print("Wrong formulation type. You shouldn't be here")

		# put a conditional here when we stablish a correct bound to send to facilityILP
		pair_best = pair_new

	else:
		print("Exceeded Memory limit.")
		pair_best = (0,0,[-1] * len(customers), [-1] * len(customers) * len(facilities))

	#return output_data
	return pair_best


if __name__ == '__main__':

	ok = False
	if len(sys.argv) > 4:
		input_type = sys.argv[1].strip()
		formulation_type = sys.argv[2].strip()
		complete_file_name = sys.argv[3].strip()
		complete_sol_file_name = sys.argv[4].strip()

		if len(sys.argv) > 5:
			time_limit = int(sys.argv[5].strip())

			if len(sys.argv) > 6:
				memory_limit = int(sys.argv[6].strip())
				
			else: # default memory limit in bytes
				memory_limit = 10 ** 9 # 1GB

		else: # default time limit in seconds
			time_limit = 900 #15 minutes

		if(input_type == '1'):
			if DEBUG >= 1:
				print("ORLIB type")
			ok = True
		elif(input_type == '2'):
			if DEBUG >= 1:
				print("SIMPLE FORMAT type")
			ok = True
		else:
			print("ERROR: Invalid first parameter value")
			print("SOLUTION: Select 1 for ORLIB inputs or 2 for SIMPLE FORMAT inputs")

		if(formulation_type == '1'):
			if DEBUG >= 1:
				print("Integer Linear Program")
			formulation_type = "ILP"
		elif(formulation_type == '2'):
			if DEBUG >= 1:
				print("Linear Program")
			formulation_type = "LP"
		elif(formulation_type == '3'):
			if DEBUG >= 1:
				print("Mixed Integer Linear Program")
			formulation_type = "MIP"
		elif(formulation_type == '4'):
			if DEBUG >= 1:
				print("Dual Linear Program")
			formulation_type = 'Dual'
		else:
			print("ERROR: Invalid second parameter value")
			print("SOLUTION: Select 1 for ILP, 2 for LP, 3 for MLP or 4 for Dual")
			ok = False

		if ok:
			if DEBUG >= 1:
				print("Input file:", complete_file_name)
			start = time.time()

			with open(complete_file_name, 'r') as input_data_file:
				input_data = input_data_file.read()

			solution = solve_it(input_type, input_data, formulation_type)

			end = time.time()
			time_spent = end - start

			if DEBUG >= 1:
				print("Time spent =", time_spent)

			file = open(complete_sol_file_name, 'w')
			sentence = str(solution[0]) + " " + str(time_spent) + " " + str(solution[1])

			if formulation_type == 'LP':
				# solution[4] is the y_values
				for y in solution[4]: 
				  sentence += " " + str(y)
				  
				# solution[3] is the x_values
				for x_f in solution[3]: # each x_f is a list of x from the facility f to every client
				  for x in x_f: # each x is a value of x from the facility f to this especific client
					  sentence += " " + str(x)

			else:
				# In Dual solution[2] is the v_values and in ILP or MIP it is the solution
				for s in solution[2]:
					sentence += " " + str(s)

				if formulation_type == 'Dual':
					# solution[3] is the w_values
					for w_f in solution[3]: # each w_f is a list of w from the facility f to every client
					  for w in w_f: # each w is a value of w from the facility f to this especific client
						  sentence += " " + str(w)

			file.write(sentence)
			file.close()

	else:
		print('This test requires an input type, the formulation type, the complete file name and the complete solution file name. \nFirst please select one: 1 for ORLIB inputs or 2 for SIMPLE FORMAT inputs. \nSecond please select one: 1 for ILP, 2 for LP, 3 for MIP or 4 for Dual')

