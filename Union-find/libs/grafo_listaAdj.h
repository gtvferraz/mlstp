#ifndef MYSTRUCT_H
#define MYSTRUCT_H

#include <iostream>
#include <vector>
#include <bitset>
#include <queue>
#include "utils.h"

using namespace std;

struct Aresta {
    int id;
    int arcoId;
    int label;
    int origem;
    int destino;
    Aresta* prox;

    Aresta(int x, int y, int label, int id, int arcoId) {
        this->id = id;
        this->arcoId = arcoId;
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
    int numTotalArestas;

    GrafoListaAdj(int numVertices, int numLabels) {
        numTotalArestas = 0;
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
        for(int i=0; i<arestas.size(); i++) {
            if(arestas[i] != nullptr)
                delete arestas[i];
        }

        for(int i=0; i<vertices.size(); i++)
            delete vertices[i];
    }

    void addAresta(int x, int y, int label) {
        Aresta* aux;
        int mapArco = 2*(numTotalArestas-1)+2;
        //par numTotalArestas = (mapArco - 2)/2 + 1
        //impar numTotalArestas = (mapArco - 2)/2 + 1 + 1

        // (k-1)*numArcos+aresta->id;

        Aresta* newArestaX = new Aresta(x, y, label, numTotalArestas, mapArco);
        aux = vertices[x]->arestas[label];
        if(aux == nullptr)
            vertices[x]->arestas[label] = newArestaX;
        else {
            while(aux->prox != nullptr) {
                aux = aux->prox;
            }
            aux->prox = newArestaX;
        }

        Aresta* newArestaY = new Aresta(y, x, label, numTotalArestas, mapArco+1);
        aux = vertices[y]->arestas[label];
        if(aux == nullptr)
            vertices[y]->arestas[label] = newArestaY;
        else {
            while(aux->prox != nullptr) {
                aux = aux->prox;
            }
            aux->prox = newArestaY;
        }

        Aresta* newArestaLabel = new Aresta(x, y, label, numTotalArestas, mapArco);
        aux = arestas[label];
        if(aux == nullptr)
            arestas[label] = newArestaLabel;
        else {
            while(aux->prox != nullptr) {
                aux = aux->prox;
            }
            aux->prox = newArestaLabel;
        }

        bitArestas[label][numTotalArestas] = 1;

        numArestasLabels[label] += 1;
        numTotalArestas++;
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

    int getRoot(vector<int>* roots, int id) {
        while(roots->at(id) != id) {
            id = roots->at(roots->at(id));
        }
        return id;
    }

    SolucaoParcial* numCompConexas3(bool* labels) {
        int numLabels = arestas.size();
        int numVertices = vertices.size();

        vector<int>* compConexas = new vector<int>;

        for(int i=0; i<numVertices; i++)
            compConexas->push_back(i);

        int numCompConexas = numVertices;
        Aresta* aux;

        for(int i=0; i<arestas.size(); i++) {
            if(labels[i]) {
                aux = arestas[i];
                while(aux != nullptr) {
                    int origemRoot = getRoot(compConexas, aux->origem);
                    int destinoRoot = getRoot(compConexas, aux->destino);
                    if(origemRoot != destinoRoot) {
                        compConexas->at(origemRoot) = destinoRoot;
                        numCompConexas--;
                        if(numCompConexas == 1) {
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

    int updateNumCompConexasParcial(vector<int>* labels, SolucaoParcial* solucaoParcial) {
        int numLabels = arestas.size();
        int numVertices = vertices.size();

        for(int i=0; i<numVertices; i++)
            visitados[i] = solucaoParcial->compConexa->at(i);

        int numCompConexa = solucaoParcial->numCompConexas;
        int numVerticesVisitados = 0;
        Aresta* aux;
        
        int auxCompConexa;

        for(int i=0; i<labels->size(); i++) {
            solucaoParcial->labels[labels->at(i)] = true;

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

        solucaoParcial->numCompConexas = numCompConexa;
        for(int i=0; i<numVertices; i++)
            solucaoParcial->compConexa->at(i) = visitados[i];

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

    vector<vector<int>*>* labelsCompConexasParcial(vector<int>* labels, SolucaoParcial* solucaoParcial) {
        int numLabels = arestas.size();
        int numVertices = vertices.size();

        int numCompConexa = solucaoParcial->numCompConexas;
        int numVerticesVisitados = 0;
        Aresta* aux;

        for(int i=0; i<numVertices; i++)
            visitados[i] = solucaoParcial->compConexa->at(i);

        int auxCompConexa;
        for(int i=0; i<labels->size(); i++) {
            aux = arestas[labels->at(i)];
            while(aux != nullptr) {
                if(visitados[aux->origem] != visitados[aux->destino]) {
                    numCompConexa--;

                    if(visitados[aux->origem] < visitados[aux->destino]) {
                        auxCompConexa = visitados[aux->destino];
                        for(int j=0; j<numVertices; j++)
                            if(visitados[j] == auxCompConexa)
                                visitados[j] = visitados[aux->origem];
                    } else {
                        auxCompConexa = visitados[aux->origem];
                        for(int j=0; j<numVertices; j++)
                            if(visitados[j] == auxCompConexa)
                                visitados[j] = visitados[aux->destino];
                    }

                    if(numCompConexa == 1) 
                        return nullptr;   
                }
                aux = aux->prox;
            }
        }

        vector<vector<int>*>* labelsCompConexas = new vector<vector<int>*>;
        for(int i=0; i<numCompConexa; i++) {
            labelsCompConexas->push_back(new vector<int>);
            for(int j=0; j<arestas.size(); j++)
                labelsCompConexas->at(i)->push_back(0);
        }

        vector<int> compConexas;
        for(int i=0; i<numVertices; i++)
            if(!taNoVetor(&compConexas, visitados[i]))
                compConexas.push_back(visitados[i]);
        sort(compConexas.begin(), compConexas.end());

        bool verifica;
        for(int i=0; !compConexas.empty(); i++) {
            verifica = false;
            for(int j=0; j<numVertices; j++) {
                if(visitados[j] == i) {
                    verifica = true;
                    break;
                }
            }
            if(!verifica) {
                for(int j=0; j<numVertices; j++) {
                    if(visitados[j] == compConexas.back()) {
                        visitados[j] = i;
                        verifica = true;
                    }
                }
                if(verifica)
                    compConexas.pop_back();
            } else {
                compConexas.erase(compConexas.begin());
            }
        }
        
        for(int i=0; i<arestas.size(); i++) {
            aux = arestas[i];
            while(aux != nullptr) {
                if(visitados[aux->origem] != visitados[aux->destino] || visitados[aux->origem] == -1) {
                    labelsCompConexas->at(visitados[aux->origem])->at(i) = 1;
                    labelsCompConexas->at(visitados[aux->destino])->at(i) = 1;
                }
                aux = aux->prox;
            }
        }
        
        /*cout << "YYY: " << labelsCompConexas->size() << endl;
        for(int j=0; j<labelsCompConexas->size(); j++) {
            for(int k=0; k<labelsCompConexas->at(j)->size(); k++)
                cout << " " << labelsCompConexas->at(j)->at(k);
            cout << endl << "---------------------" << endl;
        }*/

        return labelsCompConexas;
    }

    vector<vector<int>*>* labelsCompConexasParcial2(vector<int>* labels, SolucaoParcial* solucaoParcial) {
        int numLabels = arestas.size();
        int numVertices = vertices.size();

        vector<int>* compConexas = new vector<int>;
        for(int i=0; i<numVertices; i++) 
            compConexas->push_back(solucaoParcial->compConexa->at(i));

        bool labelsVisitados[numLabels];
        for(int i=0; i<numLabels; i++)
            labelsVisitados[i] = false; 
        
        int numCompConexa = solucaoParcial->numCompConexas;
        vector<vector<int>*>* labelsCompConexas = new vector<vector<int>*>;
        for(int i=0; i<numVertices; i++) {
            labelsCompConexas->push_back(new vector<int>);
        }

        Aresta* aux;
        for(int i=0; i<labels->size(); i++) {
            aux = arestas[labels->at(i)];
            while(aux != nullptr) {
                int origemRoot = getRoot(compConexas, aux->origem);
                int destinoRoot = getRoot(compConexas, aux->destino);
                if(origemRoot != destinoRoot) {
                    compConexas->at(origemRoot) = destinoRoot;
                    numCompConexa--;
                    if(numCompConexa == 1) {
                        delete compConexas;
                        for(int j=0; j<labelsCompConexas->size(); j++)
                            delete labelsCompConexas->at(j);
                        delete labelsCompConexas;
                        return nullptr;
                    }
                }
                aux = aux->prox;
            }
            labelsVisitados[labels->at(i)] = true;
        }

        for(int i=0; i<arestas.size(); i++) {
            if(!labelsVisitados[i]) {
                aux = arestas[i];
                while(aux != nullptr) {
                    int origemRoot = getRoot(compConexas, aux->origem);
                    int destinoRoot = getRoot(compConexas, aux->destino);
                    if(origemRoot != destinoRoot) {
                        if(labelsCompConexas->at(origemRoot)->size() == 0)
                            labelsCompConexas->at(origemRoot)->push_back(i);
                        else if(labelsCompConexas->at(origemRoot)->back() != i)
                            labelsCompConexas->at(origemRoot)->push_back(i);

                        if(labelsCompConexas->at(destinoRoot)->size() == 0)
                            labelsCompConexas->at(destinoRoot)->push_back(i);
                        else if(labelsCompConexas->at(destinoRoot)->back() != i)
                            labelsCompConexas->at(destinoRoot)->push_back(i);
                    }
                    aux = aux->prox;
                }
            }
        }
        delete compConexas;
        
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