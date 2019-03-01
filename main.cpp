#include <iostream>
#include <ctime>
#include "declaracoes.hpp"

using namespace std;

int main(int argc, char *argv[]){
    if (argc > 2)
		trataInput(argv[1], argv[2]);
    else
		cout << "ERRO! Necessario passar como argumento o arquivo a ser utilizado para ler as entradas!" << endl;
	
	return 0;
};
