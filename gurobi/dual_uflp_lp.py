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



import time

import sys
from collections import namedtuple
import math
from gurobipy import *
import csv
import re


Point = namedtuple("Point", ['x', 'y'])
Facility = namedtuple(
    "Facility", ['index', 'setup_cost'])
Customer = namedtuple("Customer", ['index', 'attribution_cost'])

# time limit in seconds
time_limit = 900 #15 minutos
# memory limit in bytes
memory_limit = 10 ** 9 # 1GB


DEBUG = 0



def dualLP(facility_list, client_list, bound):

    # Instantiate a Gurobi solver, naming it SolveLinearProblem
    solver = Model('SolveLinearProblem')

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
            print("LP model does not found a solution")

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


def solve_it(input_type, input_data):

    # parse the input
    parts = input_data.split()

    # ORLIB type input
    if (input_type == '1'):

        facility_count = int(parts[0])
        customer_count = int(parts[1])

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

        pair_new = dualLP(facilities, customers, bound)

        if DEBUG >= 2:
            print(f"Dual LP solution value = {pair_new[0]}")

        # put a conditional here when we stablish a correct bound to send to dualLP (?)
        pair_best = pair_new

    else:
        print("Exceeded Memory limit.")
        pair_best = (0,0,[-1] * len(customers), [-1] * len(customers) * len(facilities))

    #return output_data
    return pair_best



if __name__ == '__main__':

    ok = 0
    if len(sys.argv) > 1:
        input_type = sys.argv[1].strip()

        if(input_type == '1'):
            print("ORLIB type")
            file_location = 'testCases1.txt'
            ok = 1
        elif(input_type == '2'):
            print("SIMPLE FORMAT type")
            file_location = 'testCases2.txt'
            ok = 1
        else:
            print("ERROR: Invalid parameter value")
            print("SOLUTION: Select 1 for ORLIB inputs or 2 for SIMPLE FORMAT inputs")

        if ok:
            with open(file_location, 'r') as input_list_data_file:
                input_list_data = input_list_data_file.read()

            files_test = input_list_data.split()

            for file_name in files_test:
                print()
                print("Input file:", file_name)
                start = time.time()

                complete_file_name = '../baseDeTestes/facilityTestCases/tests/' + file_name
                with open(complete_file_name, 'r') as input_data_file:
                    input_data = input_data_file.read()

                solution = solve_it(input_type, input_data)

                end = time.time()
                time_spent = end - start

                if DEBUG >= 1:
                    print("Time spent =", time_spent)

                complete_sol_file_name = 'solutionsDual/' + file_name + '.sol'

                file = open(complete_sol_file_name, 'w')
                sentence = str(solution[0]) + " " + str(time_spent) + " " + str(solution[1])

                # solution[2] is the v_values
                for v in solution[2]: 
                  sentence += " " + str(v)
                  
                # solution[3] is the w_values
                for w_f in solution[3]: # each w_f is a list of w from the facility f to every client
                  for w in w_f: # each w is a value of w from the facility f to this especific client
                      sentence += " " + str(w)

                file.write(sentence)
                file.close()

    else:
        print('This test requires an input type. \nPlease select one: 1 for ORLIB inputs or 2 for SIMPLE FORMAT inputs.')




