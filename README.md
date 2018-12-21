Codigos para resolver o problema da localizacao de instalacoes sem capacidade


-------- ALGORITMO PRIMAL DUAL --------


*COMO RODAR?*


No terminal, primeiramente, digite:

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