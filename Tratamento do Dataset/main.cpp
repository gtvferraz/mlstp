#include <sstream>
#include <time.h>
//#include <direct.h> //CRIAR PASTAS MKDIR
//#include <sys/stat.h> 
#include "libs/leitura.h"

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
*/

using namespace std;

bool taNoVetor(vector<int> vetor, int x) {
    for(int i=0; i<vetor.size(); i++)
        if(vetor[i] == x)
            return true;
    return false;
}

void removeCiclosLabel(Grafo* grafo, int label) {
    vector<int> lista;
    vector<int> verticesVisitados;
    vector<int> verticeAnterior;
    vector<Edge*>* arestas;
    int numVertices = grafo->roots.size();
    int numLabels = grafo->edges.size();
    int auxVertice;

    while(verticesVisitados.size() < numVertices) {
        for(int i=0; i<numVertices; i++) {
            if(!taNoVetor(verticesVisitados, i)) {
                lista.push_back(i);
                verticeAnterior.push_back(-1);
                break;
            }
        }

        while(!lista.empty()) {
            arestas = grafo->edgesVertices[lista[0]];
            for(int i=0; i<arestas->size(); i++) {
                if(arestas->at(i)->label == label && arestas->at(i)->getProx(lista[0]) != verticeAnterior[0]) {
                    auxVertice = arestas->at(i)->getProx(lista[0]);
                    if(taNoVetor(lista, auxVertice)) {
                        grafo->removeAresta(arestas->at(i));
                        i--;
                    }
                    else {
                        lista.push_back(auxVertice);
                        verticeAnterior.push_back(lista[0]);
                    }
                }
            }
            verticesVisitados.push_back(lista[0]);
            lista.erase(lista.begin());
            verticeAnterior.erase(verticeAnterior.begin());
        }
    }
}

void removeCiclosMono(string pathLeitura, string pathEscrita) {
    FILE* file;
    Grafo* grafo = nullptr;
    stringstream ss;
 
    cout << "Instancia: " << pathLeitura << endl;
    grafo = carregaInstancias(pathLeitura.c_str());
    for(int i=0; i<grafo->edges.size(); i++)  
        removeCiclosLabel(grafo, i);
    
    //SALVANDO SOLUÇÃO EM ARQUIVO
    vector<int> verticesVisitados;
    vector<int> linha;
    string aux;

    file = fopen(pathEscrita.c_str(), "w+");

    ss.str("");
    ss.clear();
    ss << grafo->roots.size() << " " << grafo->edges.size() << "\n";
    aux = ss.str();
    fputs(aux.c_str(), file);
    
    for(int i=0; i<grafo->edgesVertices.size(); i++) {
        linha.clear();
        for(int j=0; j<grafo->edgesVertices.size()-i; j++)
            linha.push_back(grafo->edges.size());

        for(int j=0; j<grafo->edgesVertices[i]->size(); j++) {
            //cout << grafo->edgesVertices[i]->at(j)->y-i-1 << " " << i << ":" << grafo->edgesVertices[i]->at(j)->x << "-" << grafo->edgesVertices[i]->at(j)->y << endl;
            if (grafo->edgesVertices[i]->at(j)->x >= i)
                linha[grafo->edgesVertices[i]->at(j)->y-i-1] = grafo->edgesVertices[i]->at(j)->label;
        }

        for(int j=0; j<linha.size(); j++) {   
            ss.str("");
            ss.clear();
            ss << linha[j];
            if(j+1 < linha.size())
                ss << " ";
            fputs(ss.str().c_str(), file);
        }
        fputs("\n", file);
    }
    delete grafo;
    fclose(file);
}

void preparaDataset() {
    FILE* file;
    clock_t tempo[2];
    float duracao;
    string pathEscrita;
    string pathLeitura;
    stringstream ss;
    stringstream ss2;

    cout << "----- Remocao de Ciclos Monocromaticos -----" << endl;
    tempo[0] = clock();

    //mkdir("dataset-tratado");
    //mkdir("dataset-tratado/instances");
    
    //GRUPO 1
    
    int n[4] = {20, 30, 40, 50};
    string d[3] = {"ld", "md", "hd"};
    /*
    pathLeitura = "dataset/instances/g1/";
    pathEscrita = "dataset-tratado/instances/g1/";;
    mkdir(pathEscrita.c_str());
    for(int i=0; i<4; i++) {
        ss.str("");
        ss.clear();
        ss << pathEscrita << n[i] << "/";
        mkdir(ss.str().c_str());
        for(int j=0; j<3; j++) {
            ss.str("");
            ss.clear();
            ss << pathEscrita << n[i] << "/" << d[j] << "/";
            mkdir(ss.str().c_str());
            for(int k=0; k<10; k++) {
                ss.str("");
                ss.clear();
                ss << pathLeitura << n[i] << "/" << d[j] << "/" << k;

                ss2.str("");
                ss2.clear();
                ss2 << pathEscrita << n[i] << "/" << d[j] << "/" << k << ".txt";
                removeCiclosMono(ss.str(), ss2.str());
            }
        }
    }
    */
    //GRUPO 2
    int n2[6] = {50, 100, 200, 400, 500, 1000};
    int label[6][4] = {{12, 25, 50, 62}, {25, 50, 100, 125}, {50, 100, 200, 250}, {100, 200, 400, 500}, {125, 250, 500, 625}, {250, 500, 1000, 1250}};

    pathLeitura = "dataset/instances/g2/";
    pathEscrita = "dataset-tratado/instances/g2/";
    //mkdir(pathEscrita.c_str());
    for(int i=5; i<6; i++) {
        ss.str("");
        ss.clear();
        ss << pathEscrita << n2[i] << "/";
        //mkdir(ss.str().c_str());
        for(int j=1; j<3; j++) {
            ss.str("");
            ss.clear();
            ss << pathEscrita << n2[i] << "/" << d[j] << "/";
            //mkdir(ss.str().c_str());
            for(int k=3; k<4; k++) {
                ss.str("");
                ss.clear();
                ss << pathEscrita << n2[i] << "/" << d[j] << "/" << label[i][k] << "/";
                //mkdir(ss.str().c_str());
                for(int l=0; l<10; l++) {
                    ss.str("");
                    ss.clear();
                    ss << pathLeitura << n2[i] << "/" << d[j] << "/" << label[i][k] << "/" << l;

                    ss2.str("");
                    ss2.clear();
                    ss2 << pathEscrita << n2[i] << "/" << d[j] << "/" << label[i][k] << "/" << l << ".txt";
                    removeCiclosMono(ss.str(), ss2.str());
                }
            }
        }
    }
    
    tempo[1] = clock();
    duracao = (float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC;
    cout << "Tempo: " << duracao << "s" << endl << endl;
}

int main() {
    preparaDataset();
}