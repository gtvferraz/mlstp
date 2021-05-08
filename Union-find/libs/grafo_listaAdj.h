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
    vector<int> visitados;

    GrafoListaAdj(int numVertices, int numLabels) {
        for(int i=0; i<numVertices; i++)  {
            vertices.push_back(new Vertice(i, numLabels));
            visitados.push_back(-1);
        }

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

        for(int i=0; i<numVertices; i++)
            visitados[i] = -1;

        int numCompConexa = 0;
        int numVerticesVisitados = 0;
        queue<int> proximos;
        Aresta* aux;

        proximos.push(vertices[0]->id);
        while(numVerticesVisitados != numVertices) {
            numCompConexa += 1;
            visitados[proximos.front()] = numCompConexa-1;
            numVerticesVisitados++;
            
            if(numVerticesVisitados == numVertices) 
                return numCompConexa;
            
            while(!proximos.empty()) {
                for(int i=0; i<labels->size(); i++) {
                    aux = vertices[proximos.front()]->arestas[labels->at(i)];
                    while(aux != nullptr) {
                        if(visitados[aux->destino] == -1) {
                            numVerticesVisitados++;
                            visitados[aux->destino] = numCompConexa-1;
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
                if(visitados[i] == -1) {
                    proximos.push(vertices[i]->id);
                    break;
                }
            }
        }
        
        return numCompConexa;
    }

    SolucaoParcial* numCompConexas2(bool* labels) {
        int numLabels = arestas.size();
        int numVertices = vertices.size();

        vector<int>* compConexas = new vector<int>;

        for(int i=0; i<numVertices; i++)
            compConexas->push_back(-1);

        int numCompConexas = 0;
        int numVerticesVisitados = 0;
        queue<int> proximos;
        Aresta* aux;

        proximos.push(vertices[0]->id);
        while(numVerticesVisitados != numVertices) {
            numCompConexas += 1;
            compConexas->at(proximos.front()) = numCompConexas-1;
            numVerticesVisitados++;
            
            if(numVerticesVisitados == numVertices) {
                SolucaoParcial* solucaoParcial = new SolucaoParcial();
                solucaoParcial->numCompConexas = numCompConexas;
                solucaoParcial->compConexa = compConexas;
                solucaoParcial->labels = labels;
                return solucaoParcial;
            }
            
            while(!proximos.empty()) {
                for(int i=0; i<arestas.size(); i++) {
                    if(labels[i]) {
                        aux = vertices[proximos.front()]->arestas[i];
                        while(aux != nullptr) {
                            if(compConexas->at(aux->destino) == -1) {
                                numVerticesVisitados++;
                                compConexas->at(aux->destino) = numCompConexas-1;
                                proximos.push(aux->destino);
                                if(numVerticesVisitados == numVertices)  {
                                    SolucaoParcial* solucaoParcial = new SolucaoParcial();
                                    solucaoParcial->numCompConexas = numCompConexas;
                                    solucaoParcial->compConexa = compConexas;
                                    solucaoParcial->labels = labels;
                                    return solucaoParcial;
                                }
                                
                            }
                            aux = aux->prox;
                        }
                    }
                }
                proximos.pop();
            }
            
            for(int i=0; i<numVertices; i++) {
                if(compConexas->at(i) == -1) {
                    proximos.push(vertices[i]->id);
                    break;
                }
            }
        }

        SolucaoParcial* solucaoParcial = new SolucaoParcial();
        solucaoParcial->numCompConexas = numCompConexas;
        solucaoParcial->compConexa = compConexas;
        solucaoParcial->labels = labels;
        
        return solucaoParcial;
    }

    int numCompConexasParcial(vector<int>* labels, SolucaoParcial* solucaoParcial) {
        int numLabels = arestas.size();
        int numVertices = vertices.size();

        for(int i=0; i<numVertices; i++)
            visitados[i] = solucaoParcial->compConexa->at(i);

        int numCompConexa = solucaoParcial->numCompConexas;
        int numVerticesVisitados = 0;
        Aresta* aux;
        
        int auxCompConexa;
        for(int i=0; i<labels->size(); i++) {
            aux = arestas[labels->at(i)];
            while(aux != nullptr) {
                if(visitados[aux->origem] != visitados[aux->destino]) {
                    numCompConexa--;

                    auxCompConexa = visitados[aux->destino];
                    for(int j=0; j<numVertices; j++)
                        if(visitados[j] == auxCompConexa)
                            visitados[j] = visitados[aux->origem];

                    if(numCompConexa == 1) 
                        return numCompConexa;
                    
                }
                aux = aux->prox;
            }
        }
        
        return numCompConexa;
    }

    vector<vector<int>*>* labelsCompConexas(vector<int>* labels) {
        int numLabels = arestas.size();
        int numVertices = vertices.size();
        bool finaliza = false;

        vector<vector<int>*> compConexas;

        for(int i=0; i<numVertices; i++)
            visitados[i] = -1;

        int numCompConexas = 0;
        int numVerticesVisitados = 0;
        queue<Vertice*> proximos;
        Aresta* aux;

        proximos.push(vertices[0]);
        while(numVerticesVisitados != numVertices) {
            numCompConexas++;
            visitados[proximos.front()->id] = numCompConexas-1;
            numVerticesVisitados++;

            compConexas.push_back(new vector<int>);
            compConexas.back()->push_back(proximos.front()->id);
            while(!proximos.empty()) {
                for(int i=0; i<labels->size(); i++) {
                    aux = proximos.front()->arestas[labels->at(i)];
                    while(aux != nullptr) {
                        if(visitados[aux->destino] == -1) {
                            numVerticesVisitados++;
                            visitados[aux->destino] = numCompConexas-1;
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
                if(visitados[i] == -1) {
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