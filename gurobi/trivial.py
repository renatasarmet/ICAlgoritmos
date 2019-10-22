#!/usr/bin/python
# -*- coding: utf-8 -*-

# This example build a trivial solution for the UFL problem
# It just open one facility and connect everyone to it

from gurobipy import *

def facilityTrivial(facility_list, client_list, DEBUG=0):

	solution = [-1] * len(client_list)
	sol_value = 0

	facility_index = 0 # open just the first facility
	sol_value += facility_list[facility_index].setup_cost

	for client in client_list: # connect everyone to it
		solution[client.index] = facility_index
		sol_value += client.attribution_cost[facility_index]
	
	if DEBUG >= 1:
		print("Total cost = ", sol_value)

	return (sol_value, 0, solution)
