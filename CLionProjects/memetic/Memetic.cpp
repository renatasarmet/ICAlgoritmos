#include "Memetic.h"

#define DISPLAY_BASIC 1 // corresponde a exibicao da quantidade de movimentos
#define DISPLAY_MOVES 2 // corresponde a todos os cout quando um movimento é realizado de fato
#define DISPLAY_ACTIONS 3 // corresponde a todos os cout quando uma acao é feita.
#define DISPLAY_DETAILS 4 // corresponde a todos os cout mais detalhados quando uma acao é feita.
#define DISPLAY_TIME 5 // corresponde aos calculos de tempo

#define DEBUG 0 // OPCOES DE DEBUG: 1 - MOSTRAR A QTD DE MOVIMENTOS, 2 PARA EXIBIR OS MOVIMENTOS REALIZADOS, 3 PARA EXIBIR ACOES, 4 PARA EXIBIR DETALHES DAS ACOES, 5 PARA EXIBIR TEMPO, 6 PARA EXIBIR AS MUDANÇAS NO GRAFO


// Alocando memoria e inicializando valores
void Memetic::initialize(Solution *solution) {

    // Semente dos numeros aleatorios
    srand(0);

    qty_facilities = solution->getQtyFacilities();
    qty_clients = solution->getQtyClients();

    instance.copyInstance(&solution->getInstance());

    // cli_cli - matriz cliente x cliente, indica se estao conectados à mesma instalacao nos dois pais
    cli_cli = new int*[qty_clients];
    for(int i = 0; i < qty_clients; i++) {
        cli_cli[i] = new int[qty_clients];
    }

    // Inicializar a tree, chamando o construtor padrao
    tree = new Tree(&instance);

    // Quantidade de instalacoes que sofrerão mutação quando um filho for gerado
    QTY_INST_MUTATION = qty_facilities * MUTATION_RATE;

    // indica a quantidade de gerações já feitas
    qty_generations = 0;

    // indica a quantidade de gerações sem melhora
    qty_gen_no_improv = 0;

    // indica a quantidade vezes que mudou o root seguida
    qty_changes_root = 0;

    run(solution);
}



// Retornar o valor da solucao por referencia
void Memetic::run(Solution *solution) {

    cout << fixed;
    cout.precision(5);

    // Declaracao variaveis que indicam o tempo da funcao
    struct timespec start, finish, start_part, finish_part;

    double timeSpent;

    // INICIANDO A CONTAGEM DE TEMPO DA FUNCAO
    clock_gettime(CLOCK_REALTIME, &start);

    // FUTURO: SALVAR LOG E LOG_DETAILS


    if(DEBUG >= DISPLAY_MOVES){
        cout << "Initializing population" << endl;
    }

    // Inicializando a populacao
    tree->initializePopulation();

    if(DEBUG >= DISPLAY_MOVES){
        // Imprimindo a quantidade de vezes que cada inst estava aberta nessas solucoes iniciais
        tree->printCountOpenFacilities();
    }


    if(DEBUG >= DISPLAY_MOVES){
        cout << endl << "Initial tree:";
        tree->printTreeComplete();
    }

    // A partir daqui entrará no loop de restart

    // Levando as melhores solucoes para cima -> Update Population
    tree->updatePopulation();

    if(DEBUG >= DISPLAY_MOVES){
        cout << endl << "Tree after updating population:";
        tree->printTreeComplete();

        if(DEBUG >= DISPLAY_DETAILS){
            tree->printTree();
        }
    }

    // Iniciando o melhor custo encontrado ate agora
    // solution = nodes[0][INDEX_POCKET];
    tree->copyNodeToPointer(solution, 0, INDEX_POCKET);

    // Atualiza o best para o valor desse root
    temp_best_total_cost = tree->getNodeJPosI(0, INDEX_POCKET)->getFinalTotalCost();

    // A partir daqui estará em um loop até um número de iterações sem melhora for atingido
    while(qty_changes_root < MAX_CHANGES_ROOT){

        if(DEBUG >= DISPLAY_MOVES){
            cout << endl << "------------------------------ NEXT GENERATION " << qty_generations << " ------------------------------" << endl << endl;
        }


        // Se atingiu MAX_GEN_NO_IMPROVEMENT geracoes sem melhora, armazena a root em solution se ela for melhor que a que está lá e sobe a próxima para a root
        if(qty_gen_no_improv > MAX_GEN_NO_IMPROVEMENT){
            qty_gen_no_improv = 0; // zera o contador
            tree->changeRoot(solution, &qty_changes_root);


            // Atualiza o best para o valor desse root
            temp_best_total_cost = tree->getNodeJPosI(0, INDEX_POCKET)->getFinalTotalCost();
        }

        // crossover
        // Combinacoes entre cada mãe e filhos,
        // seguido da mutacao de cada child gerado e reatribuicao de todos os clientes e por fim local search close facilities em todos os filhos gerados
        for(int id_parent = 0; id_parent < tree->getQtySubs(); id_parent++){ // para cada mãe
            if(DEBUG >= DISPLAY_ACTIONS){
                cout << "------ PARENT " << id_parent << "-------" << endl;
            }

            // encontra o indice correto dos filhos
            s1 = id_parent * 3 + 1;
            s2 = id_parent * 3 + 2;
            s3 = id_parent * 3 + 3;


            // current[id_parent] = Recombine(pocket[id_parent], current[s1])

            if(DEBUG >= DISPLAY_ACTIONS){
                cout << "current[" << id_parent << "] = Recombine(pocket[" << id_parent << "], current[" << s1 << "]) " << endl;
            }

            recombine(tree->getPointerNodeJPosI(id_parent, INDEX_CURRENT), tree->getPointerNodeJPosI(id_parent, INDEX_POCKET), tree->getPointerNodeJPosI(s1, INDEX_CURRENT), id_parent, INDEX_CURRENT, id_parent);

            // current[s3] = Recombine(pocket[s3], current[id_parent])

            if(DEBUG >= DISPLAY_ACTIONS){
                cout << "current[" << s3 << "] = Recombine(pocket[" << s3 << "], current[" << id_parent << "]) " << endl;
            }

            recombine(tree->getPointerNodeJPosI(s3, INDEX_CURRENT), tree->getPointerNodeJPosI(s3, INDEX_POCKET), tree->getPointerNodeJPosI(id_parent, INDEX_CURRENT), s3, INDEX_CURRENT, id_parent);


            // current[s1] = Recombine(pocket[s1], current[s2])

            if(DEBUG >= DISPLAY_ACTIONS){
                cout << "current[" << s1 << "] = Recombine(pocket[" << s1 << "], current[" << s2 << "]) " << endl;
            }


            recombine(tree->getPointerNodeJPosI(s1, INDEX_CURRENT), tree->getPointerNodeJPosI(s1, INDEX_POCKET), tree->getPointerNodeJPosI(s2, INDEX_CURRENT), s1, INDEX_CURRENT, id_parent);


            // current[s2] = Recombine(pocket[s2], current[s3])

            if(DEBUG >= DISPLAY_ACTIONS){
                cout << "current[" << s2 << "] = Recombine(pocket[" << s2 << "], current[" << s3 << "]) " << endl;
            }

            recombine(tree->getPointerNodeJPosI(s2, INDEX_CURRENT), tree->getPointerNodeJPosI(s2, INDEX_POCKET), tree->getPointerNodeJPosI(s3, INDEX_CURRENT), s2, INDEX_CURRENT, id_parent);

        }


        if(DEBUG >= DISPLAY_MOVES){
            cout << endl << "TREE AFTER CROSSOVER, MUTATION AND LS_N0" << endl;
            tree->printTree();

            if(DEBUG >= DISPLAY_DETAILS){
                tree->printTreeComplete();
            }
        }

        // Verificar se os currents entrarao no refSet ou nao, checando diversidade
        tree->updateRefset();


        if(DEBUG >= DISPLAY_MOVES){
            cout << endl << "AFTER UPDATING REFSET";
            tree->printTreeComplete();

            if(DEBUG >= DISPLAY_DETAILS){
                tree->printTree();
            }
        }

        // Levando as melhores solucoes para cima -> Update Population
        tree->updatePopulation();

        if(DEBUG >= DISPLAY_MOVES){
            cout << endl << "AFTER UPDATING POPULATION";
            tree->printTreeComplete();

            // if(DEBUG >= DISPLAY_DETAILS){
            tree->printTree();
            // }
        }

        // Verificando se houve melhora
        if(tree->getNodeJPosI(0, INDEX_POCKET)->getFinalTotalCost() < temp_best_total_cost){ // se houve melhora, atualiza o temp_best_total_cost e zera o qty_gen_no_improv
            temp_best_total_cost = tree->getNodeJPosI(0, INDEX_POCKET)->getFinalTotalCost();
            qty_gen_no_improv = 0;

            if(DEBUG >= DISPLAY_MOVES){
                cout << "Updating the best cost in this tree (" << temp_best_total_cost << ")!!" << endl;
            }
            if(tree->getNodeJPosI(0, INDEX_POCKET)->getFinalTotalCost() < solution->getFinalTotalCost()){ // se houve melhora na melhor solução encontrada até agora
                qty_changes_root = 0; // zera a quantidade de mudanças para garantir que vai continuar a busca, pois sabe que logo vai alterar a raiz e zerar de qualquer forma

                if(DEBUG >= DISPLAY_MOVES){
                    cout << "Also it is the best cost found so far!!" << endl;
                }
            }
        }
        else{ // se nao houve melhora, aumenta o contador qty_gen_no_improv
            qty_gen_no_improv += 1;
        }

        // Aumentando o contador de geracoes
        qty_generations += 1;
    }

    // Vendo a diversidade da populacao final

    if(DEBUG >= DISPLAY_MOVES){
        for(int j=0;j<tree->getQtyNodes();j++){
            for(int i=0;i<tree->getQtySolutionsNode();i++){
                cout << tree->getNodeJPosI(j,i)->getQtyOpenFacilities() << " ";
            }
            cout << " open facilities" << endl;
        }

        // Imprimindo a quantidade de vezes que cada inst estava aberta nessas solucoes
        tree->printCountOpenFacilities();
    }


    // FINALIZANDO A CONTAGEM DE TEMPO DA FUNCAO
    clock_gettime(CLOCK_REALTIME, &finish);


    // Atualizando valor de solution
    if(tree->getNodeJPosI(0, INDEX_POCKET)->getFinalTotalCost() < solution->getFinalTotalCost()){
        tree->copyNodeToPointer(solution,0,INDEX_POCKET);
    }

    if(DEBUG >= DISPLAY_MOVES){
        solution->printOpenFacilities();
    }

    // Calculando o tempo gasto total
    solution->setTimeSpent((finish.tv_sec - start.tv_sec));
    solution->setTimeSpent(solution->getTimeSpent() + ((finish.tv_nsec - start.tv_nsec) / 1000000000.0)); // Necessario para obter uma precisao maior

    if(DEBUG >= DISPLAY_TIME){
        cout << "Final Total Function Time: " << solution->getTimeSpent() << " seconds" << endl;
    }

}

void Memetic::recombine(Solution *child, Solution *mother, Solution *father, int posNodeChild, int posIndividualChild, int idParent) {

    crossoverMutation(child, mother, father);

    // Se for do tipo uniform ou one-point crossover
    if((CROSSOVER_TYPE == 1) ||(CROSSOVER_TYPE == 2)){

        // Estratégia 1 (S1).. Se for na sub árvore da raiz, roda LA, senao roda mapTS
        if(STRATEGY_RECOMBINE == 1){
            if(idParent == 0){
                tree->callLateAcceptance(posNodeChild, posIndividualChild);
            }
            else{
                tree->mapAndCallTS(posNodeChild, posIndividualChild);
            }
        }
        // Estratégia 2 (S2).. Probabilidade de rodar cada alg de busca
        else{
            // Vai decidir qual algoritmo de busca chamar
            int prob = rand() % 100; // Generate a random number between 0 and 99

            if(prob < PROB_MAP_TS_RATE) {     // se o numero foi menor que o prob_map_ts_rate, entao chama mapTS em cada filho gerado
                tree->mapAndCallTS(posNodeChild, posIndividualChild);
            }
            else if(prob < PROB_MAP_TS_RATE + PROB_LA_RATE) {     // se o numero foi menor que o prob_la_rate, entao chama LA em cada filho gerado
                tree->callLateAcceptance(posNodeChild, posIndividualChild);
            }
            else{ // se nao, chama TS
                tree->callTabuSearch(posNodeChild, posIndividualChild);
            }
        }


        if(DEBUG >= DISPLAY_ACTIONS){
            cout << "Child after recombine: ";
            child->printIndividual();
        }
    }
        // Senão, se for do tipo union ou groups
    else if((CROSSOVER_TYPE == 3)||(CROSSOVER_TYPE == 4)){

       // tree->callLocalSearchCloseFac(posNodeChild, posIndividualChild);

        // tree->mapAndCallG(posNodeChild, posIndividualChild);

       tree->callLateAcceptance(posNodeChild, posIndividualChild);

        // if(DEBUG >= DISPLAY_ACTIONS){
        // 	cout << "Child after local search close fac: ";
        // 	child->printIndividual();
        // }

       // tree->callTabuSearch(posNodeChild, posIndividualChild);

        // if(DEBUG >= DISPLAY_ACTIONS){
        // 	cout << "Child after tabu search: ";
        // 	child->printIndividual();
        // }

        // tree->mapAndCallTS(posNodeChild, posIndividualChild);

        if(DEBUG >= DISPLAY_MOVES){
            child->printOpenFacilities();
        }

        if(DEBUG >= DISPLAY_ACTIONS){
            cout << "Child after map and tabu search: ";
            child->printIndividual();
        }
    }
}

void Memetic::crossoverMutation(Solution *child, Solution *mother, Solution *father) {

    int type_crossover = 0;

    if(DEBUG >= DISPLAY_ACTIONS){
        cout << "CROSSOVER" << endl;
        cout << "Mother: ";
        mother->printIndividual();
        cout << "Father: ";
        father->printIndividual();
    }

    // Se for 0, indica que é para sortear qual o tipo de crossover devo fazer
    if(CROSSOVER_TYPE == 0){
        type_crossover = (rand() % 3) + 1; // Generate a random number between 1 and 3
    }

    if((CROSSOVER_TYPE == 1)||(type_crossover == 1)){ // Chama o uniform crossover
        uniformCrossover(child, mother, father);
    }
    else if((CROSSOVER_TYPE == 2)||(type_crossover == 2)){ // Chama o one point crossover
        onePointCrossover(child, mother, father);
    }
    else if((CROSSOVER_TYPE == 3)||(type_crossover == 3)){ // Chama o union crossover
        unionCrossover(child, mother, father);
    }
    else { //  Chama o groups crossover
        groupsCrossover(child, mother, father);
    }

    // mutation
    if(!((CROSSOVER_TYPE == 4)||(type_crossover == 4))){
        mutation(child);
    }

    if(DEBUG >= DISPLAY_ACTIONS){
        cout << "ASSIGNING CLIENTS" << endl;
    }

    // Conectando cada cliente com a instalacao aberta mais proxima e fechando as instalacoes que nao foram conectadas a ninguem. Custo final também é atualizado
    child->connectAndUpdateFacilities(tree->getSortedCijId());

    if(DEBUG >= DISPLAY_ACTIONS){
        cout << "Child after connecting clients: ";
        child->printIndividual();
    }

     if(DEBUG >= DISPLAY_MOVES){
     	cout << "Child after connecting clients: ";
        child->printOpenFacilities();
     }
}

// crossover uniforme: copia para o filho o que é igual e sorteia o que for diferente
void Memetic::uniformCrossover(Solution *child, Solution *mother, Solution *father) {

    int randNum;
    for(int i=0; i<qty_facilities; i++){
        if(father->getOpenFacilityJ(i) == mother->getOpenFacilityJ(i)){
            child->setOpenFacilityJ(i, mother->getOpenFacilityJ(i));

            if(DEBUG >= DISPLAY_DETAILS){
                cout << "Facility " << i << " copy its parent: " << child->getOpenFacilityJ(i) << endl;
            }
        }
        else{
            randNum = rand() % 2; // Generate a random number between 0 and 1
            child->setOpenFacilityJ(i, randNum);

            if(DEBUG >= DISPLAY_DETAILS){
                cout << "Facility " << i << " generate open: " << child->getOpenFacilityJ(i) << endl;
            }
        }
    }

    if(DEBUG >= DISPLAY_ACTIONS){
        cout << "Child after uniform crossover: ";
        child->printIndividual();
    }
}



// one point crossover: sorteia um numero (de 0 a qty_facilities -1) e copia para o filho da mae o que está na esquerda desse numero e do pai o resto
void Memetic::onePointCrossover(Solution *child, Solution *mother, Solution *father) {
    int randNum;
    randNum = rand() % qty_facilities; // Generate a random number between 0 and qty_facilities - 1

    for(int i=0; i<randNum; i++){
        child->setOpenFacilityJ(i, mother->getOpenFacilityJ(i));

        if(DEBUG >= DISPLAY_DETAILS){
            cout << "Facility " << i << " copy its mother: " << child->getOpenFacilityJ(i) << endl;
        }
    }
    for(int i=randNum; i<qty_facilities; i++){
        child->setOpenFacilityJ(i, father->getOpenFacilityJ(i));

        if(DEBUG >= DISPLAY_DETAILS){
            cout << "Facility " << i << " copy its father: " << child->getOpenFacilityJ(i) << endl;
        }
    }

    if(DEBUG >= DISPLAY_ACTIONS){
        cout << "Child after one point crossover: ";
        child->printIndividual();
    }
}


// crossover união: copia para o filho todas as instalacoes abertas da mae e do pai, o resto fica fechado
void Memetic::unionCrossover(Solution *child, Solution *mother, Solution *father) {
    for(int i=0; i<qty_facilities; i++){
        if((father->getOpenFacilityJ(i)) || (mother->getOpenFacilityJ(i))){ // se no pai ou na mae está aberta, entao abre no filho
            child->setOpenFacilityJ(i, true);

            if(DEBUG >= DISPLAY_DETAILS){
                cout << "Facility " << i << ": " << child->getOpenFacilityJ(i) << endl;
            }
        }
        else{ // senao, entao deixa fechada
            child->setOpenFacilityJ(i, false);

            if(DEBUG >= DISPLAY_DETAILS){
                cout << "Facility " << i << ": " << child->getOpenFacilityJ(i) << endl;
            }
        }
    }

    if(DEBUG >= DISPLAY_ACTIONS){
        cout << "Child after union crossover: ";
        child->printIndividual();
    }
}


// crossover grupos: identifica quais clientes estao conectados tanto no pai e na mae na mesma instalacao e mantem esse grupos pós-crossover.
void Memetic::groupsCrossover(Solution *child, Solution *mother, Solution *father) {

    bool found_group;
    int qty_groups = 0;
    int i, j, k;
    double min_cost = DBL_MAX;
    int chosen_fac = -1;
    double cur_cost = 0;

    // Preenchendo o vetor cli_cli para identificar os grupos
    for(i=0;i<qty_clients;i++){

        found_group = false;

        // Matriz simetrica, antes de i=j, preencher igual [j][i] .. se achar um grupo, parar para copiar essa info
        for(j=0;j<i;j++){
            cli_cli[i][j] = cli_cli[j][i];

            // se achou um que seja do mesmo grupo, vamos copiar o restante da linha pra ele
            if(cli_cli[i][j] >= 1){
                found_group = true;
                break;
            }
        }

        if(found_group){
            for(k=j+1;k<qty_clients;k++){
                cli_cli[i][k] = cli_cli[j][k];
            }
        }
        else{ // se nao achou nenhum grupo, ele é novo, entao verificar com o futuro
            qty_groups += 1;

            // ver os com os proximos
            for(j=i;j<qty_clients;j++){
                // Se ele estiver conectado à mesma instalacao em ambos os casos
                if((mother->getAssignedFacilityJ(i) == mother->getAssignedFacilityJ(j)) && (father->getAssignedFacilityJ(i) == father->getAssignedFacilityJ(j))){
                    // Se for exatamente a mesma instalacao
                    if(mother->getAssignedFacilityJ(i) == father->getAssignedFacilityJ(i)){
                        cli_cli[i][j] = 2;
                    }
                        // Senão
                    else{
                        cli_cli[i][j] = 1;
                    }
                }
                    // Senao estiver juntos sempre
                else{
                    cli_cli[i][j] = 0;
                }
            }
        }
    }

//     cout << "ACHEI " << qty_groups << " GRUPOS" << endl;


    // Começa todas as instalacoes fechadas
    for(int i=0; i<qty_facilities; i++){
        child->setOpenFacilityJ(i, false);
    }


    // Para cada grupo, ver pra qual instalacao vai "atribuir", para decidir quais inst abrir
    // se for 2 (a msm fac exata em pai e mae), entao atribui a ela mesmo
    // se for 1, entao percorre todas as facilities e ve qual seria a melhor pra atribuir esse grupo
    for(i=0;i<qty_clients;i++){
        // LINHA i = 0 vai indicar -1 se aquele cliente já foi utilizado
        if(cli_cli[0][i] >= 0){ // se for menor que 0 siginifica q ele ja foi atribuido antes em outro grupo

            if(cli_cli[i][i] == 2){ // esse grupo está na mesma instalacao
                // escolhe essa instalacao mesmo
                chosen_fac = mother->getAssignedFacilityJ(i);

//                 cout << "CASO SIMPLES (2)" << endl;
            }
            else{ // esse grupo nao tem bem definido qual instalacao vai se atribuido. Portanto devemos percorrer todas e ver qual minimiza o custo de atribuir esses clientes + custo de abrir aquela inst
                // inicialmente nenhuma inst escolhida
                min_cost = DBL_MAX;
                chosen_fac = -1;
                for(k=0;k<qty_facilities;k++){ // Para todas as instalacoes
                    cur_cost = instance.getCostFJ(0);

                    for(j=i;j<qty_clients;j++){ // Para todos os clientes que podem ser desse grupo
                        if(cli_cli[i][j] > 0){ // se ele estiver nesse grupo mesmo
                            cur_cost += instance.getCostAIJ(j,k);
                        }
                    }

                    // Se achei uma instalacao melhor pra atribuir, atualiza
                    if(cur_cost < min_cost){
                        min_cost = cur_cost;
                        chosen_fac = k;
                    }
                }

//                 cout << "CASO DE ESCOLHA (1) : fac na mae: " << mother->getOpenFacilityJ(chosen_fac) << " e no pai: " << father->getOpenFacilityJ(chosen_fac) << endl;

            }

            // abre a instalacao escolhida
            child->setOpenFacilityJ(chosen_fac, true);

            // "atribui" o grupo inteiro a essa instalacao mesmo, coloca -1 em [0][j] para falar que ele saiu daqui
            for(j=i;j<qty_clients;j++){
                if(cli_cli[i][j] > 0){ // se ele estiver no mesmo grupo
                    cli_cli[0][j] = -1; // indica que saiu da busca
                }
            }

             if(DEBUG >= DISPLAY_DETAILS){
                cout << "Facility " << chosen_fac << ": " << child->getOpenFacilityJ(chosen_fac) << endl;
             }
        }
    }

    if(DEBUG >= DISPLAY_ACTIONS){
        cout << "Child after groups crossover: ";
        child->printIndividual();
    }

//     if(DEBUG >= DISPLAY_MOVES){
//     	cout << "Child after groups crossover: ";
//        child->printOpenFacilities();
//     }
}



// Recebe nodes por referencia. Modificacoes feitas no node aqui refletem diretamente la. OBS: nao deve alterar o pai nem a mae aqui
void Memetic::mutation(Solution *child) {
    // MUTATION_RATE % das instalações sofrerão mutação

    if(DEBUG >= DISPLAY_ACTIONS){
        cout << "MUTATION" << endl;
    }

    int randNum;

    // QTY_INST_MUTATION instalacoes sofrerao mutação
    for(int i=0; i<QTY_INST_MUTATION; i++){
        randNum = rand() % child->getInstance().getQtyFacilities(); // Generate a random number between 0 and qty_facilities
        child->setOpenFacilityJ(randNum, !child->getOpenFacilityJ(randNum));

        if(DEBUG >= DISPLAY_DETAILS){
            cout << "Facility " << randNum << " changes open to: " << child->getOpenFacilityJ(randNum) << endl;
        }
    }

    if(DEBUG >= DISPLAY_ACTIONS){
        cout << "Child after mutation: ";
        child->printIndividual();
    }
}






Memetic::~Memetic() {
    /* DESALOCAÇÃO DE MEMÓRIA */

    for(int i = 0; i < qty_clients; i++) {
        delete [] cli_cli[i];
    }
    delete [] cli_cli;

    delete [] tree;
}












