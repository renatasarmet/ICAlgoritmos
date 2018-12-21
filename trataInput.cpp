#include <iostream>
#include <fstream>
#include "declaracoes.hpp"
#define EPS 0.001

using namespace std;


// Peguei exemplo do Nicholas... adaptar pro meu problema
// GITHUB DELE:  https://github.com/Luterium/Elaborate-Set-Cover/tree/master/elaborateSetCover


// bool comparaFloats(float i, float j){
//     if(i > j-EPS && i < j+EPS)
//         return true;
//     return false;
// }

void trataInput(char inputName[]){
	cout << "To no trata input com " << inputName << endl;
   int qtd_clientes, qtd_instalacoes;
   ifstream inputFLP;
   inputFLP.open(inputName);
   inputFLP >> qtd_clientes >> qtd_instalacoes;
   cout << "qtd clientes: " << qtd_clientes << " qtd_instalacoes " << qtd_instalacoes << endl;









   inputFLP.close();

   //  int n, m, auxWeight, auxQtdSubsets, auxSubset;
   //  int totalWeight = 0;
   //  ifstream inputSCP;
   //  inputSCP.open(inputName);
   //  inputSCP >> n >> m;
   //  fstream costLog;
   //  costLog.open("costLogComum.txt", std::fstream::in | std::fstream::out | std::fstream::app);

   //  int elementosDescobertosSize = n;
   //  vector <Subset> Subsets(m);
   //  vector <int> subsetsIndexes(m);
   //  vector < vector <int> > elementos(n);
   //  vector <bool> elementosCobertos(n, false);
   //  vector <int> subconjuntosEscolhidos;



   //  for(int i = 0; i < m; i++){
   //      inputSCP >> auxWeight;
   //      Subsets[i].setWeight(auxWeight);
   //      Subsets[i].setSubsetNumber(i);
   //      subsetsIndexes[i] = i;
   //  }

   //  for (int i = 0; i < n; i++){
   //      inputSCP >> auxQtdSubsets;
   //      for(int j = 0; j < auxQtdSubsets; j++){
   //          inputSCP >> auxSubset;
   //          elementos[i].push_back(auxSubset-1);
   //          Subsets[auxSubset-1].incrementSize();
   //          Subsets[auxSubset-1].adicionaElemento(i);
   //      }
   //  }
   //  Heapify(Subsets, subsetsIndexes);
   //  while(elementosDescobertosSize){
   //      totalWeight += Subsets[0].getWeight();
   //      subconjuntosEscolhidos.push_back(Subsets[0].getSubsetNumber());
   //      coverElements(Subsets, subsetsIndexes, elementosCobertos, elementosDescobertosSize, elementos);
   //  }
   //  cout << "Total cover cost: " << totalWeight << endl;
   //  costLog << totalWeight << endl;
   // /* for(unsigned int i = 0; i < subconjuntosEscolhidos.size(); i++){
   //      cout << subconjuntosEscolhidos[i]+1 << " ";
   //  }*/

   //  inputSCP.close();
}
