import os
import sys

if __name__ == '__main__':

	inputType = "2"
	seed = "0"

	# bc1 = ["0.01","0.04","0.1"]
	# bc2 = ["0.05","0.08","0.2"]

	bc1 = ["0.01"]
	bc2 = ["0.08"]

	a1 = ["0.1","0.5","1","2.5"]

	qtd = len(bc1)

	for i in range(qtd):
		for a in a1:
			# Chamando o programa a ser testado
			os.system("python3 tester.py " + inputType + " " + a + " " + bc1[i] + " " + bc2[i] + " " + bc1[i] + " " + bc2[i] + " " + seed)

