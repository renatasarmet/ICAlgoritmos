import os
import sys

if __name__ == '__main__':

	# Chamando os testers correspondentes a serem testados	
	os.chdir("tabuSearch")
	print(os.getcwd())
	os.system("python3 call_tester.py")
	
	os.chdir("../lateAcceptance")
	print(os.getcwd())
	os.system("python3 call_tester.py")