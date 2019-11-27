import os
import sys

if __name__ == '__main__':

	print("Test Late Acceptance")

	inputType = "2"

	best_fit = ["0","1"]  # 1 para best fit 0 para first fit
	
	# a1 = ["0.1","0.5","1","2.5"]
	a1 = ["2.5"]#["1"]

	limit_idle = ["0.02"] #??? #0.1 #0.02

	for bf in best_fit:
		if bf == "1":
			a_lh = ["10"] # nesse caso sera o valor de lh mesmo
		else:
			a_lh = ["0.05","0.1","0.5","1"] # nesse caso sera multiplicado pelo numero de instalacoes
		for l in a_lh:
			for li in limit_idle:
				for a in a1:
					# Chamando o programa a ser testado
					os.system("python3 tester.py " + inputType + " " + bf + " " + a + " " + li + " " + l)

