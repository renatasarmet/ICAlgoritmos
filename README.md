Codigos para resolver o problema da localizacao de instalacoes sem capacidade


-------- ALGORITMO PRIMAL DUAL --------


*COMO RODAR?*

Primeiramente, eh provavel que seja necessario ter instalado a biblioteca Lemon Graph.


Apos isso, digite no terminal:

g++ main.cpp trataInput.cpp bipartidoPrimalDual.cpp -o main


Em seguida:

g++ tempo.cpp -o tempo


Considere que tipoEntrada significa 1 para o tipo ORLIB e 2 para o tipo SIMPLE FORMAT.
Então, por fim, basta digitar:

./tempo tipoEntrada


*SOBRE AS ENTRADAS*


Cada entrada deve estar em um arquivo, na pasta facilityTestCases/tests.

Para entradas do tipo ORLIB, existe um arquivo chamado testCases1.txt, o qual contem uma lista de todos os nomes dos arquivos presentes nessa pasta facilityTestCases/tests que deverao ser testados.
Para entradas do tipo SIMPLE FORMAT, o mesmo acontece, porem no arquivo chamado testCases2.txt.

Se quiser criar novas entradas para teste, coloque-as na pasta correspondente às entradas do ORLIB e utilize o seguinte formato:

quantidade_de_clientes quantidade_de_instalacoes
custo_f_1 custo_f_2 .... custo_f_n
custo_atribuicao_cliente_1_instalacao_1 .... custo_atribuicao_cliente_1_instalacao_n 
.
.
custo_atribuicao_cliente_m_instalacao_1 .... custo_atribuicao_cliente_m_instalacao_n  



*SOBRE A SAIDA*

A saída eh composta por um grafo Tlinha, contendo todas as instalacoes abertas e todos os clientes associados a suas respectivas instalacoes mais adequadas.
Tambem eh calculado o total gasto, sendo esse a soma dos custos de abrir todas as instalacoes escolhidas para serem abertas + o custo de atribuir cada cliente a sua instalacao indicada como a mais adequada.

Alem disso, eh gerado um arquivo timeLog.txt que possui as informacoes sobre o tempo gasto em cada execucao.




*SOBRE AS ENTRADAS ORLIB*

In the ORLIB-cap format it is possible to store instances for the uncapacitated and capacitated facility location problem. The first line of a file consists n and m:
[n] [m]
Then the next n lines consist of the opening cost and the capacity of the corresponding facility.
So for each facility i: (i = 1 ,..., n):
[capacity] [opening cost]
In the following the numbers for the cities are the demand and the connections to all facilities.
So for each city j (j = 1, ... ,m): 
[demand of j] 
[cost of allocating all demand of j to facility i] (i = 1,...,n)

Example: uncapacitated n = 4, m = 3
4 3
0 300
0 400
0 150
0 200
0
130 140 130 100
0
120 100 90 120
0 
80 50 140 150



*FORMATO DA SOLUCAO ORLIB*

The data-format of the solutions is very simple. At first there are m numbers in the range of [0,n-1]. The i-th number is the facility city i is connected to. The last number of the file is the cost of the solution. Facilities and cities are ordered by the appearance in the problem file. If the number of a facility does not appear in the solution file, it remains closed in the solution. Please note that the numbering starts with 0 here !

Example: n = 4, m = 3
2 2 2 510



*SOBRE AS ENTRADAS SIMPLE FORMAT*

The simple format is only suitable for instances of the uncapacitated facility location problem.
The first line consists of 'FILE: ' and the name of the file. In the next line n, m and 0 are denoted:
[n] [m] 0
The next n lines consist of the number of the facility, the opening cost and the connetion cost to the cities.
So for facility i (i = 1, ... , n) we have
[i] [opening cost] [cost of conneting city j to facility i] (j = 1, ... ,m)

Example: n = 4, m = 3
FILE: Exapmle.txt
4 3 0
1 300 130 120 80
2 400 140 100 50
3 150 130 90 140
4 200 100 120 150



*FORMATO DA SOLUCAO SIMPLE FORMAT*

The optimal solutions are denoted in the following form. For each city there
is the number of the facility which the city is connected to in the optimal
solution. Here the first number is for the first city, the second number for
the second city and so on. The cities are ordered according to their 
connection costs in the problem file.
