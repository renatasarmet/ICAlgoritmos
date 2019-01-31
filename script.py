from random import randint

num_inst = 100
num_cli = 600

print(num_inst, num_cli)

for i in range(num_inst):
	n = randint(0,10000)
	print("0" , n)

for i in range(num_cli):
	print(i)
	for j in range(num_inst):
		n = randint(0,10000)
		print(n, end = " ")
	print()