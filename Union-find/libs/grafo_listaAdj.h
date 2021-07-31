#ifndef MYSTRUCT_H
#define MYSTRUCT_H

#include <iostream>
#include <vector>
#include <queue>
#include "utils.h"

using namespace std;

struct Aresta {
    int id;
    int label;
    int origem;
    int destino;

    Aresta(int x, int y, int label, int id) {
        this->id = id;
        this->origem = x;
        this->destino = y;
        this->label = label;
    }
};

struct Vertice {
    int id;
    int numLabels;
    int numVertices;
    int* numArestasLabels;
    Aresta*** arestas;

    Vertice(int id, int numLabels, int numVertices) {
        this->id = id;
        this->numLabels = numLabels;
        this->numVertices = numVertices;

        arestas = (Aresta***) malloc(numLabels*sizeof(Aresta**));
        numArestasLabels = (int*) malloc(numLabels*sizeof(int));

        int numMaxArestas = numVertices*(numVertices-1)/2;
        for(int i=0; i<numLabels; i++) {
            arestas[i] = (Aresta**) malloc(numMaxArestas*sizeof(Aresta*));
            for(int j=0; j<numMaxArestas; j++) {
                arestas[i][j] = nullptr;
            }
            numArestasLabels[i] = 0;
        }
    }

    ~Vertice() {
        int numMaxArestas = numVertices*(numVertices-1)/2;
        for(int i=0; i<numLabels; i++) {
            for(int j=0; j<numMaxArestas; j++) {
                if(arestas[i][j] == nullptr)
                    break;
                    
                delete arestas[i][j];
            }
            free(arestas[i]);
        }

        free(arestas); 
        delete[] numArestasLabels;
    }
};

struct GrafoListaAdj {
    Vertice** vertices;
    Aresta*** arestas;
    int* numArestasLabels;
    int* visitados;
    int numTotalArestas;
    int numVertices;
    int numLabels;

    GrafoListaAdj(int numVertices, int numLabels) {
        this->numVertices = numVertices;
        this->numLabels = numLabels;
        numTotalArestas = 0;

        vertices = (Vertice**) malloc(numVertices*sizeof(Vertice*));
        arestas = (Aresta***) malloc(numLabels*sizeof(Aresta**));
        numArestasLabels = (int*) malloc(numLabels*sizeof(int));
        visitados = (int*) malloc(numVertices*sizeof(int));

        int numMaxArestas = numVertices*(numVertices-1)/2;
        for(int i=0; i<numLabels; i++) {
            arestas[i] = (Aresta**) malloc(numMaxArestas*sizeof(Aresta*));
            for(int j=0; j<numMaxArestas; j++) {
                arestas[i][j] = nullptr;
            }
            numArestasLabels[i] = 0;
        }

        for(int i=0; i<numVertices; i++)  {
            vertices[i] = new Vertice(i, numLabels, numVertices);
            visitados[i] = -1;
        }
    }

    ~GrafoListaAdj() {
        int numMaxArestas = numVertices*(numVertices-1)/2;
        for(int i=0; i<numLabels; i++) {
            for(int j=0; j<numMaxArestas; j++) {
                if(arestas[i][j] == nullptr)
                    break;
                delete arestas[i][j];
            }
            free(arestas[i]);
        }
        
        free(arestas);

        for(int i=0; i<numVertices; i++)
            delete vertices[i];
        free(vertices);
    }

    void addAresta(int x, int y, int label) {
        int mapArco = 2*(numTotalArestas-1)+2;

        Aresta* newArestaX = new Aresta(x, y, label, mapArco);
        vertices[x]->arestas[label][vertices[x]->numArestasLabels[label]] = newArestaX;

        Aresta* newArestaY = new Aresta(y, x, label, mapArco+1);
        vertices[y]->arestas[label][vertices[y]->numArestasLabels[label]] = newArestaY;

        Aresta* newArestaLabel = new Aresta(x, y, label, mapArco);
        arestas[label][numArestasLabels[label]] = newArestaLabel;

        numTotalArestas++;
        numArestasLabels[label] += 1;
        vertices[x]->numArestasLabels[label] += 1;
        vertices[y]->numArestasLabels[label] += 1;
    }

    int numCompConexas(vector<int>* labels) {
        for(int i=0; i<numVertices; i++)
            visitados[i] = -1;

        int numCompConexa = 0;
        int numVerticesVisitados = 0;
        queue<int> proximos;

        proximos.push(vertices[0]->id);
        Aresta* aux;
        while(numVerticesVisitados != numVertices) {
            numCompConexa += 1;
            visitados[proximos.front()] = numCompConexa-1;
            numVerticesVisitados++;
            
            if(numVerticesVisitados == numVertices) 
                return numCompConexa;
            
            while(!proximos.empty()) {
                for(int i=0; i<labels->size(); i++) {
                    for(int j=0; j<vertices[proximos.front()]->numArestasLabels[labels->at(i)]; j++) {
                        aux = vertices[proximos.front()]->arestas[labels->at(i)][j];
                        if(visitados[aux->destino] == -1) {
                            numVerticesVisitados++;
                            visitados[aux->destino] = numCompConexa-1;
                            proximos.push(aux->destino);
                            if(numVerticesVisitados == numVertices) 
                                return numCompConexa;
                            
                        }
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
                for(int i=0; i<numLabels; i++) {
                    if(labels[i]) {
                        for(int j=0; j<vertices[proximos.front()]->numArestasLabels[i]; j++) {
                            aux = vertices[proximos.front()]->arestas[i][j];
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
        vector<int>* compConexas = new vector<int>;

        for(int i=0; i<numVertices; i++)
            compConexas->push_back(i);

        int numCompConexas = numVertices;
        Aresta* aux;

        for(int i=0; i<numLabels; i++) {
            if(labels[i]) {
                for(int j=0; j<numArestasLabels[i]; j++) {
                    aux = arestas[i][j];
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
        for(int i=0; i<numVertices; i++)
            visitados[i] = solucaoParcial->compConexa->at(i);

        int numCompConexa = solucaoParcial->numCompConexas;
        int numVerticesVisitados = 0;
        Aresta* aux;
        
        int auxCompConexa;
        for(int i=0; i<labels->size(); i++) {
            for(int j=0; j<numArestasLabels[labels->at(i)]; j++) {
                aux = arestas[labels->at(i)][j];
                if(visitados[aux->origem] != visitados[aux->destino]) {
                    numCompConexa--;

                    auxCompConexa = visitados[aux->destino];
                    for(int j=0; j<numVertices; j++)
                        if(visitados[j] == auxCompConexa)
                            visitados[j] = visitados[aux->origem];

                    if(numCompConexa == 1) 
                        return numCompConexa;
                    
                }
            }
        }
        
        return numCompConexa;
    }

    vector<vector<int>*>* labelsCompConexas(vector<int>* labels) {
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
                    for(int j=0; j<proximos.front()->numArestasLabels[labels->at(i)]; j++) {
                        aux = proximos.front()->arestas[labels->at(i)][j];
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
            for(int j=0; j<numLabels; j++)
                labelsCompConexas->at(i)->push_back(0);

            for(int j=0; j<compConexas[i]->size(); j++) {
                for(int k=0; k<vertices[compConexas[i]->at(j)]->numLabels; k++) {
                    for(int l=0; l<vertices[compConexas[i]->at(j)]->numArestasLabels[k]; l++) {
                        aux = vertices[compConexas[i]->at(j)]->arestas[k][l];
                        if(!taNoVetor(compConexas[i], aux->destino)) {
                            labelsCompConexas->at(i)->at(k) = 1;
                            break;
                        }
                    }
                }
            }
        }

        for(int i=0; i<compConexas.size(); i++)
            delete compConexas[i];

        return labelsCompConexas;
    }

    vector<vector<int>*>* labelsCompConexasParcial(vector<int>* labels, SolucaoParcial* solucaoParcial) {
        int numCompConexa = solucaoParcial->numCompConexas;
        int numVerticesVisitados = 0;
        Aresta* aux;

        for(int i=0; i<numVertices; i++)
            visitados[i] = solucaoParcial->compConexa->at(i);

        int auxCompConexa;
        for(int i=0; i<labels->size(); i++) {
            for(int j=0; j<numArestasLabels[labels->at(i)]; j++) {
                aux = arestas[labels->at(i)][j];
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
            }
        }

        vector<vector<int>*>* labelsCompConexas = new vector<vector<int>*>;
        for(int i=0; i<numCompConexa; i++) {
            labelsCompConexas->push_back(new vector<int>);
            for(int j=0; j<numLabels; j++)
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
        
        for(int i=0; i<numLabels; i++) {
            for(int j=0; j<numArestasLabels[i]; j++) {
                aux = arestas[i][j];
                if(visitados[aux->origem] != visitados[aux->destino] || visitados[aux->origem] == -1) {
                    labelsCompConexas->at(visitados[aux->origem])->at(i) = 1;
                    labelsCompConexas->at(visitados[aux->destino])->at(i) = 1;
                }
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
            for(int j=0; j<numArestasLabels[labels->at(i)]; j++) {
                aux = arestas[labels->at(i)][j];
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
            }
            labelsVisitados[labels->at(i)] = true;
        }

        for(int i=0; i<numLabels; i++) {
            if(!labelsVisitados[i]) {
                for(int j=0; j<numArestasLabels[i]; j++) {
                    aux = arestas[i][j];
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
                }
            }
        }
        delete compConexas;
        
        return labelsCompConexas;
    }

    vector<int>* getLabelsInVertice(int idVertice) {
        vector<int>* labels = new vector<int>;
        Vertice* vertice = vertices[idVertice];

        for(int i=0; i<vertice->numLabels; i++)
            if(vertice->arestas[i] != nullptr)
                labels->push_back(1);
            else
                labels->push_back(0);
        
        return labels;
    }

    void imprimeGrafo() {
        Aresta* aux;
        for(int i=0; i<numVertices; i++) {
            cout << vertices[i]->id << " - ";
                for(int j=0; j<vertices[i]->numLabels; j++) {
                    for(int k=0; k<vertices[i]->numArestasLabels[j]; k++) {
                        aux = vertices[i]->arestas[j][k];
                        cout << j << ":" << aux->destino << " ";
                    }
            }
            cout << endl;
        }

        for(int i=0; i<numLabels; i++) {
            cout << "Label: " << i << ": ";
            for(int j=0; j<numArestasLabels[i]; j++) {
                aux = arestas[i][j];
                cout << aux->origem << "-" << aux->destino << " ";
            }
            cout << endl;
        }
        cout << endl;
    }
};

#endif