//
// Created by Renata Sarmet Smiderle Mendes on 8/1/20.
//

#include "Tree.h"

Tree::Tree(Instance * _instance) {
    instance = _instance;
    qty_nodes = QTY_NODES_TREE;
    qty_solutions_node = QTY_SOLUTIONS_NODE;

    nodes = new Solution*[qty_nodes];
    for(int i = 0; i < qty_nodes; i++) {
        nodes[i] = new Solution[qty_solutions_node];

        for(int j=0;j<qty_solutions_node;j++){
            nodes[i][j].initializeInstance(instance); // tambem ja inicia tudo o que faria no construtor padrao de Solution
        }
    }

    // Indicara a ordem pra preencher osc
    shuffled_facilities = new int[instance->getQtyFacilities()];

    // sorted_cij_ID_g - será o ID das instalacoes ordenadas pelo cij àquele cliente
    sorted_cijID = new int*[instance->getQtyClients()];

    for(int i = 0; i < instance->getQtyClients(); i++) {
        sorted_cijID[i] = new int[instance->getQtyFacilities()];
    }

    // colocar valores ainda nao ordenados no sorted_cij_ID_g
    for(int i=0;i<instance->getQtyClients();i++){
        for(int j=0;j<instance->getQtyFacilities();j++){
            sorted_cijID[i][j] = j;
        }

        // Ordenar sorted_cij_ID_g
        mergeSortID(instance->getCostA()[i], sorted_cijID[i], 0, instance->getQtyFacilities() - 1);
    }

    qty_shuf_made = -1; // quantos individuos ja foram feitos com esse vetor embaralhado // inicia indicando invalidez, para criar o vetor
    SLICE_SHUF = (OPEN_RANDOM_RATE * instance->getQtyFacilities())/100; // quantas solucoes abrir em cada individuo
    MAX_QTY_SHUF_MADE = instance->getQtyFacilities() / SLICE_SHUF; // maximo de individuos possiveis de se formar com 1 embaralhada

    // Alocando memoria para os auxiliares do mapping
    map = new int[instance->getQtyFacilities()];
    temp_open_facilities = new int[instance->getQtyFacilities()];

    aux_sol.initializeInstance(instance);

//    map_instance = new Instance(instance->getQtyFacilities(), instance->getQtyClients(), instance->getInputName(), instance->getSolutionName());

    // Quantidade de pais que existem (quantidade de sub arvores)
    QTY_SUBS = (QTY_NODES_TREE - 1) / QTY_CHILDREN;
}

Tree::~Tree() {

    for(int i = 0; i < qty_nodes; i++) {
        delete [] nodes[i];
    }
    delete [] nodes;

    delete [] shuffled_facilities;

    for(int i = 0; i < instance->getQtyClients(); i++) {
        delete [] sorted_cijID[i];
    }
    delete [] sorted_cijID;

    delete [] map;
    delete [] temp_open_facilities;

//    delete [] map_instance;

}

void Tree::mergeSortID(double *vector, int *vectorID, int startPosition, int endPosition) {
    int i, j, k, halfSize, *tempVectorID;
    double *tempVector;
    if(startPosition == endPosition) return;
    halfSize = (startPosition + endPosition ) / 2;

    mergeSortID(vector, vectorID, startPosition, halfSize);
    mergeSortID(vector, vectorID, halfSize + 1, endPosition);

    i = startPosition;
    j = halfSize + 1;
    k = 0;
    tempVector = (double *) malloc(sizeof(double) * (endPosition - startPosition + 1));
    tempVectorID = (int *) malloc(sizeof(int) * (endPosition - startPosition + 1));

    while(i < halfSize + 1 || j  < endPosition + 1) {
        if (i == halfSize + 1 ) {
            tempVector[k] = vector[vectorID[j]];
            tempVectorID[k] = vectorID[j];
            j++;
            k++;
        }
        else {
            if (j == endPosition + 1) {
                tempVector[k] = vector[vectorID[i]];
                tempVectorID[k] = vectorID[i];
                i++;
                k++;
            }
            else {
                if (vector[vectorID[i]] < vector[vectorID[j]]) {
                    tempVector[k] = vector[vectorID[i]];
                    tempVectorID[k] = vectorID[i];
                    i++;
                    k++;
                }
                else {
                    tempVector[k] = vector[vectorID[j]];
                    tempVectorID[k] = vectorID[j];
                    j++;
                    k++;
                }
            }
        }
    }
    for(i = startPosition; i <= endPosition; i++) {
        vectorID[i] = tempVectorID[i - startPosition];
    }
    free(tempVector);
    free(tempVectorID);
}

void Tree::callGreedy(int posNode, int posIndividual) {
    Greedy greedy;

    // Chamando a funcao que resolve o problema de fato
    greedy.initialize(&nodes[posNode][posIndividual]);

    nodes[posNode][posIndividual].setInstance(instance);
}

int Tree::getQtyNodes() const {
    return qty_nodes;
}

int Tree::getQtySolutionsNode() const {
    return qty_solutions_node;
}

Solution **Tree::getNodes() const {
    return nodes;
}

const Instance &Tree::getInstance() const {
    return *instance;
}

Solution *Tree::getNodeJ(int j) const {
    return nodes[j];
}

Solution * Tree::getNodeJPosI(int j, int i) const {
    return &nodes[j][i];
}

Solution *Tree::getPointerNodeJPosI(int j, int i) {
    return &nodes[j][i];
}

void Tree::copySolutions(int posNodeTarget, int posIndividualTarget, int posNodeModel, int posIndividualModel) {
    nodes[posNodeTarget][posIndividualTarget].copySolution(&nodes[posNodeModel][posIndividualModel]);
}

void Tree::copyNodeToPointer(Solution *target, int posNodeModel, int posIndividualModel) {
    target->copySolution(&nodes[posNodeModel][posIndividualModel]);
}


void Tree::initializePopulation() {

    /*
   Geração da população inicial
   */

    // Declaracao variaveis que indicam o tempo da funcao
    struct timespec start_part, finish_part;

    double timeSpent;

    // Iniciando os nós 0 e 1, pocket 0

    clock_gettime(CLOCK_REALTIME, &start_part);

    callGreedy(1, INDEX_POCKET); // solucao com greedy

    clock_gettime(CLOCK_REALTIME, &finish_part);

    // Calculando o tempo gasto até agora
    timeSpent =  (finish_part.tv_sec - start_part.tv_sec);
    timeSpent += (finish_part.tv_nsec - start_part.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior

    cout << "Time generate greedy: " << timeSpent << " seconds" << endl;
    cout << "Value: " << nodes[1][INDEX_POCKET].getFinalTotalCost() << endl;

    clock_gettime(CLOCK_REALTIME, &start_part);

    // colocando a solucao do greedy no LS_G
    copySolutions(0, INDEX_POCKET,1, INDEX_POCKET);
    callLocalSearch(0, INDEX_POCKET, 1); // solucao com local search completo com solucao inicial do greedy

//    callLateAcceptance(0, INDEX_POCKET);// solucao com late acceptance com solucao inicial do greedy
    clock_gettime(CLOCK_REALTIME, &finish_part);

    // Calculando o tempo gasto até agora
    timeSpent =  (finish_part.tv_sec - start_part.tv_sec);
    timeSpent += (finish_part.tv_nsec - start_part.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior

    cout << "Time LS greedy: " << timeSpent << " seconds" << endl;
    cout << "Value: " << nodes[0][INDEX_POCKET].getFinalTotalCost() << endl;

    if(DEBUG >= DISPLAY_MOVES){
        cout << "LS_G: ";
        cout << nodes[0][INDEX_POCKET].getQtyOpenFacilities() << " open facilities" << endl;
        cout << "GREEDY: ";
        cout << nodes[1][INDEX_POCKET].getQtyOpenFacilities() << " open facilities" << endl;
    }

    if(DEBUG >= DISPLAY_DETAILS){
        cout << "LS_G - node[0][0]: " << nodes[0][INDEX_POCKET].getFinalTotalCost() << endl;
        cout << "Greedy - node[1][0]: " << nodes[1][INDEX_POCKET].getFinalTotalCost() << endl << endl;
    }

    // Inicial todos os pockets de todos os nós, com solução aleatória
    // Obs: Se SHUFFLED_FACILITIES estiver marcado como True, haverá um vetor shuffled_facilities que vai ter a ordem das instalacoes para colocar nos individuos. Se nao, vai ser aleatorio mesmo

    // Preenchendo os currents dos nós 0 e 1
    for(int j=0;j<2;j++){ // para os nós 0 e 1

        clock_gettime(CLOCK_REALTIME, &start_part);

        if(SHUFFLED_FACILITIES){
            setInitialSolShuffled(j, INDEX_CURRENT);
        }
        else{
            setInitialSolRandom(j, INDEX_CURRENT); // preenche o current
        }

        mapAndCallG(j, INDEX_CURRENT);

        clock_gettime(CLOCK_REALTIME, &finish_part);

        // Calculando o tempo gasto até agora
        timeSpent =  (finish_part.tv_sec - start_part.tv_sec);
        timeSpent += (finish_part.tv_nsec - start_part.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior

        cout << "Time Random: " << timeSpent << " seconds" << endl;

        if(DEBUG >= DISPLAY_DETAILS){
            cout << "Random - node[" << j << "] << [" << INDEX_CURRENT << "]:" << nodes[j][INDEX_CURRENT].getFinalTotalCost() << endl;
        }

        if(LA_INITIAL_POP){
            clock_gettime(CLOCK_REALTIME, &start_part);

            // Roda LA completo para todas as solucoes geradas com random
             callLateAcceptance(j, INDEX_CURRENT);

            clock_gettime(CLOCK_REALTIME, &finish_part);

            // Calculando o tempo gasto até agora
            timeSpent =  (finish_part.tv_sec - start_part.tv_sec);
            timeSpent += (finish_part.tv_nsec - start_part.tv_nsec) / 1000000000.0; // Necessario para obter uma precisao maior

            cout << "Time LA random: " << timeSpent << " seconds" << endl;

            if(DEBUG >= DISPLAY_DETAILS){
                cout << "LS_R -> node[1][" << j << "]:" <<nodes[j][INDEX_CURRENT].getFinalTotalCost() << endl << endl;
            }

            if(DEBUG >= DISPLAY_MOVES){
                cout << "RANDOM after LA: ";
                cout << nodes[j][INDEX_CURRENT].getQtyOpenFacilities() << " open facilities" << endl;
            }
        }
    }

    // Preenchendo os pockets e o currents do restante dos nós (de 2 a 12)
    for(int i=2;i<QTY_NODES_TREE;i++){ // para todos os nós a partir de 2

        if(DEBUG >= DISPLAY_MOVES){
            cout << "Node " << i << endl;
        }

        for(int j=0;j<QTY_SOLUTIONS_NODE;j++){ // para o pocket e current

            if(SHUFFLED_FACILITIES){
                setInitialSolShuffled(i,j);
            }
            else{
                setInitialSolRandom(i,j);
            }

            mapAndCallG(i, j);

            if(DEBUG >= DISPLAY_DETAILS){
                cout << "Random - node[" << i << "][" << j << "]:" << nodes[i][j].getFinalTotalCost() << endl;
            }

            if(LA_INITIAL_POP){
                // Roda LA completo para todas as solucoes geradas com random
                callLateAcceptance(i,j);

                if(DEBUG >= DISPLAY_DETAILS){
                    cout << "LS_R -> node[" << i << "][" << j << "]:" <<nodes[i][j].getFinalTotalCost() << endl << endl;
                }

                if(DEBUG >= DISPLAY_MOVES){
                    cout << "RANDOM after LA: ";
                    cout << nodes[i][j].getQtyOpenFacilities() << " open facilities" << endl;
                }
            }
        }
    }
}


void Tree::setInitialSolRandom(int posNode, int posIndividual) {
    int randNum;
    int qtd_open = 0;

    // Verificando quais instalacoes estarão abertas: OPEN_RANDOM_RATE% de chance de cada uma
    for(int i=0; i<instance->getQtyFacilities(); i++){
        randNum = rand() % 100; // Generate a random number between 0 and 99

        if(randNum < OPEN_RANDOM_RATE){ // se ficou entre os OPEN_RANDOM_RATE%, abre essa instalacao
            nodes[posNode][posIndividual].setOpenFacilityJ(i, true);
            qtd_open += 1;
        }
        else{ //senao, deixa essa inst fechada
            nodes[posNode][posIndividual].setOpenFacilityJ(i, false);
        }
    }

    // Se nao abriu nenhuma, abre pelo menos uma qualquer aleatoria
    if(qtd_open == 0){
        randNum = rand() % instance->getQtyFacilities(); // Generate a random number between 0 and qty_facilities

        nodes[posNode][posIndividual].setOpenFacilityJ(randNum, true);
    }

    if(DEBUG >= DISPLAY_MOVES){
        cout << "RANDOM before connecting clients: ";
        cout << nodes[posNode][posIndividual].getQtyOpenFacilities() << " open facilities" << endl;
    }

    // Conectando cada cliente com a instalacao aberta mais proxima e fechando as instalacoes que nao foram conectadas a ninguem. Custo final também é atualizado
    nodes[posNode][posIndividual].connectAndUpdateFacilities(sorted_cijID);

    if(DEBUG >= DISPLAY_MOVES){
        cout << "RANDOM after connecting clients: ";
        cout << nodes[posNode][posIndividual].getQtyOpenFacilities()  << " open facilities" << endl;
    }
}

void Tree::setInitialSolShuffled(int posNode, int posIndividual) {

    // Primeiramente verifica se precisa embaralhar de novo
    checkShuffed(shuffled_facilities, instance->getQtyFacilities());

    // Abrindo SLICE_SHUF facilities, iniciando na posicao (qty_shuf_made * SLICE_SHUF)
    int start = qty_shuf_made * SLICE_SHUF;
    int end = start + SLICE_SHUF; // nao inclui o valor end (intervalo aberto)

    if(DEBUG >= DISPLAY_MOVES){
        cout << "START = " << start << "  END = " << end << endl;
    }

    // Todas as primeiras instalacoes estarao fechadas
    for(int i=0;i<start;i++){
        nodes[posNode][posIndividual].setOpenFacilityJ(shuffled_facilities[i], false);
    }
    // Esse intervalo de instalacoes estará aberto
    for(int i=start; i<end; i++){
        nodes[posNode][posIndividual].setOpenFacilityJ(shuffled_facilities[i], true);
    }
    // Todas as últimas instalacoes estarao fechadas
    for(int i=end;i<instance->getQtyFacilities();i++){
        nodes[posNode][posIndividual].setOpenFacilityJ(shuffled_facilities[i], false);
    }

    // Incrementa o contador de individuos feitos com essa embaralhada
    qty_shuf_made += 1;


    if(DEBUG >= DISPLAY_MOVES){
        cout << "SHUFFLED before connecting clients: ";
        cout << nodes[posNode][posIndividual].getQtyOpenFacilities()  << " open facilities" << endl;
    }

    // Conectando cada cliente com a instalacao aberta mais proxima e fechando as instalacoes que nao foram conectadas a ninguem. Custo final também é atualizado
    nodes[posNode][posIndividual].connectAndUpdateFacilities(sorted_cijID);

    if(DEBUG >= DISPLAY_MOVES){
        cout << "SHUFFLED after connecting clients: ";
        cout << nodes[posNode][posIndividual].getQtyOpenFacilities()  << " open facilities" << endl;
    }
}

// Verifica se ja usou tudo o que podia, se sim, embaralha de novo
void Tree::checkShuffed(int *vet, int vetSize) {
    // Invalidez, ainda nao foi criado o vetor
    if(qty_shuf_made < 0){
        createShuffledVector(vet, vetSize);
        qty_shuf_made = 0;
    }
        // se ja usou tudo o que podia, embaralha de novo
    else if(qty_shuf_made >= MAX_QTY_SHUF_MADE){
        shuffle(vet, vetSize);
        qty_shuf_made = 0;
    }
}

// Cria um vetor completo com os numeros de facilities ou clientes embaralhados (sem repeticao e sem faltar)
void Tree::createShuffledVector(int *vet, int vetSize) {
    // Popula o vetor em ordem
    for(int i=0;i<vetSize;i++){
        vet[i] = i;
    }

    if(DEBUG >= DISPLAY_MOVES){
        cout << "Creating and populating vector " << endl;

        if(DEBUG >= DISPLAY_ACTIONS){
            for(int j=0;j<vetSize;j++){
                cout << vet[j] << " ";
            }
            cout << endl;
        }
    }

    // embaralha
    shuffle(vet, vetSize);
}

// Função para embaralhar o vetor com indices das facilities
void Tree::shuffle(int *vet, int vetSize) {
    int r, temp;
    for(int i=0; i<vetSize; i++){
        r = rand() % vetSize;
        temp = vet[i];
        vet[i] = vet[r];
        vet[r] = temp;
    }

    if(DEBUG >= DISPLAY_MOVES){
        cout << "Shuffle vector " << endl;

        if(DEBUG >= DISPLAY_ACTIONS){
            for(int j=0;j<vetSize;j++){
                cout << vet[j] << " ";
            }
            cout << endl;
        }
    }
}



void Tree::mapAndCallG(int posNode, int posIndividual) {
    if(DEBUG >= DISPLAY_MOVES){
        cout << "Initial G_MAP solution = " << nodes[posNode][posIndividual].getFinalTotalCost() << endl;
        if(DEBUG >= DISPLAY_DETAILS){
            nodes[posNode][posIndividual].printOpenFacilities();
        }
    }

    int cont_facilities = mapping(posNode, posIndividual);


    if(DEBUG >= DISPLAY_MOVES){
        cout << "Calling G mapping with " << nodes[posNode][posIndividual].getQtyFacilities() << " open facilities" << endl;
    }

    callGreedy(posNode, posIndividual);

    unmapping(posNode, posIndividual, cont_facilities);

    if(DEBUG >= DISPLAY_MOVES){
        cout << "Final G_MAP solution = " << nodes[posNode][posIndividual].getFinalTotalCost() << endl;
        if(DEBUG >= DISPLAY_DETAILS){
            nodes[posNode][posIndividual].printOpenFacilities();
        }
    }
}

void Tree::mapAndCallTS(int posNode, int posIndividual) {

    if(DEBUG >= DISPLAY_MOVES){
        cout << "Initial TS_MAP solution = " << nodes[posNode][posIndividual].getFinalTotalCost() << endl;
        if(DEBUG >= DISPLAY_DETAILS){
            nodes[posNode][posIndividual].printOpenFacilities();
        }
    }

    int cont_facilities = mapping(posNode, posIndividual);

    if(DEBUG >= DISPLAY_MOVES){
        cout << "Calling TS mapping with " << nodes[posNode][posIndividual].getQtyFacilities()  << " open facilities" << endl;
    }

    callTabuSearch(posNode, posIndividual);

    unmapping(posNode, posIndividual, cont_facilities);

    if(DEBUG >= DISPLAY_MOVES){
        cout << "Final TS_MAP solution = " << nodes[posNode][posIndividual].getFinalTotalCost() << endl;
        if(DEBUG >= DISPLAY_DETAILS){
            nodes[posNode][posIndividual].printOpenFacilities();
        }
    }
}

// Mapeando para chamar o TS ou o G com menos instalacoes que o total
// map[i] indica o valor que realmente corresponde
// retorna cont
int Tree::mapping(int posNode, int posIndividual) {
    int cont = 0;
    bool update_assigned = false;
    // Mapear costF e open_facilities, salvando o map
    for(int i=0; i< instance->getQtyFacilities(); i++){
        // se a instalacao está aberta, ela vai entrar nesse conjunto para chamar
        if(nodes[posNode][posIndividual].getOpenFacilityJ(i)){
            map[cont] = i; // indica que na verdade essa inst será correspondente à i
            nodes[posNode][posIndividual].getInstance().setCostFJ(cont, instance->getCostFJ(i));
//            cout << "to colocando em " << cont << ": " << nodes[posNode][posIndividual].getInstance().getCostFJ(cont) << endl;
            nodes[posNode][posIndividual].setOpenFacilityJ(cont, true);
            cont += 1;
        }
    }

    // marcar as que nao foram mapeadas, como fechadas
    for(int i=cont;i<instance->getQtyFacilities();i++){
        // map[i] = -1;
        nodes[posNode][posIndividual].setOpenFacilityJ(i, false);
    }


    // Mapear costA e assigned_facilities
    for(int j=0;j < instance->getQtyClients(); j++){
        update_assigned = false;
        for(int i=0;i<cont;i++){
            nodes[posNode][posIndividual].getInstance().setCostAIJ(j,i, instance->getCostAIJ(j,map[i]));
            if(!update_assigned){
                if(nodes[posNode][posIndividual].getAssignedFacilityJ(j) == map[i]){
                    nodes[posNode][posIndividual].setAssignedFacilityJ(j, i);
                    update_assigned = true;
                }
            }
        }
    }

    if(DEBUG >= DISPLAY_DETAILS){
        for(int i=0; i< cont; i++){
            cout << "Map[" << i << "] = " << map[i] << endl;
        }
    }

    nodes[posNode][posIndividual].getInstance().setQtyFacilities(cont);

    return cont;
}

// Desmapeando, depois que retornou do TS ou G com menos instalacoes
// map[i] indica o valor que realmente corresponde
void Tree::unmapping(int posNode, int posIndividual, int cont_facilities) {
    // salva todos os valores em temp_open_facilities
    for(int i=0;i<cont_facilities;i++){
        temp_open_facilities[i] = nodes[posNode][posIndividual].getOpenFacilityJ(i);
    }
    // zera todas as instalacoes de solution->open_facilities
    for(int i=0; i<instance->getQtyFacilities(); i++){
        nodes[posNode][posIndividual].setOpenFacilityJ(i, false);
    }
    // Coloca de volta as instalacoes aberta em solution->open_facilities
    for(int i = 0; i < cont_facilities; i++){
        if(DEBUG >= DISPLAY_DETAILS){
            cout << "Unmap " << i << " = " << map[i] << endl;
        }
        nodes[posNode][posIndividual].setOpenFacilityJ(map[i], temp_open_facilities[i]);
    }
    // Atualizando as atribuicoes dos clientes
    for(int j=0; j<instance->getQtyClients(); j++){
        nodes[posNode][posIndividual].setAssignedFacilityJ(j, map[nodes[posNode][posIndividual].getAssignedFacilityJ(j)]);
    }

    // Voltando à instancia original
    nodes[posNode][posIndividual].getInstance().copyInstance(instance);
}

// Imprime um vetor com do tamanho de intalacoes. Cada posicao é um inteiro indicando em quantas solucoes essa instalacao estava aberta.
void Tree::printCountOpenFacilities() {

    int qty_fac_open_all = 0;
    int qty_solutions = QTY_SOLUTIONS_NODE * QTY_NODES_TREE;
    int * count_open_facilities = (int*) malloc((instance->getQtyFacilities()) * sizeof(int));

    if(!count_open_facilities){
        cout << "Memory Allocation Failed";
        exit(1);
    }

    // Zerando todas as posicoes
    for(int i=0; i<instance->getQtyFacilities(); i++){
        count_open_facilities[i] = 0;
    }

    // Contando para cada nó
    for(int i = 0; i < QTY_NODES_TREE; i++){
        for(int k = 0; k < QTY_SOLUTIONS_NODE; k++){ // percorre o pocket e o current
            for(int j=0;j<instance->getQtyFacilities();j++){
                count_open_facilities[j] += nodes[i][k].getOpenFacilityJ(j);
            }
        }
    }


    // Imprimindo o resultado
    cout << endl << "Number of sulutions each facility was open" << endl;
    for(int i=0; i<instance->getQtyFacilities(); i++){
        cout << count_open_facilities[i] << " ";
        if(count_open_facilities[i] == qty_solutions){
            qty_fac_open_all += 1;
        }
    }
    cout << endl << "Qty fac open in all (" << qty_solutions << ") solutions: " << qty_fac_open_all << endl;
    cout << endl;

    free(count_open_facilities);
}

void Tree::printTreeComplete() {
    int index_child;

    cout.precision(0);

    cout << endl << "TREE COMPLETE"<< endl;
    // Imprimindo a raiz
    cout << "									" << nodes[0][INDEX_POCKET].getFinalTotalCost() << "("<< nodes[0][INDEX_CURRENT].getFinalTotalCost()  << ")" << endl;

    // Imprimindo os nós intermediários (1,2,3)
    for(int i=0;i<QTY_CHILDREN;i++){
        index_child = i + 1; // encontra o indice correto do filho
        cout << "		" << nodes[index_child][INDEX_POCKET].getFinalTotalCost() << "("<< nodes[index_child][INDEX_CURRENT].getFinalTotalCost() << ")" << "				";
    }
    cout << endl;

    // Imprimindo as folhas
    for(int id_parent=1;id_parent<=QTY_CHILDREN;id_parent++){
        for(int i=0;i<QTY_CHILDREN;i++){
            index_child = id_parent * 3 + i + 1; // encontra o indice correto do filho
            cout << nodes[index_child][INDEX_POCKET].getFinalTotalCost() << "("<< nodes[index_child][INDEX_CURRENT].getFinalTotalCost() << ")" << "	";
        }
        cout << "	";
    }
    cout << endl << endl;

    cout.precision(5);
}

void Tree::updateSubPop(int idParent) {

    int index_child;
    int randNum;
    for(int i=0; i< QTY_CHILDREN; i++){ // para todos os filhos
        index_child = idParent * 3 + i + 1; // encontra o indice correto do filho
        if(nodes[index_child][INDEX_POCKET].getFinalTotalCost() < nodes[idParent][INDEX_POCKET].getFinalTotalCost()){ // Se o best do filho for menor que o best do pai (pocket com pocket)
            if(DEBUG >= DISPLAY_MOVES){
                cout << "Swapping child " << index_child << "(" << nodes[index_child][INDEX_POCKET].getFinalTotalCost() << ") with parent " << idParent << " (" << nodes[idParent][INDEX_POCKET].getFinalTotalCost() << ")" << endl;
            }

            // Inverte
            invertNodes(idParent, INDEX_POCKET, index_child, INDEX_POCKET);

            // Update indice de pocket/current do filho (pocket sempre deve ser melhor que o current)
            if(nodes[index_child][INDEX_CURRENT].getFinalTotalCost() < nodes[index_child][INDEX_POCKET].getFinalTotalCost()){ // se agora o current é melhor que o pocket, atualiza
                // Inverte
                invertNodes(index_child, INDEX_POCKET, index_child, INDEX_CURRENT);
            }

            // Se o que inverteu, o pai era a raiz (nodes[0][0]), então roda o tabu_search nele
            if(idParent == 0){

                // randNum = rand() % QTY_NODES_TREE; // Generate a random number between 0 and QTY_NODES_TREE
                randNum = 0;

                if(DEBUG >= DISPLAY_MOVES){
                    cout << "Calling TS for nodes[" << randNum << "][" << INDEX_POCKET << "] (" << nodes[randNum][INDEX_POCKET].getFinalTotalCost() <<")" << endl;
                }

                callTabuSearch(randNum, INDEX_POCKET);

                if(DEBUG >= DISPLAY_MOVES){
                    cout << "New value for nodes[" << randNum << "][" << INDEX_POCKET << "] = " << nodes[randNum][INDEX_POCKET].getFinalTotalCost() << endl;
                }

                if(DEBUG >= DISPLAY_ACTIONS){
                    cout << "Nodes[" << randNum << "][" << INDEX_POCKET << "] after tabu search: ";
                    nodes[randNum][INDEX_POCKET].printIndividual();
                }
            }
        }
    }
}

void Tree::invertNodes(int posNode1, int posIndividual1, int posNode2, int posIndividual2) {
    // Inverte
    aux_sol.copySolution(&nodes[posNode1][posIndividual1]);
    nodes[posNode1][posIndividual1].copySolution(&nodes[posNode2][posIndividual2]);
    nodes[posNode2][posIndividual2].copySolution(&aux_sol);
}

void Tree::updatePopulation() {

    // compara o pocket do filho com o pocket do pai (best com best)
    // Para cada pai, chama para atualizar essa sub populacao
    for(int i = QTY_SUBS-1; i>=0; i--){
        updateSubPop(i);
    }
}

void Tree::printTree() {
    for(int i=0;i<QTY_NODES_TREE;i++){
        for(int j=0; j<QTY_SOLUTIONS_NODE; j++){
            cout << "node[" << i << "][" << j << "]:" << nodes[i][j].getFinalTotalCost() << endl;
        }
        cout << endl;
    }
}


void Tree::printSolutions() {
    for(int i=0;i<QTY_NODES_TREE;i++){
        for(int j=0; j<QTY_SOLUTIONS_NODE; j++){
            cout << "node[" << i << "][" << j << "]:" << endl;
            nodes[i][j].showSolution();
        }
        cout << endl;
    }
}


// Recebe por referencia, entao as alteracoes sao salvas.
// Armazena a root em solution se ela for melhor que a que está lá e sobe a próxima para a root, atualizando o resto da populacao
void Tree::changeRoot(Solution *solution, int *qtyChangesRoot) {

    if(DEBUG >= DISPLAY_MOVES){
        cout << "----------------------------------- Changing root " << endl;
    }

    // Salva a raiz, caso ela seja melhor do que a melhor salva até agora
    if(nodes[0][INDEX_POCKET].getFinalTotalCost() < solution->getFinalTotalCost()){
        solution->copySolution(&nodes[0][INDEX_POCKET]); // atualiza a melhor encontrada até agora

        *qtyChangesRoot = 0; // zera o contador

        if(DEBUG >= DISPLAY_MOVES){
            cout << "Updating best solution found so far: " << solution->getFinalTotalCost() << endl;
        }
    }
    else{
        *qtyChangesRoot += 1; // aumenta o contador
    }

    // Escolhe qual filho vai substituir a raiz
    int child_root = 1; // Inicia com o primeiro filho

    for(int i=2;i<QTY_CHILDREN+1;i++){ // verifica se algum outro filho é melhor
        if(nodes[i][INDEX_POCKET].getFinalTotalCost() < nodes[child_root][INDEX_POCKET].getFinalTotalCost()){ // se for melhor, substitui
            child_root = i;
        }
    }

    // Substitui a raiz com o melhor filho escolhido
    nodes[0][INDEX_POCKET].copySolution(&nodes[child_root][INDEX_POCKET]);

    // Verifica se o pocket da raiz continua melhor que o current, senao inverte
    if(nodes[0][INDEX_POCKET].getFinalTotalCost() > nodes[0][INDEX_CURRENT].getFinalTotalCost()){ // se o pocket for pior que o current, inverte
        invertNodes(0, INDEX_POCKET, 0, INDEX_CURRENT);
    }

    // Atualiza o espaço que vagou no melhor filho escolhido com uma solucao vinda do random + LA
    setInitialSolRandom(child_root, INDEX_POCKET);

    if(DEBUG >= DISPLAY_DETAILS){
        cout << "Random child " << child_root << ":" << nodes[child_root][INDEX_POCKET].getFinalTotalCost() << endl;
    }

    // Roda LA ou LS completo para a solucao gerada com random
//    callLocalSearch(child_root, INDEX_POCKET, 1);
    callLateAcceptance(child_root, INDEX_POCKET);

    if(DEBUG >= DISPLAY_DETAILS){
        cout << "LS_R:" << nodes[child_root][INDEX_POCKET].getFinalTotalCost() << endl << endl;
    }

    // Update população, levando esse novo gerado para o seu devido lugar
    updatePopChangeRoot(child_root);
}

void Tree::updatePopChangeRoot(int id_parent) {

    int index_child;

    // Verifico se ele é melhor do que a raiz atual
    if(nodes[id_parent][INDEX_POCKET].getFinalTotalCost() < nodes[0][INDEX_POCKET].getFinalTotalCost()){ // se for, substitui
        if(DEBUG >= DISPLAY_MOVES){
            cout << "Swapping (" << nodes[id_parent][INDEX_POCKET].getFinalTotalCost() << ") with the root (" << nodes[0][INDEX_POCKET].getFinalTotalCost() << ")" << endl;
        }

        // Inverte
        invertNodes(id_parent, INDEX_POCKET, 0, INDEX_POCKET);
    }
    else{ // se aconteceu o caso anterior, eu sei que o resto já está ordenado, pois só voltou pro mesmo lugar (ou veio um melhor ainda). Senao, entao tenho que ordenar essa subarvore

        // Atualiza o POCKET e CURRENT do pai primeiro. só esse novo que entrou pode ser pior do que o que ja era
        if(nodes[id_parent][INDEX_POCKET].getFinalTotalCost() > nodes[id_parent][INDEX_CURRENT].getFinalTotalCost()){ // se esse que chegou é pior que o current, atualiza
            // Inverte
            invertNodes(id_parent, INDEX_POCKET, id_parent, INDEX_CURRENT);
        }

        updateSubPop(id_parent);
    }
}

int Tree::getQtySubs() const {
    return QTY_SUBS;
}

void Tree::updateRefset() {

    double aux_comparison;
    bool ok = false;
    // Verificar se os currents entrarao no refSet ou nao
    // verifica se o current é melhor que pocket e se nao tem nada igual


    /*
    Para a RAIZ
    */
    while(!ok){
        ok = true;
        aux_comparison = comparePocketCurrent(0);

        // Se o current for menor que o pocket, atualiza invertendo e pronto
        if(aux_comparison < 0){
            swapPocketCurrent(0);
        }
            // Senao, se for diferentes, o current é menor que o pocket entao está ok
            // Senao se forem iguais, se só o custo for igual, ta ok mantem como está
            // Senão se realmente forem iguais
        else if(!areDifferent(0, INDEX_CURRENT, 0, INDEX_POCKET)){
            // Gera uma solução aleatória e repete o processo
            setInitialSolRandom(0, INDEX_CURRENT);
            mapAndCallG(0, INDEX_CURRENT);

            if(DEBUG >= DISPLAY_DETAILS){
                cout << "Random - node[" << 0 << "][" << INDEX_CURRENT << "]:" << nodes[0][INDEX_CURRENT].getFinalTotalCost() << endl;
            }
            ok = false;
        }
    }


    /*
    para todos pais intermediarios (1,2,3)
    */

    for(int i = 1; i < QTY_CHILDREN+1; i++){
        ok = false;

        while(!ok){
            ok = true;

            // Compara com seu próprio pocket
            aux_comparison = comparePocketCurrent(i);

            // Se o custo for igual
            if(aux_comparison == 0){
                // Se só o custo for igual mas a solução for diferente, ta ok, pode deixar assim mesmo.. verificar com os outros pais intermediarios
                // Se realmente forem iguais
                if(!areDifferent(i, INDEX_CURRENT, i, INDEX_POCKET)){
                    // Gera uma solução aleatória e repete o processo
                    setInitialSolRandom(i, INDEX_CURRENT);
                    mapAndCallG(i, INDEX_CURRENT);

                    if(DEBUG >= DISPLAY_DETAILS){
                        cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].getFinalTotalCost() << endl;
                    }

                    ok = false;
                }
            }

            // Se ja nao achou um problema, continuar procurando
            if(ok){
                // Compara com os outros pais intermediarios anteriores
                for(int j=i-1; j>0;j--){

                    // Compara com o pocket e current
                    // se for menor que o pocket, ta ok
                    // se for maior que o pocket, compara com o current
                    if(nodes[i][INDEX_CURRENT].getFinalTotalCost() > nodes[j][INDEX_POCKET].getFinalTotalCost()){
                        // se for igual ao current, gera aleatoria
                        if(!areDifferent(i, INDEX_CURRENT, j, INDEX_CURRENT)){
                            // Gera uma solução aleatória e repete o processo
                            setInitialSolRandom(i, INDEX_CURRENT);
                            mapAndCallG(i, INDEX_CURRENT);

                            if(DEBUG >= DISPLAY_DETAILS){
                                cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].getFinalTotalCost() << endl;
                            }

                            ok = false;
                            break;
                        }
                    }
                        // se for igual o pocket, ver se é igual mesmo, e gerar aletaroia
                    else if(!areDifferent(i, INDEX_CURRENT, j, INDEX_POCKET)){
                        // Gera uma solução aleatória e repete o processo
                        setInitialSolRandom(i, INDEX_CURRENT);
                        mapAndCallG(i, INDEX_CURRENT);

                        if(DEBUG >= DISPLAY_DETAILS){
                            cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].getFinalTotalCost() << endl;
                        }

                        ok = false;
                        break;
                    }
                }
            }

            // Se ja nao achou um problema, continuar procurando
            if(ok){
                if(aux_comparison < 0){
                    // Verifica se é melhor que o pocket da raiz
                    if(nodes[i][INDEX_CURRENT].getFinalTotalCost() < nodes[0][INDEX_POCKET].getFinalTotalCost()){

                        // Se sim, atualiza pocket <-> current
                        swapPocketCurrent(i);
                    }
                        // se for maior
                    else if(nodes[i][INDEX_CURRENT].getFinalTotalCost() > nodes[0][INDEX_POCKET].getFinalTotalCost()){
                        // se for igual ao current, gera aleatoria
                        if(!areDifferent(i, INDEX_CURRENT, 0, INDEX_CURRENT)){
                            // Gera uma solução aleatória e repete o processo
                            setInitialSolRandom(i, INDEX_CURRENT);
                            mapAndCallG(i, INDEX_CURRENT);

                            if(DEBUG >= DISPLAY_DETAILS){
                                cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].getFinalTotalCost() << endl;
                            }

                            ok = false;
                        }
                            // se for diferente, atualiza
                        else {
                            // atualiza pocket <-> current
                            swapPocketCurrent(i);
                        }
                    }
                        // se for igual mesmo ao pocket da raiz
                    else if(!areDifferent(i, INDEX_CURRENT, 0, INDEX_POCKET)){
                        // Gera uma solução aleatória e repete o processo
                        setInitialSolRandom(i, INDEX_CURRENT);
                        mapAndCallG(i, INDEX_CURRENT);

                        if(DEBUG >= DISPLAY_DETAILS){
                            cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].getFinalTotalCost() << endl;
                        }

                        ok = false;
                    }
                }
                    // se for pior ou igual, só precisa confirmar que ele não é igual ao current da raiz
                else {
                    // se for igual ao current, gera aleatoria
                    if(!areDifferent(i, INDEX_CURRENT, 0, INDEX_CURRENT)){
                        // Gera uma solução aleatória e repete o processo
                        setInitialSolRandom(i, INDEX_CURRENT);
                        mapAndCallG(i, INDEX_CURRENT);

                        if(DEBUG >= DISPLAY_DETAILS){
                            cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].getFinalTotalCost() << endl;
                        }

                        ok = false;
                    }
                }
            }
        }
    }

    /*
    para todos os filhos folhas (4,5,6,7,8,9,10,11,12)
    */

    for(int i = QTY_CHILDREN + 1; i < QTY_NODES_TREE; i++){
        ok = false;

        while(!ok){
            ok = true;

            // Compara com seu próprio pocket
            aux_comparison = comparePocketCurrent(i);

            // Se o custo for igual
            if(aux_comparison == 0){
                // Se só o custo for igual mas a solução for diferente, ta ok, pode deixar assim mesmo.. verificar com os outros filhos folhas anteriores
                // Se realmente forem iguais
                if(!areDifferent(i, INDEX_CURRENT, i, INDEX_POCKET)){
                    // Gera uma solução aleatória e repete o processo
                    setInitialSolRandom(i, INDEX_CURRENT);
                    mapAndCallG(i, INDEX_CURRENT);

                    if(DEBUG >= DISPLAY_DETAILS){
                        cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].getFinalTotalCost() << endl;
                    }

                    ok = false;
                }
            }

            // Se ja nao achou um problema, continuar procurando
            if(ok){

                // Compara com os outros filhos folha anteriores
                for(int j=i-1; j>QTY_CHILDREN;j--){

                    // Compara com o pocket e current
                    // se for menor que o pocket, ta ok
                    // se for maior que o pocket, compara com o current
                    if(nodes[i][INDEX_CURRENT].getFinalTotalCost() > nodes[j][INDEX_POCKET].getFinalTotalCost()){
                        // se for igual ao current, gera aleatoria
                        if(!areDifferent(i, INDEX_CURRENT, j, INDEX_CURRENT)){
                            // Gera uma solução aleatória e repete o processo
                            setInitialSolRandom(i, INDEX_CURRENT);
                            mapAndCallG(i, INDEX_CURRENT);

                            if(DEBUG >= DISPLAY_DETAILS){
                                cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].getFinalTotalCost() << endl;
                            }

                            ok = false;
                            break;
                        }
                    }
                        // se for igual o pocket, ver se é igual mesmo, e gerar aletaroia
                    else if(!areDifferent(i, INDEX_CURRENT, j, INDEX_POCKET)){
                        // Gera uma solução aleatória e repete o processo
                        setInitialSolRandom(i, INDEX_CURRENT);
                        mapAndCallG(i, INDEX_CURRENT);

                        if(DEBUG >= DISPLAY_DETAILS){
                            cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].getFinalTotalCost() << endl;
                        }

                        ok = false;
                        break;
                    }
                }
            }

            // Se ja nao achou um problema, continuar procurando
            if(ok){
                // Verificar igualdade com pocket e current do pai e dos tios
                for(int j=1; j<QTY_CHILDREN+1; j++){
                    // Compara com o pocket desse nó
                    // se for menor, ta ok, depois já vou ver o da raiz mesmo
                    // se for maior
                    if(nodes[i][INDEX_CURRENT].getFinalTotalCost() > nodes[j][INDEX_POCKET].getFinalTotalCost()){
                        // compara com o current desse nó
                        if(!areDifferent(i, INDEX_CURRENT, j, INDEX_CURRENT)){
                            // Gera uma solução aleatória e repete o processo
                            setInitialSolRandom(i, INDEX_CURRENT);
                            mapAndCallG(i, INDEX_CURRENT);

                            if(DEBUG >= DISPLAY_DETAILS){
                                cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].getFinalTotalCost() << endl;
                            }

                            ok = false;
                            break;
                        }
                    }
                    // se nao, compara se sao iguais
                    if(!areDifferent(i, INDEX_CURRENT, j, INDEX_POCKET)){
                        // Gera uma solução aleatória e repete o processo
                        setInitialSolRandom(i, INDEX_CURRENT);
                        mapAndCallG(i, INDEX_CURRENT);

                        if(DEBUG >= DISPLAY_DETAILS){
                            cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].getFinalTotalCost() << endl;
                        }

                        ok = false;
                        break;
                    }
                }
            }

            // Se ja nao achou um problema, continuar procurando
            if(ok){
                // se ele for melhor que o pocket atual
                if(aux_comparison < 0){
                    // Verifica se é melhor que o pocket da raiz
                    if(nodes[i][INDEX_CURRENT].getFinalTotalCost() < nodes[0][INDEX_POCKET].getFinalTotalCost()){
                        // Se sim, atualiza pocket <-> current
                        swapPocketCurrent(i);
                    }
                        // se for maior
                    else if(nodes[i][INDEX_CURRENT].getFinalTotalCost() > nodes[0][INDEX_POCKET].getFinalTotalCost()){
                        // se for igual ao current, gera aleatoria
                        if(!areDifferent(i, INDEX_CURRENT, 0, INDEX_CURRENT)){
                            // Gera uma solução aleatória e repete o processo
                            setInitialSolRandom(i, INDEX_CURRENT);
                            mapAndCallG(i, INDEX_CURRENT);

                            if(DEBUG >= DISPLAY_DETAILS){
                                cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].getFinalTotalCost() << endl;
                            }

                            ok = false;
                        }
                            // se forem diferentes, atualiza
                        else{
                            // atualiza pocket <-> current
                            swapPocketCurrent(i);
                        }
                    }
                        // se for igual mesmo ao pocket da raiz
                    else if(!areDifferent(i, INDEX_CURRENT, 0, INDEX_POCKET)){
                        // Gera uma solução aleatória e repete o processo
                        setInitialSolRandom(i, INDEX_CURRENT);
                        mapAndCallG(i, INDEX_CURRENT);

                        if(DEBUG >= DISPLAY_DETAILS){
                            cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].getFinalTotalCost() << endl;
                        }

                        ok = false;
                    }
                }
                    // se for pior ou igual, só precisa confirmar que ele não é igual ao current da raiz
                else {
                    // se for igual ao current, gera aleatoria
                    if(!areDifferent(i, INDEX_CURRENT, 0, INDEX_CURRENT)){
                        // Gera uma solução aleatória e repete o processo
                        setInitialSolRandom(i, INDEX_CURRENT);
                        mapAndCallG(i, INDEX_CURRENT);
                        
                        if(DEBUG >= DISPLAY_DETAILS){
                            cout << "Random - node[" << i << "][" << INDEX_CURRENT << "]:" << nodes[i][INDEX_CURRENT].getFinalTotalCost() << endl;
                        }

                        ok = false;
                    }
                }
            }
        }
    }
}


// se o current for menor que o pocket, retorna negativo
// se for igual, fica 0
// se o current for maior, retorna positivo
double Tree::comparePocketCurrent(int posNode) {
    return nodes[posNode][INDEX_CURRENT].getFinalTotalCost() - nodes[posNode][INDEX_POCKET].getFinalTotalCost();
}


// Indica se dois individuos são diferentes
bool Tree::areDifferent(int posNode1, int posIndividual1, int posNode2, int posIndividual2) {
    if(nodes[posNode1][posIndividual1].getFinalTotalCost() != nodes[posNode2][posIndividual2].getFinalTotalCost()){
        return true;
    }
    else if(qtyDiversity(nodes[posNode1][posIndividual1].getOpenFacilities(),
                         nodes[posNode2][posIndividual2].getOpenFacilities()) > 0){
        if(DEBUG >= DISPLAY_MOVES){
            cout << "***** It has the same cost (" << nodes[posNode1][posIndividual1].getFinalTotalCost() << "), but the config is different *****" << endl;
        }

        return true;
    }
    if(DEBUG >= DISPLAY_MOVES){
        cout << "***** IT IS EQUAL! It has the same cost (" << nodes[posNode1][posIndividual1].getFinalTotalCost() << ") and config! *****" << endl;
    }
    return false;
}

// Indica quantas instalacoes possuem valores diferentes, dado dois individuos
int Tree::qtyDiversity(bool *n1OpenFacilities, bool *n2OpenFacilities) {
    int qty = 0;
    for(int i=0; i< instance->getQtyFacilities(); i++){
        if(n1OpenFacilities[i] != n2OpenFacilities[i]){
            qty += 1;

            if(DEBUG >= DISPLAY_ACTIONS){
                cout << "** Dif fac: " << i << "**" << endl;
            }
        }
    }

    return qty;
}

void Tree::swapPocketCurrent(int posNode) {
    // Inverte
    invertNodes(posNode, INDEX_POCKET, posNode, INDEX_CURRENT);
}

int **Tree::getSortedCijId() const {
    return sorted_cijID;
}

void Tree::printTreeBest() {

    int index_child;

    cout << endl << "TREE BEST POCKET"<< endl;
    // Imprimindo a raiz
    cout << "									" << nodes[0][INDEX_POCKET].getFinalTotalCost() << endl;

    // Imprimindo os nós intermediários (1,2,3)
    for(int i=0;i<QTY_CHILDREN;i++){
        index_child = i + 1; // encontra o indice correto do filho
        cout << "		" << nodes[index_child][INDEX_POCKET].getFinalTotalCost() << "				";
    }
    cout << endl;

    // Imprimindo as folhas
    for(int id_parent=1;id_parent<=QTY_CHILDREN;id_parent++){
        for(int i=0;i<QTY_CHILDREN;i++){
            index_child = id_parent * 3 + i + 1; // encontra o indice correto do filho
            cout << nodes[index_child][INDEX_POCKET].getFinalTotalCost() << "	";
        }
        cout << "	";
    }
    cout << endl << endl;
}

void Tree::callLocalSearch(int posNode, int posIndividual, int lsType) {
    LocalSearch localSearch;

    localSearch.initialize(&nodes[posNode][posIndividual], lsType); // chamando local search do tipo lsType
}



void Tree::callLateAcceptance(int posNode, int posIndividual) {
    LateAcceptance lateAcceptance;

    lateAcceptance.initialize(&nodes[posNode][posIndividual], true, 2.5, 0.02, 10); // best_fit = true, a1 = 2.5, limit_idle = 0.02, lh = 10

}

void Tree::callTabuSearch(int posNode, int posIndividual) {
    int lc1 = 0.01 * nodes[posNode][posIndividual].getInstance().getQtyFacilities();
    int lc2 = 0.05 * nodes[posNode][posIndividual].getInstance().getQtyFacilities();

    TabuSearch tabuSearch;

    // Chamando a funcao que resolve o problema de fato
    tabuSearch.initialize(&nodes[posNode][posIndividual], true, 0.5, lc1, lc2, lc1, lc2, 0);  // best_fit = true, a1 = 0.5, (lc1 = lo1 = 0.01 * qty_facilities), (lc2 = lo2 = 0.05 * qty_facilities), seed = 0

}


void Tree::callLocalSearchCloseFac(int posNode, int posIndividual) {
    LSCloseFac lsCloseFac;

    lsCloseFac.initialize(&nodes[posNode][posIndividual]);
}

