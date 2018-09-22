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
public:
	D(int ID);
	int get_id();
	void set_id(int id);
};

D::D(int id){
	this->id = id;
}

int D::get_id(){
	return this->id;
}

void D::set_id(int id){
	this->id = id;
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
	int peso;
public:
	Aresta(D vD, F vF, int peso);
	D getVD();
	F getVF();
	int getPeso();
};

Aresta::Aresta(D _vD, F _vF, int peso) : vD(_vD.get_id()), vF(_vF.get_id(), _vF.get_custo_inst()){
	this->peso = peso;
}

D Aresta::getVD(){
	return this->vD;
}

F Aresta::getVF(){
	return this->vF;
}

int Aresta::getPeso(){
	return this->peso;
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
	void adicionar_aresta(D vD, F vF, int peso);
	bool eh_vizinho(D vD, F vF);
};

Grafo::Grafo(int qtd_D, int qtd_F){
	this->qtd_D = qtd_D;
	this->qtd_F = qtd_F;
}

void Grafo::adicionar_aresta(D vD, F vF, int peso){
	Aresta aresta(vD,vF,peso);
	arestas.push_back(aresta);
}

bool Grafo::eh_vizinho(D vD, F vF){
	for(vector<Aresta>::iterator it = arestas.begin() ; it != arestas.end(); ++it){
		if((it->getVD().get_id() == vD.get_id()) && (it->getVF().get_id() == vF.get_id()))
			return true;
	}
	return false;
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
			g.adicionar_aresta(cli[i],inst[j], ((i+j)/2));

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


	return 0;
}