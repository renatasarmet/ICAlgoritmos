#!/usr/bin/python
# -*- coding: utf-8 -*-

import time

import sys
from collections import namedtuple
import math
from gurobipy import *


Point = namedtuple("Point", ['x', 'y'])
Facility = namedtuple(
    "Facility", ['index', 'setup_cost'])
Customer = namedtuple("Customer", ['index', 'attribution_cost'])

# time limit in seconds
time_limit = 100
# memory limit in bytes
memory_limit = 10 ** 9


DEBUG = 2


def length(point1, point2):
    return math.sqrt((point1.x - point2.x)**2 + (point1.y - point2.y)**2)


def facilityILP(facility_list, client_list, bound):

    # Instantiate a Gurobi solver, naming it SolveIntegerProblem
    solver = Model('SolveIntegerProblem')

    # Turn verbose off
    # solver.Params.OutputFlag = 0
    solver.setParam(GRB.Param.OutputFlag, 0)
    # solver.setParam("OutputFlag", 0)

    # Set a time limit
    solver.setParam(GRB.Param.TimeLimit, time_limit)

    # Create the variables and define their types.
    # y[i] are binary variables.
    y = []
    for i in range(0, len(facility_list)):
        y.append(solver.addVar(vtype=GRB.BINARY, name='y[%d]' % i))

    # x[i][j] are binary variables.
    x = []
    for i in range(0, len(facility_list)):
        x.append([])
        for j in range(0, len(client_list)):
            x[i].append(solver.addVar(
                vtype=GRB.BINARY, name='x[%d][%d]' % (i, j)))

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
            print("ILP model does not found a solution")

        return (bound + 1, [], intOPT)

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

        return (bound + 1, [], 0)

    elif solver.status == GRB.Status.CUTOFF:

        if DEBUG >= 2:
            print("LP model solution is worse than bound")

        return (bound + 1, [], 0)

    elif solver.status == GRB.Status.LOADED:

        if DEBUG >= 2:
            print("LP solver does not found a solution")

        return (bound + 1, [], 0)

    else:
        print("Weird return from LP solver")
        exit(1)

    # if DEBUG >= 2:
    #     # Display instance information and results.
    #     print(f"Number of variables = {solver.NumVariables()}")
    #     print(f"Number of constraints = {solver.NumConstraints()}")

    # The value of each variable in the solution.
    if DEBUG >= 3:
        print('Solution for y variables:')
        y_values = solver.getAttr('X', y)
        print(y_values)

    if DEBUG >= 4:
        print('Solution for x variables:')
        x_values = []
        for i in range(0, len(facility_list)):
            x_values.append(solver.getAttr('X', x[i]))
            print(x_values[i])

    if DEBUG >= 2:
        # The objective value of the solution.
        print("Optimal objective value = %.2f" % solver.objVal)

    sol_value = int(solver.objVal)

    solution = [-1] * len(client_list)

    for j in range(0, len(client_list)):
        for i in range(0, len(facility_list)):
            if x[i][j].getAttr('X') == 1:
                solution[j] = i
                break

    if DEBUG >= 3:
        print(solution)

    return (sol_value, solution, intOPT)


def solve_it(input_data):

    # parse the input
    lines = input_data.split('\n')

    parts = lines[0].split()
    facility_count = int(parts[0])
    customer_count = int(parts[1])

    print("QTD inst: ", facility_count)
    print("QTD cli: ", customer_count)

    facilities = []
    for i in range(1, facility_count+1):
        parts = lines[i].split()
        facilities.append(Facility(
            i-1, float(parts[1])))
        # print("para inst:",facilities[i-1].index, "temos custo:", facilities[i-1].setup_cost )

    customers = []
    cont = 0
    for i in range(facility_count+2, facility_count+(2*customer_count)+1,2):
        parts = lines[i].split()
        list_attr_cost = []
        for j in range(len(facilities)):
            list_attr_cost.append(float(parts[j]))
           
        customers.append(Customer(
            cont, list_attr_cost))
        # print("para cli", customers[-1].index, "temos custo atr:", customers[-1].attribution_cost)
        cont += 1
    
    bound = 0 # just because I dont know yet what to do with this

    if facility_count * customer_count * facility_count <= memory_limit:
        # if 20 * facility_count * customer_count * facility_count <= memory_limit:
        # if (customer_count * facility_count) ** 2 <= memory_limit:

        pair_new = facilityILP(facilities, customers, bound)

        if DEBUG >= 2:
            print(f"Facility ILP solution value = {pair_new[0]}")

        pair_best = pair_new

    # prepare the solution in the specified output format
    output_data = str(pair_best[0]) + ' ' + str(pair_best[2]) + '\n'
    output_data += ' '.join(map(str, pair_best[1]))

    return output_data


start = time.time()

if __name__ == '__main__':
    import sys
    if len(sys.argv) > 1:
        file_location = sys.argv[1].strip()
        with open(file_location, 'r') as input_data_file:
            input_data = input_data_file.read()
        print(solve_it(input_data))
    else:
        print('This test requires an input file.  Please select one from the data directory. (i.e. python solver.py ./data/fl_16_2)')

end = time.time()
if DEBUG >= 1:
    print("Time spent =", end - start)
