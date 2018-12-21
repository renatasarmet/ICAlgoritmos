#include <iostream>
#include <ctime>
#include "declaracoes.hpp"

using namespace std;

int main(int argc, char *argv[]){
    if (argc > 1){
		trataInput(argv[1]);
    }
    else
		cout << "OPS DEU RUIM O ARGUMENTO" << endl;
	return 0;
};
