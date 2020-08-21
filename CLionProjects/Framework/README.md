This is the code for the Evolutive Framework, proposed by
Pedro Hokama, Mario C. San Felice, Evandro Bracht and Fabio Usberti

run:

cd Release
make all
./Framework inst.txt -1 3

This will generate the binary to solve the example problem SUFL.
a example instance can be found in /Release/inst.txt

To run a generated instance (ex: cap71-txt_5s_110-150i_50c.txt):

./Framework ../createInstance/generatedInstances/cap71-txt_5s_110-150i_50c.txt  -1 3


TO TEST ALL Release/testCases.txt: 

cd Release
python3 tester.py

TO READ SOLUTIONS AND CREATE CSV WITH ALL Release/testCasesReader.csv:
cd Release
python3 readerSolParameters.py


Compile Gurobi:

g++ -m64 -g -o SUFLP SUFLP.cpp -I/Library/gurobi900/mac64/include/ -L/Library/gurobi900/mac64/lib -lgurobi_c++ -lgurobi90 -lm


Run Gurobi:

./SUFLP < createInstance/generatedInstances/cap71-txt_5s_110-150i_50c.txt
