#include <iostream>
#include <fstream>
#include "declaracoes.hpp"
#define EPS 0.001

using namespace std;

double trataInput(char inputName[], char tipoEntrada[]){

   // Declaracoes iniciais
   int qtd_clientes, qtd_instalacoes, cont;
   double auxRead;
   char auxCRead[30];
   ifstream inputFLP;
   double sol_cost = 0;
   int debug = 0; // OPCOES DE DEBUG: 0 PARA NAO EXIBIR NADA, 1 PARA EXIBIR AS INFORMACOES SENDO SALVAS

   // Abrindo arquivo 
   inputFLP.open(inputName); 


   if(strcmp(tipoEntrada,"1")==0){
      // Lendo do arquivo os valores que indicam a quantidade de instalacoes e clientes
      inputFLP >> qtd_instalacoes >> qtd_clientes; 

      cout << "QTD INST: " << qtd_instalacoes << " E QTD CLI: " << qtd_clientes << endl;

      // Declaracao vetores que salvarao custos lidos no arquivo
      double custoF[qtd_instalacoes], custoA[qtd_clientes*qtd_instalacoes]; 

      // Lendo do arquivo os custos de abertura das instalacoes e salvando no vetor custoF
      for(int i=0;i<qtd_instalacoes;i++){
         inputFLP >> auxRead; // Descartando a capacidade da instalação
         // cout << auxRead << endl;
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
            inputFLP >> auxRead;                                 // PROBLEMA: Pq só está pegando 2 casas decimais???
            custoA[cont] = auxRead;

            if(debug > 0){
               cout << "CA = " << custoA[cont] << endl; 
            }

            cont+=1;
         }
      }


      // Chamando a funcao que resolve o problema de fato
      sol_cost = primalDual(qtd_clientes, qtd_instalacoes, custoF, custoA);
   }
   else if(strcmp(tipoEntrada,"2")==0){
      inputFLP >> auxCRead; // Descartando a palavra FILE
      inputFLP >> auxCRead; // Descartando o nome do arquivo

      // Lendo do arquivo os valores que indicam a quantidade de instalacoes e clientes. E descartando o 0
      inputFLP >> qtd_instalacoes >> qtd_clientes >> auxRead; 

      cout << "QTD INST: " << qtd_instalacoes << " E QTD CLI: " << qtd_clientes << endl;

      // Declaracao vetores que salvarao custos lidos no arquivo
      double custoF[qtd_instalacoes], custoA[qtd_clientes*qtd_instalacoes]; 

      // Lendo do arquivo os custos de abertura das instalacoes e salvando no vetor custoF
      cont = 0;
      for(int i=0;i<qtd_instalacoes;i++){
         inputFLP >> auxRead; // Descartando o nome da instalação
         // cout << auxRead << endl;
         inputFLP >> auxRead; // Pegando o valor do custo de abertura da instalação
         custoF[i] = auxRead;

         if(debug > 0){
            cout << "Fi =  " << custoF[i] << endl;  
         }  

         // Lendo do arquivo os custos de atribuicao do clientes com as instalacoes e salvando no vetor custoA
         for(int j=0;j<qtd_clientes;j++){

            inputFLP >> auxRead;                                 // PROBLEMA: Pq só está pegando 2 casas decimais???
            custoA[cont] = auxRead;

            if(debug > 0){
               cout << "CA = " << custoA[cont] << endl; 
            }
            cont+=1;
         }
      }

      // Chamando a funcao que resolve o problema de fato
      sol_cost = primalDual(qtd_clientes, qtd_instalacoes, custoF, custoA);
   }
   else{
      cout << "Tipo de entrada invalida." << endl;
      return sol_cost;
   }
   

   // Fechando o arquivo
   inputFLP.close();
   return sol_cost;
}
