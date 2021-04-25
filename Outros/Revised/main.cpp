#include <iostream>
#include <time.h>
//#include "libs/grafo.h"
#include "libs/grafo_listaAdj.h"
#include "leitura.h"

#define NUM 100

using namespace std;


int main() {
    /*
    clock_t tempo[2];
    double duracaoInst;
    double duracaoComp;
    Grafo* grafo = new Grafo(8);
    GrafoT* grafo2 = new GrafoT(8, 6);
    //A
    tempo[0] = clock();
    grafo->addAresta(0, 1);
    grafo->addAresta(2, 5);
    grafo->addAresta(5, 6);
    //B
    grafo->addAresta(0, 6);
    grafo->addAresta(1, 5);
    grafo->addAresta(2, 4);
    //F
    grafo->addAresta(0, 7);
    grafo->addAresta(2, 3);
    grafo->addAresta(3, 5);
    tempo[1] = clock();

    duracaoInst = (double)(tempo[1] - tempo[0])*1000 / CLOCKS_PER_SEC;

    tempo[0] = clock();
    grafo->conexo();
    tempo[1] = clock();

    duracaoComp = (double)(tempo[1] - tempo[0])*1000 / CLOCKS_PER_SEC;

    cout << "Tempo Instanciacao: " << duracaoInst << "ms, Tempo Comp: " << duracaoComp << "ms" << endl;

    //A
    tempo[0] = clock();
    grafo2->addAresta(0, 1, 0);
    grafo2->addAresta(2, 5, 0);
    grafo2->addAresta(5, 6, 0);
    //B
    grafo2->addAresta(0, 6, 1);
    grafo2->addAresta(1, 5, 1);
    grafo2->addAresta(2, 4, 1);
    //F
    grafo2->addAresta(0, 7, 5);
    grafo2->addAresta(2, 3, 5);
    grafo2->addAresta(3, 5, 5);
    tempo[1] = clock();

    duracaoInst = (double)(tempo[1] - tempo[0])*1000 / CLOCKS_PER_SEC;

    tempo[0] = clock();
    grafo2->contracao(0);
    grafo2->contracao(1);
    grafo2->contracao(5);
    tempo[1] = clock();

    duracaoComp = (double)(tempo[1] - tempo[0])*1000 / CLOCKS_PER_SEC;

    cout << "Tempo Instanciacao: " << duracaoInst << "ms, Tempo Comp: " << duracaoComp << "ms" << endl;

    delete grafo;
    */

    clock_t tempo[2];
    //GrafoT* original = carregaInstanciasT("3.txt");

    //Grafo* grafo;
    GrafoListaAdj* grafoListaAdj;
    //GrafoT* grafoUF;

    //Edge* atual;
    double duracao;
    int i;
    int compConexas;
    bool aux;


/*
    cout << endl << endl << "---- REVISED ----" << endl << endl;
    tempo[0] = clock();
    grafo = new Grafo(200);
    i = 0;
    do {
        atual = original->edges[i]->next;
        while(atual != nullptr) {
            grafo->addAresta(atual->x, atual->y);
            atual = atual->next;
        }
        compConexas = grafo->conexo();
        i++;
    }while(compConexas > 1);
    tempo[1] = clock();

    duracao = (double)(tempo[1] - tempo[0])*1000 / CLOCKS_PER_SEC;
    cout << i << ", Comp: " << compConexas << ", Tempo: " << duracao << "ms" << endl;

    tempo[0] = clock();
    compConexas = grafo->conexo();
    tempo[1] = clock();
    duracao = (double)(tempo[1] - tempo[0])*1000 / CLOCKS_PER_SEC;
    cout << i << ", Comp: " << compConexas << ", Tempo: " << duracao << "ms" << endl;
*/


    cout << endl << endl << "---- LISTA ADJ ----" << endl << endl;
    tempo[0] = clock();
    grafoListaAdj = carregaInstanciasAdj("3.txt");
    tempo[1] = clock();
    duracao = (double)(tempo[1] - tempo[0])*1000 / CLOCKS_PER_SEC;
    cout << "Intanciacao: " << duracao << "ms" << endl;

    tempo[0] = clock();
    vector<int>* labels = new vector<int>;
    int numCompConexas;
    i = 0;
    do {
        labels->push_back(i);
        numCompConexas = grafoListaAdj->numCompConexas(labels);
        i++;
    }while(numCompConexas != 1);
    delete labels;
    tempo[1] = clock();
    duracao = (double)(tempo[1] - tempo[0])*1000 / CLOCKS_PER_SEC;
    cout << i << ", Comp: " << numCompConexas << ", Tempo: " << duracao << "ms" << endl;

/*8

    cout << endl << endl << "---- UNION FIND ----" << endl << endl;
    tempo[0] = clock();
    grafoUF = new GrafoT(200, 250);
    i = 0;
    do {
        grafoUF->addLabel(original->edges, i);
        i++;
    } while(grafoUF->numCompConexas > 1);
    tempo[1] = clock();

    duracao = (double)(tempo[1] - tempo[0])*1000 / CLOCKS_PER_SEC;
    cout << i << ", Comp: " << grafoUF->numCompConexas << ", Tempo: " << duracao << "ms" << endl;
*/
    //delete grafo;
    //delete grafoUF;
    //delete original;
    delete grafoListaAdj;
}