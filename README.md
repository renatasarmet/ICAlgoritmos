# ICAlgoritmos
Codigos para resolver o problema da localizacao de instalacoes sem capacidade

## Conteudo das pastas 
- A pasta gurobi contem os arquivos relacionados a resolucao do programa linear inteiro com Gurobi.
- A pasta primalDual contem os arquivos relacionados a resolucao aproximada do problema, utilizando o metodo Primal Dual.
- A pasta baseDeTestes contem diversos casos de teste, com suas respectivas solucoes otimas. Todos os casos que forem escolhidos para compor os testes de determinado algoritmo, devem ser copiados para pasta de dados do mesmo.
- O arquivo solutions.txt contem a analise e comparacao dos resultados

- OBS: Para criar novas instancias pode-se utilizar o script.py. Para isso, considere qtd_inst como a quantidade de instalacoes que deseja, qtd_cli como a quantidade de clientes, arquivo.txt como o arquivo no qual deseja que a instancia seja criada, então basta digitar no terminal:

python3 script.py qtd_inst qtd_cli > arquivo.txt

### Mais detalhes
Os detalhes de cada pasta estao em um READ.ME especifico da mesma.
