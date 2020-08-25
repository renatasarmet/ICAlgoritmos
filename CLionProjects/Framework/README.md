This is the code for the Evolutive Framework, proposed by
Pedro Hokama, Mario C. San Felice, Evandro Bracht and Fabio Usberti. 
We coupled the implemented greedy algorithm.

## Framework

### To compile:

cd Release
make all

This will generate the binary to solve the example problem SUFL.
an example instance can be found in /Release/inst.txt

### To run with the example instance:
./Framework inst.txt -1 3

### To run with a generated instance (ex: cap71-txt_5s_110-150i_50c.txt):

./Framework ../createInstance/generatedInstances/cap71-txt_5s_110-150i_50c.txt  -1 3

### To test all Release/testCases.txt: 

cd Release
python3 tester.py

### To read solutions and create csv with all Release/testCasesReader.csv:
cd Release
python3 readerSolParameters.py


## Gurobi

### To compile:
cd ILP
g++ -m64 -g -o SUFLP SUFLP.cpp -I/Library/gurobi900/mac64/include/ -L/Library/gurobi900/mac64/lib -lgurobi_c++ -lgurobi90 -lm


### To run with a generated instance (ex: cap71-txt_5s_110-150i_50c.txt):
cd ILP
./SUFLP < ../createInstance/generatedInstances/cap71-txt_5s_110-150i_50c.txt


### To test all ILP/testCases.txt: 

cd ILP
python3 tester.py

### To read solutions and create csv with all ILP/testCasesReader.csv:
cd ILP
python3 readerSolParameters.py
