from random import randint
import sys


if __name__ == '__main__':

	if len(sys.argv) > 2:
		num_inst = int(sys.argv[1].strip())
		num_cli = int(sys.argv[2].strip())
	else:
		num_inst = 100
		num_cli = 600

max_custo_inst = 10000
max_custo_cli = 10000

print(num_inst, num_cli)

for i in range(num_inst):
	n = randint(0,max_custo_inst)
	print("0" , n)

for i in range(num_cli):
	print(i)
	for j in range(num_inst):
		n = randint(0,max_custo_cli)
		print(n, end = " ")
	print()