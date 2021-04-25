#include <iostream>
#include <vector>
#include <climits>
#include <time.h>
#include <string>
#include <sstream>
#include <direct.h>
#include <random>
#include <algorithm>
#include "grafo.h"
#include "leitura.h"

using namespace std;

/*
INSTANCIAS

GRUPO 1
Número de vértices = Número de cores
n = {20, 30, 40, 50}
d = {ld, md, hd}
inst = {0, 1, ..., 9}
dataset/instances/g1/n/d/inst

GRUPO 2
n = {50, 100, 200, 400, 500, 1000}
l = {{12, 25, 50, 62}, {25, 50, 100, 125}, {50, 100, 200, 250}, {100, 200, 400, 500}, {125, 250, 500, 625}, {250, 500, 1000, 1250}}
d = {ld, md, hd}
inst = {0, 1, ..., 9}
dataset/instances/g2/n/d/l/inst

RESULTADOS

best - média dos melhores resultados de cada instância em 10 iterações dada uma configuração n - l - d
br - média dos de cada instância em apenas uma iteração dada uma configuração n - l - d
*/

struct No {
    int f;
    vector<int> solucao;

    No(vector<int> solucaoAnt, int labelAdicionado) {
        for(int i=0; i<solucaoAnt.size(); i++)
            solucao.push_back(solucaoAnt[i]);
        solucao.push_back(labelAdicionado);
    }

    No() {
        f = 0;
    };
};

struct Label {
    int label;
    int numArestas;

    Label(int label, int numArestas) {
        this->label = label;
        this->numArestas = numArestas;
    }
};

void removeCiclos(Grafo* grafo) {
    vector<int> lista;
    vector<int> verticesVisitados;
    vector<int> verticeAnterior;
    vector<Aresta*>* arestas;
    int numVertices = grafo->numVertices;
    int auxVertice;
    bool valida;
    
    while(verticesVisitados.size() < numVertices) {
        for(int i=0; i<numVertices; i++) {
            valida = true;
            for(int j=0; j<verticesVisitados.size(); j++) 
                if(i == verticesVisitados[j])
                    valida = false;
            if(valida) {
                lista.push_back(i);
                verticeAnterior.push_back(-1);
                break;
            }
        }

        while(!lista.empty()) {
            arestas = grafo->arestasVertice[lista[0]];
            
            for(int i=0; i<grafo->arestasVertice[lista[0]]->size(); i++) {
                if(arestas->at(i) != NULL) {
                    if(arestas->at(i)->getProx(lista[0]) != verticeAnterior[0]) {
                        auxVertice = arestas->at(i)->getProx(lista[0]);
                        valida = true;
                        for(int j=0; j<lista.size(); j++)
                            if(auxVertice == lista[j])
                                valida = false;
                        if(!valida)
                            grafo->removeAresta(arestas->at(i));
                        else {
                            lista.push_back(auxVertice);
                            verticeAnterior.push_back(lista[0]);
                        }
                    }
                }
            }
            verticesVisitados.push_back(lista[0]);
            lista.erase(lista.begin());
            verticeAnterior.erase(verticeAnterior.begin());
        }
    }
}

bool compara_sort_f(No* a, No* b) {
    return (a->f < b->f);
}

bool compara_sort_arestas(Label* a, Label* b) {
    return (a->numArestas > b->numArestas);
}

bool taNoVetor(vector<int> vetor, int x) {
    for(int i=0; i<vetor.size(); i++)
        if(vetor[i] == x)
            return true;
    return false;
}

vector<int>* exato(Grafo* grafo) {
    Grafo* arvore;
    No* novoNo;
    vector<No*> open;
    vector<No*> closed;
    vector<vector<Label*>*> espacoLabels;
    vector<int>* solucao;
    int arestasNecessarias;
    int count;
    int camada;
    bool finaliza;

    espacoLabels.push_back(new vector<Label*>);
    for(int i=0; i<grafo->numLabels; i++)
        espacoLabels[0]->push_back(new Label(i, grafo->numArestasLabel[i]));
    sort(espacoLabels[0]->begin(), espacoLabels[0]->end(), compara_sort_arestas);

    open.push_back(new No());
    finaliza = false;
    do {
        sort(open.begin(), open.end(), compara_sort_f);
        closed.push_back(open[0]);
        open.erase(open.begin());

        camada = 0;
        for(int i=0; i<espacoLabels[camada]->size(); i++) {
            camada = closed[closed.size()-1]->solucao.size();
            if(camada >= espacoLabels.size()) {
                espacoLabels.push_back(new vector<Label*>);
                for(int j=0; j<grafo->numLabels; j++)
                    if(!taNoVetor(closed[closed.size()-1]->solucao, j))
                        espacoLabels[camada]->push_back(new Label(j, grafo->numArestasLabel[j]));
            }

            novoNo = new No(closed[closed.size()-1]->solucao, espacoLabels[camada]->at(i)->label);
            open.push_back(novoNo);

            /*cout << "No: ";
            for(int j=0; j<novoNo->solucao.size(); j++)
                cout << novoNo->solucao[j] << ", ";
            cout << endl;*/
            
            arvore = new Grafo(grafo->numVertices, grafo->limiteLabels);
            for(int j=0; j<novoNo->solucao.size(); j++)
                arvore->addLabel(grafo, novoNo->solucao[j]);
            removeCiclos(arvore);
            
            arestasNecessarias = grafo->numVertices - 1 - arvore->numArestas;
            delete arvore;
            
            count = 0;
            for(int j=0; j<arestasNecessarias; ) {
                j += espacoLabels[camada]->at(count)->numArestas;
                count++;
            }
            if(count == 0) {
                finaliza = true;
                break;
            }
            novoNo->f = count + camada + 1;
        }
        if(open.empty())
            return NULL;
    }while(!finaliza);

    solucao = new vector<int>;
    for(int i=0; i<novoNo->solucao.size(); i++)
        solucao->push_back(novoNo->solucao[i]);

    for(int i=0; i<espacoLabels.size(); i++) {
        for(int j=0; j<espacoLabels[i]->size(); j++)
            delete espacoLabels[i]->at(j);
        delete espacoLabels[i];
    }

    for(int i=0; i<open.size(); i++)
        delete open[i];
    
    for(int i=0; i<closed.size(); i++)
        delete closed[i];
        
    return solucao;
}

void cenarioUm() {
    Grafo* grafos[10];
    vector<int>* solucao;
    vector<int>* melhorSolucao;
    clock_t tempo[2];
    stringstream ss;
    int tamanhoMelhorSolucao;
    float duracao;
    float mediaTempoMelhorSolucao;
    float br;
    const char* filePath;

    //PARAMETRIZÇÃO
    //filePath = "dataset-tratado/instances/g1/50/md/";
    filePath = "dataset-tratado/instances/g2/100/hd/125/";
    
    for(int i=0; i<10; i++)
        grafos[i] = nullptr;

    cout << "----- Carregando Instancias -----" << endl;
    tempo[0] = clock();
    for(int i=0; i<10; i++) {
        ss.str("");
        ss.clear();
        ss << filePath << i << ".txt";
        grafos[i] = carregaInstancias(ss.str().c_str());
    }
    tempo[1] = clock();
    duracao = (float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC;
    cout << "Tempo para Carregar a Instancia: " << duracao << "s" << endl << endl;
    
    for(int i=0; i<10; i++)
        if(grafos[i] == nullptr) {
            cout << "Grafo nulo" << endl;
            system("pause");
            return;  
        }
    /*
    tempo[0] = clock();
    Grafo* grafa = new Grafo(grafos[2]->numVertices, grafos[2]->limiteLabels);
    for(int i=0; i<grafos[2]->arestasLabel.size(); i++)
        grafa->addLabel(grafos[2], i);
    grafa->contracao(14);
    grafa->contracao(26);
    grafa->contracao(43);
    grafa->contracao(0);
    grafa->contracao(22);
    grafa->contracao(35);
    grafa->contracao(47);
    grafa->contracao(1);
    tempo[1] = clock();
    cout << grafa->numVertices << " " << (float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC << "s" << endl;
    delete grafa;
    */
    /*
    Grafo* grafa = new Grafo(6, 5);
    grafa->addAresta(1, 2, 0);
    grafa->addAresta(1, 4, 0);
    grafa->addAresta(1, 3, 1);
    grafa->addAresta(2, 3, 1);
    grafa->addAresta(2, 4, 1);
    grafa->addAresta(0, 4, 2);
    grafa->addAresta(3, 4, 3);
    grafa->addAresta(4, 5, 3);
    grafa->addAresta(0, 1, 4);
    grafa->addAresta(1, 5, 4);
    solucao = exato(grafa);
    cout << "Solucao: ";
    for(int i=0; i<solucao->size(); i++) {
        cout << solucao->at(i);
        if(i+1 < solucao->size())
            cout << ", ";
    }
    cout << endl;
    delete grafa;
    */
    
    cout << "----- EXATO -----" << endl;
    br = 0;
    tamanhoMelhorSolucao = INT_MAX;
    melhorSolucao = nullptr;
    mediaTempoMelhorSolucao = 0;
    for(int i=0; i<10; i++) {
        tempo[0] = clock();
        solucao = exato(grafos[i]);
        tempo[1] = clock();
        mediaTempoMelhorSolucao += (float)(tempo[1] - tempo[0]);
        br += solucao->size();
        if(solucao->size() < tamanhoMelhorSolucao) {
            if(melhorSolucao != nullptr)
                delete melhorSolucao;
            melhorSolucao = solucao;
            tamanhoMelhorSolucao = solucao->size();
        } else
            delete solucao;
        cout << "Custo: " << solucao->size() << ", Tempo: " << (float)(tempo[1] - tempo[0]) / (1.657*CLOCKS_PER_SEC) << "s" << endl;
    }
    mediaTempoMelhorSolucao /= CLOCKS_PER_SEC;
    mediaTempoMelhorSolucao /= 10;
    br /= 10;
    cout << "Melhor Solucao: ";
    for(int i=0; i<melhorSolucao->size(); i++) {
        cout << melhorSolucao->at(i);
        if(i+1 < melhorSolucao->size())
            cout << ", ";
    }
    cout << endl << "Menor Tamanho de Solucao: " << tamanhoMelhorSolucao << endl;
    cout << "Tamanho da Solucao Media (br): " << br;
    cout << ", Tempo: " << mediaTempoMelhorSolucao/1.657<< "s" << endl << endl;
    delete solucao;
    
    for(int i=0; i<10; i++)
        delete grafos[i];

    system("pause");
}

int main() {
    int opcao;
    do {
        cout << "Escolha uma das opcoes abaixo:" << endl;
        cout << "1 - Executar Exato" << endl;
        cout << endl;

        cin >> opcao;
        cout << endl << endl;
        switch(opcao) {
            case 1: cenarioUm();
            break;

            default: cout << "Opcao invalida" << endl;
            opcao = -1;
            break;
        }
    }while(opcao == -1);
    
   /*Grafo* grafo = new Grafo(8, 5);
   grafo->addAresta(1, 4, 0);
   grafo->addAresta(4, 5, 0);
   grafo->addAresta(2, 3, 1);
   grafo->addAresta(6, 7, 1);
   grafo->addAresta(5, 6, 2);
   grafo->addAresta(0, 7, 2);
   grafo->addAresta(0, 1, 3);
   grafo->addAresta(3, 4, 3);
   grafo->addAresta(1, 2, 4);
   grafo->contracao(3);
   grafo->contracao(4);
   cout << grafo->numVertices << endl;
   delete grafo;
   system("pause");*/
   /*
   Grafo* grafo = new Grafo(6, 5);
   grafo->addAresta(0, 1, 4);
   grafo->addAresta(0, 4, 2);
   grafo->addAresta(1, 2, 0);
   grafo->addAresta(1, 3, 1);
   grafo->addAresta(1, 4, 0);
   grafo->addAresta(1, 5, 4);
   grafo->addAresta(2, 3, 1);
   grafo->addAresta(2, 4, 1);
   grafo->addAresta(3, 4, 3);
   grafo->addAresta(4, 5, 3);
   grafo->contracao(1);
   grafo->contracao(4);
   cout << grafo->numVertices << endl;
   delete grafo;
   system("pause");
   */
}