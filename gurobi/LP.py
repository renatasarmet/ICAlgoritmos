#!/usr/bin/python
# -*- coding: utf-8 -*-

# This example formulates and solves the following model:
#  minimize
#       SUM(􏰂fi * yi)_{i \in F} + 􏰂 SUM(􏰂cij * xij)_{i \in F, j \in D} 
#  subject to
#       SUM(xij)_{i \in F} = 1 , Aj \in D
#       xij <= yi              , Ai \in F, Aj \in D
#       xij >= 0               , Ai \in F, Aj \in D
#       yi  >= 0               , Ai \in F

from gurobipy import *

def facilityLP(facility_list, client_list, time_limit=900, DEBUG=0):

	# Instantiate a Gurobi solver, naming it SolveLinearProblem
	solver = Model('SolveLinearProblem')

	# Turn verbose off
	# solver.Params.OutputFlag = 0
	solver.setParam(GRB.Param.OutputFlag, 0)
	# solver.setParam("OutputFlag", 0)

	# Set a time limit
	solver.setParam(GRB.Param.TimeLimit, time_limit)

	# Create the variables and define their types.
	# y[i] are continuous variables.
	y = []
	for i in range(0, len(facility_list)):
		y.append(solver.addVar(vtype=GRB.CONTINUOUS, name='y[%d]' % i))

	# x[i][j] are continuous variables.
	x = []
	for i in range(0, len(facility_list)):
		x.append([])
		for j in range(0, len(client_list)):
			x[i].append(solver.addVar(
				vtype=GRB.CONTINUOUS, name='x[%d][%d]' % (i, j)))

	# Define the constraints.
	# Constraint type 1: xij <= yi for each facility i and client j
	# Create a constraint for each pair facility and client
	for i in range(0, len(facility_list)):
		for j in range(0, len(client_list)):
			solver.addConstr(y[i] - x[i][j] >= 0, 'c1[%d][%d]' % (i, j))

	# Constraint type 2: sum_{i=1^m} xij = 1 for each client j
	# Create a constraint for each client

	for j in range(0, len(client_list)):
		left_side = LinExpr()
		for i in range(0, len(facility_list)):
			left_side += 1 * x[i][j]
		solver.addConstr(left_side, GRB.EQUAL, 1, 'c2[%d]' % j)
		# solver.addConstr(left_side == 1, 'c2[%d]' % j)


	# Define the objective function.
	# Objective function: min sum_{i=1^m} fi yi + sum_{j=1^n} sum_{i=1^m} cij xij
	solver.setObjective(
		(quicksum(facility_list[i].setup_cost * y[i]
				  for i in range(0, len(facility_list)))
		 + quicksum(quicksum(client_list[j].attribution_cost[i] * x[i][j]
							 for i in range(0, len(facility_list)))
					for j in range(0, len(client_list)))
		 ), GRB.MINIMIZE)

	# Solve the system.
	solver.optimize()

	if DEBUG >= 2:
		print(f"Result status = {solver.status}")

	intOPT = 0
	if solver.SolCount == 0:

		if DEBUG >= 2:
			print("LP model does not found a solution")

		return (0, intOPT, [], [], [])

	# The problem has an optimal solution.
	if solver.status == GRB.Status.OPTIMAL:
		intOPT = 1

		if DEBUG >= 2:
			print("LP solver found optimal solution")

	elif solver.status == GRB.Status.TIME_LIMIT:

		if DEBUG >= 2:
			print("LP solver ended due to time limit")

	elif solver.status == GRB.Status.SUBOPTIMAL:

		if DEBUG >= 2:
			print("LP solver found feasible solution")

	elif solver.status == GRB.Status.INF_OR_UNBD:

		if DEBUG >= 2:
			print("LP model is infeasible or unbounded")

		return (0, 0, [], [], [])

	elif solver.status == GRB.Status.LOADED:

		if DEBUG >= 2:
			print("LP solver does not found a solution")

		return (0, 0, [], [], [])

	else:
		print("Weird return from LP solver")
		exit(1)

	# if DEBUG >= 2:
	#     # Display instance information and results.
	#     print(f"Number of variables = {solver.NumVariables()}")
	#     print(f"Number of constraints = {solver.NumConstraints()}")

	# The value of each variable in the solution.
	y_values = solver.getAttr('X', y)
	if DEBUG >= 3:
		print('Solution for y variables:')
		print(y_values)

	if DEBUG >= 4:
		print('Solution for x variables:')
	x_values = []
	for i in range(0, len(facility_list)):
		x_values.append(solver.getAttr('X', x[i]))
		if DEBUG >= 4:
			print(x_values[i])

	if DEBUG >= 2:
		# The objective value of the solution.
		print("Optimal objective value = %.2f" % solver.objVal)

	# Choose here if we want the solution cost to be integer or not 
	#sol_value = int(solver.objVal) 
	sol_value = solver.objVal

	solution = [-1] * len(client_list)

	for j in range(0, len(client_list)):
		for i in range(0, len(facility_list)):
			if x[i][j].getAttr('X') == 1:
				solution[j] = i
				break

	if DEBUG >= 3:
		print(solution)

	return (sol_value, intOPT, solution, x_values, y_values)
