#ifndef MYSTRUCT_H
#define MYSTRUCT_H

#include <iostream>
#include <vector>

using namespace std;

struct Edge {
    int x;
    int y;
    int label;
    Edge* ant;
    Edge* next = nullptr;

    Edge(){}

    Edge(int x, int y, int label){
        this->x = x;
        this->y = y;
        this->label = label;
    }

    ~Edge() {
        if(next != nullptr)
            delete next;
    }

    int getProx(int x) {
        if(this->x == x)
            return y;
        return this->x;
    }
};

struct Grafo {
    int numArestas;
    vector<int> numArestasLabels;
    vector<int> roots; //raízes de cada vértice
    vector<int> sizes; //número de vértices descen
    vector<Edge*> edges;
    vector<vector<Edge*>*> edgesVertices;
    int numCompConexas;

    Grafo(int numVertices, int numLabels) {
        numArestas = 0;
        numCompConexas = numVertices;
        for(int i=0; i<numVertices; i++) {
            roots.push_back(i);
            sizes.push_back(1);
            edgesVertices.push_back(new vector<Edge*>);
        }
        for(int i=0; i<numLabels; i++) {
            edges.push_back(new Edge());
            numArestasLabels.push_back(0);
        }
    }

    ~Grafo() {
        for(int i=0; i<edges.size(); i++)
            delete edges[i];

        for(int i=0; i<edgesVertices.size(); i++) {
            delete edgesVertices[i];
        }
    }

    int root(int x) { 
        while(roots[x] != x) { 
            roots[x] = roots[roots[x]] ; 
            x = roots[x]; 
        } 
        return x; 
    } 

    vector<int>* getLabelsInVertice(int vertice) {
        int label;
        vector<int>* labels = new vector<int>;

        for(int i=0; i<edges.size(); i++)
            labels->push_back(0);
        
        for(int i=0; i<edgesVertices[vertice]->size(); i++) {
            label = edgesVertices[vertice]->at(i)->label;
            labels->at(label) = 1;
        }

        return labels;
    }

    void addAresta(int x, int y, int label) { //union
        int rootX = root(x); 
        int rootY = root(y); 
        Edge* novaAresta;

        Edge* atual = edges[label];
        while(atual->next != nullptr)
            atual = atual->next;
        novaAresta = new Edge(x, y, label);
        atual->next = novaAresta;
        novaAresta->ant = atual;

        edgesVertices[x]->push_back(novaAresta);
        edgesVertices[y]->push_back(novaAresta);
        numArestas++;
        numArestasLabels[label]++;

        if(rootX != rootY) {
            numCompConexas--;
            if(sizes[rootX] < sizes[rootY]) { 
                roots[rootX] = roots[rootY]; 
                sizes[rootY] += sizes[rootX]; 
                sizes[rootX] = 1;
            } else { 
                roots[rootY] = roots[rootX]; 
                sizes[rootX] += sizes[rootY]; 
                sizes[rootY] = 1;
            } 
        }
    }

    void addLabel(vector<Edge*> edges, int label) {
        Edge* atual = edges[label]->next;
        int count = 0;

        while(atual != nullptr) {
            addAresta(atual->x, atual->y, label);
            atual = atual->next;
            count++;
        }
    }

    void removeAresta(Edge* aresta) {
        cout << "C: " << aresta->ant << endl;
        aresta->ant->next = aresta->next;
        cout << "D" << endl;
        
        if(aresta->next != nullptr)
            aresta->next->ant = aresta->ant;
        aresta->next = nullptr;
        
        for(int i=0; i<edgesVertices[aresta->x]->size(); i++) {
            if(edgesVertices[aresta->x]->at(i)->x == aresta->x && edgesVertices[aresta->x]->at(i)->y == aresta->y)
                edgesVertices[aresta->x]->erase(edgesVertices[aresta->x]->begin() + i);
        }

        for(int i=0; i<edgesVertices[aresta->y]->size(); i++) {
            if(edgesVertices[aresta->y]->at(i)->x == aresta->x && edgesVertices[aresta->y]->at(i)->y == aresta->y)
                edgesVertices[aresta->y]->erase(edgesVertices[aresta->y]->begin() + i);
        }

        numArestas--;
        numArestasLabels[aresta->label]--;
        delete aresta;
    }
};

#endif