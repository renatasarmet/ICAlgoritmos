import os
import sys

if __name__ == '__main__':

	inputType = "2"

	best_fit = ["1"]#["0","1"]  # 1 para best fit 0 para first fit
	
	# a1 = ["0.1","0.5","1","2.5"]
	a1 = ["2.5"]#["1"]

	limit_idle = ["0.02"] #??? #0.1 #0.02

	# lh = ["1","10","50","250","500","1000","10000"] #????
	lh = ["1","10"]


	for bf in best_fit:
		for l in lh:
			for li in limit_idle:
				for a in a1:
					# Chamando o programa a ser testado
					os.system("python3 tester.py " + inputType + " " + bf + " " + a + " " + li + " " + l)

