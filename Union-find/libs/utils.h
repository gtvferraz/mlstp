#include <vector>
#include "grafo.h"

using namespace std;

struct No {
    int f;
    vector<int> solucao;

    No(vector<int> solucaoAnt, int labelAdicionado) {
        for(int i=0; i<solucaoAnt.size(); i++)
            solucao.push_back(solucaoAnt[i]);
        solucao.push_back(labelAdicionado);
    }

    No() {
        f = 0;
    };
};

struct Label {
    int label;
    int numArestas;

    Label(int label, int numArestas) {
        this->label = label;
        this->numArestas = numArestas;
    }
};

struct AuxiliaOrdenacao {
    Grafo* grafo;
    int label;
    int posLabel;
    int numCompConexas;

    AuxiliaOrdenacao(Grafo* grafo, int label) {
        this->grafo = grafo;
        this->label = label;
    }

    AuxiliaOrdenacao(int numCompConexas, int posLabel) {
        grafo = NULL;
        this->numCompConexas = numCompConexas;
        this->posLabel = posLabel;
    }

    ~AuxiliaOrdenacao() {
        if(grafo != NULL)
            delete grafo;
    }
};

bool compara_sort(AuxiliaOrdenacao* a, AuxiliaOrdenacao* b) {
    return (a->grafo->numCompConexas < b->grafo->numCompConexas);
}

bool compara_sort_b(AuxiliaOrdenacao* a, AuxiliaOrdenacao* b) {
    return (a->numCompConexas < b->numCompConexas);
}

bool compara_sort_f(No* a, No* b) {
    return (a->f < b->f);
}

bool compara_sort_arestas(Label* a, Label* b) {
    return (a->numArestas > b->numArestas);
}

bool taNoVetor(vector<int>* vetor, int x) {
    for(int i=0; i<vetor->size(); i++)
        if(vetor->at(i) == x)
            return true;
    return false;
}

void removeCiclos(Grafo* grafo) {
    vector<int> lista;
    vector<int> verticesVisitados;
    vector<int> verticeAnterior;
    vector<Edge*>* arestas;
    int numVertices = grafo->roots.size();
    int auxVertice;    
    while(verticesVisitados.size() < numVertices) {
        for(int i=0; i<numVertices; i++) {
            if(!taNoVetor(&verticesVisitados, i)) {
                lista.push_back(i);
                verticeAnterior.push_back(-1);
                break;
            }
        }

        while(!lista.empty()) {
            arestas = grafo->edgesVertices[lista[0]];
            for(int i=0; i<arestas->size(); i++) {
                if(arestas->at(i)->getProx(lista[0]) != verticeAnterior[0]) {
                    auxVertice = arestas->at(i)->getProx(lista[0]);
                    if(taNoVetor(&lista, auxVertice)) {
                        cout << "E" << endl;
                        grafo->removeAresta(arestas->at(i));
                        cout << "F" << endl;
                        i--;
                    }
                    else {
                        lista.push_back(auxVertice);
                        verticeAnterior.push_back(lista[0]);
                    }
                }
            }
            verticesVisitados.push_back(lista[0]);
            lista.erase(lista.begin());
            verticeAnterior.erase(verticeAnterior.begin());
        }
    }
}