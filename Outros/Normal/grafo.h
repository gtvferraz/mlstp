#ifndef MYSTRUCT_H
#define MYSTRUCT_H

#include <iostream>
#include <vector>

using namespace std;

struct Aresta {
    int x;
    int y;
    int label;
    int id;

    Aresta(int x, int y, int label, int id) {
        this->x = x;
        this->y = y;
        this->label = label;
        this->id = id;
    }

    Aresta() {
    }

    ~Aresta() {
    }

    int getProx(int x) {
        if(this->x == x)
            return y;
        return this->x;
    }
};

struct Grafo {
    int limiteLabels;
    int numVertices;
    int numLabels;
    int numArestas;
    int idNovaAresta;
    vector<int> numArestasLabel;
    vector<int> numArestasVertice;
    //vector<vector<vector<Aresta*>*>*> arestas;
    //vector<Aresta*> arestasCronologico;
    vector<vector<Aresta*>*> arestasVertice;
    vector<vector<Aresta*>*> arestasLabel;
    vector<vector<int>*> indicesArestas;

    Grafo(int numVertices, int numLabels) {
        this->limiteLabels = numLabels;
        this->numVertices = numVertices;
        this->numLabels = 0;
        this->numArestas = 0;
        this->idNovaAresta = 0;

        for(int i=0; i<numVertices; i++) {
            arestasVertice.push_back(new vector<Aresta*>);
            numArestasVertice.push_back(0);
        }

        for(int i=0; i<limiteLabels; i++) {
            arestasLabel.push_back(new vector<Aresta*>);
            /*arestas.push_back(new vector<vector<Aresta*>*>);
            for(int j=0; j<numVertices; j++) {
                arestas[i]->push_back(new vector<Aresta*>);
                for(int k=j+1; k<numVertices; k++)
                    arestas[i]->at(j)->push_back(NULL);
            }*/
            numArestasLabel.push_back(0);
        }
    }

    ~Grafo() {
        /*for(int i=0; i<limiteLabels; i++) {
            for(int j=0; j<numVertices; j++) {
                for(int k=0; k<numVertices-j-1; k++) {
                    if(arestas[i]->at(j)->at(k) != NULL)
                        delete arestas[i]->at(j)->at(k);
                }
                delete arestas[i]->at(j);       
            }
            delete arestas[i];
        }*/

        for(int i=0; i<limiteLabels; i++)
            if(arestasLabel[i] != NULL)
                delete arestasLabel[i];

        for(int i=0; i<numVertices; i++)
            if(arestasVertice[i] != NULL)
                delete arestasVertice[i];

        for(int i=0; i<indicesArestas.size(); i++)
            if(indicesArestas[i] != NULL)
                delete indicesArestas[i];
    }

    void addAresta(int x, int y, int label) {
        Aresta* novaAresta = new Aresta(x, y, label, idNovaAresta);

        //arestas[label]->at(x)->at(y-x-1) = novaAresta;
        //arestasCronologico.push_back(novaAresta);
        arestasVertice[x]->push_back(novaAresta);
        arestasVertice[y]->push_back(novaAresta);
        arestasLabel[label]->push_back(novaAresta);

        indicesArestas.push_back(new vector<int>);
        indicesArestas[idNovaAresta]->push_back(arestasVertice[x]->size()-1);
        indicesArestas[idNovaAresta]->push_back(arestasVertice[y]->size()-1);
        indicesArestas[idNovaAresta]->push_back(arestasLabel[label]->size()-1);

        numArestas++;
         if(numArestasLabel[label] == 0)
            numLabels++;
        numArestasLabel[label]++;
        numArestasVertice[x]++;
        numArestasVertice[y]++;
        idNovaAresta++;
    }

    void addAresta(Aresta* aresta) {
        addAresta(aresta->x, aresta->y, aresta->label);
    }

    void addLabel(Grafo* grafo, int label) {
        if(numArestasLabel[label] == 0)
            numLabels++;
        for(int i=0; i<grafo->numArestasLabel[label]; i++) {
            addAresta(grafo->arestasLabel[label]->at(i));
        }
    }

    void removeAresta(Aresta* aresta) {
        //cout << "Deleta: " << aresta->x << " - " << aresta->y << endl;
        //arestas[aresta->label]->at(aresta->x)->at(aresta->y-aresta->x-1) = NULL;
        //arestasCronologico[aresta->id] = NULL;
        arestasVertice[aresta->x]->at(indicesArestas[aresta->id]->at(0)) = NULL;
        arestasVertice[aresta->y]->at(indicesArestas[aresta->id]->at(1)) = NULL;
        arestasLabel[aresta->label]->at(indicesArestas[aresta->id]->at(2)) = NULL;

        delete indicesArestas[aresta->id];
        indicesArestas[aresta->id] = NULL;

        numArestasLabel[aresta->label]--;
        numArestas--;
        numArestasVertice[aresta->x]--;
        numArestasVertice[aresta->y]--;
        if(numArestasLabel[aresta->label] == 0)
            numLabels--;

        delete aresta;
    }

    void contracao(int label) {
        int x;
        int y;
        bool valida;
        for(int i=0; i<arestasLabel[label]->size(); i++) {
            if(arestasLabel[label]->at(i) != NULL) {
                x = arestasLabel[label]->at(i)->x;
                y = arestasLabel[label]->at(i)->y;   

                for(int j=0; j<arestasVertice[x]->size(); j++) {
                    if(arestasVertice[x]->at(j) != NULL && arestasLabel[label]->at(i)->id != arestasVertice[x]->at(j)->id) {
                        //cout << arestasVertice[x]->at(j)->x << " - " << arestasVertice[x]->at(j)->y << ", ID: " << arestasVertice[x]->at(j)->id << endl;
                        //arestas[arestasVertice[x]->at(j)->label]->at(arestasVertice[x]->at(j)->x)->at(arestasVertice[x]->at(j)->y-arestasVertice[x]->at(j)->x-1) = NULL;
                        
                        valida = true;
                        if(arestasVertice[x]->at(j)->x == x) {
                            if(arestasVertice[x]->at(j)->y == y) {
                                //cout << "ID: " << arestasVertice[x]->at(j)->id << endl; 
                                removeAresta(arestasVertice[x]->at(j));
                                valida = false;
                            }
                            else if(y > arestasVertice[x]->at(j)->y) {
                                //cout << "A" << endl;
                                indicesArestas[arestasVertice[x]->at(j)->id]->at(0) = indicesArestas[arestasVertice[x]->at(j)->id]->at(1);
                                indicesArestas[arestasVertice[x]->at(j)->id]->at(1) = arestasVertice[y]->size();
                                arestasVertice[x]->at(j)->x = arestasVertice[x]->at(j)->y;
                                arestasVertice[x]->at(j)->y = y;
                            } else {
                                //cout << "B: " << arestasVertice[x]->at(j)->y << endl;
                                arestasVertice[x]->at(j)->x = y;
                                indicesArestas[arestasVertice[x]->at(j)->id]->at(0) = arestasVertice[y]->size();
                            }
                        } else {
                            arestasVertice[x]->at(j)->y = y; 
                            //if(indicesArestas[arestasVertice[x]->at(j)->id] == NULL)
                            //    cout << "Z: " << indicesArestas[arestasVertice[x]->at(j)->id] << endl;
                            //cout << "C" << endl;
                            indicesArestas[arestasVertice[x]->at(j)->id]->at(1) = arestasVertice[y]->size();
                            //cout << "C2" << endl;
                        }      
                        
                        if(valida) {

                            /*cout << arestasVertice[x]->at(j)->label << endl;
                            cout << arestasVertice[x]->at(j)->x << endl;
                            cout << arestasVertice[x]->at(j)->y << endl;*/
                            //arestas[arestasVertice[x]->at(j)->label]->at(arestasVertice[x]->at(j)->x)->at(arestasVertice[x]->at(j)->y-arestasVertice[x]->at(j)->x-1) = arestasVertice[x]->at(j);
                            arestasVertice[y]->push_back(arestasVertice[x]->at(j));
                            arestasVertice[x]->at(j) = NULL;
                            numArestasVertice[x]--;
                            numArestasVertice[y]++;
                        }
                    }
                }    
                numVertices--;
                /*cout << "Arestas contraida: " << x << " - " << y << endl;
                cout << "Numero de Arestas: " << numArestasVertice[x] << endl;
                cout << "Numero de Vertices:" << numVertices << endl;*/
                removeAresta(arestasLabel[label]->at(i));
                //imprime();
                //system("pause");
            }
        }
    }

    /*void imprime() {
        for(int i=0; i<arestas.size(); i++)
            for(int j=0; j<arestas[i]->size(); j++)
                for(int k=0; k<arestas[i]->at(j)->size(); k++)
                    if(arestas[i]->at(j)->at(k) != NULL)
                     cout << "Label: " << i << ", " << j << " - " << k+j+1 << endl;
        cout << endl;
    }*/
};

#endif