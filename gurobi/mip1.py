#!/usr/bin/python

# This example formulates and solves the following model:
#  minimize
#       SUM(􏰂fi * yi)_{i \in F} + 􏰂 SUM(􏰂cij * xij)_{i \in F, j \in D} 
#  subject to
#       SUM(xij)_{i \in F} = 1 , Aj \in D
#       xij <= yi              , Ai \in F, Aj \in D
#       xij >= 0               , Ai \in F, Aj \in D
#       yi >= 0                , Ai \in F


from gurobipy import *

try:

    # Create a new model
    m = Model("flp")

    # Create variables
    c = m.addVars(F,D,vtype=GRB.CONTINUOUS, name="c")
    x = m.addVars(F,D,vtype=GRB.CONTINUOUS, name="x")

    f = m.addVars(F,vtype=GRB.CONTINUOUS, name="f")
    y = m.addVars(F,vtype=GRB.CONTINUOUS, name="y")

    # Set objective COMO FAZER ISSO???
    m.setObjective(sum(f_i*y_i for f_i in F for y_i in ??), GRB.MINIMIZE)

    # Add constraint: SUM(xij)_{i \in F} = 1 , Aj \in D
    for j in D:
        m.addConstrs((x.sum('*',i,j) == 1 for i in F), "c1")

    # Add constraint: xij <= yi    , Ai \in F, Aj \in D
        m.addConstrs(x <= y for i in F for j in D , "c2")

    # Add constraint: xij >= 0     , Ai \in F, Aj \in D
        m.addConstrs(x >= 0 for i in F for j in D , "c3")

    # Add constraint: yi >= 0      , Ai \in F
        m.addConstrs(y >= 0 for i in F , "c4")


    # Optimize model
    m.optimize()

    for v in m.getVars():
        print('%s %g' % (v.varName, v.x))

    print('Obj: %g' % m.objVal)

except GurobiError as e:
    print('Error code ' + str(e.errno) + ": " + str(e))

except AttributeError:
    print('Encountered an attribute error')