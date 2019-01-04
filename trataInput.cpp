#include <iostream>
#include <fstream>
#include "declaracoes.hpp"
#define EPS 0.001

using namespace std;

void trataInput(char inputName[]){

   // Declaracoes iniciais
   int qtd_clientes, qtd_instalacoes;
   float auxRead;
   ifstream inputFLP;

   cout << "teste 1" << endl;

   // Abrindo arquivo 
   inputFLP.open(inputName); 

   cout << "teste 2" << endl;

   // Lendo do arquivo os valores que indicam a quantidade de clientes e instalacoes
   inputFLP >> qtd_clientes >> qtd_instalacoes; 

   cout << "teste 3" << endl;

   // Declaracao vetores que salvarao custos lidos no arquivo
   float custoF[qtd_instalacoes], custoA[qtd_clientes*qtd_instalacoes]; 

   cout << "teste 4" << endl;


   // Lendo do arquivo os custos de abertura das instalacoes e salvando no vetor custoF
   for(int i=0;i<qtd_instalacoes;i++){
      inputFLP >> auxRead;
      custoF[i] = auxRead;
   }


   cout << "teste 5" << endl;

   // Lendo do arquivo os custos de atribuicao do clientes com as instalacoes e salvando no vetor custoA
   for(int i=0;i<(qtd_clientes*qtd_instalacoes);i++){
         inputFLP >> auxRead;
         custoA[i] = auxRead;
   }


   cout << "teste 6" << endl;

   // Chamando a funcao que resolve o problema de fato
   primalDual(qtd_clientes, qtd_instalacoes, custoF, custoA);


   // Fechando o arquivo
   inputFLP.close();
}
