#include <iostream>
#include <stdlib.h>
#include <list>
#include <vector>


#define qtd_clientes 10
#define qtd_instalacoes 10

using namespace std;


/**

CLASSE DE CLIENTES

**/

class D
{
	int id;
	float v; // quantia que o cliente vai contribuir
public:
	D(int ID);
	int get_id();
	void set_id(int id);
	float get_v();
	void set_v(float v);
	void aumentar_v(float qtd);
};

D::D(int id){
	this->id = id;
	this->v = 0; // inicialmente nao contribui com nada
}

int D::get_id(){
	return this->id;
}

void D::set_id(int id){
	this->id = id;
}

float D::get_v(){
	return this->v;
}

void D::set_v(float v){
	this->v = v;
}

void D::aumentar_v(float qtd){
	this->v += qtd;
	cout << "aumentando aqui sim = " << this->v << endl;
}



/**

CLASSE DE INSTALAÇÕES

**/

class F
{
	int id;
	float custo_inst;
public:
	F(int ID, float custo_inst);
	int get_id();
	void set_id(int id);
	float get_custo_inst();
	void set_custo_inst(float custo_inst);
};

F::F(int id, float custo_inst){
	this->id = id;
	this->custo_inst = custo_inst;
}

int F::get_id(){
	return this->id;
}

void F::set_id(int id){
	this->id = id;
}

float F::get_custo_inst(){
	return this->custo_inst;
}

void F::set_custo_inst(float custo_inst){
	this->custo_inst = custo_inst;
}

/**

CLASSE ARESTA

**/

class Aresta
{
	D vD;
	F vF;
	float custo_atribuicao; // custo de atribuir o cliente vD a instalacao vF
	float w; // parte do custo da instalacao, que o cliente vai contribuir
public:
	Aresta(D vD, F vF, float custo_atribuicao);
	D get_vD();
	F get_vF();
	float get_custo_atribuicao();
	void set_custo_atribuicao(float custo_atribuicao);
	float get_w();
	void set_w(float w);
};

Aresta::Aresta(D _vD, F _vF, float custo_atribuicao) : vD(_vD.get_id()), vF(_vF.get_id(), _vF.get_custo_inst()){
	this->custo_atribuicao = custo_atribuicao;
	this->w = 0; // inicialmente nao contribui com nada
}

D Aresta::get_vD(){
	return this->vD;
}

F Aresta::get_vF(){
	return this->vF;
}

float Aresta::get_custo_atribuicao(){
	return this->custo_atribuicao;
}

void Aresta::set_custo_atribuicao(float custo_atribuicao){
	this->custo_atribuicao = custo_atribuicao;
}

float Aresta::get_w(){
	return this->w;
}

void Aresta::set_w(float w){
	this->w = w;
}




/**

CLASSE GRAFO

**/

class Grafo
{
	int qtd_D; // quantidade de vertices de clientes
	int qtd_F; // quantidade de vertices de instalacoes
	vector<Aresta> arestas;
public:
	Grafo(int qtd_D, int qtd_F);
	void adicionar_aresta(D vD, F vF, float custo_atribuicao);
	bool eh_vizinho(D vD, F vF);
	vector<Aresta> get_arestas_d(int id_vD);
	vector<Aresta> get_arestas();
	int qtd_vizinho_f(int id_vF);
	int get_qtd_D();
	int get_qtd_F();
};

Grafo::Grafo(int qtd_D, int qtd_F){
	this->qtd_D = qtd_D;
	this->qtd_F = qtd_F;
}

void Grafo::adicionar_aresta(D vD, F vF, float custo_atribuicao){
	Aresta aresta(vD,vF,custo_atribuicao);
	arestas.push_back(aresta);
}

bool Grafo::eh_vizinho(D vD, F vF){
	for(vector<Aresta>::iterator it = arestas.begin() ; it != arestas.end(); ++it){
		if((it->get_vD().get_id() == vD.get_id()) && (it->get_vF().get_id() == vF.get_id()))
			return true;
	}
	return false;
}


int Grafo::qtd_vizinho_f(int id_vF){
	int qtd = 0;
	for(vector<Aresta>::iterator it = arestas.begin() ; it != arestas.end(); ++it){
		if(it->get_vF().get_id() == id_vF)
			qtd+=1;
	}
	return qtd;
}


int Grafo::get_qtd_D(){
	return this->qtd_D;
}


int Grafo::get_qtd_F(){
	return this->qtd_F;
}

vector<Aresta> Grafo::get_arestas_d(int id_vD){
	vector<Aresta> arestas_D;
	for(vector<Aresta>::iterator it = arestas.begin() ; it != arestas.end(); ++it){
		if(it->get_vD().get_id() == id_vD)
			arestas_D.push_back(*it);
	}
	return arestas_D;
}

vector<Aresta> Grafo::get_arestas(){
	return this->arestas;
}


vector<D> popular_clientes(){
	vector<D> cli;

	D cliente(0);
	cli.push_back(cliente);

	for(int i=1;i<qtd_clientes;i++){
		cliente.set_id(i);
		cli.push_back(cliente);
	}

	return cli;
}


vector<F> popular_instalacoes(){
	vector<F> inst;

	F instalacao(0,4);
	inst.push_back(instalacao);

	for(int i=1;i<qtd_instalacoes;i++){
		instalacao.set_id(i);
		instalacao.set_custo_inst(4+i/3.0);
		inst.push_back(instalacao);
	}

	return inst;
}


Grafo popular_grafo(vector<D> cli, vector<F> inst){
	Grafo g(qtd_clientes,qtd_instalacoes);

	for(int i=0;i<qtd_clientes;i++)
		for(int j=0;j<qtd_instalacoes;j++)
			g.adicionar_aresta(cli[i],inst[j], (300 - i - j));

	return g;
}


int main (){

	// Criando vetores que armazenam todas as possíveis instalações e todos os clientes
	vector<F> inst;
	vector<D> cli;

	// Populando os vetores de maneira prática
	cli = popular_clientes();
	inst = popular_instalacoes();

	// Colocando no grafo os clientes e as instalações de forma a associar os custos de atribuição de maneira prática
	Grafo grafo(qtd_clientes,qtd_instalacoes);
	grafo = popular_grafo(cli,inst);

	// v <- 0, w <- 0 ja acontece na propria inicializacao 

	

	// Conjunto das instalacoes que possuem desigualdade justa
	vector<F> t; // inicia vazio


	// Grafo contendo conjunto dos clientes que estamos aumentando as variaveis duais e as instalacoes
	Grafo g(qtd_clientes,qtd_instalacoes);
	g = grafo; // inicia com o grafo todo



	// Aresta auxiliar, inicia com o valor da primeira aresta
	Aresta menor = g.get_arestas()[0];


	// Percorrer todas as arestas para ver qual o menor cij
	for(vector<Aresta>::iterator it = (g.get_arestas().begin()+1) ; it != g.get_arestas().end(); ++it){
		if(it->get_custo_atribuicao() < menor.get_custo_atribuicao()){
			menor = *it;
		}	
	}


	cout << "menor custo: " << menor.get_custo_atribuicao() << endl;

	/** NAO TO CONSEGUINDO .. O QUE QUERO EH PERCORRER TODOS OS CLIENTES PRA AUMETNAR ESSE VALOR DE MENOR CUSTO **/



	// // Percorrer todos os clientes para aumentar em todos esse valor
	//vector<int> ja_aumentado;
	// for(vector<Aresta>::iterator it = g.get_arestas().begin() ; it != g.get_arestas().end(); ++it){
	// 	//if (find(ja_aumentado.begin(), ja_aumentado.end(), it->get_vD().get_id()) == ja_aumentado.end()){
	// 		//it->get_vD().aumentar_v(menor.get_custo_atribuicao());
	// 		it->get_vD().aumentar_v(1);
	// 		cout <<"olha aqui aumentando " << it->get_vD().get_id() << " com " << it->get_vD().get_v() << endl;
	// 	//}
	// }

	for(int i=0;i<(g.get_qtd_F() * g.get_qtd_D());i++){
		g.get_arestas()[i].get_vD().aumentar_v(1);
		cout <<"olha aqui aumentando " << g.get_arestas()[i].get_vD().get_id() << " com " << g.get_arestas()[i].get_vD().get_v() << endl;
	}

	// for(vector<D>::iterator it = s.begin() ; it != s.end(); ++it){
	// 	cout <<"*olha aqui aumentando " << it->get_id() << " com " << it->get_v() << endl;
	// }



	cout <<" -- depois olha : " << g.get_arestas()[0].get_vD().get_id() << " com " << g.get_arestas()[0].get_vD().get_v()<< endl;

	/**

	como definir N(j)? se v começa com 0 e depois muda pro valor do menor cij

	**/

	// Aresta auxiliar e variavel auxiliar, inicia com o valor da primeira aresta
	menor = g.get_arestas()[0];
	int menor_valor = menor.get_custo_atribuicao() + menor.get_w() - menor.get_vD().get_v();

	cout << "temos aqui um valor inicial = " << menor.get_custo_atribuicao() << "+"<< menor.get_w() << "-" << menor.get_vD().get_v() << " = " << menor_valor << endl;

	// Percorrer todas as arestas para ver qual o menor cij + wij - vj (o q ja foi acrescentado anteriormente)
	for(vector<Aresta>::iterator it = (g.get_arestas().begin()+1) ; it != g.get_arestas().end(); ++it){
		if(it->get_custo_atribuicao() + it->get_w() - it->get_vD().get_v() < menor_valor){
			menor = *it;
			menor_valor = menor.get_custo_atribuicao() + menor.get_w() - menor.get_vD().get_v();
		}	
	}


	cout << "menor valor: " << menor_valor << endl;


	// // Percorrer todos os clientes para aumentar em todos esse valor
	// for(vector<D>::iterator it = s.begin() ; it != s.end(); ++it){
	// 	it->aumentar_v(menor_valor);
	// }



	return 0;
}