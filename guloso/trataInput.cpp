#include <iostream>
#include <cstdlib>
#include <fstream>
#include "declaracoes.hpp"
#define EPS 0.001

using namespace std;

solutionType trataInput(char inputName[], char tipoEntrada[]){

   // Declaracoes iniciais
   int qtd_clientes, qtd_instalacoes, cont;
   double auxRead;
   char auxCRead[30];
   ifstream inputFLP;

   double * custoA;
   double * custoF;

   solutionType solution;

   int debug = 0; // OPCOES DE DEBUG: 0 PARA NAO EXIBIR NADA, 1 PARA EXIBIR AS INFORMACOES SENDO SALVAS

   // Abrindo arquivo 
   inputFLP.open(inputName); 

   cout << fixed;
   cout.precision(5);

   if(strcmp(tipoEntrada,"1")==0){
      // Lendo do arquivo os valores que indicam a quantidade de instalacoes e clientes
      inputFLP >> qtd_instalacoes >> qtd_clientes; 

      cout << "QTD INST: " << qtd_instalacoes << " E QTD CLI: " << qtd_clientes << endl;

      // Vetores que salvarao custos lidos no arquivo
      custoF = (double*) malloc((qtd_instalacoes) * sizeof(double));
      if(!custoF){
         cout << "Memory Allocation Failed";
         exit(1);
      }

      custoA = (double*) malloc((qtd_clientes*qtd_instalacoes) * sizeof(double));
      if(!custoA){
         cout << "Memory Allocation Failed";
         exit(1);
      }


      // Lendo do arquivo os custos de abertura das instalacoes e salvando no vetor custoF
      for(int i=0;i<qtd_instalacoes;i++){
         inputFLP >> auxCRead; // Descartando a capacidade da instalação
         // cout << "DESCARTE: " << auxCRead << endl;
         inputFLP >> auxRead; // Pegando o valor do custo de abertura da instalação
         custoF[i] = auxRead;

         if(debug > 0){
            cout << "Fi =  " << custoF[i] << endl;  
         }  
      }


      // Lendo do arquivo os custos de atribuicao do clientes com as instalacoes e salvando no vetor custoA
      cont = 0;
      for(int i=0;i<qtd_clientes;i++){
         inputFLP >> auxRead; // Descartando aqui a demanda do cliente

         if(debug > 0){
            cout << "Demanda = " << auxRead << endl;
         }

         for(int j=0;j<qtd_instalacoes;j++){
            inputFLP >> auxRead;
            custoA[cont] = auxRead;

            if(debug > 0){
               cout << "CA = " << custoA[cont] << endl; 
            }

            cont+=1;
         }
      }

      // Chamando a funcao que resolve o problema de fato
      solution = guloso(qtd_clientes, qtd_instalacoes, custoF, custoA);
   }
   else if(strcmp(tipoEntrada,"2")==0){
      inputFLP >> auxCRead; // Descartando a palavra FILE
      inputFLP >> auxCRead; // Descartando o nome do arquivo

      // Lendo do arquivo os valores que indicam a quantidade de instalacoes e clientes. E descartando o 0
      inputFLP >> qtd_instalacoes >> qtd_clientes >> auxRead; 

      cout << "QTD INST: " << qtd_instalacoes << " E QTD CLI: " << qtd_clientes << endl;

      // Vetores que salvarao custos lidos no arquivo
      custoF = (double*) malloc((qtd_instalacoes + 1) * sizeof(double));
      if(!custoF){
         cout << "Memory Allocation Failed";
         exit(1);
      }

      custoA = (double*) malloc((qtd_clientes*qtd_instalacoes + 1) * sizeof(double));
      if(!custoA){
         cout << "Memory Allocation Failed";
         exit(1);
      }


      // Lendo do arquivo os custos de abertura das instalacoes e salvando no vetor custoF
      for(int i=0;i<qtd_instalacoes;i++){
         inputFLP >> auxRead; // Descartando o nome da instalação
         // cout << "DESCARTE:" << auxRead << endl;
         inputFLP >> auxRead; // Pegando o valor do custo de abertura da instalação
         custoF[i] = auxRead;

         if(debug > 0){
            cout << "Fi =  " << custoF[i] << endl;  
         }  

         // Lendo do arquivo os custos de atribuicao do clientes com as instalacoes e salvando no vetor custoA
         for(int j=0;j<qtd_clientes;j++){

            inputFLP >> auxRead;
            custoA[i + j * qtd_instalacoes] = auxRead;       // Esse indice estranho eh para deixa no mesmo formato que o caso ORLIB

            if(debug > 0){
               cout << "CA = " << custoA[i + j * qtd_instalacoes] << endl; 
            }
         }
      }

      // Chamando a funcao que resolve o problema de fato
      solution = guloso(qtd_clientes, qtd_instalacoes, custoF, custoA);
   }
   else{
      cout << "Tipo de entrada invalida." << endl;
      return solution;
   }

   // Fechando o arquivo
   inputFLP.close();
   free(custoF);
   free(custoA);
   return solution;
}
