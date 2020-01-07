#ifndef CLIONPROJECTS_LOCALSEARCH_H
#define CLIONPROJECTS_LOCALSEARCH_H

#include "../global/Solution.h"
#include <set>


class LocalSearch {
private:
    int qty_facilities;
    int qty_clients;
    double biggest_cij;
    int qty_moves;
    double aux_cij;

    bool swap_done; // Variavel que indica se foi feito uma troca, se sim, devemos dar um break no for para voltar pro while
    double extra_cost; // Variavel que indica o custo extra de abrir ou fechar uma certa instalacao
    double complete_extra_cost; // Variavel utilizada para calcular o custo completo extra do movimento de troca
    bool closed_nearest; // Variavel auxiliar para ajudar quando houver substituicao de informacoes na troca

    set <int, greater <int> > open_facs; // conjunto de instalacoes que encontram-se abertas no momento
    set <int, greater <int> > closed_facs; // conjunto de instalacoes que encontram-se fechadas no momento

    set <int, greater <int> > :: iterator itr;  // Iteratores para os conjuntos
    set <int, greater <int> > :: iterator itr2;  // Iteratores para os conjuntos

    // RedNode
    double * c_minX;  // c_minX - será correnpondente a c(j,X) = min_{i \in X} cij, sendo X as inst abertas
    double * c2_minX;  // c2_minX - será correnpondente ao segundo menor c(j,X) = min_{i \in X} cij, sendo X as inst abertas exceto a nearest_open_fac
    int * nearest_open_fac; // nearest_open_fac - será correnpondente ao ID da instalacao i tal que min_{i \in X} cij, sendo X as inst abertas
    int * nearest2_open_fac;// nearest2_open_fac - será correnpondente ao ID da segunda instalacao i tal que min_{i \in X} cij, sendo X as inst abertas exceto a nearest_open_fac (segunda instalacao aberta mais proxima)
    int * temp_nearest_fac; // temp_nearest_fac - será correnpondente ao ID da instalacao i mais proxima que sera conectada (temporario)
    int * temp2_nearest_fac; // temp_nearest2_fac - será correnpondente ao ID da segunda instalacao i mais proxima que sera conectada (temporario)

public:
    virtual ~LocalSearch();

    void initialize(Solution * solution, int ls_type);

    void run(Solution * solution, int ls_type);

};


#endif //CLIONPROJECTS_LOCALSEARCH_H
