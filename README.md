Codigos para resolver o problema da localizacao de instalacoes sem capacidade


-------- ALGORITMO PRIMAL DUAL --------


*COMO RODAR?*

Primeiramente, eh provavel que seja necessario ter instalado a biblioteca Lemon Graph.


Apos isso, digite no terminal:

g++ main.cpp trataInput.cpp bipartidoPrimalDual.cpp -o main


Em seguida:

g++ tempo.cpp -o tempo


Por fim:

./tempo



*SOBRE AS ENTRADAS*


Cada entrada deve estar em um arquivo txt, na pasta facilityTestCases.

Existe um arquivo chamado testCases.txt o qual contem uma lista de todos os nomes dos arquivos presentes nessa pasta facilityTestCases.

Atualmente, a entrada possui o formato:

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