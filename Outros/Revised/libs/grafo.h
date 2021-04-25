#include <iostream>
#include <vector>

using namespace std;

struct Grafo {
    vector<int> vertices;
    vector<vector<bool>*> arestas;

    Grafo(int numVertices) {
        for(int i=0; i<numVertices; i++) {
            vertices.push_back(i);
            arestas.push_back(new vector<bool>);
            for(int j=0; j<numVertices; j++)
                arestas[i]->push_back(false);
        }
    }

    ~Grafo() {
        for(int i=0; i<arestas.size(); i++)
            delete arestas[i];
    }

    void addAresta(int x, int y) {
        arestas[x]->at(y) = true;
        arestas[y]->at(x) = true;
    }

    int conexo() {
        int numVertices;
        int auxValorVertice;
        vector<int> auxVertices;
        vector<vector<bool>*> auxArestas;

        numVertices = vertices.size();
        for(int i=0; i<numVertices; i++) {
            auxVertices.push_back(i);
            auxArestas.push_back(new vector<bool>);
            for(int j=0; j<numVertices; j++)
                auxArestas[i]->push_back(arestas[i]->at(j));
        }

        for(int i=0; i<numVertices; i++) {
            for(int j=0; j<numVertices; j++) {
                if(auxArestas[i]->at(j) == true) {   
                    auxArestas[i]->at(j) = false;
                    auxArestas[j]->at(i) = false;
                    auxValorVertice = auxVertices[j];
                    for(int k=0; k<numVertices; k++) {
                        if(auxVertices[k] == auxValorVertice)
                            auxVertices[k] = auxVertices[i];
                        if(i != k && auxArestas[k]->at(j) == true) {
                            auxArestas[k]->at(j) = false;
                            auxArestas[j]->at(k) = false;
                            auxArestas[k]->at(i) = true;
                            auxArestas[i]->at(k) = true;
                        }
                    }
                }
            }
        }

        for(int i=0; i<auxArestas.size(); i++)
            delete auxArestas[i];

        int countCompConexas = 0;
        vector<int> countVertices;
        for(int i=0; i<auxVertices.size(); i++)
            countVertices.push_back(0);
        for(int i=0; i<auxVertices.size(); i++)
            countVertices[auxVertices[i]]++;
        for(int i=0; i<countVertices.size(); i++)
            if(countVertices[i] > 0)
                countCompConexas++;
        return countCompConexas;
    }
};