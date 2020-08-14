Codigos para criar novas instancias para two-stage stochastic UFL  

-------- GERACAO DE INSTANCIAS PARA A VERSAO ESTOCASTICA DE DOIS ESTAGIOS --------


*COMO RODAR?*

Digite no terminal:

g++ handlesInput.cpp ../src/Instance.cpp -o create -Ofast


Considere que input_type significa 1 para o tipo ORLIB e 2 para o tipo SIMPLE FORMAT.

Então, para rodar nosso testador, basta digitar:

python3 tester.py input_type

Também é possível passar alguns parametros para o testador, como numero de cenarios, intervalo de inflacao e chance dos clientes estarem em cada cenario.

Caso queira executar apenas um exemplo, considere que input_name é o caminho completo do problema UFL a ser resolvido, solutionName é o caminho completo onde deseja-se guardar a solucao e o input_type é o tipo da entrada (1 para o tipo ORLIB e 2 para o tipo SIMPLE FORMAT).
Então, basta digitar:

./create input_name solutionName input_type number_of_scenarios inflation_start inflation_end chance_cli_scenario


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

\#Facilities = m \#Clientes = n
f1 f2 ... fm
c11 c12 ... c1m # custo de conexão de cada cliente
..
cn1 cn2 ... cnm
\#Scenarios = S
p1 p2 ... pS
s1d1 s1d2 ... s1dn # quais clientes estão em quais cenários (vetor binário 0 1)
s2d1 s2d2 ... s2dn
..
sSd1 sSd2 ... sSdn
s1f1 s1f2 ... s1fm # qual custo de cada instalação em cada cenário
s2f1 s2f2 ... s2fm
..
sSf1 sSf2 ... sSfm



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

