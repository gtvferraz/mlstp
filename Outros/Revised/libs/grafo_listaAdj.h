#include <iostream>
#include <vector>
#include <queue>

using namespace std;

struct Aresta {
    int label;
    int origem;
    int destino;
    Aresta* prox;
    Aresta* copias[2];

    Aresta(int x, int y, int label) {
        this->origem = x;
        this->destino = y;
        this->label = label;
        this->prox = nullptr;
        this->copias[0] = nullptr;
        this->copias[1] = nullptr;
    }

    ~Aresta() {
        if(prox != nullptr)
            delete prox;
    }
};

struct Vertice {
    int id;
    vector<Aresta*> arestas;

    Vertice(int id, int numLabels) {
        this->id = id;
        for(int i=0; i<numLabels; i++)
            arestas.push_back(nullptr);
    }

    ~Vertice() {
        for(int i=0; i<arestas.size(); i++)
            if(arestas[i] != nullptr)
                delete arestas[i];
    }
};

struct GrafoListaAdj {
    vector<Vertice*> vertices;
    vector<Aresta*> arestas;

    GrafoListaAdj(int numVertices, int numLabels) {
        for(int i=0; i<numVertices; i++) 
            vertices.push_back(new Vertice(i, numLabels));

        for(int i=0; i<numLabels; i++)
            arestas.push_back(nullptr);
    }

    ~GrafoListaAdj() {
        for(int i=0; i<arestas.size(); i++)
            if(arestas[i] != nullptr)
                delete arestas[i];

        for(int i=0; i<vertices.size(); i++)
            delete vertices[i];
    }

    void addAresta(int x, int y, int label) {
        Aresta* aux;

        Aresta* newArestaX = new Aresta(x, y, label);
        aux = vertices[x]->arestas[label];
        if(aux == nullptr)
            vertices[x]->arestas[label] = newArestaX;
        else {
            while(aux->prox != nullptr) {
                aux = aux->prox;
            }
            aux->prox = newArestaX;
        }

        Aresta* newArestaY = new Aresta(y, x, label);
        aux = vertices[y]->arestas[label];
        if(aux == nullptr)
            vertices[y]->arestas[label] = newArestaY;
        else {
            while(aux->prox != nullptr) {
                aux = aux->prox;
            }
            aux->prox = newArestaY;
        }

        Aresta* newArestaLabel = new Aresta(x, y, label);
        aux = arestas[label];
        if(aux == nullptr)
            arestas[label] = newArestaLabel;
        else {
            while(aux->prox != nullptr) {
                aux = aux->prox;
            }
            aux->prox = newArestaLabel;
        }

        newArestaX->copias[0] = newArestaY;
        newArestaX->copias[1] = newArestaLabel;
        newArestaY->copias[0] = newArestaX;
        newArestaY->copias[1] = newArestaLabel;
        newArestaLabel->copias[0] = newArestaX;
        newArestaLabel->copias[1] = newArestaY;
    }

    int numCompConexas(vector<int>* labels) {
        int numLabels = arestas.size();

        bool visitados[numLabels];
        for(int i=0; i<numLabels; i++)
            visitados[i] = false;

        int numCompConexas = 0;
        int numVerticesVisitados = 0;
        queue<Vertice*> proximos;
        Aresta* aux;

        proximos.push(vertices[0]);
        while(numVerticesVisitados != vertices.size()) {
            numCompConexas++;
            visitados[proximos.front()->id] = true;
            numVerticesVisitados++;
            while(!proximos.empty()) {
                for(int i=0; i<labels->size(); i++) {
                    aux = proximos.front()->arestas[labels->at(i)];
                    while(aux != nullptr) {
                        if(!visitados[aux->destino]) {
                            numVerticesVisitados++;
                            visitados[aux->destino] = true;
                            proximos.push(vertices[aux->destino]);
                            if(numVerticesVisitados == vertices.size()) {
                                return numCompConexas;
                            }
                        }
                        aux = aux->prox;
                    }
                }
                proximos.pop();
            }
            for(int i=0; i<numLabels; i++)
                if(!visitados[i]) {
                    proximos.push(vertices[i]);
                    break;
                }
        }
        return numCompConexas;
    }
};