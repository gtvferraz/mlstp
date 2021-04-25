#ifndef MYSTRUCT_H
#define MYSTRUCT_H

#include <iostream>
#include <vector>
#include <queue>
#include "utils.h"

using namespace std;

struct Aresta {
    int label;
    int origem;
    int destino;
    Aresta* prox;

    Aresta(int x, int y, int label) {
        this->origem = x;
        this->destino = y;
        this->label = label;
        this->prox = nullptr;
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
    vector<int> numArestasLabels;

    GrafoListaAdj(int numVertices, int numLabels) {
        for(int i=0; i<numVertices; i++) 
            vertices.push_back(new Vertice(i, numLabels));

        for(int i=0; i<numLabels; i++) {
            arestas.push_back(nullptr);
            numArestasLabels.push_back(0);
        }
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

        numArestasLabels[label] += 1;
    }

    int numCompConexas(vector<int>* labels) {
        int numLabels = arestas.size();
        int numVertices = vertices.size();

        bool visitados[numVertices];
        for(int i=0; i<numVertices; i++)
            visitados[i] = false;

        int numCompConexa = 0;
        int numVerticesVisitados = 0;
        queue<int> proximos;
        Aresta* aux;

        proximos.push(vertices[0]->id);
        while(numVerticesVisitados != numVertices) {
            numCompConexa += 1;
            visitados[proximos.front()] = true;
            numVerticesVisitados++;
            
            if(numVerticesVisitados == numVertices) 
                return numCompConexa;
            
            while(!proximos.empty()) {
                for(int i=0; i<labels->size(); i++) {
                    aux = vertices[proximos.front()]->arestas[labels->at(i)];
                    while(aux != nullptr) {
                        if(!visitados[aux->destino]) {
                            numVerticesVisitados++;
                            visitados[aux->destino] = true;
                            proximos.push(aux->destino);
                            if(numVerticesVisitados == numVertices) 
                                return numCompConexa;
                            
                        }
                        aux = aux->prox;
                    }
                }
                proximos.pop();
            }
            
            for(int i=0; i<numVertices; i++) {
                if(!visitados[i]) {
                    proximos.push(vertices[i]->id);
                    break;
                }
            }
        }
        
        return numCompConexa;
    }

    vector<vector<int>*>* labelsCompConexas(vector<int>* labels) {
        int numLabels = arestas.size();
        int numVertices = vertices.size();
        bool finaliza = false;

        vector<vector<int>*> compConexas;

        bool visitados[numVertices];
        for(int i=0; i<numVertices; i++)
            visitados[i] = false;

        int numCompConexas = 0;
        int numVerticesVisitados = 0;
        queue<Vertice*> proximos;
        Aresta* aux;

        proximos.push(vertices[0]);
        while(numVerticesVisitados != numVertices) {
            numCompConexas++;
            visitados[proximos.front()->id] = true;
            numVerticesVisitados++;

            compConexas.push_back(new vector<int>);
            compConexas.back()->push_back(proximos.front()->id);
            while(!proximos.empty()) {
                for(int i=0; i<labels->size(); i++) {
                    aux = proximos.front()->arestas[labels->at(i)];
                    while(aux != nullptr) {
                        if(!visitados[aux->destino]) {
                            numVerticesVisitados++;
                            visitados[aux->destino] = true;
                            proximos.push(vertices[aux->destino]);
                            compConexas.back()->push_back(aux->destino);
                            if(numVerticesVisitados == numVertices) {
                                finaliza = true;
                                break;
                            }
                        }
                        aux = aux->prox;
                    }
                    if(finaliza)
                        break;
                }
                if(finaliza)
                    break;
                proximos.pop();
            }
            if(finaliza)
                break;
            for(int i=0; i<numVertices; i++)
                if(!visitados[i]) {
                    proximos.push(vertices[i]);
                    break;
                }
        }

        vector<vector<int>*>* labelsCompConexas = new vector<vector<int>*>;
        for(int i=0; i<compConexas.size(); i++) {
            labelsCompConexas->push_back(new vector<int>);
            for(int j=0; j<arestas.size(); j++)
                labelsCompConexas->at(i)->push_back(0);

            for(int j=0; j<compConexas[i]->size(); j++) {
                for(int k=0; k<vertices[compConexas[i]->at(j)]->arestas.size(); k++) {
                    aux = vertices[compConexas[i]->at(j)]->arestas[k];
                    while(aux != nullptr) {
                        if(!taNoVetor(compConexas[i], aux->destino)) {
                            labelsCompConexas->at(i)->at(k) = 1;
                            break;
                        }
                        aux = aux->prox;
                    }
                }
            }
        }

        for(int i=0; i<compConexas.size(); i++)
            delete compConexas[i];

        return labelsCompConexas;
    }

    vector<int>* getLabelsInVertice(int idVertice) {
        vector<int>* labels = new vector<int>;
        Vertice* vertice = vertices[idVertice];

        for(int i=0; i<vertice->arestas.size(); i++)
            if(vertice->arestas[i] != nullptr)
                labels->push_back(1);
            else
                labels->push_back(0);
        
        return labels;
    }

    void imprimeGrafo() {
        Aresta* aux;
        for(int i=0; i<vertices.size(); i++) {
            cout << vertices[i]->id << " - ";
                for(int j=0; j<vertices[i]->arestas.size(); j++) {
                    aux = vertices[i]->arestas[j];
                    while(aux != nullptr) {
                        cout << vertices[i]->arestas[j]->label << ":" << aux->destino << " ";
                        aux = aux->prox;
                    }
            }
            cout << endl;
        }

        for(int i=0; i<arestas.size(); i++) {
            aux = arestas[i];
            cout << "Label: " << i << ": ";
            while(aux != nullptr) {
                cout << aux->origem << "-" << aux->destino << " ";
                aux = aux->prox;
            }
            cout << endl;
        }
        cout << endl;
    }
};

#endif