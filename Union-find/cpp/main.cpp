#include <iostream>
#include <vector>
#include <climits>
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

        mycallback(int numVars, GRBVar* vars, GrafoListaAdj* grafo, clock_t tempoInicio, int custoOtimo) {
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

    int storeCompConexas;
    int indice;
    if(iteracao > 2) {
        aleatorio = rand() % numLabels;
        solucao->push_back(aleatorio);
        //if(iteracao < 5)
        //    cout << "Escolhido inicial: " << aleatorio << endl;
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

    listaAtual = listaOrdenadaInicial;
    solucao->push_back(0);
    bool first = true;
    /*if(iteracao < 5) {
        cout << "Depois do sort: " << endl;
        for(int i=0; i<listaAtual->size(); i++)
            cout << listaAtual->at(i)->posLabel << "-" << listaAtual->at(i)->numCompConexas << " ";
        cout << endl;
    }*/
    do {
        count = 0;
        for(int i=0; i<listaAtual->size(); i++) {
            if(listaAtual->at(i)->numCompConexas != INT_MAX) {
                if(listaAtual->at(i)->numCompConexas > listaAtual->at(0)->numCompConexas*alpha)
                    break;
                count++;
            }
        }
        aleatorio = rand() % count;
        while(listaAtual->at(aleatorio)->numCompConexas == INT_MAX)
            aleatorio = (aleatorio+1)%numLabels;

        solucao->back() = listaAtual->at(aleatorio)->posLabel;
        numCompConexas = listaAtual->at(aleatorio)->numCompConexas;
        //if(iteracao < 5)
        //    cout << "Escolhido: " << aleatorio << "-" << listaAtual->at(aleatorio)->posLabel << endl;
        //cout << "Componentes: " << numCompConexas << " - " << solucao->size() << endl;
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
            for(int i=0; i<numLabels; i++) {
                if(listaOrdenada->at(i)->numCompConexas != INT_MAX) {
                    solucao->back() = listaOrdenada->at(i)->posLabel;
                    listaOrdenada->at(i)->numCompConexas = grafo->numCompConexas(solucao);
                }
            }
            /*if(iteracao < 5) {
                for(int i=0; i<listaOrdenada->size(); i++)
                    cout << listaOrdenada->at(i)->posLabel << "-" << listaOrdenada->at(i)->numCompConexas << " ";
                cout << endl;
            }*/
            //sort(listaOrdenada->begin(), listaOrdenada->end()-(solucao->size()-1), compara_sort_b);
            sort(listaOrdenada->begin(), listaOrdenada->end(), compara_sort_b);  
            /*if(iteracao < 5) {
                cout << "Depois do sort: " << endl;
                for(int i=0; i<listaOrdenada->size(); i++)
                    cout << listaOrdenada->at(i)->posLabel << "-" << listaOrdenada->at(i)->numCompConexas << " ";
                cout << endl;
            }*/
        }
        listaAtual = listaOrdenada;
        
    }while(numCompConexas > 1);
    //cout << "FIM: " << solucao->size() << endl;
    if(iteracao > 2)
        listaOrdenadaInicial->at(indice)->numCompConexas = storeCompConexas;

    for(int i=numLabels-1; i>numLabels-solucao->size(); i--) {
        listaOrdenada->at(i)->numCompConexas = 0;
        //cout << i << " ";
    }
    //cout << endl;

    /*
    for(int i=0; i<numLabels; i++)
        cout << listaOrdenada->at(i)->posLabel << "-" << listaOrdenada->at(i)->numCompConexas << " ";
    cout << endl << endl;*/
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
    vector<vector<int>> solucoes; 

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

            /*verifica = false;
            for(int j=0; j<solucoes.size(); j++) {
                if(solucao->size() != solucoes[j].size())
                    continue;
                verifica = true;
                for(int k=0; k<solucao->size(); k++)
                    if(!taNoVetor(&(solucoes[j]), solucao->at(k))) {
                        verifica = false;
                        break;
                    }
                if(verifica) {
                    *numSolucoesRepetidas += 1;
                    break;
                }
            }
            if(!verifica) {
                vector<int> copia;
                for(int j=0; j<solucao->size(); j++)
                    copia.push_back(solucao->at(j));
                solucoes.push_back(copia);
            }*/

            return solucao;
        }
        auxTempoBuscaLocal[0] = clock();
        if(solucao->size() > 1)
            buscaLocalMIP(grafo, solucao, env, &mipGap, 2);
        auxTempoBuscaLocal[1] = clock();
        if(tempoBuscaLocal !=  nullptr)
            *tempoBuscaLocal += (float)(auxTempoBuscaLocal[1] - auxTempoBuscaLocal[0]) / CLOCKS_PER_SEC;

        /*verifica = false;
        for(int j=0; j<solucoes.size(); j++) {
            if(solucao->size() != solucoes[j].size())
                continue;
            verifica = true;
            for(int k=0; k<solucao->size(); k++)
                if(!taNoVetor(&(solucoes[j]), solucao->at(k))) {
                    verifica = false;
                    break;
                }
            if(verifica) {
                *numSolucoesRepetidas += 1;
                break;
            }
        }
        if(!verifica) {
            vector<int> copia;
            for(int j=0; j<solucao->size(); j++)
                copia.push_back(solucao->at(j));
            solucoes.push_back(copia);
        }*/

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

vector<int>* pertubacaoMIP(GrafoListaAdj* grafo, vector<int>* solucao, float* tempoBuscaLocal, float alpha, GRBModel* model, GRBVar* z, mycallback* cb, vector<SolucaoParcial*>* parciais, clock_t tempoInicio, int custoOtimo) {
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
        aleatorio = rand()%solucao->size();
        if(solucaoParcial[solucao->at(i)])
            solucaoParcial[solucao->at(i)] = false;
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
            novaSolucao = parciais->at(i);
            return nullptr;
        }
    }

    novaSolucao = grafo->numCompConexas3(solucaoParcial);
    parciais->push_back(novaSolucao);
    
    vector<int>* vizinha;
    if(novaSolucao->numCompConexas != 1) {
        double mipGap;
        int count = 0;
        for(int i=0; i<grafo->arestas.size(); i++)
            if(novaSolucao->labels[i])
                count++;
        vizinha = mip(grafo, novaSolucao, solucao, model, z, cb, &mipGap, tempoInicio, custoOtimo);
    } else {
        vizinha = new vector<int>;
        for(int i=0; i<numLabels; i++) {
            if(novaSolucao->labels[i])
                vizinha->push_back(i);
        }
    }

    return vizinha;
}

vector<int>* IG(GrafoListaAdj* grafo, vector<int>* initialSolution, int numIteracoes, clock_t* tempoMelhorSolucao, float* tempoBuscaLocal, int custoOtimo, int* numSolucoes, int* numSolucoesRepetidas, GRBEnv* env, int* numParciaisRepetidas, clock_t tempoInicio) {
    vector<int>* solucao;
    vector<int>* novaSolucao;
    vector<int>* melhorSolucao;
    clock_t tempo[2];
    float tempoSolucaoInicial;
    float auxTempoBuscaLocal;
    int solucaoConstrutivo;
    bool aux;
    double mipGap;
    float acumulada;
    int aleatorio;

    int numAlphas = 6;
    float alphas[numAlphas] = {0.05, 0.10, 0.15, 0.20, 0.25, 0.3};
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

    vector<SolucaoParcial*>* parciais = new vector<SolucaoParcial*>;

    //CRIAÇÃO DO MODELO MIP
    GRBModel model = GRBModel(*env);
    GRBVar* z;

    model.set(GRB_StringAttr_ModelName, "MLST");
    model.set(GRB_IntParam_OutputFlag, 0);
    model.set(GRB_DoubleParam_TimeLimit, 30); //Tempo de limite de 10h
    //model.set(GRB_DoubleParam_MIPGap, 0.05); //Termina quando o gap estiver menor que 5%
    model.set(GRB_IntParam_LazyConstraints, 1);
    //model.set(GRB_IntParam_MIPFocus, 1); //Foco em achar soluções viáveis
    
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
    
    z = model.addVars(lb, ub, obj, type, variaveis, numLabels);
    GRBLinExpr sum;

    model.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);
    //model.addConstr(sum <= initialSolution->size()-1, "solucao final diferente da inicial");
    
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
        
        model.addConstr(sum >= 1.0, ss.str());

        delete labels;
    }

    sum = 0;
    for(int i=0; i<numLabels; i++)
        sum += z[i] * grafo->numArestasLabels[i];
    model.addConstr(sum >= grafo->vertices.size()-1, "numero minimo de arestas");

    mycallback cb = mycallback(numLabels, z, grafo, tempoInicio, custoOtimo);
    model.setCallback(&cb);

    delete []variaveis;
    delete []lb;
    delete []ub;
    delete []obj;
    delete []type;
    //FIM DA CRIAÇÃO DO MODELO MIP

    /*vector<vector<int>> contabiliza;  
    vector<int> a;
    for(int j=0; j<initialSolution->size(); j++)
        a.push_back(initialSolution->at(j));
    contabiliza.push_back(a);*/

    vector<vector<int>*> construtivas;  
    construtivas.push_back(initialSolution);

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
    solucao->clear();
    for(int i=0; i<1000; i++) {
        auxMVCAGRASP2(grafo, i, 1, solucao, listaOrdenadaInicial, listaOrdenada);
        //solucao = auxMVCAGRASP(grafo, i, 1);
        buscaLocalExcedente(grafo, solucao);

        if(solucao->size() == custoOtimo) {
            *tempoMelhorSolucao = clock();
            *numSolucoes = i+1;

            for(int j=0; j<construtivas.size(); j++)
                delete construtivas[j];

            return solucao;
        }

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
            if(construtivas.size() == 1000) {
                *numSolucoes = i+1;
                break;
            } else 
                solucao = new vector<int>;
        }
    }

    for(int i=0; i<numLabels; i++) {
        delete listaOrdenada->at(i);
        delete listaOrdenadaInicial->at(i);
    }
    delete listaOrdenada;
    delete listaOrdenadaInicial;
    delete solucao;

    cout << "Quantidade: " << construtivas.size() << endl;

    aux = true;
    *tempoBuscaLocal = 0;
    *numParciaisRepetidas = 0;
    solucao = initialSolution;
    melhorSolucao = initialSolution;
    for(int i=0; i<numIteracoes; i++) { 
        solucao = construtivas[i%construtivas.size()];

        aleatorio = rand() % 100;
        acumulada = 0;
        for(int j=0; j<numAlphas; j++) {
            acumulada += probAlphas[j];
            if(aleatorio < acumulada) {
                indiceAlpha = j;
                break;
            }
        }
        novaSolucao = pertubacaoMIP(grafo, solucao, tempoBuscaLocal, alphas[indiceAlpha], &model, z, &cb, parciais, tempoInicio, custoOtimo);
        *numSolucoes += 1;
        if(novaSolucao != nullptr) { 
            countAlphas[indiceAlpha]++;
            sumCustoAlphas[indiceAlpha] += novaSolucao->size(); 

            //if(!aux)
            //    delete solucao;

            if(novaSolucao->size() < melhorSolucao->size()) {
                *tempoMelhorSolucao = clock();
                delete melhorSolucao;
                aux = true;
                melhorSolucao = novaSolucao;

                if(melhorSolucao->size() == custoOtimo) {
                    for(int i=0; i<parciais->size(); i++)
                        delete parciais->at(i);
                    delete parciais;

                    return melhorSolucao;
                }
            } else {
                delete novaSolucao;
                //aux = false;
            }
        } else 
            *numParciaisRepetidas += 1;

        /*
        auxTempoBuscaLocal = clock();
        if(novaSolucao->size() > 1)
            buscaLocalMIP(grafo, novaSolucao, env, &mipGap, 2);
        *tempoBuscaLocal += (float)(clock() - auxTempoBuscaLocal);

        if(!aux) {
            if(novaSolucao->size() < melhorSolucao->size()) {
                *tempoMelhorSolucao = clock();
                delete melhorSolucao;
                aux = true;
                melhorSolucao = novaSolucao;

                if(melhorSolucao->size() == custoOtimo) {
                    *parciaisGeradas = parciais->size();
                    for(int i=0; i<parciais->size(); i++)
                        delete parciais->at(i);
                    delete parciais;

                    return melhorSolucao;
                }
            } else
                aux = false;
        }*/

        if(i != 0 && i%50 == 0) {
            sumMediaCustoAlphas = 0;
            for(int j=0; j<numAlphas; j++) 
                sumMediaCustoAlphas += 1.0/pow(5,sumCustoAlphas[j]/countAlphas[j]);
            for(int j=0; j<numAlphas; j++)
                probAlphas[j] = (1/(sumMediaCustoAlphas * pow(5,sumCustoAlphas[j]/countAlphas[j]))) * 100;
        }

        //solucao = auxMVCAGRASP(grafo, i, 1);
        //solucao = novaSolucao;
        /*
        vector<int> a;
        bool testa = false;
        for(int j=0; j<solucao->size(); j++)
            a.push_back(solucao->at(j));
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
    }

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

    if(!aux)
        delete novaSolucao;
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

void cenarioSeis(string entrada, string saida, int numIteracoes, float tempoLimite, int seed) {
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
    float tempoBuscaLocal;
    float tempoMelhorSolucao;

    grafo = nullptr;
    grafo = carregaInstancias(entrada.c_str());
    
    if(grafo == nullptr) {
        cout << "Grafo nulo" << endl;
        return;  
    } 

    vector<int>* solucaoInicial;
    vector<int>* solucaoSA;

    int numLabels = grafo->arestas.size();

    srand(seed);
    custoOtimo = custoSolucaoExata(entrada);
    env = new GRBEnv();
    
    tempo[0] = clock();
    solucaoInicial = GRASP(grafo, 1, &tempo[1], nullptr, &solucaoConstrutivo, env, custoOtimo, &numIteracoesGrasp, &numSolucoesRepetidasGrasp);
    tempoSolucaoInicial = (float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC;
    tempoSolucaoInicial *= 1000;
    tempoMelhorSolucao = tempoSolucaoInicial;
    
    custoSolucaoInicial = solucaoInicial->size();
    cout << "Solucao Inicial: " << solucaoInicial->size() << ", Tempo: " << tempoSolucaoInicial << "ms" << endl;
    
    srand(seed);
    int custoSolSa;
    if(solucaoInicial->size() != custoOtimo) {
        tempo[0] = clock();
        solucaoSA = IG(grafo, solucaoInicial, numIteracoes, &tempo[1], &tempoBuscaLocal, custoOtimo, &numSolucoesIG, &numSolucoesRepetidasIG, env, &numParciaisRepetidas, tempo[0]);
        tempoIG = (float)(clock() - tempo[0]) / CLOCKS_PER_SEC;
        
        custoSolSa = solucaoSA->size();
        if(custoSolSa < custoSolucaoInicial) {
            tempoMelhorSolucao = (float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC;
            tempoMelhorSolucao *= 1000;
        }
        tempoIG *= 1000; //passando para ms
        tempoIG += tempoSolucaoInicial;
    } else {
        numSolucoesIG = 0;
        numSolucoesRepetidasIG = 0;
        solucaoSA = solucaoInicial;
        custoSolSa = solucaoInicial->size();
        tempoIG = 0;
        tempoBuscaLocal = 0;
    }

    file = fopen(saida.c_str(), "a+");

    ss.str("");
    ss.clear();
    ss << "\n" << custoSolSa << ";" << custoSolucaoInicial << ";" << tempoSolucaoInicial << ";" << tempoIG << ";" << tempoIG - tempoBuscaLocal << ";" << tempoBuscaLocal << ";" << tempoMelhorSolucao << ";" << tempoLimite*1000 << ";" << numIteracoesGrasp << ";" << numSolucoesRepetidasGrasp << ";" << numSolucoesIG << ";" << numSolucoesRepetidasIG << ";" << numParciaisRepetidas << ";" << seed;
    fputs(ss.str().c_str(), file);
    
    fclose(file);
    cout << "Tamanho da Solucao: " << custoSolSa;
    cout << ", Tempo Solucao Inicial: " << tempoSolucaoInicial << "ms" << ", Tempo IG: " << tempoIG << "ms" << ", Tempo IG (Busca Local): " << tempoBuscaLocal << "ms" << ", Tempo IG (Construtivo): " << tempoIG - tempoBuscaLocal << "ms" << ", Tempo melhor solucao: " << tempoMelhorSolucao << "ms" << ", Iterações do GRASP: " << numIteracoesGrasp << ", Numero de Solucoes Repetidas: " << numSolucoesRepetidasIG << ", Numero de Solucoes Parciais Repetidas: " << numParciaisRepetidas << endl << endl;

    //delete [] solucaoSA;
    delete solucaoSA;
    delete grafo;
    delete env;
}

int main(int argc, char **argv) { 
    int metodo = stoi(argv[1]) ;
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
        if(argc < 7) {
            cout << "Parametros necessarios: arquivo de entrada, arquivo de saida, numero de iteracoes, tempo limite do GRASP, seed" << endl;
            return 0;
        }
        cenarioSeis(argv[2], argv[3], stoi(argv[4]), stof(argv[5]), stoi(argv[6]));
    }
}