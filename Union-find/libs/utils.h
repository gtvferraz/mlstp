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

    AuxiliaOrdenacao() {}
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

void copiaVetor(bool* origem, bool* destino, int tamanho) {
    for(int i=0; i<tamanho; i++)
        destino[i] = origem[i];
}

#endif