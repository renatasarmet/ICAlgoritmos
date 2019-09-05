#include <iostream>

struct solutionType {
	int * instalacoes_conectadas;
	double gastoTotalFinal;
	double timeSpent;
	int qtd_clientes;
};

solutionType trataInput(char inputName[], char tipoEntrada[]);
solutionType guloso(int qtdCli, int qtdInst, double * custoF, double * custoA);
