#ifndef UTILS_H
#define UTILS_H

#include <vector>

using namespace std;

struct AuxiliaOrdenacao {
    int posLabel;
    int numCompConexas;

    AuxiliaOrdenacao(int numCompConexas, int posLabel) {
        this->numCompConexas = numCompConexas;
        this->posLabel = posLabel;
    }

    AuxiliaOrdenacao(){}
};

struct SolucaoParcial {
    int numCompConexas;
    bool* labels;
    vector<int>* compConexa;

    ~SolucaoParcial() {
        delete []labels;
        delete compConexa;
    }
};

struct ContabilizaArestas {
    int v1;
    int v2;
    int count;
};

bool compara_sort_b(AuxiliaOrdenacao* a, AuxiliaOrdenacao* b) {
    return (a->numCompConexas < b->numCompConexas);
}

bool taNoVetor(vector<int>* vetor, int x) {
    for(int i=0; i<vetor->size(); i++)
        if(vetor->at(i) == x)
            return true;
    return false;
}

bool solucoesIguais(vector<int>* solucao1, vector<int>* solucao2) {
    if(solucao1->size() != solucao2->size())
        return false;

    for(int i=0; i<solucao1->size(); i++) {
        if(!taNoVetor(solucao2, solucao1->at(i)))
            return false;
    }
    
    return true;
}

void printSolution(vector<int>* solucao) {
    for(int i=0; i<solucao->size(); i++)
        cout << solucao->at(i) << " ";
    cout << endl;
}

#endif