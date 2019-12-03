import os
import sys

if __name__ == '__main__':

	print("Test Tabu Search")

	inputType = "2"
	seed = "0"

	bc1 = ["0.01","0.04"] #["0.01","0.04","0.01"]
	bc2 = ["0.05","0.08"] #["0.05","0.08","0.08"]

	qtd = len(bc1)

	a1 = ["2.5"]

	limit_idle = ["0.02"] #??? #0.1 #0.02
	
	lh = ["10", "50", "100", "500"] 

	for l in lh:
		for li in limit_idle:
			for i in range(qtd):
				for a in a1:
					# Chamando o programa a ser testado
					os.system("python3 tester.py " + inputType + " " + a + " " + bc1[i] + " " + bc2[i] + " " + bc1[i] + " " + bc2[i] + " " + seed + " " + li + " " + l)

