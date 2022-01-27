#include <iostream>
#include <vector>
#include <bitset>
#include <map>
#include <climits>
#include <chrono>
#include <time.h>
#include <string>
#include <sstream>
#include <random>
#include <algorithm>
#include <math.h>
#include "gurobi_c++.h"
#include "../libs/grafo_listaAdj.h"
#include "../libs/leitura.h"
#include "../libs/utils.h"

using namespace std;

#define NUMALPHAS 6

/*
g++ -m64 -g -O3 -o main.out cpp/main.cpp -I ~/../../opt/gurobi911/linux64/include/ -L ~/../../opt/gurobi911/linux64/lib -l gurobi_c++ -l gurobi91 -lm
./main.out 0 dataset/instances/g2/100/hd/50/0.txt saida.txt 200 50 0
./main.out 1 dataset/instances/g2/100/hd/50/0.txt saida.txt 10 0
./main.out 2 dataset/instances/g2/100/hd/50/0.txt saida.txt
./main.out 3 dataset/instances/g2/100/ld/125/3.txt saida.txt 50 0.9 300 0.001 1 0
./main.out 4 dataset/instances/g2/100/hd/50/0.txt saida.txt 10 0
./main.out 5 dataset/instances/g2/200/ld/200/1.txt saida.txt 1000 0.1 0
./main.out 6 dataset/instances/g2/200/ld/200/1.txt saida.txt

valgrind --tool=callgrind ./main.out 5 dataset/instances/g1/50/md/2.txt saida.txt 1000 0.001 4
kcachegrind callgrind.out.111111
*/

class mycallback: public GRBCallback {
    public:
        int numVars;
        GRBVar* vars;
        GrafoListaAdj* grafo;
        SolucaoParcial* solucaoParcial;
        clock_t tempoInicio;
        int custoOtimo;

        mycallback() {}

        mycallback(int numVars, GRBVar* vars, GrafoListaAdj* grafo, clock_t tempoInicio, int custoOtimo) {
            inicializa(numVars, vars, grafo, tempoInicio, custoOtimo);
        }

        void inicializa(int numVars, GRBVar* vars, GrafoListaAdj* grafo, clock_t tempoInicio, int custoOtimo) {
            this->numVars = numVars;
            this->vars = vars;
            this->grafo = grafo;
            this->tempoInicio = tempoInicio;
            this->custoOtimo = custoOtimo;
        }

        void setSolucaoParcial(SolucaoParcial* solucaoParcial) {
            this->solucaoParcial = solucaoParcial;
        }

    protected:
        void callback () {
            vector<int>* solution;
            stringstream ss;
            vector<vector<int>*>* labelsComp;
            Aresta* atual;
            int numCompConexas;
            
            if(where == GRB_CB_MIPSOL) {
                //cout << "*" ;
                if(solucaoParcial == nullptr) {
                    double* z = getSolution(vars, numVars);

                    solution = new vector<int>;
                    for(int i=0; i<grafo->arestas.size(); i++) {
                        if(z[i] >= 0.99)
                            solution->push_back(i);
                    }
                    numCompConexas = grafo->numCompConexas(solution); //Number of connected components on the graph with the edges labeled by the labels on solution
                    //Verify if the solution graph is connected
                    if(numCompConexas > 1) {
                        labelsComp = grafo->labelsCompConexas(solution);
                        GRBLinExpr sum;
                        for(int i=0; i<labelsComp->size(); i++) {
                            ss.str("");
                            ss.clear();
                            ss << "corte_novo-" << i;

                            sum = 0;
                            for(int j=0; j<grafo->arestas.size(); j++)
                                sum += vars[j] * labelsComp->at(i)->at(j);
                            addLazy(sum >= 1.0);

                            if(labelsComp->size() == 2)
                                break;  
                        }

                        for(int i=0; i<labelsComp->size(); i++)
                            delete labelsComp->at(i);
                        delete labelsComp;
                    }
                    delete solution;
                    delete[] z;
                } else {
                    
                    double* z = getSolution(vars, numVars);

                    solution = new vector<int>;
                    for(int i=0; i<grafo->arestas.size(); i++)
                        if(z[i] >= 0.99 && !solucaoParcial->labels[i])
                            solution->push_back(i);
                    
                    /*
                    labelsComp = grafo->labelsCompConexasParcial(solution, solucaoParcial); //Number of connected components on the graph with the edges labeled by the labels on solution
                    //Verify if the solution graph is connected
                    if(labelsComp != nullptr) {
                        GRBLinExpr sum;
                        for(int i=0; i<labelsComp->size(); i++) {
                            ss.str("");
                            ss.clear();
                            ss << "corte_novo-" << i;

                            sum = 0;
                            for(int j=0; j<grafo->arestas.size(); j++)
                                sum += vars[j] * labelsComp->at(i)->at(j);
                            addLazy(sum >= 1.0);

                            if(labelsComp->size() == 2)
                                break;  
                        }

                        for(int i=0; i<labelsComp->size(); i++)
                            delete labelsComp->at(i);
                        delete labelsComp;
                    } else {
                        int count = 0;
                        for(int i=0; i<grafo->arestas.size(); i++)
                            if(solucaoParcial->labels[i])
                                count++;
                        if(count + solution->size() == 3)
                            cout << count << " - " << solution->size() << " - " << 1000*(float)(clock() - tempoInicio) / CLOCKS_PER_SEC << endl;
                    }
                    */

                    labelsComp = grafo->labelsCompConexasParcial2(solution, solucaoParcial); //Number of connected components on the graph with the edges labeled by the labels on solution
                    //Verify if the solution graph is connected
                    if(labelsComp != nullptr) {
                        GRBLinExpr sum;

                        int numCompConexas = 0;
                        for(int i=0; i<labelsComp->size(); i++)
                            if(labelsComp->at(i)->size() != 0)
                                numCompConexas++;

                        for(int i=0; i<labelsComp->size(); i++) {
                            if(labelsComp->at(i)->size() != 0) {
                                ss.str("");
                                ss.clear();
                                ss << "corte_novo-" << i;

                                sum = 0;
                                for(int j=0; j<labelsComp->at(i)->size(); j++)
                                    sum += vars[labelsComp->at(i)->at(j)];
                                addLazy(sum >= 1.0);

                                if(numCompConexas == 2)
                                    break;  
                            }
                        }

                        for(int i=0; i<labelsComp->size(); i++)
                            delete labelsComp->at(i);
                        delete labelsComp;
                    } /*else {
                        int count = 0;
                        for(int i=0; i<grafo->arestas.size(); i++)
                            if(solucaoParcial->labels[i])
                                count++;
                        cout << "Encontrada solução viavel: " << count << " - " << solution->size() << " - " << 1000*(float)(clock() - tempoInicio) / CLOCKS_PER_SEC << "ms" << endl;
                        if(count + solution->size() == custoOtimo)
                            cout << "--------------------------- ENCONTRADA SOLUÇÃO ÓTIMAAAAAAAAAAAAAAAAAAAAAAAAA -------------------------" << endl;
                    }*/

                    delete solution;
                    delete[] z;
                }
            }
        }
};

void buscaLocalMIP(GrafoListaAdj* grafo, vector<int>* solucao, GRBEnv* env, double* mipGap, int raio) {
    stringstream ss;
    GRBVar* z;
    
    double* lb;
    double* ub;
    double* obj;
    char* type;
    string* variaveis;

    vector<int>* labels;
    
    int numLabels = grafo->arestas.size(); //number of labels on the graph
    GRBModel model = GRBModel(*env);
    model.set(GRB_StringAttr_ModelName, "MLST");
    model.set(GRB_IntParam_OutputFlag, 0);
    model.set(GRB_DoubleParam_TimeLimit, 36000); //10h
    model.set(GRB_IntParam_LazyConstraints, 1);
    //model.set(GRB_IntParam_MIPFocus, 1);
    
    lb = new double[numLabels];
    ub = new double[numLabels];
    obj = new double[numLabels];
    type = new char[numLabels];
    variaveis = new string[numLabels];
    
    for(int i=0; i<numLabels; i++) {
        ss.str("");
        ss.clear();
        ss << "z" << i;
        lb[i] = 0;
        ub[i] = 1;
        obj[i] = 1;
        type[i] = GRB_BINARY;
        variaveis[i] = ss.str();
    }
    
    z = model.addVars(lb, ub, obj, type, variaveis, numLabels);
    
    for(int i=0; i<numLabels; i++)
        z[i].set(GRB_DoubleAttr_Start, 0);
    for(int i=0; i<solucao->size(); i++)
        z[solucao->at(i)].set(GRB_DoubleAttr_Start, 1);

    model.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);
    
    GRBLinExpr sum;

    //add the restriction about each node being present in the solution graph
    for(int i=0; i<grafo->vertices.size(); i++) {
        ss.str("");
        ss.clear();

        ss << "corte-" << i;
        labels = grafo->getLabelsInVertice(i); //Returns the label of all the edges that reach the node i
        sum = 0;

        for(int j=0; j<labels->size(); j++)
            sum += z[j] * labels->at(j);
        
        model.addConstr(sum >= 1.0, ss.str());

        delete labels;
    }
    
    sum = 0;
    for(int i=0; i<numLabels; i++)
        sum += z[i] * grafo->numArestasLabels[i];
    model.addConstr(sum >= grafo->vertices.size()-1, "numero minimo de arestas");
    
    sum = 0;
    for(int i=0; i<solucao->size(); i++) {
        sum += z[solucao->at(i)]; 
    }
    model.addConstr(sum >= solucao->size()-raio, "restrição de vizinhança");

    mycallback cb = mycallback(numLabels, z, grafo, clock(), -1);
    model.setCallback(&cb);

    model.optimize();  
    
    solucao->clear();

    for(int i=0; i<numLabels; i++) {
        if(z[i].get(GRB_DoubleAttr_X) >= 0.99)
            solucao->push_back(i);
    }
    
    *mipGap = model.get(GRB_DoubleAttr_MIPGap);
    
    delete []variaveis;
    delete []lb;
    delete []ub;
    delete []obj;
    delete []type;
    delete []z;
}

void buscaLocalExcedente(GrafoListaAdj* grafo, vector<int>* solucao) {
    int labelRemovido;
    int size = solucao->size();
    for(int i=0; i<size; i++) {
        labelRemovido = solucao->at(0);
        solucao->erase(solucao->begin());
        if(grafo->numCompConexas(solucao) > 1) {
            solucao->push_back(labelRemovido);
        }
    }
}

vector<int>* auxMVCAGRASP(GrafoListaAdj* grafo, int iteracao, float alpha) { 
    int numVertices = grafo->vertices.size();
    int numLabels = grafo->arestas.size();
    int aleatorio;
    int numCompConexas;
    int count;
    vector<int>* solucao;
    vector<int> espacoLabels;
    vector<AuxiliaOrdenacao*> listaOrdenada;

    for(int i=0; i<numLabels; i++) {
        espacoLabels.push_back(i);
    }

    solucao = new vector<int>;
    if(iteracao > 2) {
        aleatorio = rand() % numLabels;
        solucao->push_back(aleatorio);
        espacoLabels.erase(espacoLabels.begin()+aleatorio);
        //if(iteracao < 5)
        //    cout << "Escolhido inicial: " << aleatorio << endl;
    }

    do {
        listaOrdenada.clear();
        solucao->push_back(0);
        for(int i=0; i<espacoLabels.size(); i++) {
            solucao->at(solucao->size()-1) = espacoLabels[i];
            listaOrdenada.push_back(new AuxiliaOrdenacao(grafo->numCompConexas(solucao), i));  
        }
        /*if(iteracao < 5) {
            for(int i=0; i<listaOrdenada.size(); i++)
                cout << espacoLabels[listaOrdenada[i]->posLabel] << "-" << listaOrdenada[i]->numCompConexas << " ";
            cout << endl;
        }*/
        sort(listaOrdenada.begin(), listaOrdenada.end(), compara_sort_b);

        /*if(iteracao < 5) {
            cout << "Depois do sort: " << endl;
            for(int i=0; i<listaOrdenada.size(); i++)
                cout << espacoLabels[listaOrdenada[i]->posLabel] << "-" << listaOrdenada[i]->numCompConexas << " ";
            cout << endl;
        }*/
        
        count = 0;
        for(int i=0; i<listaOrdenada.size(); i++) {
            if(listaOrdenada[i]->numCompConexas > listaOrdenada[0]->numCompConexas*alpha)
                break;
            count++;
        }
        
        aleatorio = rand() % count;
        //aleatorio = rand() % (int)ceil(listaOrdenada.size()*(alpha-1));
        solucao->at(solucao->size()-1) = espacoLabels[listaOrdenada[aleatorio]->posLabel];
        numCompConexas = listaOrdenada[aleatorio]->numCompConexas;
        //if(iteracao < 5)
        //    cout << "Escolhido: " << aleatorio << "-" << espacoLabels[listaOrdenada[aleatorio]->posLabel] << endl;
        espacoLabels.erase(espacoLabels.begin()+listaOrdenada[aleatorio]->posLabel);
        for(int i=0; i<listaOrdenada.size(); i++)
            delete listaOrdenada[i];
    }while(numCompConexas > 1);
    
    return solucao;
}

void auxMVCAGRASP2(GrafoListaAdj* grafo, int iteracao, float alpha, vector<int>* solucao, vector<AuxiliaOrdenacao*>* listaOrdenadaInicial, vector<AuxiliaOrdenacao*>* listaOrdenada) {  
    int numVertices = grafo->vertices.size();
    int numLabels = grafo->arestas.size();
    int aleatorio;
    int numCompConexas;
    int count;

    vector<AuxiliaOrdenacao*>* listaAtual;
    SolucaoParcial* solucoesParciais[numLabels];
    bool labelsSolucao[numLabels];

    for(int i=0; i<numLabels; i++) {
        solucoesParciais[i] = nullptr;
        labelsSolucao[i] = false;
    }

    int storeCompConexas;
    int indice = -1;
    if(iteracao > 2) {
        aleatorio = rand() % numLabels;
        solucao->push_back(aleatorio);
        labelsSolucao[aleatorio] = true;

        for(int i=0; i<numLabels; i++) {
            if(listaOrdenadaInicial->at(i)->posLabel == aleatorio) {
                indice = i;
                storeCompConexas = listaOrdenadaInicial->at(i)->numCompConexas;
                listaOrdenadaInicial->at(i)->numCompConexas = INT_MAX;

                for(int j=0; j<numLabels; j++)
                    if(listaOrdenadaInicial->at(i)->posLabel == listaOrdenada->at(j)->posLabel) {
                        listaOrdenada->at(j)->numCompConexas = INT_MAX;

                        break;
                    }
                
                break;
            }
        }
    }

    int indMenor = 0;
    if(indice == 0)
        indMenor = 1;

    listaAtual = listaOrdenadaInicial;
    solucao->push_back(0);
    bool first = true;
    float somatorio;
    int label;

    do {
        count = 0;
        for(int i=0; i<listaAtual->size(); i++) {
            if(listaAtual->at(i)->numCompConexas != INT_MAX) {
                if(listaAtual->at(i)->numCompConexas > listaAtual->at(indMenor)->numCompConexas*alpha && listaAtual->at(i)->numCompConexas != INT_MAX)
                    break;
                count++;
            }
        }

        indMenor = 0;

        aleatorio = rand() % count;
        while(listaAtual->at(aleatorio)->numCompConexas == INT_MAX)
            aleatorio = (aleatorio+1)%numLabels;

        int selectedLabel = listaAtual->at(aleatorio)->posLabel;
        solucao->back() = selectedLabel;
        labelsSolucao[selectedLabel] = true;
        numCompConexas = listaAtual->at(aleatorio)->numCompConexas;

        if(!first) {
            for(int i=0; i<listaAtual->size(); i++) {
                if(i != selectedLabel && !labelsSolucao[i]) {
                    solucoesParciais[i]->numCompConexas = solucoesParciais[selectedLabel]->numCompConexas;
                    solucoesParciais[i]->compConexa->clear();
                    for(int j=0; j<numVertices; j++) {
                        solucoesParciais[i]->compConexa->push_back(solucoesParciais[selectedLabel]->compConexa->at(j));
                    }
                }
            }
        }

        if(numCompConexas > 1) {
            if(first) {
                for(int i=0; i<numLabels; i++)
                    if(solucao->back() == listaOrdenada->at(i)->posLabel) {
                        listaOrdenada->push_back(listaOrdenada->at(i));
                        listaOrdenada->erase(listaOrdenada->begin()+i);
                        break;
                    }
                first = false;
            }
            else {
                listaOrdenada->push_back(listaOrdenada->at(aleatorio));
                listaOrdenada->erase(listaOrdenada->begin()+aleatorio);
            }

            listaOrdenada->back()->numCompConexas = INT_MAX;

            solucao->push_back(0);
            for(int i=0; i<listaOrdenada->size(); i++) {
                int label = listaOrdenada->at(i)->posLabel;
                
                if(listaOrdenada->at(i)->numCompConexas != INT_MAX) {
                    solucao->back() = label;
                    labelsSolucao[label] = true;
                    
                    if(solucoesParciais[label] == nullptr) {
                        solucoesParciais[label] = grafo->numCompConexas2(labelsSolucao);
                        solucoesParciais[label]->deletaLabels = false;
                        listaOrdenada->at(i)->numCompConexas = solucoesParciais[label]->numCompConexas;
                    } else {
                        listaOrdenada->at(i)->numCompConexas = grafo->updateNumCompConexasParcial(solucao, solucoesParciais[label]);
                    }

                    //listaOrdenada->at(i)->numCompConexas = grafo->numCompConexas(solucao);

                    labelsSolucao[label] = false;
                }
            }
            sort(listaOrdenada->begin(), listaOrdenada->end(), compara_sort_b);
        }

        listaAtual = listaOrdenada;
    } while(numCompConexas > 1);

    if(iteracao > 2)
        listaOrdenadaInicial->at(indice)->numCompConexas = storeCompConexas;

    for(int i=0; i<numLabels; i++) {
        listaOrdenada->at(i)->numCompConexas = 0;

        if(solucoesParciais[i] != nullptr)
            delete solucoesParciais[i];
    }
}

//IMPLEMENTADA A QUESTÃO DE QUANDO UMA COMPONENTE CONEXA POSSUI APENAS UM VÉRTICE, APENAS OS RÓTULOS QUE ATINGEM ESSE VÉRTICE SÃO CONSIDERADOS COMO CANDIDATOS
void auxMVCAGRASP3(GrafoListaAdj* grafo, int iteracao, float alpha, vector<int>* solucao, vector<AuxiliaOrdenacao*>* listaOrdenadaInicial, vector<AuxiliaOrdenacao*>* listaOrdenada) {  
    int numVertices = grafo->vertices.size();
    int numLabels = grafo->arestas.size();
    int aleatorio;
    int numCompConexas;
    int count;
    bool pula;

    vector<AuxiliaOrdenacao*>* listaAtual;
    SolucaoParcial* solucoesParciais[numLabels];
    bool labelsSolucao[numLabels];

    for(int i=0; i<numLabels; i++) {
        solucoesParciais[i] = nullptr;
        labelsSolucao[i] = false;
    }

    int storeCompConexas;
    int indice = -1;
    if(iteracao > 2) {
        aleatorio = rand() % numLabels;
        solucao->push_back(aleatorio);
        labelsSolucao[aleatorio] = true;

        for(int i=0; i<numLabels; i++) {
            if(listaOrdenadaInicial->at(i)->posLabel == aleatorio) {
                indice = i;
                storeCompConexas = listaOrdenadaInicial->at(i)->numCompConexas;
                listaOrdenadaInicial->at(i)->numCompConexas = INT_MAX;

                for(int j=0; j<numLabels; j++)
                    if(listaOrdenadaInicial->at(i)->posLabel == listaOrdenada->at(j)->posLabel) {
                        listaOrdenada->at(j)->numCompConexas = INT_MAX;

                        break;
                    }
                
                break;
            }
        }
    }

    int indMenor = 0;
    if(indice == 0)
        indMenor = 1;

    listaAtual = listaOrdenadaInicial;
    solucao->push_back(0);
    bool first = true;
    float somatorio;
    int label;

    do {
        pula = false;
        if(solucao->size() > 1) {
            SolucaoParcial* solucaoParcial = grafo->numCompConexas2(labelsSolucao);
            solucaoParcial->deletaLabels = false;

            for(int i=0; i<numVertices; i++) {
                int countVert = 0;
                for(int j=0; j<numVertices; j++) {
                    if(solucaoParcial->compConexa->at(j) == i) {
                        countVert++;
                        if(countVert > 1)
                            break;
                    }
                }
                if(countVert == 1) {
                    vector<AuxiliaOrdenacao*> countNumComp;
                    for(int j=0; j<numLabels; j++) {
                        if(!labelsSolucao[j]) {
                            solucao->back() = j;
                            countNumComp.push_back(new AuxiliaOrdenacao(grafo->numCompConexas(solucao), j));
                        }
                    }

                    sort(countNumComp.begin(), countNumComp.end(), compara_sort_b);
                    count = 0;
                    for(int j=0; j<countNumComp.size(); j++) {
                        if(countNumComp[j]->numCompConexas > countNumComp[0]->numCompConexas*alpha)
                            break;
                        count++;
                    }

                    aleatorio = rand()%count;
                    solucao->back() = countNumComp[aleatorio]->posLabel;
                    labelsSolucao[countNumComp[aleatorio]->posLabel] = true;
                    numCompConexas = countNumComp[aleatorio]->numCompConexas;

                    if(numCompConexas > 1)
                        solucao->push_back(0);
                    pula = true;

                    for(int j=0; j<countNumComp.size(); j++)
                        delete countNumComp[j];
                
                    break;
                }
            }

            delete solucaoParcial;
        }

        if(!pula) {
            count = 0;
            for(int i=0; i<listaAtual->size(); i++) {
                if(listaAtual->at(i)->numCompConexas != INT_MAX) {
                    if(listaAtual->at(i)->numCompConexas > listaAtual->at(indMenor)->numCompConexas*alpha && listaAtual->at(i)->numCompConexas != INT_MAX)
                        break;
                    count++;
                }
            }

            indMenor = 0;

            aleatorio = rand() % count;
            while(listaAtual->at(aleatorio)->numCompConexas == INT_MAX)
                aleatorio = (aleatorio+1)%numLabels;

            int selectedLabel = listaAtual->at(aleatorio)->posLabel;
            solucao->back() = selectedLabel;
            labelsSolucao[selectedLabel] = true;
            numCompConexas = listaAtual->at(aleatorio)->numCompConexas;

            if(!first) {
                for(int i=0; i<listaAtual->size(); i++) {
                    if(i != selectedLabel && !labelsSolucao[i]) {
                        solucoesParciais[i]->numCompConexas = solucoesParciais[selectedLabel]->numCompConexas;
                        solucoesParciais[i]->compConexa->clear();
                        for(int j=0; j<numVertices; j++) {
                            solucoesParciais[i]->compConexa->push_back(solucoesParciais[selectedLabel]->compConexa->at(j));
                        }
                    }
                }
            }

            if(numCompConexas > 1) {
                if(first) {
                    for(int i=0; i<numLabels; i++)
                        if(solucao->back() == listaOrdenada->at(i)->posLabel) {
                            listaOrdenada->push_back(listaOrdenada->at(i));
                            listaOrdenada->erase(listaOrdenada->begin()+i);
                            break;
                        }
                    first = false;
                }
                else {
                    listaOrdenada->push_back(listaOrdenada->at(aleatorio));
                    listaOrdenada->erase(listaOrdenada->begin()+aleatorio);
                }

                listaOrdenada->back()->numCompConexas = INT_MAX;

                solucao->push_back(0);
                for(int i=0; i<listaOrdenada->size(); i++) {
                    int label = listaOrdenada->at(i)->posLabel;
                    
                    if(listaOrdenada->at(i)->numCompConexas != INT_MAX) {
                        solucao->back() = label;
                        labelsSolucao[label] = true;
                        
                        if(solucoesParciais[label] == nullptr) {
                            solucoesParciais[label] = grafo->numCompConexas2(labelsSolucao);
                            solucoesParciais[label]->deletaLabels = false;
                            listaOrdenada->at(i)->numCompConexas = solucoesParciais[label]->numCompConexas;
                        } else {
                            listaOrdenada->at(i)->numCompConexas = grafo->updateNumCompConexasParcial(solucao, solucoesParciais[label]);
                        }

                        //listaOrdenada->at(i)->numCompConexas = grafo->numCompConexas(solucao);

                        labelsSolucao[label] = false;
                    }
                }
                sort(listaOrdenada->begin(), listaOrdenada->end(), compara_sort_b);
            }

            listaAtual = listaOrdenada;
        }
    } while(numCompConexas > 1);

    if(iteracao > 2)
        listaOrdenadaInicial->at(indice)->numCompConexas = storeCompConexas;

    for(int i=0; i<numLabels; i++) {
        listaOrdenada->at(i)->numCompConexas = 0;

        if(solucoesParciais[i] != nullptr)
            delete solucoesParciais[i];
    }
}

//IMPLEMENTADA A QUESTÃO DO CONSTRUTIVO QUE UTILIZA A FREQUÊNCIA DO RÓTULO COMO HEURÍSTICA, E É REATIVO
void novoConstrutivo(GrafoListaAdj* grafo, int iteracao, float alpha, vector<int>* solucao, vector<AuxiliaOrdenacao*>* listaOrdenadaInicial, vector<AuxiliaOrdenacao*>* listaOrdenada, vector<int>* countLabels) {  
    int numVertices = grafo->vertices.size();
    int numLabels = grafo->arestas.size();
    int aleatorio;
    int numCompConexas;
    int count;

    vector<AuxiliaOrdenacao*> candidatos;
    bool labelsSolucao[numLabels];

    for(int i=0; i<numLabels; i++) {
        labelsSolucao[i] = false;
    }

    if(iteracao > 2) {
        aleatorio = rand() % numLabels;
        solucao->push_back(aleatorio);
        labelsSolucao[aleatorio] = true;
        countLabels->at(aleatorio)++;
    }

    do {
        solucao->push_back(0);

        for(int i=0; i<numLabels; i++) {
            if(!labelsSolucao[i]) {
                //candidatos.push_back(new AuxiliaOrdenacao(grafo->pesos[i], i));
                candidatos.push_back(new AuxiliaOrdenacao(grafo->pesos[i]*(countLabels->at(i)/2), i));
            }
        }
        sort(candidatos.begin(), candidatos.end(), compara_sort_c);

        count = 0;
        for(int i=0; i<candidatos.size(); i++) {
            if(candidatos[i]->peso > candidatos[0]->peso*alpha)
                break;
            count++;
        }
        aleatorio = rand() % count;

        int selectedLabel = candidatos[aleatorio]->posLabel;
        solucao->back() = selectedLabel;
        labelsSolucao[selectedLabel] = true;
        numCompConexas = grafo->numCompConexas(solucao);
        countLabels->at(selectedLabel)++;

        for(int i=0; i<candidatos.size(); i++)
            delete candidatos[i];
        candidatos.clear();
    } while(numCompConexas > 1);
}

void auxPMVCAGRASP2(GrafoListaAdj* grafo, int iteracao, float alpha, float beta, vector<int>* solucao, vector<AuxiliaOrdenacao*>* listaOrdenadaInicial, vector<AuxiliaOrdenacao*>* listaOrdenada) {  
    int numVertices = grafo->vertices.size();
    int numLabels = grafo->arestas.size();
    int aleatorio;
    int numCompConexas;
    int count;

    vector<AuxiliaOrdenacao*>* listaAtual;
    SolucaoParcial* solucoesParciais[numLabels];
    bool labelsSolucao[numLabels];

    for(int i=0; i<numLabels; i++) {
        solucoesParciais[i] = nullptr;
        labelsSolucao[i] = false;
    }

    int storeCompConexas;
    int indice = -1;
    if(iteracao > 2) {
        aleatorio = rand() % numLabels;
        solucao->push_back(aleatorio);
        labelsSolucao[aleatorio] = true;

        for(int i=0; i<numLabels; i++) {
            if(listaOrdenadaInicial->at(i)->posLabel == aleatorio) {
                indice = i;
                storeCompConexas = listaOrdenadaInicial->at(i)->numCompConexas;
                listaOrdenadaInicial->at(i)->numCompConexas = INT_MAX;

                for(int j=0; j<numLabels; j++)
                    if(listaOrdenadaInicial->at(i)->posLabel == listaOrdenada->at(j)->posLabel) {
                        listaOrdenada->at(j)->numCompConexas = INT_MAX;

                        break;
                    }
                
                break;
            }
        }
    }

    int indMenor = 0;
    if(indice == 0)
        indMenor = 1;

    listaAtual = listaOrdenadaInicial;
    solucao->push_back(0);
    bool first = true;
    float somatorio;
    int label;

    int it = 0;
    do {
        count = 0;

        float theta = 1 + alpha*pow(beta, it);
        for(int i=0; i<listaAtual->size(); i++) {
            if(listaAtual->at(i)->numCompConexas != INT_MAX) {
                if(listaAtual->at(i)->numCompConexas > listaAtual->at(indMenor)->numCompConexas*theta && listaAtual->at(i)->numCompConexas != INT_MAX)
                    break;
                count++;
            }
        }

        indMenor = 0;

        aleatorio = rand() % count;
        while(listaAtual->at(aleatorio)->numCompConexas == INT_MAX)
            aleatorio = (aleatorio+1)%numLabels;

        int selectedLabel = listaAtual->at(aleatorio)->posLabel;
        solucao->back() = selectedLabel;
        labelsSolucao[selectedLabel] = true;
        numCompConexas = listaAtual->at(aleatorio)->numCompConexas;

        if(!first) {
            for(int i=0; i<listaAtual->size(); i++) {
                if(i != selectedLabel && !labelsSolucao[i]) {
                    solucoesParciais[i]->numCompConexas = solucoesParciais[selectedLabel]->numCompConexas;
                    solucoesParciais[i]->compConexa->clear();
                    for(int j=0; j<numVertices; j++) {
                        solucoesParciais[i]->compConexa->push_back(solucoesParciais[selectedLabel]->compConexa->at(j));
                    }
                }
            }
        }

        if(numCompConexas > 1) {
            if(first) {
                for(int i=0; i<numLabels; i++)
                    if(solucao->back() == listaOrdenada->at(i)->posLabel) {
                        listaOrdenada->push_back(listaOrdenada->at(i));
                        listaOrdenada->erase(listaOrdenada->begin()+i);
                        break;
                    }
                first = false;
            }
            else {
                listaOrdenada->push_back(listaOrdenada->at(aleatorio));
                listaOrdenada->erase(listaOrdenada->begin()+aleatorio);
            }

            listaOrdenada->back()->numCompConexas = INT_MAX;

            solucao->push_back(0);
            for(int i=0; i<listaOrdenada->size(); i++) {
                int label = listaOrdenada->at(i)->posLabel;
                
                if(listaOrdenada->at(i)->numCompConexas != INT_MAX) {
                    solucao->back() = label;
                    labelsSolucao[label] = true;
                    
                    if(solucoesParciais[label] == nullptr) {
                        solucoesParciais[label] = grafo->numCompConexas2(labelsSolucao);
                        solucoesParciais[label]->deletaLabels = false;
                        listaOrdenada->at(i)->numCompConexas = solucoesParciais[label]->numCompConexas;
                    } else {
                        listaOrdenada->at(i)->numCompConexas = grafo->updateNumCompConexasParcial(solucao, solucoesParciais[label]);
                    }

                    //listaOrdenada->at(i)->numCompConexas = grafo->numCompConexas(solucao);

                    labelsSolucao[label] = false;
                }
            }
            sort(listaOrdenada->begin(), listaOrdenada->end(), compara_sort_b);
        }

        it++;
        listaAtual = listaOrdenada;
    } while(numCompConexas > 1);

    if(iteracao > 2)
        listaOrdenadaInicial->at(indice)->numCompConexas = storeCompConexas;

    for(int i=0; i<numLabels; i++) {
        listaOrdenada->at(i)->numCompConexas = 0;

        if(solucoesParciais[i] != nullptr)
            delete solucoesParciais[i];
    }
}

void construtivo(GrafoListaAdj* grafo, int iteracao, float alpha, float beta, vector<int>* solucao, vector<LabelInfo*>* labelsInfo, vector<LabelInfo*>* labelsControl, bool compControl[], bool reachComp[], int compConexas[]) {
    auto tempoInicio = std::chrono::high_resolution_clock::now();
    
    int numLabels = grafo->arestas.size();
    int numVertices = grafo->vertices.size();
    int numCompConexas = numVertices;

    for(int i=0; i<numVertices; i++) {
        compConexas[i] = i;
    }

    for(int i=0; i<numLabels; i++)
        labelsControl->at(i)->valido = true;
    
    int randomLabel;
    int count;
    if(iteracao > 2) {
        randomLabel = rand()%numLabels;
    } else {
        count = 1;
        for(int i=1; i<numLabels; i++) {
            if(labelsInfo->at(i)->numCompConexas < labelsInfo->at(0)->numCompConexas*(1-alpha))
                break;
            count++;
        }
        randomLabel = rand()%count;
    }
    solucao->push_back(labelsInfo->at(randomLabel)->label);

    labelsControl->at(randomLabel)->valido = false;
    
    Aresta* aresta = grafo->arestas[labelsInfo->at(randomLabel)->label];
    while(aresta != nullptr) {
        if(compConexas[aresta->origem] != compConexas[aresta->destino]) {
            numCompConexas--;

            int root = compConexas[aresta->destino];
            for(int i=0; i<numVertices; i++) {
                if(compConexas[i] == root)
                    compConexas[i] = compConexas[aresta->origem];
            }
        }
        aresta = aresta->prox;
    }

    while(numCompConexas > 1) {
        for(int i=0; i<numLabels; i++)
            labelsControl->at(i)->numCompConexas = 0;

        for(int i=0; i<numLabels; i++) {
            if(labelsControl->at(i)->valido) {
                for(int j=0; j<numVertices; j++)
                    compControl[j] = false;
                
                for(int j=0; j<numVertices; j++) {
                    if(compControl[j])
                        continue;

                    int compAtual = compConexas[j];

                    for(int k=0; k<numVertices; k++) {
                        if(compConexas[k] == compAtual)
                            reachComp[k] = true;
                        else
                            reachComp[k] = false;
                    }

                    for(int k=0; k<numVertices; k++) {
                        if(compConexas[k] == compAtual) {
                            compControl[k] = true;

                            aresta = grafo->vertices[k]->arestas[labelsControl->at(i)->label];
                            while(aresta != nullptr) {
                                if(!reachComp[aresta->destino]) {
                                    labelsControl->at(i)->numCompConexas += 1;

                                    /*for(int l=0; l<numVertices; l++) {
                                        if(compConexas[l] == compConexas[aresta->destino])
                                            reachComp[l] = true;
                                    }*/
                                }

                                aresta = aresta->prox;
                            }
                        }
                    }
                }
            }
        }

        sort(labelsControl->begin(), labelsControl->end(), comparaLabelsInfo);

        count = 1;
        for(int i=1; i<numLabels; i++) {
            if(labelsControl->at(i)->numCompConexas < labelsControl->at(0)->numCompConexas*(1-alpha))
                break;
            count++;
        }

        /*for(int i=0; i<labelsControl->size(); i++)
            cout << labelsControl->at(i)->label << "-" << labelsControl->at(i)->numCompConexas << " ";
        cout << endl;*/

        randomLabel = rand()%count;
        solucao->push_back(labelsControl->at(randomLabel)->label);

        labelsControl->at(randomLabel)->valido = false;

        /*for(int i=0; i<solucao->size(); i++)
            cout << solucao->at(i) << " ";
        cout << endl;*/

        aresta = grafo->arestas[labelsControl->at(randomLabel)->label];
        while(aresta != nullptr) {
            if(compConexas[aresta->origem] != compConexas[aresta->destino]) {
                numCompConexas--;

                int root = compConexas[aresta->destino];
                for(int i=0; i<numVertices; i++) {
                    if(compConexas[i] == root)
                        compConexas[i] = compConexas[aresta->origem];
                }
            }

            aresta = aresta->prox;
        }

        //cout << "Comp: " << numCompConexas << endl;
    }

    auto diff = std::chrono::high_resolution_clock::now() - tempoInicio;
    auto tempo = std::chrono::duration_cast<std::chrono::microseconds>(diff);

    /*for(int i=0; i<solucao->size(); i++)
        cout << solucao->at(i) << " ";
    cout << endl;*/

    //cout << "Num Comp: " << grafo->numCompConexas(solucao) << " " << solucao->size() << " " << tempo.count()/1000.0 << "ms" << endl;
}

vector<int>* GRASP(GrafoListaAdj* grafo, int iteracoes, clock_t* tempoMelhorSolucao, float* tempoBuscaLocal, int* solucaoConstrutivo, GRBEnv* env, int custoOtimo, int* numIteracoes, int* numSolucoesRepetidas) {
    vector<int>* melhorSolucao = nullptr;
    vector<int>* solucao;
    clock_t tempo[2];
    clock_t auxTempoBuscaLocal[2];
    int tamanhoMelhorSolucao = INT_MAX;
    int auxSolucaoConstrutivo;
    int i;

    bool verifica;
    double mipGap;

    tempo[0] = clock();
    i = 0;
    *numSolucoesRepetidas = 0;
    if(tempoBuscaLocal !=  nullptr)
        *tempoBuscaLocal = 0;
    do {
        solucao = auxMVCAGRASP(grafo, i, 1); 
        auxSolucaoConstrutivo = solucao->size();

        if(solucao->size() == custoOtimo) {
            if(tempoMelhorSolucao != nullptr)
                *tempoMelhorSolucao = clock();
            if(solucaoConstrutivo != nullptr)
                *solucaoConstrutivo = auxSolucaoConstrutivo;
            if(melhorSolucao != nullptr)
                delete melhorSolucao;
            *numIteracoes = i+1;

            return solucao;
        }
        auxTempoBuscaLocal[0] = clock();
        if(solucao->size() > 1)
            buscaLocalMIP(grafo, solucao, env, &mipGap, 2);
        auxTempoBuscaLocal[1] = clock();
        if(tempoBuscaLocal !=  nullptr)
            *tempoBuscaLocal += (float)(auxTempoBuscaLocal[1] - auxTempoBuscaLocal[0]) / CLOCKS_PER_SEC;

        if(solucao->size() < tamanhoMelhorSolucao) {
            if(tempoMelhorSolucao != nullptr)
                *tempoMelhorSolucao = clock();
            if(melhorSolucao != nullptr)
                delete melhorSolucao;
            melhorSolucao = solucao;
            if(solucaoConstrutivo != nullptr)
                *solucaoConstrutivo = auxSolucaoConstrutivo;

            if(solucao->size() == custoOtimo) {
                *numIteracoes = i+1;
                return melhorSolucao;
            }

            tamanhoMelhorSolucao = solucao->size();
        }
        else
            delete solucao;
        tempo[1] = clock();
        
        i++;
    } while(i < iteracoes);

    *numIteracoes = i;
    //cout << "Numero de Iteracoes: " << i << endl;
    return melhorSolucao;
}

vector<int>* pertubacao(GrafoListaAdj* grafo, vector<int>* solucao, float* tempoBuscaLocal, bool* valida, float alpha, float beta, GRBEnv* env, vector<SolucaoParcial*>* parciais) {
    int aleatorio;
    int maxArestas;
    int numCompConexas;
    int numLabels;
    int count;
    int totalArestas;
    int totalProb;
    int acumulada;
    bool adicionou;
    float tempo;
    bool auxRetirados[solucao->size()];
    vector<AuxiliaOrdenacao*> listaOrdenada;
    vector<int> espacoLabels;
    vector<int> retirados;
    
    numLabels = grafo->arestas.size();
    bool* solucaoParcial = new bool[numLabels];
    for(int i=0; i<numLabels; i++)
        solucaoParcial[i] = false;
    
    //REMOVE 20% DOS LABELS
    /*
    for(int i=0; i<solucao->size(); i++) { 
        if(rand() % 100 >= 80) //Com 70 a 200-0.2-200.2 foi
            retirados.push_back(solucao->at(i));
        else
            solucaoParcial->push_back(solucao->at(i));
    }*/
    
    //FIM REMOVE 20% DOS LABELS
    //REMOVE 20% DOS LABELS, ONDE QUANTO MENOS ARESTAS O LABEL POSSUI, MAIOR A CHANCE DELE SER REMOVIDO
    
    totalArestas = 0;
    for(int i=0; i<solucao->size(); i++) {
        totalArestas += grafo->numArestasLabels[solucao->at(i)];
        auxRetirados[i] = true;
    }

    for(int i=0; i<ceil(solucao->size()*beta); i++) {
    //for(int i=0; i<ceil(solucao->size()*0.8); i++) {
        aleatorio = rand() % totalArestas;
        acumulada = 0;
        for(int j=0; j<solucao->size(); j++) {
            acumulada += grafo->numArestasLabels[solucao->at(j)];

            if(acumulada > aleatorio) {
                if(auxRetirados[j]) {
                    solucaoParcial[solucao->at(j)] = true;
                    //retirados.push_back(solucao->at(j));
                    auxRetirados[j] = false;
                } 
                /*else {
                    for(int k=1; k<solucao->size(); k++) {
                        int indice = (j+k)%solucao->size();
                        if(auxRetirados[indice]) {
                            solucaoParcial->push_back(solucao->at(indice));
                            //retirados.push_back(solucao->at(j));
                            auxRetirados[indice] = false;
                            break;
                        }
                    }
                }*/
                break;
            }
        }
    }

    for(int i=0; i<solucao->size(); i++)
        if(auxRetirados[i])
            retirados.push_back(solucao->at(i));
            //solucaoParcial->push_back(solucao->at(i));
    
    //FIM REMOVE 20% DOS LABELS, ONDE QUANTO MENOS ARESTAS O LABEL POSSUI, MAIOR A CHANCE DELE SER REMOVIDO
    
    for(int i=0; i<numLabels; i++) {
        //if(!taNoVetor(solucaoParcial, i) && !taNoVetor(&retirados, i)) {
        if(!solucaoParcial[i]) {
            espacoLabels.push_back(i);
        }
    }
    
    SolucaoParcial* novaSolucao = nullptr;
    bool verifica;
    for(int i=0; i<parciais->size(); i++) {
        verifica = true;
        for(int j=0; j<numLabels; j++)
            if(solucaoParcial[j] != parciais->at(i)->labels[j]) {
                verifica = false;
                break;
            }
        if(verifica) {
            delete []solucaoParcial;
            novaSolucao = parciais->at(i);
            break;
        }
    }
    if(novaSolucao == nullptr) {
        novaSolucao = grafo->numCompConexas2(solucaoParcial);
        parciais->push_back(novaSolucao);
    }

    vector<int>* vizinha = new vector<int>;
    do {
        if(espacoLabels.empty()) {
            *valida = false;
            for(int i=0; i<numLabels; i++)
                if(novaSolucao->labels[i])
                    vizinha->push_back(i);
            return vizinha;
        }
            listaOrdenada.clear();
            vizinha->push_back(0);
            for(int i=0; i<espacoLabels.size(); i++) {
                vizinha->at(vizinha->size()-1) = espacoLabels[i];
                //listaOrdenada.push_back(new AuxiliaOrdenacao(grafo->numCompConexas(vizinha), i)); 
                listaOrdenada.push_back(new AuxiliaOrdenacao(grafo->numCompConexasParcial(vizinha, novaSolucao), i));  
                /*int aux = grafo->numCompConexas(vizinha);
                if(aux != listaOrdenada[i]->numCompConexas) {
                    cout << "Componentes: " <<  aux << ", " << listaOrdenada[i]->numCompConexas << endl;
                    for(int j=0; j<vizinha->size(); j++)
                        cout << vizinha->at(j) << " ";
                    cout << endl;
                }*/
            }
            sort(listaOrdenada.begin(), listaOrdenada.end(), compara_sort_b);
            
            count = 0;
            for(int i=0; i<listaOrdenada.size(); i++) {
                if(listaOrdenada[i]->numCompConexas > listaOrdenada[0]->numCompConexas*alpha)
                    break;
                count++;
            }
            aleatorio = rand() % count;
            //aleatorio = rand() % (int)ceil(listaOrdenada.size()*(alpha-1));

            vizinha->at(vizinha->size()-1) = espacoLabels[listaOrdenada[aleatorio]->posLabel];
            espacoLabels.erase(espacoLabels.begin()+listaOrdenada[aleatorio]->posLabel);

            numCompConexas = listaOrdenada[aleatorio]->numCompConexas;
            for(int i=0; i<listaOrdenada.size(); i++)
                delete listaOrdenada[i];
    }while(numCompConexas > 1);
        

    //double mipGap;
    tempo = clock();
    buscaLocalExcedente(grafo, vizinha);
    /*if(solucao->size() > 1)
        buscaLocalMIP(grafo, vizinha, env, &mipGap, 2);*/
    *tempoBuscaLocal += ((float)(clock() - tempo));

    *valida = true;

    for(int i=0; i<numLabels; i++)
        if(novaSolucao->labels[i])
            vizinha->push_back(i);
    
    return vizinha;
}

vector<int>* SA(GrafoListaAdj* grafo, vector<int>* initialSolution, double tempInicial, double tempFinal, int numIteracoes, double alpha, clock_t* tempoMelhorSolucao, float* tempoBuscaLocal, int custoOtimo, int* numSolucoes, int* numSolucoesRepetidas, GRBEnv* env, int* parciaisRepetidas) {
    vector<int>* solucao;
    vector<int>* novaSolucao;
    vector<int>* melhorSolucao;
    clock_t tempo[2];
    stringstream ss;
    double temp;
    double prob;
    float tempoSolucaoInicial;
    float auxTempoBuscaLocal;
    float tempoSA;
    int solucaoConstrutivo;
    int difQualidade;
    bool aux;
    bool valida;

    double mipGap = 50;

    solucao = initialSolution;
    melhorSolucao = initialSolution;
    
    temp = tempInicial;
    aux = true;
    int count;
    int count2;
    double novaTempInicial;
    count2 = 0;
    *tempoBuscaLocal = 0;

    int numAlphas = 6;
    float alphas[numAlphas] = {1.05, 1.10, 1.15, 1.20, 1.25, 1.3};
    float acumulada;
    int aleatorio;
    int indiceAlpha;
    float sumMediaCustoAlphas;
    vector<float> probAlphas;
    vector<int> countAlphas;
    vector<int> sumCustoSolucoes;

    for(int i=0; i<numAlphas; i++)
        probAlphas.push_back(100.0/numAlphas);

    *numSolucoesRepetidas = 0;
    *numSolucoes = 0;
    /*
    vector<vector<int>> contabiliza;  
    
    vector<int> a;
    for(int j=0; j<solucao->size(); j++)
        a.push_back(solucao->at(j));
    contabiliza.push_back(a);*/

    vector<SolucaoParcial*>* parciais = new vector<SolucaoParcial*>;
    float beta;
    int menorCusto = initialSolution->size();
    while(temp > tempFinal) {
        /*
        cout << "Temp: " << temp  << ", " << solucao->size() << ", ";
        for(int i=0; i<solucao->size(); i++)
            cout << solucao->at(i) << " ";
        cout << endl;*/
        count = 0;
        /*for(int i=0; i<numAlphas; i++)
            cout << probAlphas[i] << " ";
        cout << endl;*/
        beta = -0.00033*temp + 0.9;
        for(int i=0; i<numIteracoes; i++) {       
            if(*numSolucoes < numAlphas) {
                novaSolucao = pertubacao(grafo, solucao, tempoBuscaLocal, &valida, alphas[*numSolucoes], beta, env, parciais);
                //novaSolucao = pertubacao(grafo, solucao, &tempoBuscaLocal, &valida, alphas[0]);
                countAlphas.push_back(1);
                sumCustoSolucoes.push_back(novaSolucao->size());
            }
            else {
                aleatorio = rand() % 100;
                acumulada = 0;
                for(int j=0; j<numAlphas; j++) {
                    acumulada += probAlphas[j];
                    if(aleatorio < acumulada) {
                        indiceAlpha = j;
                        break;
                    }
                }
                novaSolucao = pertubacao(grafo, solucao, tempoBuscaLocal, &valida, alphas[indiceAlpha], beta, env, parciais);
                //novaSolucao = pertubacao(grafo, solucao, &tempoBuscaLocal, &valida, alphas[0]);
                countAlphas[indiceAlpha]++;
                sumCustoSolucoes[indiceAlpha] += novaSolucao->size();
            }
            
            *numSolucoes += 1;
            
            if(!valida) {
                //CONTABILIZAR QUANTAS SOLUÇÕES INVÁLIDAS SÃO GERADAS PELA PERTUBAÇÃO
                *numSolucoesRepetidas += 1;
                count++;
                continue;
            }
            /*
            vector<int> a;
            bool testa = false;
            for(int j=0; j<novaSolucao->size(); j++)
                a.push_back(novaSolucao->at(j));
            for(int j=0; j<contabiliza.size(); j++) {
                if(a.size() != contabiliza[j].size())
                    continue;
                testa = true;
                for(int k=0; k<a.size(); k++)
                    if(!taNoVetor(&(contabiliza[j]), a[k])) {
                        testa = false;
                        break;
                    }
                if(testa) {
                    *numSolucoesRepetidas += 1;
                    break;
                }
            }
            if(!testa)
                contabiliza.push_back(a);
                */

            difQualidade = novaSolucao->size() - solucao->size();
            if(difQualidade <= 0) {
                count++;
                if(!aux)
                    delete solucao;
                solucao = novaSolucao;

                if(solucao->size() < melhorSolucao->size()) {
                    //cout << "A: " << solucao->size() << " - " << melhorSolucao->size() << endl;
                    *tempoMelhorSolucao = clock();

                    aux = true;
                    delete melhorSolucao;
                    melhorSolucao = solucao;
                        
                    if(melhorSolucao->size() == custoOtimo) {
                        *tempoBuscaLocal /= CLOCKS_PER_SEC;
                        *tempoBuscaLocal *= 1000;

                        if(!aux)
                            delete solucao;
                        
                        *parciaisRepetidas = *numSolucoes - parciais->size();
                        for(int i=0; i<parciais->size(); i++)
                            delete parciais->at(i);
                        delete parciais;

                        return melhorSolucao;
                    }
                } else 
                    aux = false;
            } else {
                prob = 1.0/(exp(difQualidade/temp));
                if(rand()%100 >= 100-(prob*100)) {
                    count++;
                    if(!aux)
                        delete solucao;
                    solucao = novaSolucao;
                    aux = false;
                } else
                    delete novaSolucao;
            }
        }
        sumMediaCustoAlphas = 0;
        for(int j=0; j<numAlphas; j++)
            sumMediaCustoAlphas += 1.0/pow(5,sumCustoSolucoes[j]/countAlphas[j]);
        for(int j=0; j<numAlphas; j++)
            probAlphas[j] = (1/(sumMediaCustoAlphas * pow(5,sumCustoSolucoes[j]/countAlphas[j]))) * 100;

        /*
        if(count <= (double)numIteracoes*0.95) {
            if(count2 == 0)
                novaTempInicial = temp;
            count2++;
        } else
            count2 = 0;
        if(count2 >= 10)
            cout << novaTempInicial << endl;*/
        /*auxTempoBuscaLocal = clock();
        if(solucao->size() > 1)
            buscaLocalMIP(grafo, solucao, env, &mipGap, 2);
        *tempoBuscaLocal += (float)(clock() - auxTempoBuscaLocal);
        if(solucao->size() < melhorSolucao->size()) {
            *tempoMelhorSolucao = clock();
            cout << "B: " << solucao->size() << " - " << melhorSolucao->size() << endl;
            delete melhorSolucao;
            melhorSolucao = solucao;
            aux = true;

            if(melhorSolucao->size() == custoOtimo) {
                *tempoBuscaLocal /= CLOCKS_PER_SEC;
                *tempoBuscaLocal *= 1000;

                if(!aux)
                    delete solucao;

                for(int i=0; i<parciais->size(); i++)
                    delete parciais->at(i);
                delete parciais;

                return melhorSolucao;
            }
        }*/
        temp = alpha*temp;
    }
    //cout << "TOTAL: " << contabiliza.size() << ", de " << total << endl;
    //cout << "REPETIDOS: " << repetidos << endl;

    /*
    Grafo *g = new Grafo(grafo->roots.size(), grafo->edges.size());
    for(int i=0; i<melhorSolucao->size(); i++)
        g->addLabel(grafo->edges, melhorSolucao->at(i));
    cout << "NumComp: " << g->numCompConexas << endl;
    delete g;*/

    *tempoBuscaLocal /= CLOCKS_PER_SEC;
    *tempoBuscaLocal *= 1000;

    if(!aux)
        delete solucao;
    
    *parciaisRepetidas = *numSolucoes - parciais->size();
    for(int i=0; i<parciais->size(); i++)
        delete parciais->at(i);
    delete parciais;

    return melhorSolucao;
}

vector<int>* mip(GrafoListaAdj* grafo, SolucaoParcial* partialSolution, vector<int>* initialSolution, GRBModel* model, GRBVar* z, mycallback* cb, double* mipGap, clock_t tempoInicio, int custoOtimo) {
    vector<int>* labelsSolucao; //labels in the solution
    
    int numLabels = grafo->arestas.size(); //number of labels on the graph
    
    model->reset(1);
    
    //GRBLinExpr sum = 0;
    for(int i=0; i<numLabels; i++) {
        z[i].set(GRB_DoubleAttr_Start, 0);
        //sum += z[i];
        if(partialSolution->labels[i])
            z[i].set(GRB_DoubleAttr_LB, 0.99);
        else
            z[i].set(GRB_DoubleAttr_LB, 0);
    }
    //GRBConstr restricao = model->addConstr(sum >= initialSolution->size()-1, "remover apenas um label da solução");
    /*int labelsConstr[numLabels];
    for(int i=0; i<numLabels; i++)
        labelsConstr[i] = 1;*/
    for(int i=0; i<initialSolution->size(); i++) {
        z[initialSolution->at(i)].set(GRB_DoubleAttr_Start, 1);
        //labelsConstr[initialSolution->at(i)] = 0;
    }
    /*sum = 0;
    for(int i=0; i<numLabels; i++)
        sum += z[i] * labelsConstr[i];
    GRBConstr restricao2 = model->addConstr(sum >= 1.0, "solucao diferente da inicial");*/

    tempoInicio = clock();

    cb->setSolucaoParcial(partialSolution);
    model->optimize();    
    //model->remove(restricao);
    //model->remove(restricao2);

    labelsSolucao = new vector<int>;
    for(int i=0; i<numLabels; i++) {
        try{
            if(z[i].get(GRB_DoubleAttr_X) >= 0.99)
                labelsSolucao->push_back(i);
        } catch(GRBException e) {
            cout << e.getErrorCode() << endl;

            delete labelsSolucao;
            return nullptr;
        }
    }

    //if(labelsSolucao->size() == custoOtimo)
    //    cout << "Tempo de saída do mip: " << 1000*(float)(clock() - tempoInicio) / CLOCKS_PER_SEC << "ms" << endl << endl << endl;

    *mipGap = model->get(GRB_DoubleAttr_MIPGap);

    return labelsSolucao;
}

GRBVar* constroiModelo(GrafoListaAdj* grafo, GRBModel* model, mycallback* cb, clock_t tempoInicio, int custoOtimo) {
    GRBVar* z;

    model->set(GRB_StringAttr_ModelName, "MLST");
    model->set(GRB_IntParam_OutputFlag, 0);
    model->set(GRB_DoubleParam_TimeLimit, 30); //Tempo de limite de 10h
    //model->set(GRB_DoubleParam_MIPGap, 0.05); //Termina quando o gap estiver menor que 5%
    model->set(GRB_IntParam_LazyConstraints, 1);
    //model->set(GRB_IntParam_MIPFocus, 1); //Foco em achar soluções viáveis
    
    double* lb;
    double* ub;
    double* obj;
    char* type;
    string* variaveis;
    stringstream ss;

    int numLabels = grafo->arestas.size();

    lb = new double[numLabels];
    ub = new double[numLabels];
    obj = new double[numLabels];
    type = new char[numLabels];
    variaveis = new string[numLabels];
    
    for(int i=0; i<numLabels; i++) {
        ss.str("");
        ss.clear();
        ss << "z" << i;
        lb[i] = 0;
        ub[i] = 1;
        obj[i] = 1;
        type[i] = GRB_BINARY;
        variaveis[i] = ss.str();
    }
    
    z = model->addVars(lb, ub, obj, type, variaveis, numLabels);
    GRBLinExpr sum;

    model->set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);
    //model->addConstr(sum <= initialSolution->size()-1, "solucao final diferente da inicial");
    
    //add the restriction about each node being present in the solution graph
    vector<int>* labels;
    for(int i=0; i<grafo->vertices.size(); i++) {
        ss.str("");
        ss.clear();

        ss << "corte-" << i;
        labels = grafo->getLabelsInVertice(i); //Returns the label of all the edges that reach the node i
        sum = 0;

        for(int j=0; j<labels->size(); j++)
            sum += z[j] * labels->at(j);
        
        model->addConstr(sum >= 1.0, ss.str());

        delete labels;
    }

    sum = 0;
    for(int i=0; i<numLabels; i++)
        sum += z[i] * grafo->numArestasLabels[i];
    model->addConstr(sum >= grafo->vertices.size()-1, "numero minimo de arestas");

    cb->inicializa(numLabels, z, grafo, tempoInicio, custoOtimo);
    model->setCallback(cb);

    delete []variaveis;
    delete []lb;
    delete []ub;
    delete []obj;
    delete []type;

    return z;
}

vector<int>* mipFluxo(GrafoListaAdj* grafo, SolucaoParcial* partialSolution, vector<int>* initialSolution, GRBModel* model, GRBVar* z) {
    vector<int>* labelsSolucao; //labels in the solution
    
    int numLabels = grafo->arestas.size();
    int numVertices = grafo->vertices.size();
    int numArestas = grafo->numTotalArestas;
    int numArcos = 2*numArestas;
        
    int count = 0;
    for(int i=0; i<numLabels; i++) {
        z[i].set(GRB_DoubleAttr_Start, 0);
        if(partialSolution->labels[i]) {
            count++;
            z[i].set(GRB_DoubleAttr_LB, 0.99);
        } else
            z[i].set(GRB_DoubleAttr_LB, 0);
    }

    for(int i=0; i<initialSolution->size(); i++) {
        z[initialSolution->at(i)].set(GRB_DoubleAttr_Start, 1);
    }

    //cout << "PARCIAL: " << count << endl;

    model->optimize();    

    labelsSolucao = new vector<int>;
    for(int i=0; i<numLabels; i++) {
        try{
            if(z[i].get(GRB_DoubleAttr_X) >= 0.99)
                labelsSolucao->push_back(i);
        } catch(GRBException e) {
            cout << e.getErrorCode() << endl;

            delete labelsSolucao;
            return nullptr;
        }
    }

    return labelsSolucao;
}

GRBVar* constroiModeloFluxo(GrafoListaAdj* grafo, GRBModel* model) {
    model->set(GRB_StringAttr_ModelName, "MLST");
    model->set(GRB_IntParam_OutputFlag, 0);
    model->set(GRB_DoubleParam_TimeLimit, 30); //Tempo de limite de 10h
    //model->set(GRB_DoubleParam_MIPGap, 0.05); //Termina quando o gap estiver menor que 5%
    //model->set(GRB_IntParam_MIPFocus, 1); //Foco em achar soluções viáveis
    
    double* lb;
    double* ub;
    double* obj;
    char* type;
    string* variaveis;
    stringstream ss;

    double* lbY;
    double* ubY;
    double* objY;
    char* typeY;
    string* variaveisY;
    stringstream ssY;

    int numLabels = grafo->arestas.size();
    int numVertices = grafo->vertices.size();
    int numArestas = grafo->numTotalArestas;
    int numArcos = 2*numArestas;

    lb = new double[numLabels];
    ub = new double[numLabels];
    obj = new double[numLabels];
    type = new char[numLabels];
    variaveis = new string[numLabels];
    
    for(int i=0; i<numLabels; i++) {
        ss.str("");
        ss.clear();
        ss << "z" << i;
        lb[i] = 0;
        ub[i] = 1;
        obj[i] = 1;
        type[i] = GRB_BINARY;
        variaveis[i] = ss.str();
    }
    
    GRBVar* x;
    GRBVar* y;
    GRBVar* z;
    z = model->addVars(lb, ub, obj, type, variaveis, numLabels);

    delete []variaveis;
    delete []lb;
    delete []ub;
    delete []obj;
    delete []type;

    lb = new double[numArcos];
    ub = new double[numArcos];
    obj = new double[numArcos];
    type = new char[numArcos];
    variaveis = new string[numArcos];

    lbY = new double[numArcos];
    ubY = new double[numArcos];
    objY = new double[numArcos];
    typeY = new char[numArcos];
    variaveisY = new string[numArcos];
    
    int arcosCount = 0;
    for(int i=0; i<numArcos; i++) {
        ss.str("");
        ss.clear();
        ss << "x" << i;
        lb[i] = 0;
        ub[i] = 1;
        obj[i] = 0;
        type[i] = GRB_BINARY;
        variaveis[i] = ss.str();

        ssY.str("");
        ssY.clear();
        ssY << "y" << i;
        lbY[i] = 0;
        ubY[i] = numVertices-1;
        objY[i] = 0;
        typeY[i] = GRB_CONTINUOUS;
        variaveisY[i] = ss.str();
    }

    x = model->addVars(lb, ub, obj, type, variaveis, numArcos);
    y = model->addVars(lbY, ubY, objY, typeY, variaveisY, numArcos);

    delete []variaveis;
    delete []lb;
    delete []ub;
    delete []obj;
    delete []type;

    delete []variaveisY;
    delete []lbY;
    delete []ubY;
    delete []objY;
    delete []typeY;

    model->set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);

    GRBLinExpr sum;
    
    //add the restriction about each node being present in the solution graph
    /*vector<int>* labels;
    for(int i=0; i<numVertices; i++) {
        ss.str("");
        ss.clear();

        ss << "corte-" << i;
        labels = grafo->getLabelsInVertice(i); //Returns the label of all the edges that reach the node i
        sum = 0;

        for(int j=0; j<labels->size(); j++)
            sum += z[j] * labels->at(j);
        
        model->addConstr(sum >= 1.0, ss.str());

        delete labels;
    }*/

    sum = 0;
    for(int i=0; i<numLabels; i++)
        sum += z[i] * grafo->numArestasLabels[i];
    model->addConstr(sum >= numVertices-1, "numero minimo de arestas");

    GRBLinExpr sumOut = 0;
    GRBLinExpr sumIn = 0;
    int arcoId;

    Aresta* aresta;
    for(int i=0; i<grafo->vertices[0]->arestas.size(); i++) {
        aresta = grafo->vertices[0]->arestas[i];
        while(aresta != nullptr) {
            sumOut += y[aresta->arcoId];
            if(aresta->arcoId%2 == 0) {
                sumIn += y[aresta->arcoId+1];
            } else {
                sumIn += y[aresta->arcoId-1];
            }

            aresta = aresta->prox;
        }
    }

    ss.str("");
    ss << "fluxo deixado no vertice " << 0;
    model->addConstr(sumOut - sumIn == numVertices-1, ss.str());

    sum = 0;
    for(int i=0; i<numVertices; i++) {
        sumOut = 0;
        sumIn = 0;
        
        for(int j=0; j<grafo->vertices[i]->arestas.size(); j++) {
            aresta = grafo->vertices[i]->arestas[j];
            while(aresta != nullptr) {
                ss.str("");
                ss << "fluxo maximo no arco " << aresta->arcoId;
                model->addConstr(y[aresta->arcoId] <= (numVertices-1)*x[aresta->arcoId], ss.str());

                ss.str("");
                ss << "relacao entre labels e arcos " << aresta->arcoId;
                model->addConstr(z[aresta->label] >= x[aresta->arcoId], ss.str());

                if(aresta->arcoId%2 == 0) {
                    sumOut += y[aresta->arcoId];
                    sumIn += y[aresta->arcoId+1];
                } else {
                    sumOut += y[aresta->arcoId];
                    sumIn += y[aresta->arcoId-1];
                }

                aresta = aresta->prox;
            }
        }
        if(i > 0) {
            ss.str("");
            ss << "fluxo deixado no vertice " << i;
            model->addConstr(sumIn - sumOut == 1, ss.str());
        }
    }

    return z;
}

GRBVar* constroiModeloFluxo2(GrafoListaAdj* grafo, GRBModel* model) {
    model->set(GRB_StringAttr_ModelName, "MLST");
    model->set(GRB_IntParam_OutputFlag, 0);
    model->set(GRB_DoubleParam_TimeLimit, 30); //Tempo de limite de 10h
    //model->set(GRB_DoubleParam_MIPGap, 0.05); //Termina quando o gap estiver menor que 5%
    //model->set(GRB_IntParam_MIPFocus, 1); //Foco em achar soluções viáveis
    
    double* lb;
    double* ub;
    double* obj;
    char* type;
    string* variaveis;
    stringstream ss;

    double* lbY;
    double* ubY;
    double* objY;
    char* typeY;
    string* variaveisY;
    stringstream ssY;

    int numLabels = grafo->arestas.size();
    int numVertices = grafo->vertices.size();
    int numArestas = grafo->numTotalArestas;
    int numArcos = 2*numArestas;

    lb = new double[numLabels];
    ub = new double[numLabels];
    obj = new double[numLabels];
    type = new char[numLabels];
    variaveis = new string[numLabels];
    
    for(int i=0; i<numLabels; i++) {
        ss.str("");
        ss.clear();
        ss << "z" << i;
        lb[i] = 0;
        ub[i] = 1;
        obj[i] = 1;
        type[i] = GRB_BINARY;
        variaveis[i] = ss.str();
    }
    
    GRBVar* x;
    GRBVar* y;
    GRBVar* z;
    z = model->addVars(lb, ub, obj, type, variaveis, numLabels);

    delete []variaveis;
    delete []lb;
    delete []ub;
    delete []obj;
    delete []type;

    lb = new double[numArcos];
    ub = new double[numArcos];
    obj = new double[numArcos];
    type = new char[numArcos];
    variaveis = new string[numArcos];

    lbY = new double[numArcos*(numVertices-1)];
    ubY = new double[numArcos*(numVertices-1)];
    objY = new double[numArcos*(numVertices-1)];
    typeY = new char[numArcos*(numVertices-1)];
    variaveisY = new string[numArcos*(numVertices-1)];

    int arcosCount = 0;
    for(int i=0; i<numArcos; i++) {
        ss.str("");
        ss.clear();
        ss << "x" << i;
        lb[i] = 0;
        ub[i] = 1;
        obj[i] = 0;
        type[i] = GRB_CONTINUOUS;
        variaveis[i] = ss.str();

        for(int k=1; k<numVertices; k++) {
            int index = i*(numVertices-1)+k-1;
            ssY.str("");
            ssY.clear();
            ssY << "y" << index;
            lbY[index] = 0;
            ubY[index] = 1;
            objY[index] = 0;
            typeY[index] = GRB_CONTINUOUS;
            variaveisY[index] = ss.str();
        }
    }

    x = model->addVars(lb, ub, obj, type, variaveis, numArcos);
    y = model->addVars(lbY, ubY, objY, typeY, variaveisY, numArcos*(numVertices-1));

    delete []variaveis;
    delete []lb;
    delete []ub;
    delete []obj;
    delete []type;

    delete []variaveisY;
    delete []lbY;
    delete []ubY;
    delete []objY;
    delete []typeY;

    model->set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);

    GRBLinExpr sumOut = 0;
    GRBLinExpr sumIn = 0;
    Aresta* aresta;

    sumIn = 0;
    for(int i=0; i<numLabels; i++)
        sumIn += z[i] * grafo->numArestasLabels[i];
    model->addConstr(sumIn >= numVertices-1, "numero minimo de arestas");

    for(int k=1; k<numVertices; k++) {
        for(int i=1; i<numVertices; i++) {
            sumOut = 0;
            sumIn = 0;
            for(int j=0; j<grafo->vertices[i]->arestas.size(); j++) {
                aresta = grafo->vertices[i]->arestas[j];
                while(aresta != nullptr) {
                    int index = (k-1)*numArcos+aresta->arcoId;
                    
                    if(index%2 == 0) {
                        sumOut += y[index];
                        sumIn += y[index+1];
                    } else {
                        sumOut += y[index];
                        sumIn += y[index-1];
                    }

                    model->addConstr(y[index] <= x[aresta->arcoId]);
                    model->addConstr(z[aresta->label] >= x[aresta->arcoId]);

                    aresta = aresta->prox;
                }
            }
            if(i == k) {
                model->addConstr(sumIn - sumOut == 1);
            } else {
                model->addConstr(sumIn - sumOut == 0);
            }
        }

        sumOut = 0;
        sumIn = 0;
        for(int j=0; j<grafo->vertices[0]->arestas.size(); j++) {
            aresta = grafo->vertices[0]->arestas[j];
            while(aresta != nullptr) {
                int index = (k-1)*numArcos+aresta->arcoId;

                if(index%2 == 0) {
                    sumOut += y[index];
                    sumIn += y[index+1];
                } else {
                    sumOut += y[index];
                    sumIn += y[index-1];
                }
                model->addConstr(y[index] <= x[aresta->arcoId]);
                model->addConstr(z[aresta->label] >= x[aresta->arcoId]);

                aresta = aresta->prox;
            }
        }
        model->addConstr(sumOut - sumIn == 1);
    }

    return z;
}

GRBVar* constroiModeloFluxo3(GrafoListaAdj* grafo, GRBModel* model) {
    model->set(GRB_StringAttr_ModelName, "MLST");
    model->set(GRB_IntParam_OutputFlag, 0);
    model->set(GRB_DoubleParam_TimeLimit, 30); //Tempo de limite de 10h
    //model->set(GRB_DoubleParam_MIPGap, 0.05); //Termina quando o gap estiver menor que 5%
    //model->set(GRB_IntParam_MIPFocus, 1); //Foco em achar soluções viáveis
    
    double* lb;
    double* ub;
    double* obj;
    char* type;
    string* variaveis;
    stringstream ss;

    double* lbY;
    double* ubY;
    double* objY;
    char* typeY;
    string* variaveisY;
    stringstream ssY;

    int numLabels = grafo->arestas.size();
    int numVertices = grafo->vertices.size();
    int numArestas = grafo->numTotalArestas;
    int numArcos = 2*numArestas;

    lb = new double[numLabels];
    ub = new double[numLabels];
    obj = new double[numLabels];
    type = new char[numLabels];
    variaveis = new string[numLabels];
    
    for(int i=0; i<numLabels; i++) {
        ss.str("");
        ss.clear();
        ss << "z" << i;
        lb[i] = 0;
        ub[i] = 1;
        obj[i] = 1;
        type[i] = GRB_BINARY;
        variaveis[i] = ss.str();
    }
    
    GRBVar* x;
    GRBVar* y;
    GRBVar* z;
    z = model->addVars(lb, ub, obj, type, variaveis, numLabels);

    delete []variaveis;
    delete []lb;
    delete []ub;
    delete []obj;
    delete []type;

    lb = new double[numArcos];
    ub = new double[numArcos];
    obj = new double[numArcos];
    type = new char[numArcos];
    variaveis = new string[numArcos];

    int arcosCount = 0;
    for(int i=0; i<numArcos; i++) {
        ss.str("");
        ss.clear();
        ss << "x" << i;
        lb[i] = 0;
        ub[i] = 1;
        obj[i] = 0;
        type[i] = GRB_CONTINUOUS;
        variaveis[i] = ss.str();
    }

    lbY = new double[numVertices];
    ubY = new double[numVertices];
    objY = new double[numVertices];
    typeY = new char[numVertices];
    variaveisY = new string[numVertices];

    for(int i=0; i<numVertices; i++) {
        ssY.str("");
        ssY.clear();
        ssY << "y" << i;
        lbY[i] = 0;
        ubY[i] = GRB_INFINITY;
        objY[i] = 0;
        typeY[i] = GRB_CONTINUOUS;
        variaveisY[i] = ss.str();
    }

    x = model->addVars(lb, ub, obj, type, variaveis, numArcos);
    y = model->addVars(lbY, ubY, objY, typeY, variaveisY, numVertices);

    delete []variaveis;
    delete []lb;
    delete []ub;
    delete []obj;
    delete []type;

    delete []variaveisY;
    delete []lbY;
    delete []ubY;
    delete []objY;
    delete []typeY;

    model->set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);

    GRBLinExpr sumOut = 0;
    Aresta* aresta;

    for(int i=1; i<numVertices; i++) {
        sumOut = 0;
        for(int j=0; j<grafo->vertices[i]->arestas.size(); j++) {
            aresta = grafo->vertices[i]->arestas[j];
            while(aresta != nullptr) {                
                sumOut += x[aresta->arcoId];

                model->addConstr(y[i] >= y[aresta->destino] + x[aresta->arcoId] - numVertices*(1 - x[aresta->arcoId]));
                model->addConstr(z[aresta->label] >= x[aresta->arcoId]);

                aresta = aresta->prox;
            }
        }
        model->addConstr(sumOut == 1);
    }

    sumOut = 0;
    for(int j=0; j<grafo->vertices[0]->arestas.size(); j++) {
        aresta = grafo->vertices[0]->arestas[j];
        while(aresta != nullptr) {            
            sumOut += x[aresta->arcoId];

            model->addConstr(z[aresta->label] >= x[aresta->arcoId]);

            aresta = aresta->prox;
        }
    }
    model->addConstr(sumOut == 0);
    model->addConstr(y[0] == 0);

    return z;
}

vector<int>* pertubacaoMIP(GrafoListaAdj* grafo, vector<int>* solucao, float* tempoMip, float alpha, GRBModel* model, GRBVar* z, mycallback* cb, vector<SolucaoParcial*>* parciais, clock_t tempoInicio, int custoOtimo, vector<int>* vetNumCompConexas) {
    int aleatorio;
    int numLabels;
    int totalArestas;
    int acumulada;
    
    numLabels = grafo->arestas.size();
    bool* solucaoParcial = new bool[numLabels];
    
    for(int i=0; i<numLabels; i++)
        solucaoParcial[i] = false;
    for(int i=0; i<solucao->size(); i++)
        solucaoParcial[solucao->at(i)] = true;

    //REMOVE 20% DOS LABELS
    /*for(int i=0; i<max((int)ceil(solucao->size()*alpha),1); i++) { 
    //for(int i=0; i<alpha; i++) { 
        aleatorio = rand()%solucao->size();
        if(solucaoParcial[solucao->at(aleatorio)])
            solucaoParcial[solucao->at(aleatorio)] = false;
        else
            i--;
    }*/
    //FIM REMOVE 20% DOS LABELS
    //REMOVE 20% DOS LABELS, ONDE QUANTO MENOS ARESTAS O LABEL POSSUI, MAIOR A CHANCE DELE SER REMOVIDO
    totalArestas = 0;
    int numArestasLabels[solucao->size()];
    for(int i=0; i<solucao->size(); i++) {
        numArestasLabels[i] = round(1000.0/grafo->numArestasLabels[solucao->at(i)]);
        totalArestas += numArestasLabels[i];
    }

    //for(int i=0; i<min(max((int)ceil(solucao->size()*alpha),1), 3); i++) { // PARA REMOVER NO MÍNIMO UM RÓTULO E NO MÁXIMO TRÊS
    for(int i=0; i<max((int)ceil(solucao->size()*alpha),1); i++) {
        aleatorio = (rand()%totalArestas) + 1;
        acumulada = 0;
        for(int j=0; j<solucao->size(); j++) {
            acumulada += numArestasLabels[j];

            if(acumulada >= aleatorio) {
                solucaoParcial[solucao->at(j)] = false;
                totalArestas -= numArestasLabels[j];
                numArestasLabels[j] = 0;
                break;
            }
        }
    }
    /*for(int i=0; i<numLabels; i++)
        solucaoParcial[i] = false;

    totalArestas = 0;
    int numArestasLabels[solucao->size()];
    for(int i=0; i<solucao->size(); i++) {
        numArestasLabels[i] = grafo->numArestasLabels[solucao->at(i)];
        totalArestas += numArestasLabels[i];
    }

    for(int i=0; i<min((int)ceil(solucao->size()*alpha),(int)(solucao->size()-1)); i++) {
        aleatorio = (rand()%totalArestas) + 1;
        acumulada = 0;
        for(int j=0; j<solucao->size(); j++) {
            acumulada += numArestasLabels[j];

            if(acumulada >= aleatorio) {
                solucaoParcial[solucao->at(j)] = true;
                totalArestas -= numArestasLabels[j];
                numArestasLabels[j] = 0;
                break;
            }
        }
    }*/
    //FIM REMOVE 20% DOS LABELS, ONDE QUANTO MENOS ARESTAS O LABEL POSSUI, MAIOR A CHANCE DELE SER REMOVIDO
    
    /*cout << "Original: ";
    for(int i=0; i<solucao->size(); i++)
        cout << solucao->at(i) << " ";
    cout << endl << "Parcial: ";
    for(int i=0; i<numLabels; i++)
        if(solucaoParcial[i])   
            cout << i << " ";
    cout << endl;
    cout << "Tam: " << parciais->size() << endl; */

    SolucaoParcial* novaSolucao = nullptr;
    bool verifica;
    for(int i=0; i<parciais->size(); i++) {
        verifica = true;
        for(int j=0; j<numLabels; j++)
            if(solucaoParcial[j] != parciais->at(i)->labels[j]) {
                verifica = false;
                vetNumCompConexas->push_back(parciais->at(i)->numCompConexas);
                break;
            }
        if(verifica) {
            delete []solucaoParcial;
            return nullptr;
        }
    }

    novaSolucao = grafo->numCompConexas3(solucaoParcial);
    parciais->push_back(novaSolucao);
    vetNumCompConexas->push_back(novaSolucao->numCompConexas);
    
    vector<int>* vizinha;
    if(novaSolucao->numCompConexas != 1) {
        double mipGap;
        int count = 0;
        for(int i=0; i<grafo->arestas.size(); i++)
            if(novaSolucao->labels[i])
                count++;
        auto start = std::chrono::high_resolution_clock::now();
        //vizinha = mipFluxo(grafo, novaSolucao, solucao, model, z);
        vizinha = mip(grafo, novaSolucao, solucao, model, z, cb, &mipGap, tempoInicio, custoOtimo);
        auto diff = std::chrono::high_resolution_clock::now() - start;
        auto t1 = std::chrono::duration_cast<std::chrono::microseconds>(diff);
        *tempoMip += t1.count()/1000.0;
    } else {
        vizinha = new vector<int>;
        for(int i=0; i<numLabels; i++) {
            if(novaSolucao->labels[i])
                vizinha->push_back(i);
        }
    }

    return vizinha;
}

vector<int>* IG(GrafoListaAdj* grafo, vector<int>* initialSolution, int numIteracoes, int numSolucoesConst, int numTentativas, float betas[2], std::chrono::high_resolution_clock::time_point* tempoMelhorSolucao, float* tempoMip, int custoOtimo, int* numSolucoes, int* numSolucoesRepetidas, GRBEnv* env, int* numParciaisRepetidas, clock_t tempoInicio, float *mediaItMelhora, float *minItMelhora, float *maxItMelhora, vector<int>* vetNumCompConexas) {
    vector<int>* solucao;
    vector<int>* novaSolucao;
    vector<int>* melhorSolucao;
    clock_t tempo[2];
    float tempoSolucaoInicial;
    int solucaoConstrutivo;
    bool aux;
    double mipGap;
    float acumulada;
    int aleatorio;
    int numLabels = grafo->arestas.size();

    vector<SolucaoParcial*>* parciais = new vector<SolucaoParcial*>;
    vector<int> countLabels;

    for(int i=0; i<numLabels; i++) {
        countLabels.push_back(1);
    }

    //vector<vector<int>> contabiliza;  
    vector<int> a;
    //for(int j=0; j<initialSolution->size(); j++)
    //    a.push_back(initialSolution->at(j));
    //contabiliza.push_back(a);

    vector<vector<int>*> construtivas;  
    construtivas.push_back(initialSolution);

    *numSolucoes = 0;
    *numSolucoesRepetidas = 0;
    solucao = new vector<int>;
    vector<AuxiliaOrdenacao*>* listaOrdenada = new vector<AuxiliaOrdenacao*>;
    vector<AuxiliaOrdenacao*>* listaOrdenadaInicial = new vector<AuxiliaOrdenacao*>;
    //int numSorteios[numLabels];
    //float tickets[numLabels];

    solucao->push_back(0);
    int numCompConexas;
    for(int i=0; i<numLabels; i++) {
        solucao->back() = i;

        numCompConexas = grafo->numCompConexas(solucao);
        AuxiliaOrdenacao* auxilia = new AuxiliaOrdenacao(numCompConexas, i);
        listaOrdenadaInicial->push_back(auxilia);

        auxilia = new AuxiliaOrdenacao(numCompConexas, i);
        listaOrdenada->push_back(auxilia);

        //numSorteios[i] = 1;
    }
    sort(listaOrdenadaInicial->begin(), listaOrdenadaInicial->end(), compara_sort_b);
    solucao->clear();

    int numAlphas = 6;
    float alphas[numAlphas] = {0.0, 0.05, 0.10, 0.15, 0.20, 0.25};
    //float alphas[numAlphas] = {0.6, 0.65, 0.70, 0.75, 0.80, 0.85};
    int indiceAlpha;
    float sumMediaCustoAlphas;
    vector<float> probAlphas;
    vector<int> countAlphas;
    vector<int> sumCustoAlphas;

    /*int numBetas = 4;
    float betas[numBetas] = {0.3, 0.4, 0.5, 0.6};
    int indiceBeta;
    float sumMediaCustoBetas;
    vector<float> probBetas;
    vector<int> countBetas;
    vector<int> sumCustoBetas;*/

    for(int i=0; i<numAlphas; i++)
        probAlphas.push_back(100.0/numAlphas);

    for(int i=0; i<numAlphas; i++) {
        countAlphas.push_back(1);
        sumCustoAlphas.push_back(initialSolution->size());
    }

    /*for(int i=0; i<numBetas; i++)
        probBetas.push_back(100.0/numBetas);

    for(int i=0; i<numBetas; i++) {
        countBetas.push_back(1);
        sumCustoBetas.push_back(initialSolution->size());
    }*/

    *mediaItMelhora = 0;
    *minItMelhora = 0;
    *maxItMelhora = 0;

    //int numSolucoesConst = 200;
    int iteracaoMelhor = 0;
    melhorSolucao = initialSolution;
    for(int i=0; i<numIteracoes; i++) {
        *numSolucoes += 1;

        aleatorio = rand() % 100;
        acumulada = 0;
        for(int j=0; j<numAlphas; j++) {
            acumulada += probAlphas[j];
            if(aleatorio < acumulada) {
                indiceAlpha = j;
                break;
            }
        }

        /*aleatorio = rand() % 100;
        acumulada = 0;
        for(int j=0; j<numBetas; j++) {
            acumulada += probBetas[j];
            if(aleatorio < acumulada) {
                indiceBeta = j;
                break;
            }
        }*/

        //auxMVCAGRASP2(grafo, i, 1+alphas[indiceAlpha], solucao, listaOrdenadaInicial, listaOrdenada);
        auxMVCAGRASP3(grafo, i, 1+alphas[indiceAlpha], solucao, listaOrdenadaInicial, listaOrdenada);
        //novoConstrutivo(grafo, i, 1+alphas[indiceAlpha], solucao, listaOrdenadaInicial, listaOrdenada, &countLabels);
        //auxPMVCAGRASP2(grafo, i, 1+alphas[indiceAlpha], betas[indiceBeta], solucao, listaOrdenadaInicial, listaOrdenada);
        buscaLocalExcedente(grafo, solucao);

        countAlphas[indiceAlpha]++;
        sumCustoAlphas[indiceAlpha] += solucao->size(); 

        /*countBetas[indiceBeta]++;
        sumCustoBetas[indiceBeta] += solucao->size();*/

        if(solucao->size() < melhorSolucao->size()) {
            *tempoMelhorSolucao = std::chrono::high_resolution_clock::now();
            melhorSolucao = solucao;
            iteracaoMelhor = construtivas.size();

            if(solucao->size() == custoOtimo) {
                for(int j=0; j<construtivas.size(); j++)
                    delete construtivas[j];

                for(int i=0; i<numLabels; i++) {
                    delete listaOrdenada->at(i);
                    delete listaOrdenadaInicial->at(i);
                }
                delete listaOrdenada;
                delete listaOrdenadaInicial;

                return solucao;
            }

            construtivas.push_back(solucao);
            if(construtivas.size() == numSolucoesConst) {
                break;
            } else 
                solucao = new vector<int>;
        } else {
            aux = true;
            for(int j=0; j<construtivas.size(); j++)
                if(solucoesIguais(solucao, construtivas[j])) {
                    aux = false;
                    *numSolucoesRepetidas += 1;
                    solucao->clear();
                    break;
                }

            if(aux) {
                construtivas.push_back(solucao);
                if(construtivas.size() == numSolucoesConst) {
                    break;
                } else 
                    solucao = new vector<int>;
            }
        }

        if(i != 0 && i%50 == 0) {
            sumMediaCustoAlphas = 0;
            for(int j=0; j<numAlphas; j++) 
                sumMediaCustoAlphas += 1.0/pow(5,sumCustoAlphas[j]/countAlphas[j]);
            for(int j=0; j<numAlphas; j++)
                probAlphas[j] = (1/(sumMediaCustoAlphas * pow(5,sumCustoAlphas[j]/countAlphas[j]))) * 100;
        
            /*sumMediaCustoBetas = 0;
            for(int j=0; j<numBetas; j++) 
                sumMediaCustoBetas += 1.0/pow(5,sumCustoBetas[j]/countBetas[j]);
            for(int j=0; j<numBetas; j++)
                probBetas[j] = (1/(sumMediaCustoBetas * pow(5,sumCustoBetas[j]/countBetas[j]))) * 100;*/
        }
    }

    /*for(int i=0; i<construtivas.size(); i++) {
        vector<int> a;
        for(int j=0; j<construtivas[i]->size(); j++)
            a.push_back(construtivas[i]->at(j));
        contabiliza.push_back(a);
    }*/

    for(int i=0; i<numLabels; i++) {
        delete listaOrdenada->at(i);
        delete listaOrdenadaInicial->at(i);
    }
    delete listaOrdenada;
    delete listaOrdenadaInicial;
    if(construtivas.size() < numSolucoesConst)
        delete solucao;

    //alphas[0] = 0.4;
    //alphas[1] = 0.6;
    alphas[0] = betas[0];
    alphas[1] = betas[1];
    /*alphas[0] = 1;
    alphas[1] = 2;*/
    numAlphas = 2;
    
    for(int i=0; i<numAlphas; i++)
        probAlphas[i] = 100.0/numAlphas;

    for(int i=0; i<numAlphas; i++) {
        countAlphas[i] = 1;
        sumCustoAlphas[i] = initialSolution->size();
    }

    aux = true;
    *numParciaisRepetidas = 0;
    *mediaItMelhora = 0;
    *minItMelhora = INT_MAX;
    *maxItMelhora = 0;
    int tentativas;
    int iteracao = 0;
    int countMelhoras = 0;

    //CRIAÇÃO DO MODELO MIP
    auto start = std::chrono::high_resolution_clock::now();
    mycallback cb;
    GRBModel model = GRBModel(*env);
    GRBVar* z;

    z = constroiModelo(grafo, &model, &cb, tempoInicio, custoOtimo);
    //z = constroiModeloFluxo(grafo, &model);
    //z = constroiModeloFluxo2(grafo, &model);
    //z = constroiModeloFluxo3(grafo, &model);
    auto diff = std::chrono::high_resolution_clock::now() - start;
    auto t1 = std::chrono::duration_cast<std::chrono::microseconds>(diff);
    *tempoMip = t1.count()/1000.0;
    //FIM DA CRIAÇÃO DO MODELO MIP

    for(int i=0; i<construtivas.size(); i++) { 
        tentativas = 0;
        while(tentativas < numTentativas) { 
            solucao = construtivas[i];

            aleatorio = rand() % 100;
            acumulada = 0;
            for(int j=0; j<numAlphas; j++) {
                acumulada += probAlphas[j];
                if(aleatorio < acumulada) {
                    indiceAlpha = j;
                    break;
                }
            }
            novaSolucao = pertubacaoMIP(grafo, solucao, tempoMip, alphas[indiceAlpha], &model, z, &cb, parciais, tempoInicio, custoOtimo, vetNumCompConexas);
            *numSolucoes += 1;
            
            if(novaSolucao != nullptr) { 
                /*vector<int> a;
                bool testa = false;
                for(int j=0; j<novaSolucao->size(); j++)
                    a.push_back(novaSolucao->at(j));
                for(int j=0; j<contabiliza.size(); j++) {
                    if(a.size() != contabiliza[j].size())
                        continue;
                    testa = true;
                    for(int k=0; k<a.size(); k++)
                        if(!taNoVetor(&(contabiliza[j]), a[k])) {
                            testa = false;
                            break;
                        }
                    if(testa) {
                        *numSolucoesRepetidas += 1;
                        break;
                    }
                }
                if(!testa)
                    contabiliza.push_back(a);*/

                countAlphas[indiceAlpha]++;
                sumCustoAlphas[indiceAlpha] += novaSolucao->size(); 

                //if(!aux)
                //    delete solucao;
                if(novaSolucao->size() < construtivas[i]->size()) {
                    delete construtivas[i];

                    countMelhoras++;
                    *mediaItMelhora += tentativas+1;

                    if(tentativas+1 < *minItMelhora)
                        *minItMelhora = tentativas+1;

                    if(tentativas+1 > *maxItMelhora)
                        *maxItMelhora = tentativas+1;

                    tentativas = -1; //recebe -1 pois tem o tentativas++ no final da iteração, dessa forma ele vai pra zero
                    construtivas[i] = novaSolucao;
                    if(novaSolucao->size() < melhorSolucao->size()) {
                        *tempoMelhorSolucao = std::chrono::high_resolution_clock::now();
                        iteracaoMelhor = i;
                        
                        aux = true;
                        melhorSolucao = novaSolucao;

                        if(melhorSolucao->size() == custoOtimo) {
                            for(int j=0; j<parciais->size(); j++)
                                delete parciais->at(j);
                            delete parciais;

                            *mediaItMelhora /= countMelhoras;

                            delete []z;

                            return melhorSolucao;
                        }
                    }
                } else {
                    delete novaSolucao;
                    //aux = false;
                }
            } else  {
                *numParciaisRepetidas += 1;
            }

            /*
            auxTempoBuscaLocal = clock();
            if(novaSolucao->size() > 1)
                buscaLocalMIP(grafo, novaSolucao, env, &mipGap, 2);
            *tempoBuscaLocal += (float)(clock() - auxTempoBuscaLocal);

            if(!aux) {
                if(novaSolucao->size() < melhorSolucao->size()) {
                    *tempoMelhorSolucao = std::chrono::high_resolution_clock::now();
                    delete melhorSolucao;
                    aux = true;
                    melhorSolucao = novaSolucao;

                    if(melhorSolucao->size() == custoOtimo) {
                        *parciaisGeradas = parciais->size();
                        for(int i=0; i<parciais->size(); i++)
                            delete parciais->at(i);
                        delete parciais;

                        delete []z;

                        return melhorSolucao;
                    }
                } else
                    aux = false;
            }*/

            if(iteracao != 0 && iteracao%50 == 0) {
                sumMediaCustoAlphas = 0;
                for(int j=0; j<numAlphas; j++) 
                    sumMediaCustoAlphas += 1.0/pow(5,sumCustoAlphas[j]/countAlphas[j]);
                for(int j=0; j<numAlphas; j++)
                    probAlphas[j] = (1/(sumMediaCustoAlphas * pow(5,sumCustoAlphas[j]/countAlphas[j]))) * 100;
            }

            //solucao = auxMVCAGRASP(grafo, i, 1);
            //solucao = novaSolucao;

            tentativas++;
            iteracao++;
        }
    }

    //cout << "TAM PARCIAIS: " << parciais->size() << endl;

    if(*mediaItMelhora == 0) {
        *minItMelhora = 0;
    }

    if(countMelhoras > 0)
        *mediaItMelhora /= countMelhoras;

    /*for(int j=0; j<6; j++)
        cout << " " << probAlphas[j];
    cout << endl;*/
    for(int i=0; i<parciais->size(); i++) {
        /*for(int j=0; j<grafo->arestas.size(); j++)
            if(parciais->at(i)->labels[j])
                cout << " " << j;
        cout << endl;*/

        delete parciais->at(i);
    }
    delete parciais;
    //começa em i=1 para pular a initialSolution que já é deletada quando tem delete na melhorSolucao, pois melhorSolucao é inicializada com initialSolution
    for(int i=0; i<construtivas.size(); i++)
        if(iteracaoMelhor != i)
            delete construtivas[i];

    delete []z;

    if(!aux)
        delete novaSolucao;
    return melhorSolucao;
}

vector<int>* IG2(GrafoListaAdj* grafo, vector<int>* initialSolution, int numIteracoes, std::chrono::high_resolution_clock::time_point* tempoMelhorSolucao, float* tempoMip, int custoOtimo, int* numSolucoes, int* numSolucoesRepetidas, GRBEnv* env, int* numParciaisRepetidas, clock_t tempoInicio, float *mediaItMelhora, float *minItMelhora, float *maxItMelhora, vector<int>* vetNumCompConexas, float* avgDiff, float* minDiff, float* maxDiff) {
    vector<int>* solucao;
    vector<int>* melhorSolucao;
    clock_t tempo[2];
    float tempoSolucaoInicial;
    int solucaoConstrutivo;
    bool aux;
    double mipGap;
    float acumulada;
    int aleatorio;
    int numLabels = grafo->arestas.size();

    *numSolucoes = 0;
    *numSolucoesRepetidas = 0;
    solucao = new vector<int>;
    vector<AuxiliaOrdenacao*>* listaOrdenada = new vector<AuxiliaOrdenacao*>;
    vector<AuxiliaOrdenacao*>* listaOrdenadaInicial = new vector<AuxiliaOrdenacao*>;

    solucao->push_back(0);
    int numCompConexas;
    for(int i=0; i<numLabels; i++) {
        solucao->back() = i;

        numCompConexas = grafo->numCompConexas(solucao);
        AuxiliaOrdenacao* auxilia = new AuxiliaOrdenacao(numCompConexas, i);
        listaOrdenadaInicial->push_back(auxilia);

        auxilia = new AuxiliaOrdenacao(numCompConexas, i);
        listaOrdenada->push_back(auxilia);
    }
    sort(listaOrdenadaInicial->begin(), listaOrdenadaInicial->end(), compara_sort_b);
    delete solucao;

    int numAlphas = 6;
    float alphas[numAlphas] = {0.0, 0.05, 0.10, 0.15, 0.20, 0.25};
    int indiceAlpha;
    float sumMediaCustoAlphas;
    vector<float> probAlphas;
    vector<int> countAlphas;
    vector<int> sumCustoAlphas;

    int numBetas = 4;
    float betas[numBetas] = {0.3, 0.4, 0.5, 0.6};
    int indiceBeta;
    float sumMediaCustoBetas;
    vector<float> probBetas;
    vector<int> countBetas;
    vector<int> sumCustoBetas;

    for(int i=0; i<numAlphas; i++)
        probAlphas.push_back(100.0/numAlphas);

    for(int i=0; i<numAlphas; i++) {
        countAlphas.push_back(1);
        sumCustoAlphas.push_back(initialSolution->size());
    }

    for(int i=0; i<numBetas; i++)
        probBetas.push_back(100.0/numBetas);

    for(int i=0; i<numBetas; i++) {
        countBetas.push_back(1);
        sumCustoBetas.push_back(initialSolution->size());
    }

    int indMelhorSol = 0;
    vector<vector<int>*> solucoesConstr;

    *avgDiff = 0;
    *maxDiff = 0;
    *minDiff = (float)INT_MAX;

    int sumDiff;
    int quantDiff = 0;

    *mediaItMelhora = 0;
    *minItMelhora = 0;
    *maxItMelhora = 0;

    int numSolucoesConst = 1000;
    int iteracaoMelhor = 0;
    melhorSolucao = initialSolution;
    for(int i=0; i<numIteracoes; i++) {
        *numSolucoes += 1;

        solucao = new vector<int>;

        aleatorio = rand() % 100;
        acumulada = 0;
        for(int j=0; j<numAlphas; j++) {
            acumulada += probAlphas[j];
            if(aleatorio < acumulada) {
                indiceAlpha = j;
                break;
            }
        }

        aleatorio = rand() % 100;
        acumulada = 0;
        for(int j=0; j<numBetas; j++) {
            acumulada += probBetas[j];
            if(aleatorio < acumulada) {
                indiceBeta = j;
                break;
            }
        }

        auxPMVCAGRASP2(grafo, i, alphas[indiceAlpha], betas[indiceBeta], solucao, listaOrdenadaInicial, listaOrdenada);
        buscaLocalExcedente(grafo, solucao);

        countAlphas[indiceAlpha]++;
        sumCustoAlphas[indiceAlpha] += solucao->size(); 

        countBetas[indiceBeta]++;
        sumCustoBetas[indiceBeta] += solucao->size(); 

        bool checkSolRep = false;

        for(int j=0; j<solucoesConstr.size(); j++) {
            if(solucoesIguais(solucoesConstr[j], solucao)) {
                checkSolRep = true;
                break;
            }
        }

        if(!checkSolRep) {
            for(int j=0; j<solucoesConstr.size(); j++) {
                sumDiff = 0;
                for(int k=0; k<solucao->size(); k++) {
                    bool checkLabel = true;

                    for(int l=0; l<solucoesConstr[j]->size(); l++) {
                        if(solucao->at(k) == solucoesConstr[j]->at(l)) {
                            checkLabel = false;
                            break;
                        }
                    }

                    if(checkLabel)
                        sumDiff++;
                }

                for(int k=0; k<solucoesConstr[j]->size(); k++) {
                    bool checkLabel = true;

                    for(int l=0; l<solucao->size(); l++) {
                        if(solucoesConstr[j]->at(k) == solucao->at(l)) {
                            checkLabel = false;
                            break;
                        }
                    }

                    if(checkLabel)
                        sumDiff++;
                }

                if(sumDiff < *minDiff)
                    *minDiff = sumDiff;
                if(sumDiff > *maxDiff)
                    *maxDiff = sumDiff;

                *avgDiff += sumDiff;
                quantDiff++;
            }

            solucoesConstr.push_back(solucao);
        }

        if(solucao->size() < melhorSolucao->size()) {
            *tempoMelhorSolucao = std::chrono::high_resolution_clock::now();

            //delete melhorSolucao;
            indMelhorSol = solucoesConstr.size()-1;
            melhorSolucao = solucao;
            
            if(solucao->size() == custoOtimo) {
                for(int i=0; i<numLabels; i++) {
                    delete listaOrdenada->at(i);
                    delete listaOrdenadaInicial->at(i);
                }
                delete listaOrdenada;
                delete listaOrdenadaInicial;

                for(int i=0; i<solucoesConstr.size(); i++) {
                    if(i != indMelhorSol)
                        delete solucoesConstr[i];
                }

                *avgDiff /= quantDiff;

                return melhorSolucao;
            }
        } else {
            if(checkSolRep)
                delete solucao;
        }

        if(i != 0 && i%50 == 0) {
            sumMediaCustoAlphas = 0;
            for(int j=0; j<numAlphas; j++) 
                sumMediaCustoAlphas += 1.0/pow(5,sumCustoAlphas[j]/countAlphas[j]);
            for(int j=0; j<numAlphas; j++)
                probAlphas[j] = (1/(sumMediaCustoAlphas * pow(5,sumCustoAlphas[j]/countAlphas[j]))) * 100;
        
            sumMediaCustoBetas = 0;
            for(int j=0; j<numBetas; j++) 
                sumMediaCustoBetas += 1.0/pow(5,sumCustoBetas[j]/countBetas[j]);
            for(int j=0; j<numBetas; j++)
                probBetas[j] = (1/(sumMediaCustoBetas * pow(5,sumCustoBetas[j]/countBetas[j]))) * 100;
        }
    }

    for(int i=0; i<numLabels; i++) {
        delete listaOrdenada->at(i);
        delete listaOrdenadaInicial->at(i);
    }
    delete listaOrdenada;
    delete listaOrdenadaInicial;

    for(int i=0; i<solucoesConstr.size(); i++) {
        if(i != indMelhorSol)
            delete solucoesConstr[i];
    }

    *avgDiff /= quantDiff;

    return melhorSolucao;
}

vector<int>* IG3(GrafoListaAdj* grafo, vector<int>* initialSolution, int numIteracoes, std::chrono::high_resolution_clock::time_point* tempoMelhorSolucao, float* tempoMip, int custoOtimo, int* numSolucoes, int* numSolucoesRepetidas, GRBEnv* env, int* numParciaisRepetidas, clock_t tempoInicio, float *mediaItMelhora, float *minItMelhora, float *maxItMelhora, vector<int>* vetNumCompConexas, float* avgDiff, float* minDiff, float* maxDiff) {
    vector<int>* solucao;
    vector<int>* melhorSolucao;
    clock_t tempo[2];
    float tempoSolucaoInicial;
    int solucaoConstrutivo;
    bool aux;
    double mipGap;
    float acumulada;
    int aleatorio;

    int numLabels = grafo->arestas.size();
    int numVertices = grafo->vertices.size();

    *numSolucoes = 0;
    *numSolucoesRepetidas = 0;

    vector<LabelInfo*>* labelsInfo = new vector<LabelInfo*>;
    vector<LabelInfo*>* labelsControl = new vector<LabelInfo*>;
    bool compControl[numVertices];
    bool reachComp[numVertices];
    int compConexas[numVertices];

    for(int i=0; i<numLabels; i++) {
        LabelInfo* labelInfo = new LabelInfo(grafo->numArestasLabels[i], i, true);
        labelsInfo->push_back(labelInfo);

        labelInfo = new LabelInfo(0, i, false);
        labelsControl->push_back(labelInfo);
    }

    sort(labelsInfo->begin(), labelsInfo->end(), comparaLabelsInfo);
    
    int numAlphas = 6;
    //float alphas[numAlphas] = {0.05, 0.10, 0.15, 0.20, 0.25, 0.30};
    float alphas[numAlphas] = {0.0, 0.05, 0.10, 0.15, 0.20, 0.25};
    int indiceAlpha;
    float sumMediaCustoAlphas;
    vector<float> probAlphas;
    vector<int> countAlphas;
    vector<int> sumCustoAlphas;

    for(int i=0; i<numAlphas; i++)
        probAlphas.push_back(100.0/numAlphas);

    for(int i=0; i<numAlphas; i++) {
        countAlphas.push_back(1);
        sumCustoAlphas.push_back(initialSolution->size());
    }

    int indMelhorSol = 0;
    vector<vector<int>*> solucoesConstr;

    *avgDiff = 0;
    *maxDiff = 0;
    *minDiff = (float)INT_MAX;

    int sumDiff;
    int quantDiff = 0;

    *mediaItMelhora = 0;
    *minItMelhora = 0;
    *maxItMelhora = 0;

    int numSolucoesConst = 1000;
    int iteracaoMelhor = 0;
    melhorSolucao = initialSolution;
    solucoesConstr.push_back(initialSolution);
    for(int i=0; i<numIteracoes; i++) {
        *numSolucoes += 1;

        solucao = new vector<int>;

        aleatorio = rand() % 100;
        acumulada = 0;
        for(int j=0; j<numAlphas; j++) {
            acumulada += probAlphas[j];
            if(aleatorio < acumulada) {
                indiceAlpha = j;
                break;
            }
        }
        construtivo(grafo, i, alphas[indiceAlpha], alphas[indiceAlpha], solucao, labelsInfo, labelsControl, compControl, reachComp, compConexas);
        buscaLocalExcedente(grafo, solucao);
        countAlphas[indiceAlpha]++;
        sumCustoAlphas[indiceAlpha] += solucao->size(); 

        bool checkSolRep = false;

        for(int j=0; j<solucoesConstr.size(); j++) {
            if(solucoesIguais(solucoesConstr[j], solucao)) {
                checkSolRep = true;
                break;
            }
        }

        if(!checkSolRep) {
            for(int j=0; j<solucoesConstr.size(); j++) {
                sumDiff = 0;
                for(int k=0; k<solucao->size(); k++) {
                    bool checkLabel = true;

                    for(int l=0; l<solucoesConstr[j]->size(); l++) {
                        if(solucao->at(k) == solucoesConstr[j]->at(l)) {
                            checkLabel = false;
                            break;
                        }
                    }

                    if(checkLabel)
                        sumDiff++;
                }

                for(int k=0; k<solucoesConstr[j]->size(); k++) {
                    bool checkLabel = true;

                    for(int l=0; l<solucao->size(); l++) {
                        if(solucoesConstr[j]->at(k) == solucao->at(l)) {
                            checkLabel = false;
                            break;
                        }
                    }

                    if(checkLabel)
                        sumDiff++;
                }

                if(sumDiff < *minDiff)
                    *minDiff = sumDiff;
                if(sumDiff > *maxDiff)
                    *maxDiff = sumDiff;

                *avgDiff += sumDiff;
                quantDiff++;
            }

            solucoesConstr.push_back(solucao);
        }

        if(solucao->size() < melhorSolucao->size()) {
            *tempoMelhorSolucao = std::chrono::high_resolution_clock::now();

            //delete melhorSolucao;
            melhorSolucao = solucao;
            indMelhorSol = solucoesConstr.size()-1;
            
            if(solucao->size() == custoOtimo) {
                for(int i=0; i<numLabels; i++)
                    delete labelsInfo->at(i);
                delete labelsInfo;

                for(int i=0; i<numLabels; i++)
                    delete labelsControl->at(i);
                delete labelsControl;

                for(int i=0; i<solucoesConstr.size(); i++) {
                    if(i != indMelhorSol)
                        delete solucoesConstr[i];
                }

                *avgDiff /= quantDiff;

                return melhorSolucao;
            }
        } else {
            if(checkSolRep)
                delete solucao;
        }

        if(i != 0 && i%50 == 0) {
            sumMediaCustoAlphas = 0;
            for(int j=0; j<numAlphas; j++) 
                sumMediaCustoAlphas += 1.0/pow(5,sumCustoAlphas[j]/countAlphas[j]);
            for(int j=0; j<numAlphas; j++)
                probAlphas[j] = (1/(sumMediaCustoAlphas * pow(5,sumCustoAlphas[j]/countAlphas[j]))) * 100;
        }
    }

    for(int i=0; i<numLabels; i++)
        delete labelsInfo->at(i);
    delete labelsInfo;

    for(int i=0; i<numLabels; i++)
        delete labelsControl->at(i);
    delete labelsControl;

    for(int i=0; i<solucoesConstr.size(); i++) {
        if(i != indMelhorSol)
            delete solucoesConstr[i];
    }

    for(int i=0; i<melhorSolucao->size(); i++) {
        cout << melhorSolucao->at(i) << " ";
    }
    cout << endl << endl;

    *avgDiff /= quantDiff;

    return melhorSolucao;
}

int custoSolucaoExata(string entrada) {
    char* aux;
    stringstream ss;
    string path;

    aux = strtok((char*)entrada.c_str(), "/");

    ss << "saidasMIP";
    while(aux != NULL) {
        if(aux == "")
            break;
        aux = strtok(NULL, "/");
        ss << "/";
        ss << aux;
    }
    path = ss.str();
    path.pop_back();

    FILE* file;
    char* split;
    char linha[300];
    file = fopen(path.c_str(), "r");
    
    if(file == NULL) {
        cout << "Arquivo inexistente" << endl;
        return INT_MAX;
    }

    char *unusedReturn;
    unusedReturn = fgets(linha, 300, file);
    unusedReturn = fgets(linha, 300, file);
    split = strtok(linha, ";");

    return atoi(split);
}

void cenarioCinco(string entrada, string saida, int numIteracoes, double alpha, double tempInicial, double tempFinal, float tempoLimite, int seed) {
    FILE* file;
    GrafoListaAdj* grafo;
    GRBEnv* env;
    clock_t tempo[2];
    stringstream ss;
    int custoOtimo;
    int custoSolucaoInicial;
    int solucaoConstrutivo;
    int numIteracoesGrasp;
    int numSolucoesSA;
    int numSolucoesRepetidasGrasp;
    int numSolucoesRepetidasSA;
    int numParciaisRepetidas = 0;
    float tempoSolucaoInicial;
    float tempoSA;
    float tempoBuscaLocal;
    float tempoMelhorSolucao;

    grafo = nullptr;
    grafo = carregaInstancias(entrada.c_str());
    
    if(grafo == nullptr) {
        cout << "Grafo nulo" << endl;
        return;  
    } 

    vector<int>* solucaoInicial;
    //bool* solucaoSA;
    vector<int>* solucaoSA;

    int numLabels = grafo->arestas.size();

    srand(seed);
    custoOtimo = custoSolucaoExata(entrada);
    env = new GRBEnv();
    
    tempo[0] = clock();
    solucaoInicial = GRASP(grafo, 100, &tempo[1], nullptr, &solucaoConstrutivo, env, custoOtimo, &numIteracoesGrasp, &numSolucoesRepetidasGrasp);
    tempoSolucaoInicial = (float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC;
    tempoSolucaoInicial *= 1000;
    tempoMelhorSolucao = tempoSolucaoInicial;
    
    custoSolucaoInicial = solucaoInicial->size();
    cout << "Solucao Inicial: " << solucaoInicial->size() << ", Tempo: " << tempoSolucaoInicial << "ms" << endl;
    
    srand(seed);
    int custoSolSa;
    if(solucaoInicial->size() != custoOtimo) {
        tempo[0] = clock();
        solucaoSA = SA(grafo, solucaoInicial, tempInicial, tempFinal, numIteracoes, alpha, &tempo[1], &tempoBuscaLocal, custoOtimo, &numSolucoesSA, &numSolucoesRepetidasSA, env, &numParciaisRepetidas);
        tempoSA = (float)(clock() - tempo[0]) / CLOCKS_PER_SEC;
        
        custoSolSa = solucaoSA->size();
        /*custoSolSa = 0;
        for(int i=0; i<numLabels; i++)
            if(solucaoSA[i])
                custoSolSa++;*/
        if(custoSolSa < custoSolucaoInicial) {
            tempoMelhorSolucao = (float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC;
            tempoMelhorSolucao *= 1000;
        }
        tempoSA *= 1000; //passando para ms
        tempoSA += tempoSolucaoInicial;
    } else {
        numSolucoesSA = 0;
        numSolucoesRepetidasSA = 0;
        solucaoSA = solucaoInicial;
        custoSolSa = solucaoInicial->size();
        tempoSA = 0;
        tempoBuscaLocal = 0;
    }

    file = fopen(saida.c_str(), "a+");

    ss.str("");
    ss.clear();
    ss << "\n" << custoSolSa << ";" << custoSolucaoInicial << ";" << tempoSolucaoInicial << ";" << tempoSA << ";" << tempoSA - tempoBuscaLocal << ";" << tempoBuscaLocal << ";" << tempoMelhorSolucao << ";" << tempoLimite*1000 << ";" << numIteracoesGrasp << ";" << numSolucoesRepetidasGrasp << ";" << numSolucoesSA << ";" << numSolucoesRepetidasSA << ";" << numParciaisRepetidas << ";" << seed;
    fputs(ss.str().c_str(), file);
    
    fclose(file);
    cout << "Tamanho da Solucao: " << custoSolSa;
    cout << ", Tempo Solucao Inicial: " << tempoSolucaoInicial << "ms" << ", Tempo SA: " << tempoSA << "ms" << ", Tempo SA (Busca Local): " << tempoBuscaLocal << "ms" << ", Tempo SA (Construtivo): " << tempoSA - tempoBuscaLocal << "ms" << ", Tempo melhor solucao: " << tempoMelhorSolucao << "ms" << ", Numero de Solucoes Parciais Repetidas: " << numParciaisRepetidas << endl << endl;

    //delete [] solucaoSA;
    delete solucaoSA;
    delete grafo;
    delete env;
}

void cenarioSeis(string entrada, string saida, int numIteracoes, int numSolucoesConst, int numTentativas, float betas[2], float tempoLimite, int seed) {
    FILE* file;
    GrafoListaAdj* grafo;
    GRBEnv* env;
    clock_t tempo[2];
    stringstream ss;
    int custoOtimo;
    int custoSolucaoInicial;
    int solucaoConstrutivo;
    int numIteracoesGrasp;
    int numSolucoesIG;
    int numSolucoesRepetidasGrasp;
    int numSolucoesRepetidasIG;
    int numParciaisRepetidas = 0;
    float tempoSolucaoInicial;
    float tempoIG;
    float tempoMip = 0;
    float tempoMelhorSolucao;
    float mediaItMelhora = 0;
    float minItMelhora = 0;
    float maxItMelhora = 0;
    float avgDiff = 0;
    float minDiff = 0;
    float maxDiff = 0;

    grafo = nullptr;

    grafo = carregaInstancias(entrada.c_str());
    
    if(grafo == nullptr) {
        cout << "Grafo nulo" << endl;
        return;  
    } 

    int numLabels = grafo->arestas.size();

    for(int i=0; i<numLabels; i++)
        grafo->pesos.push_back(1.0/grafo->numArestasLabels[i]);

    vector<int>* solucaoInicial;
    vector<int>* solucaoIG;

    vector<int>* vetNumCompConexas = new vector<int>;

    srand(seed);
    custoOtimo = custoSolucaoExata(entrada);
    //custoOtimo = -1;
    env = new GRBEnv();
    
    std::chrono::high_resolution_clock::time_point stopMelhor = std::chrono::high_resolution_clock::now();
    auto start = std::chrono::high_resolution_clock::now();
    solucaoInicial = GRASP(grafo, 1, &tempo[1], nullptr, &solucaoConstrutivo, env, custoOtimo, &numIteracoesGrasp, &numSolucoesRepetidasGrasp);
    auto diff = std::chrono::high_resolution_clock::now() - start;
    auto t1 = std::chrono::duration_cast<std::chrono::microseconds>(diff);
    tempoSolucaoInicial = t1.count()/1000.0;
    tempoMelhorSolucao = tempoSolucaoInicial;
    
    custoSolucaoInicial = solucaoInicial->size();
    //cout << "Solucao Inicial: " << solucaoInicial->size() << ", Tempo: " << tempoSolucaoInicial << "ms" << endl;
    
    srand(seed);
    int custoSolIG;
    if(solucaoInicial->size() != custoOtimo) {
        start = std::chrono::high_resolution_clock::now();
        solucaoIG = IG(grafo, solucaoInicial, numIteracoes, numSolucoesConst, numTentativas, betas, &stopMelhor, &tempoMip, custoOtimo, &numSolucoesIG, &numSolucoesRepetidasIG, env, &numParciaisRepetidas, tempo[0], &mediaItMelhora, &minItMelhora, &maxItMelhora, vetNumCompConexas);
        //solucaoIG = IG3(grafo, solucaoInicial, numIteracoes, &stopMelhor, &tempoMip, custoOtimo, &numSolucoesIG, &numSolucoesRepetidasIG, env, &numParciaisRepetidas, tempo[0], &mediaItMelhora, &minItMelhora, &maxItMelhora, vetNumCompConexas, &avgDiff, &minDiff, &maxDiff);
        //solucaoIG = IG2(grafo, solucaoInicial, numIteracoes, &stopMelhor, &tempoMip, custoOtimo, &numSolucoesIG, &numSolucoesRepetidasIG, env, &numParciaisRepetidas, tempo[0], &mediaItMelhora, &minItMelhora, &maxItMelhora, vetNumCompConexas, &avgDiff, &minDiff, &maxDiff);
        diff = std::chrono::high_resolution_clock::now() - start;
        t1 = std::chrono::duration_cast<std::chrono::microseconds>(diff);
        tempoIG = t1.count()/1000.0;
        custoSolIG = solucaoIG->size();
        if(custoSolIG < custoSolucaoInicial) {
            diff = stopMelhor - start;
            t1 = std::chrono::duration_cast<std::chrono::microseconds>(diff);
            tempoMelhorSolucao = t1.count()/1000.0;
        }
        tempoIG += tempoSolucaoInicial;

        if(minDiff == INT_MAX) {
            avgDiff = 0;
            minDiff = 0;
            maxDiff = 0;
        }
        
    } else {
        numSolucoesIG = 0;
        numSolucoesRepetidasIG = 0;
        solucaoIG = solucaoInicial;
        custoSolIG = solucaoInicial->size();
        tempoIG = 0;
        tempoMip = 0;
    }

    file = fopen(saida.c_str(), "a+");

    ss.str("");
    ss.clear();
    //ss << "\n" << custoSolIG << ";" << custoSolucaoInicial << ";" << tempoSolucaoInicial << ";" << tempoIG << ";" << tempoIG - tempoMip << ";" << tempoMip << ";" << tempoMelhorSolucao << ";" << tempoLimite*1000 << ";"  << mediaItMelhora << ";"  << minItMelhora << ";"  << maxItMelhora << ";" << numIteracoesGrasp << ";" << numSolucoesRepetidasGrasp << ";" << numSolucoesIG << ";" << numSolucoesRepetidasIG << ";" << numParciaisRepetidas << ";" << avgDiff << ";" << minDiff << ";" << maxDiff << ";" << seed;
    ss << "\n" << entrada << " - " << custoSolIG;
    fputs(ss.str().c_str(), file);
    
    fclose(file);
    cout << custoSolIG;
    //cout << "Tamanho da Solucao: " << custoSolIG;
    //cout << ", Tempo Solucao Inicial: " << tempoSolucaoInicial << "ms" << ", Tempo IG: " << tempoIG << "ms" << ", Tempo IG (Mip): " << tempoMip << "ms" << ", Tempo IG (Construtivo): " << tempoIG - tempoMip << "ms" << ", Tempo melhor solucao: " << tempoMelhorSolucao << "ms" << ", Iterações do GRASP: " << numIteracoesGrasp << ", Numero de Solucoes Repetidas: " << numSolucoesRepetidasIG << ", Numero de Solucoes Parciais Repetidas: " << numParciaisRepetidas << endl << endl;
    
    /*if(seed == 0) {
        float minNumCompConexas = INT_MAX;
        float maxNumCompConexas = 0;
        float avgNumCompConexas = 0.0;
        float desvNumCompConexas = 0.0;
        float sum = 0;

        if(vetNumCompConexas->size() == 0) {
            minNumCompConexas = 0;
        } else {
            for(int i=0; i<vetNumCompConexas->size(); i++) {
                if(vetNumCompConexas->at(i) < minNumCompConexas)
                    minNumCompConexas = vetNumCompConexas->at(i);
                
                if(vetNumCompConexas->at(i) > maxNumCompConexas)
                    maxNumCompConexas = vetNumCompConexas->at(i);
                
                sum += vetNumCompConexas->at(i);
            }

            avgNumCompConexas = sum / vetNumCompConexas->size();

            sum = 0;
            for(int i=0; i<vetNumCompConexas->size(); i++) {
                sum += pow(vetNumCompConexas->at(i) - avgNumCompConexas, 2);
            }
            desvNumCompConexas = sqrt(sum/vetNumCompConexas->size());
        }

        stringstream grupo;
        stringstream densidade;
        stringstream instancia;
        char* split;
        split = strtok((char*)entrada.c_str(), "/");
        int i=0;
        while(split != NULL) {
            if(split == "")
                break;
            split = strtok(NULL, "/");
            if(i == 1)
                grupo << split;
            if(i == 3)
                densidade << split;
            if((grupo.str() == "g1" && i == 4) || (grupo.str() == "g2" && i == 5))
                instancia << split;
            i++;
        }
        if(densidade.str() == "hd") {
            densidade.str("");
            densidade << "0.8";
        } else if(densidade.str() == "md") {
            densidade.str("");
            densidade << "0.5";
        } else {
            densidade.str("");
            densidade << "0.2";
        }
        
        file = fopen("numCompConexas.csv", "a+");

        ss.str("");
        ss.clear();
        
        ss << "\n" << grafo->vertices.size() << "-" << densidade.str() << "-" << numLabels << "-"  << instancia.str().substr(0, 1) << ";" << avgNumCompConexas << ";" << desvNumCompConexas << ";" << minNumCompConexas << ";" << maxNumCompConexas;
        fputs(ss.str().c_str(), file);
        
        fclose(file);
    }*/

    //delete [] solucaoIG;
    delete solucaoIG;
    delete grafo;
    delete env;
    delete vetNumCompConexas;
    //delete file;
}

void cenarioSete(string entrada, string saida) {
    FILE* file;
    GrafoListaAdj* grafo;
    stringstream ss;

    grafo = nullptr;
    grafo = carregaInstancias(entrada.c_str());
    
    if(grafo == nullptr) {
        cout << "Grafo nulo" << endl;
        return;  
    } 

    int numLabels = grafo->arestas.size();
    int numVertices = grafo->vertices.size();

    vector<ContabilizaArestas> contabiliza;
    for(int i=0; i<numLabels; i++) {
        Aresta* aresta = grafo->arestas[i];
        while(aresta != nullptr) {
            int indice = -1;
            for(int j=0; j<contabiliza.size(); j++) {
                if((contabiliza[j].v1 == aresta->origem && contabiliza[j].v2 == aresta->destino) || (contabiliza[j].v1 == aresta->destino && contabiliza[j].v2 == aresta->origem)) {
                    indice = j;
                    contabiliza[j].count += 1;
                    break;
                }
            }
            if(indice == -1) {
                ContabilizaArestas t;
                t.v1 = aresta->origem;
                t.v2 = aresta->destino;
                t.count = 1;
                contabiliza.push_back(t);
            }
            aresta = aresta->prox;
        }
    }

    float sum = 0;
    float mediaArestas = 0;
    float minArestas = INT_MAX;
    float maxArestas = 0;
    float desvArestas;
    for(int i=0; i<numLabels; i++) {
        sum += grafo->numArestasLabels[i];
        if(grafo->numArestasLabels[i] < minArestas)
            minArestas = grafo->numArestasLabels[i];
        if(grafo->numArestasLabels[i] > maxArestas)
            maxArestas = grafo->numArestasLabels[i];
    }
    mediaArestas = sum/numLabels;

    sum = 0;
    for(int i=0; i<numLabels; i++) {
        sum += pow(grafo->numArestasLabels[i] - mediaArestas, 2);
    }
    desvArestas = sqrt(sum/numLabels);

    float mediaGrau = 0;
    float minGrau = INT_MAX;
    float maxGrau = 0;
    float desvGrau;
    int countGrauUm = 0;
    int countArestas;
    sum = 0;
    for(int i=0; i<grafo->vertices.size(); i++) {
        countArestas = 0;
        for(int j=0; j<grafo->vertices[i]->arestas.size(); j++) {
            Aresta* aresta = grafo->vertices[i]->arestas[j];

            while(aresta != nullptr) {
                sum++;
                countArestas++;
                aresta = aresta->prox;
            }
        }
        if(countArestas == 1) {
            countGrauUm++;
        }
        if(countArestas < minGrau) {
            minGrau = countArestas;
        }
        if(countArestas > maxGrau) {
            maxGrau = countArestas;
        }
    }
    mediaGrau = sum/numVertices;

    sum = 0;
    for(int i=0; i<grafo->vertices.size(); i++) {
        countArestas = 0;
        for(int j=0; j<grafo->vertices[i]->arestas.size(); j++) {
            Aresta* aresta = grafo->vertices[i]->arestas[j];

            while(aresta != nullptr) {
                countArestas++;
                aresta = aresta->prox;
            }
        }
        sum += pow(countArestas - mediaGrau, 2);
    }
    desvGrau = sqrt(sum/numVertices);

    /*cout << "Arestas: " << contabiliza.size() << ", Med Arestas por Rótulos: " << mediaArestas << ", Desv Rel Arestas por Rótulos: " << (100*desv)/mediaArestas
     << ", Min Arestas por Rótulos: " << min << ", Max Arestas por Rótulos: " << max << endl;
    for(int i=0; i<numLabels; i++) {
        if(contabiliza[i].count != 1)
            cout << contabiliza[i].count << endl;
    }*/

    cout << "Num Vértices com Grau 1: " << countGrauUm << ", Med Grau dos Vértices: " << mediaGrau << ", Desv Rel Grau dos Vértices: " << (100*desvGrau)/mediaGrau
     << ", Min Grau dos Vértices: " << minGrau << ", Max Grau dos Vértices: " << maxGrau << endl;

    stringstream grupo;
    stringstream densidade;
    stringstream instancia;
    char* split;
    split = strtok((char*)entrada.c_str(), "/");
    int i=0;
    while(split != NULL) {
        if(split == "")
            break;
        split = strtok(NULL, "/");
        if(i == 1)
            grupo << split;
        if(i == 3)
            densidade << split;
        if((grupo.str() == "g1" && i == 4) || (grupo.str() == "g2" && i == 5))
            instancia << split;
        i++;
    }
    if(densidade.str() == "hd") {
        densidade.str("");
        densidade << "0.8";
    } else if(densidade.str() == "md") {
        densidade.str("");
        densidade << "0.5";
    } else {
        densidade.str("");
        densidade << "0.2";
    }
    
    file = fopen(saida.c_str(), "a+");

    ss.str("");
    ss.clear();
    
    ss << "\n" << grafo->vertices.size() << "-" << densidade.str() << "-" << numLabels << "-"  << instancia.str().substr(0, 1) << ";" << contabiliza.size() << ";" << mediaArestas << ";" << (100*desvArestas)/mediaArestas << ";" << minArestas << ";" << maxArestas << ";" << countGrauUm << ";" << mediaGrau << ";" << (100*desvGrau)/mediaGrau << ";" << minGrau << ";" << maxGrau;
    fputs(ss.str().c_str(), file);
    
    fclose(file);
    
    delete grafo;
}

int main(int argc, char **argv) { 
    int metodo = stoi(argv[1]);
    int seed = stoi(argv[2]);

    string str;
    string parameters[6] = {
        "--numIteracoes",
        "--numSolucoesConst", 
        "--numTentativas",
        "--beta1",
        "--beta2",
        "--tempoLimite"
    };

    str = argv[4];
    int numIteracoes = stoi(str.substr(parameters[0].length(), str.length()-parameters[0].length()));
    str = argv[5];
    int numSolucoesConst = stoi(str.substr(parameters[1].length(), str.length()-parameters[1].length()));
    str = argv[6];
    int numTentativas = stoi(str.substr(parameters[2].length(), str.length()-parameters[2].length()));
    str = argv[7];
    float beta1 = stof(str.substr(parameters[3].length(), str.length()-parameters[3].length()));
    str = argv[8];
    float beta2 = stof(str.substr(parameters[4].length(), str.length()-parameters[4].length()));
    str = argv[9];
    float tempoLimite = stof(str.substr(parameters[5].length(), str.length()-parameters[5].length()));
    float betas[2] = { beta1, beta2 };

    if(argc < 2) {
        cout << "Parametro necessario:metodo(0 - Reativo, 1 - GRASP, 2 - MIP)" << endl;
        return 0;
    }
    if(metodo == 3){
        if(argc < 10) {
            cout << "Parametros necessarios: arquivo de entrada, arquivo de saida, numero de iteracoes, taxa de decaimento, temperatura inicial e final, tempo limite do GRASP, seed" << endl;
            return 0;
        }
        cenarioCinco(argv[2], argv[3], stoi(argv[4]), stof(argv[5]), stof(argv[6]), stof(argv[7]), stof(argv[8]), stoi(argv[9]));
    } else if(metodo == 5){
        if(argc < 10) {
            cout << "Parametros necessarios: arquivo de entrada, arquivo de saida, numero de iteracoes, número de soluções construtivas, número de tentativas, coeficiente de remoção 1, coeficiente de remoção 2, tempo limite do GRASP, seed" << endl;
            return 0;
        }
        /*float betas[2];
        stringstream ss;
        string strBetas = argv[7];
        string token;
        size_t pos=-1;
        int i = 0;

        strBetas = strBetas.substr(1);
        strBetas = strBetas.substr(0, strBetas.length()-1);

        ss << strBetas;
        cout << strBetas << endl;
        while(std::getline(ss, token, ',')) {
            betas[i] = stof(token);
            i++;
        }*/

        cenarioSeis(argv[3], "saida.txt", numIteracoes, numSolucoesConst, numTentativas, betas, tempoLimite, seed);
    } else if(metodo == 6){
        if(argc < 4) {
            cout << "Parametros necessarios: arquivo de entrada, arquivo de saida" << endl;
            return 0;
        }
        cenarioSete(argv[2], argv[3]);
    }
}