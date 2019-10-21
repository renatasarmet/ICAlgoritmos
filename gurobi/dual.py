#!/usr/bin/python
# -*- coding: utf-8 -*-

# This example formulates and solves the following model:
# Dual LP:
#  maximize
#       SUM(􏰂vj)_{j \in D}
#  subject to
#       SUM(wij)_{j \in D} <= fi , Ai \in F
#       vj - wij <= cij          , Ai \in F, Aj \in D
#       wij >= 0                 , Ai \in F, Aj \in D

from gurobipy import *

def dualLP(facility_list, client_list, time_limit=900, DEBUG=0):

	# Instantiate a Gurobi solver, naming it SolveDualLinearProblem
	solver = Model('SolveDualLinearProblem')

	# Turn verbose off
	# solver.Params.OutputFlag = 0
	solver.setParam(GRB.Param.OutputFlag, 0)
	# solver.setParam("OutputFlag", 0)

	# Set a time limit
	solver.setParam(GRB.Param.TimeLimit, time_limit)

	# Create the variables and define their types.
	# v[j] are continuous variables.
	v = []
	for j in range(0, len(client_list)):
		v.append(solver.addVar(vtype=GRB.CONTINUOUS, name='v[%d]' % j))

	# w[i][j] are continuous variables.
	w = []
	for i in range(0, len(facility_list)):
		w.append([])
		for j in range(0, len(client_list)):
			w[i].append(solver.addVar(
				vtype=GRB.CONTINUOUS, name='w[%d][%d]' % (i, j)))


	# Define the constraints.
	# Constraint type 1: vj - wij <= cij for each facility i and client j
	# Create a constraint for each pair facility and client
	for i in range(0, len(facility_list)):
		for j in range(0, len(client_list)):
			solver.addConstr(client_list[j].attribution_cost[i] - v[j] + w[i][j] >= 0, 'c1[%d][%d]' % (i, j))

	# Constraint type 2: sum(wij)_{j \in D} <= fi , for each facility i
	# Create a constraint for each facility
	for i in range(0, len(facility_list)):
		left_side = LinExpr()
		for j in range(0, len(client_list)):
			left_side += 1 * w[i][j]
		solver.addConstr(left_side, GRB.LESS_EQUAL, facility_list[i].setup_cost, 'c2[%d]' % j)
		# solver.addConstr(left_side <= fi, 'c2[%d]' % j)

	# Constraint type 3: wij >= 0 , for each facility i and for each client j
	# Create a constraint for each pair facility and client
	for i in range(0, len(facility_list)):
		for j in range(0, len(client_list)):
			solver.addConstr(w[i][j] >= 0, 'c3[%d][%d]' % (i, j))


	# Define the objective function.
	# Objective function: max sum_{j \in D} vj
	solver.setObjective(
		(quicksum(v[j]
				  for j in range(0, len(client_list)))
		 ), GRB.MAXIMIZE)

	# Solve the system.
	solver.optimize()

	if DEBUG >= 2:
		print(f"Result status = {solver.status}")

	intOPT = 0
	if solver.SolCount == 0:

		if DEBUG >= 2:
			print("Dual model does not found a solution")

		return (0, intOPT, [], [])

	# The problem has an optimal solution.
	if solver.status == GRB.Status.OPTIMAL:
		intOPT = 1

		if DEBUG >= 2:
			print("Dual solver found optimal solution")

	elif solver.status == GRB.Status.TIME_LIMIT:

		if DEBUG >= 2:
			print("Dual solver ended due to time limit")

	elif solver.status == GRB.Status.SUBOPTIMAL:

		if DEBUG >= 2:
			print("Dual solver found feasible solution")

	elif solver.status == GRB.Status.INF_OR_UNBD:

		if DEBUG >= 2:
			print("Dual model is infeasible or unbounded")

		return (0, 0, [], [])

	elif solver.status == GRB.Status.LOADED:

		if DEBUG >= 2:
			print("LP solver does not found a solution")

		return (0, 0, [], [])

	else:
		print("Weird return from LP solver")
		exit(1)

	# if DEBUG >= 2:
	#     # Display instance information and results.
	#     print(f"Number of variables = {solver.NumVariables()}")
	#     print(f"Number of constraints = {solver.NumConstraints()}")

	# The value of each variable in the solution.
	v_values = solver.getAttr('X', v)
	if DEBUG >= 3:
		print('Solution for v variables:')
		print(v_values)

	if DEBUG >= 4:
		print('Solution for w variables:')
	w_values = []
	for i in range(0, len(facility_list)):
		w_values.append(solver.getAttr('X', w[i]))
		if DEBUG >= 4:
			print(w_values[i])

	if DEBUG >= 2:
		# The objective value of the solution.
		print("Optimal objective value = %.2f" % solver.objVal)

	# Choose here if we want the solution cost to be integer or not 
	#sol_value = int(solver.objVal) 
	sol_value = solver.objVal

	return (sol_value, intOPT, v_values, w_values)
