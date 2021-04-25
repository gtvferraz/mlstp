#include <iostream>
#include <vector>
#include <climits>
#include <time.h>
#include <string>
#include <sstream>
#include <random>
#include <algorithm>
#include "gurobi_c++.h"
#include "../libs/grafo.h"
#include "../libs/leitura.h"
#include "../libs/utils.h"

using namespace std;

#define NUMALPHAS 6

/*
g++ -m64 -g -O3 -o main.out cpp/main.cpp -I ~/../../opt/gurobi911/linux64/include/ -L ~/../../opt/gurobi911/linux64/lib -l gurobi_c++ -l gurobi91 -lm
./main.out 0 dataset/instances/g2/100/hd/50/0.txt saida.txt 200 50 0
./main.out 1 dataset/instances/g2/100/hd/50/0.txt saida.txt 10 0
./main.out 2 dataset/instances/g2/100/hd/50/0.txt saida.txt
./main.out 3 dataset/instances/g2/100/ld/125/3.txt saida.txt 50 0.9 300 0.001 0.05 0
./main.out 4 dataset/instances/g2/100/hd/50/0.txt saida.txt 10 0
*/

class mycallback: public GRBCallback {
    public:
        int numVars;
        GRBVar* vars;
        Grafo* grafo;

        mycallback(int numVars, GRBVar* vars, Grafo* grafo) {
            this->numVars = numVars;
            this->vars = vars;
            this->grafo = grafo;
        }

    protected:
        void callback () {
            Grafo* solution;
            stringstream ss;
            vector<int> compConexas;
            vector<int*> labelsComp;
            vector<int> tamanhoComp;
            Edge* atual;
            int numCompConexas;
            
            if(where == GRB_CB_MIPSOL) {
                double* z = getSolution(vars, numVars);

                solution = new Grafo(grafo->roots.size(), grafo->edges.size());
                for(int i=0; i<grafo->edges.size(); i++) {
                    if(z[i] >= 0.99)
                        solution->addLabel(grafo->edges, i);
                }

                numCompConexas = solution->numCompConexas; //Number of connected components on the graph with the edges labeled by the labels on solution
                
                //Verify if the solution graph is connected
                if(numCompConexas > 1) {
                    for(int i=0; i<solution->roots.size(); i++) {
                        if(!taNoVetor(&compConexas, solution->root(solution->roots[i]))) {
                            compConexas.push_back(solution->root(solution->roots[i]));
                            labelsComp.push_back(new int[solution->edges.size()]);
                            for(int j=0; j<solution->edges.size(); j++)
                                labelsComp[compConexas.size()-1][j] = 0;
                        }
                    }
                    if(numCompConexas == 2) {
                        for(int i=0; i<grafo->edges.size(); i++) {
                            atual = grafo->edges[i]->next;
                            while(atual != nullptr) {
                                if(solution->root(atual->x) != solution->root(atual->y)) {
                                    labelsComp[0][atual->label] = 1;
                                    labelsComp[1][atual->label] = 1;
                                }
                                atual = atual->next;
                            }
                        }
                    } else {
                        for(int i=0; i<grafo->edges.size(); i++) {
                            atual = grafo->edges[i]->next;
                            while(atual != nullptr) {
                                if(solution->root(atual->x) != solution->root(atual->y)) {
                                    for(int j=0; j<compConexas.size(); j++) {
                                        if(compConexas[j] == solution->root(atual->x))
                                            labelsComp[j][atual->label] = 1;
                                        else if(compConexas[j] == solution->root(atual->y))
                                            labelsComp[j][atual->label] = 1;
                                    }
                                }
                                atual = atual->next;
                            }
                        }
                    }

                    GRBLinExpr sum;
                    for(int i=0; i<compConexas.size(); i++) {
                        ss.str("");
                        ss.clear();
                        ss << "corte_novo-" << compConexas[i];

                        sum = 0;
                        for(int j=0; j<solution->edges.size(); j++)
                            sum += vars[j] * labelsComp[i][j];
                        addLazy(sum >= 1.0);

                        if(compConexas.size() == 2)
                            break;  
                    }

                    for(int i=0; i<labelsComp.size(); i++)
                        delete []labelsComp[i];
                }
                delete solution;
                delete[] z;
            }
        }
};

void buscaLocalExcedente(Grafo* grafo, vector<int>* solucao) {
    Grafo* novoGrafo;
    for(int i=0; i<solucao->size(); i++) {
        novoGrafo = new Grafo(grafo->roots.size(), grafo->edges.size());
        for(int j=0; j<solucao->size(); j++) {
            if(i != j)
                novoGrafo->addLabel(grafo->edges, solucao->at(j));
        }
        if(novoGrafo->numCompConexas == 1) {
            solucao->erase(solucao->begin()+i);
            i--;
            //cout << "Antes: " << solucao->size()+1 << ", Depois: " << solucao->size() << endl;
        }
        delete novoGrafo;
    }
}

void buscaLocalDoisPorUm(Grafo* grafo, vector<int>* solucao) {
    bool atualizou;
    Grafo* novoGrafo;
    do {
        atualizou = false;
        for(int i=0; !atualizou && i<solucao->size(); i++) {
            for(int j=i+1; !atualizou && j<solucao->size(); j++) {
                for(int k=0; k<grafo->edges.size(); k++) {
                    if(!taNoVetor(solucao, k)) {
                        novoGrafo = new Grafo(grafo->roots.size(), grafo->edges.size());
                        for(int l=0; l<solucao->size(); l++)
                            if(l != i && l != j)
                                novoGrafo->addLabel(grafo->edges, solucao->at(l));
                        novoGrafo->addLabel(grafo->edges, k);
                        if(novoGrafo->numCompConexas == 1) {
                            solucao->erase(solucao->begin()+i);
                            j--;
                            solucao->erase(solucao->begin()+j);
                            solucao->push_back(k);
                            atualizou = true;
                            delete novoGrafo;
                            break;
                        }
                        delete novoGrafo;
                    }
                }
                if(atualizou)
                    break;
            }
            if(atualizou)
                break;
        }
    }while(atualizou);
}

void buscaLocalMIP(Grafo* grafo, vector<int>* solucao, GRBEnv* env, double* mipGap, int raio) {
    stringstream ss;
    GRBVar* z;
    
    double* lb;
    double* ub;
    double* obj;
    char* type;
    string* variaveis;

    vector<int>* labels;
    
    int numLabels = grafo->edges.size(); //number of labels on the graph
    GRBModel model = GRBModel(*env);
    model.set(GRB_StringAttr_ModelName, "MLST");
    model.set(GRB_IntParam_OutputFlag, 0);
    model.set(GRB_DoubleParam_TimeLimit, 36000); //10h
    model.set(GRB_IntParam_LazyConstraints, 1);
    
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
    for(int i=0; i<grafo->roots.size(); i++) {
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
    model.addConstr(sum >= grafo->roots.size()-1, "numero minimo de arestas");

    sum = 0;
    for(int i=0; i<solucao->size(); i++) {
        sum += z[solucao->at(i)]; 
    }
    model.addConstr(sum >= solucao->size()-raio, "restrição de vizinhança");

    mycallback cb = mycallback(numLabels, z, grafo);
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

vector<int>* auxMVCA(Grafo* grafo, float alpha, int heuristica) {
    int numVertices = grafo->roots.size();
    int numLabels = grafo->edges.size();
    int aleatorio;
    int numCompConexas;
    int count;
    vector<Grafo*> possibilidades;
    vector<int>* solucao;
    vector<int> espacoLabels;
    vector<AuxiliaOrdenacao*> listaOrdenada;

    for(int i=0; i<numLabels; i++) {
        possibilidades.push_back(new Grafo(numVertices, numLabels));
        possibilidades[i]->addLabel(grafo->edges, i);
        espacoLabels.push_back(i);
    }

    solucao = new vector<int>;
    do {
        listaOrdenada.clear();
        for(int i=0; i<espacoLabels.size(); i++)
            listaOrdenada.push_back(new AuxiliaOrdenacao(possibilidades[espacoLabels[i]]->numCompConexas, i));  
        sort(listaOrdenada.begin(), listaOrdenada.end(), compara_sort_b);
        
        if(alpha == 0)
            aleatorio = 0;
        else if(heuristica == 0)
            aleatorio = rand() % (int)ceil(espacoLabels.size() * alpha);
        else if(heuristica == 1) {
            count = 0;
            for(int i=0; i<listaOrdenada.size(); i++) {
                if(listaOrdenada[i]->numCompConexas > listaOrdenada[0]->numCompConexas*(1+alpha))
                    break;
                count++;
            }
            aleatorio = rand() % count;
        }

        solucao->push_back(espacoLabels[listaOrdenada[aleatorio]->posLabel]);
        espacoLabels.erase(espacoLabels.begin()+listaOrdenada[aleatorio]->posLabel);
        
        numCompConexas = listaOrdenada[aleatorio]->numCompConexas;
        if(numCompConexas > 1)
            for(int i=0; i<espacoLabels.size(); i++)
                possibilidades[espacoLabels[i]]->addLabel(grafo->edges, solucao->at(solucao->size()-1));
        
        for(int i=0; i<listaOrdenada.size(); i++)
            delete listaOrdenada[i];
    }while(numCompConexas > 1);

    for(int i=0; i<numLabels; i++)
        delete possibilidades[i];

    return solucao;
}

vector<int>* auxMVCAGRASP(Grafo* grafo, int iteracao, float alpha) { 
    int numVertices = grafo->roots.size();
    int numLabels = grafo->edges.size();
    int aleatorio;
    int numCompConexas;
    int count;
    vector<Grafo*> possibilidades;
    vector<int>* solucao;
    vector<int> espacoLabels;
    vector<AuxiliaOrdenacao*> listaOrdenada;

    for(int i=0; i<numLabels; i++) {
        possibilidades.push_back(new Grafo(numVertices, numLabels));
        possibilidades[i]->addLabel(grafo->edges, i);
        espacoLabels.push_back(i);
    }

    solucao = new vector<int>;
    if(iteracao > 2) {
        aleatorio = rand() % numLabels;
        solucao->push_back(aleatorio);
        espacoLabels.erase(espacoLabels.begin()+aleatorio);
        for(int i=0; i<espacoLabels.size(); i++)
            possibilidades[espacoLabels[i]]->addLabel(grafo->edges, aleatorio);
    }
    
    do {
        listaOrdenada.clear();
        for(int i=0; i<espacoLabels.size(); i++)
            listaOrdenada.push_back(new AuxiliaOrdenacao(possibilidades[espacoLabels[i]]->numCompConexas, i));  
        sort(listaOrdenada.begin(), listaOrdenada.end(), compara_sort_b);
        
        count = 0;
        for(int i=0; i<listaOrdenada.size(); i++) {
            if(listaOrdenada[i]->numCompConexas > listaOrdenada[0]->numCompConexas*alpha)
                break;
            count++;
        }
        aleatorio = rand() % count;
        //aleatorio = rand() % (int)ceil(listaOrdenada.size()*(alpha-1));
        solucao->push_back(espacoLabels[listaOrdenada[aleatorio]->posLabel]);
        espacoLabels.erase(espacoLabels.begin()+listaOrdenada[aleatorio]->posLabel);
        
        numCompConexas = listaOrdenada[aleatorio]->numCompConexas;
        if(numCompConexas > 1)
            for(int i=0; i<espacoLabels.size(); i++)
                possibilidades[espacoLabels[i]]->addLabel(grafo->edges, solucao->at(solucao->size()-1));
        
        for(int i=0; i<listaOrdenada.size(); i++)
            delete listaOrdenada[i];
    }while(numCompConexas > 1);

    for(int i=0; i<numLabels; i++)
        delete possibilidades[i];

    return solucao;
}

vector<int>* MVCA(Grafo* grafo) {
    return auxMVCA(grafo, 0, 0);
}

vector<int>* MVCARandomizado(Grafo* grafo, float alpha, int heuristica) { //heuristica(0 - alpha aplicado no tamanho do vetor, 1 - alpha aplicado no menor número de componentes conexas)
    return auxMVCA(grafo, alpha, heuristica);
}

vector<int>* MVCAReativo(Grafo* grafo, int numInteracoes, int tamanhoBloco, float* tempoBuscaLocal, float* melhorAlpha, int* iteracao, int* solucaoConstrutivo, clock_t* tempoMelhorSolucao) {
    vector<int>* melhorSolucao = nullptr;
    vector<int>* solucao;
    vector<float> alphas;
    vector<float> probAlphas;
    vector<int> countAlphas;
    vector<int> sumCustoSolucoes;
    float acumulada;
    float sumMediaCustoAlphas;
    int tamanhoMelhorSolucao = INT_MAX;
    int countBloco;
    int aleatorio;
    int indiceAlpha;
    int auxSolucaoConstrutivo;
    clock_t tempo[2];

    *tempoBuscaLocal = 0;
    for(int i=1; i<=NUMALPHAS; i++) {
        alphas.push_back(0.05 * i);
        solucao = MVCARandomizado(grafo, alphas[i-1], 1); //Tirei o "alpha[i-1]/100" pra porcentagem voltar para forma de 0.93
        tempo[0] = clock();
        buscaLocalDoisPorUm(grafo, solucao);
        tempo[1] = clock();
        *tempoBuscaLocal += (float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC;
        countAlphas.push_back(1);
        sumCustoSolucoes.push_back(solucao->size());
        delete solucao;
    }

    for(int i=0; i<alphas.size(); i++)
        probAlphas.push_back(100.0/NUMALPHAS);

    countBloco = 0;
    for(int i=NUMALPHAS; i<numInteracoes; i++) {
        if(countBloco == tamanhoBloco) {
            countBloco = 0;
            sumMediaCustoAlphas = 0;
            for(int j=0; j<alphas.size(); j++)
                sumMediaCustoAlphas += 1.0/(sumCustoSolucoes[j]/countAlphas[j]);
            for(int j=0; j<alphas.size(); j++)
                probAlphas[j] = ((1.0/(sumCustoSolucoes[j]/countAlphas[j])) / sumMediaCustoAlphas) * 100;
        }

        aleatorio = rand() % 100;
        acumulada = 0;
        for(int j=0; j<alphas.size(); j++) {
            acumulada += probAlphas[j];
            if(aleatorio < acumulada) {
                indiceAlpha = j;
                break;
            }
        }

        solucao = MVCARandomizado(grafo, alphas[indiceAlpha], 1); //Tirei o "alpha[i-1]/100" pra porcentagem voltar para forma de 0.93
        auxSolucaoConstrutivo = solucao->size();
        tempo[0] = clock();
        buscaLocalDoisPorUm(grafo, solucao);
        tempo[1] = clock();
        *tempoBuscaLocal += (float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC;

        countAlphas[indiceAlpha]++;
        sumCustoSolucoes[indiceAlpha] += solucao->size();
        if(solucao->size() < tamanhoMelhorSolucao) {
            *tempoMelhorSolucao = clock();
            if(melhorSolucao != nullptr)
                delete melhorSolucao;
            *solucaoConstrutivo = auxSolucaoConstrutivo;
            melhorSolucao = solucao;
            tamanhoMelhorSolucao = solucao->size();
            *melhorAlpha = alphas[indiceAlpha];
            *iteracao = i;
        }
        else
            delete solucao;
        countBloco++;
        
    }
    
    return melhorSolucao;
}

vector<int>* GRASP(Grafo* grafo, float tempoLimite, clock_t* tempoMelhorSolucao, float* tempoBuscaLocal, int* solucaoConstrutivo, GRBEnv* env, int custoOtimo, int* numIteracoes, int* numSolucoesRepetidas) {
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

            verifica = false;
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
            }

            return solucao;
        }
        
        auxTempoBuscaLocal[0] = clock();
        //buscaLocalExcedente(grafo, solucao);
        if(solucao->size() > 1)
            buscaLocalMIP(grafo, solucao, env, &mipGap, 2);
        auxTempoBuscaLocal[1] = clock();
        if(tempoBuscaLocal !=  nullptr)
            *tempoBuscaLocal += (float)(auxTempoBuscaLocal[1] - auxTempoBuscaLocal[0]) / CLOCKS_PER_SEC;

        verifica = false;
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
        }

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
    } while((float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC < tempoLimite);

    *numIteracoes = i;
    //cout << "Numero de Iteracoes: " << i << endl;
    return melhorSolucao;
}

vector<int>* GRASPReativo(Grafo* grafo, int tempoLimite, int tamanhoBloco, clock_t* tempoMelhorSolucao, float* tempoBuscaLocal, int* solucaoConstrutivo, GRBEnv* env, int custoOtimo, int* numIteracoes, int* numSolucoesRepetidas) {
    vector<int>* melhorSolucao = nullptr;
    vector<int>* solucao;
    clock_t tempo[2];
    clock_t auxTempoBuscaLocal[2];
    int tamanhoMelhorSolucao = INT_MAX;
    int auxSolucaoConstrutivo;
    int countIteracoes;

    int numAlphas = 6;
    int aleatorio;
    int indiceAlpha;
    float acumulada;
    float alphas[numAlphas] = {1.05, 1.10, 1.15, 1.20, 1.25, 1.30};
    float sumMediaCustoAlphas;
    vector<float> probAlphas;
    vector<int> countAlphas;
    vector<int> sumCustoSolucoes;

    bool verifica;
    vector<vector<int>> solucoes;

    double mipGap;

    for(int i=0; i<numAlphas; i++)
        probAlphas.push_back(100.0/numAlphas);

    tempo[0] = clock();
    countIteracoes = 0;
    *tempoBuscaLocal = 0;
    *numSolucoesRepetidas = 0;
    do {
        if(countIteracoes < numAlphas) {
            solucao = auxMVCAGRASP(grafo, 0, alphas[countIteracoes]); 
            indiceAlpha = countIteracoes;
            countAlphas.push_back(0);
            sumCustoSolucoes.push_back(0);
        } else {
            acumulada = 0;
            for(int j=0; j<numAlphas; j++) {
                acumulada += probAlphas[j];
                if(aleatorio < acumulada) {
                    indiceAlpha = j;
                    break;
                }
            }
            solucao = auxMVCAGRASP(grafo, countIteracoes-numAlphas, alphas[indiceAlpha]); 
        }
        auxSolucaoConstrutivo = solucao->size();

        if(solucao->size() == custoOtimo) {
            *tempoMelhorSolucao = clock();
            *solucaoConstrutivo = auxSolucaoConstrutivo;
            *numIteracoes = countIteracoes+1;

            verifica = false;
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
            }

            return solucao;
        }

        auxTempoBuscaLocal[0] = clock();
        //buscaLocalExcedente(grafo, solucao);
        //buscaLocalDoisPorUm(grafo, solucao);
        if(solucao->size() > 1)
            buscaLocalMIP(grafo, solucao, env, &mipGap, 2);
        auxTempoBuscaLocal[1] = clock();
        *tempoBuscaLocal += (float)(auxTempoBuscaLocal[1] - auxTempoBuscaLocal[0]) / CLOCKS_PER_SEC;

        countAlphas[indiceAlpha]++;
        sumCustoSolucoes[indiceAlpha] += solucao->size();

        verifica = false;
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
        }
        
        if(solucao->size() < tamanhoMelhorSolucao) {
            *tempoMelhorSolucao = clock();
            if(melhorSolucao != nullptr)
                delete melhorSolucao;
            melhorSolucao = solucao;
            *solucaoConstrutivo = auxSolucaoConstrutivo;

            if(solucao->size() == custoOtimo) {
                *numIteracoes = countIteracoes+1;
                return melhorSolucao;
            }

            tamanhoMelhorSolucao = solucao->size();
        }
        else
            delete solucao;
        
        countIteracoes++;
        if(countIteracoes % tamanhoBloco == 0) {
            for(int i=0; i<numAlphas; i++)
                cout << probAlphas[i] << " ";
            cout << endl;

            sumMediaCustoAlphas = 0;
            for(int j=0; j<numAlphas; j++)
                sumMediaCustoAlphas += 1.0/pow(2,sumCustoSolucoes[j]/countAlphas[j]);
            for(int j=0; j<numAlphas; j++)
                probAlphas[j] = (1/(sumMediaCustoAlphas * pow(2,sumCustoSolucoes[j]/countAlphas[j]))) * 100;
        }
        tempo[1] = clock();
    } while((float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC < tempoLimite);

    delete env;

    *numIteracoes = countIteracoes;

    //cout << "Numero de Iteracoes: " << countIteracoes << endl;
    return melhorSolucao;
}

vector<int>* auxPMVCA(Grafo* grafo, float beta, float theta) {
    int numVertices = grafo->roots.size();
    int numLabels = grafo->edges.size();
    int aleatorio;
    int iteracao;
    int numCompConexas;
    float alpha;
    vector<int>* solucao;
    vector<AuxiliaOrdenacao*> listaOrdenada;

    for(int i=0; i<numLabels; i++) {
        listaOrdenada.push_back(new AuxiliaOrdenacao(new Grafo(numVertices, numLabels), i));
        listaOrdenada[i]->grafo->addLabel(grafo->edges, i);
    }

    solucao = new vector<int>;
    iteracao = 0;  
    do {
        sort(listaOrdenada.begin(), listaOrdenada.end(), compara_sort);
        alpha = listaOrdenada.size() * beta * pow(theta, iteracao);
        if(alpha > 1)
            aleatorio = rand() % (int)ceil(alpha);
        else
            aleatorio = 0;
        
        numCompConexas = listaOrdenada[aleatorio]->grafo->numCompConexas;
        solucao->push_back(listaOrdenada[aleatorio]->label);
        
        delete listaOrdenada[aleatorio];
        listaOrdenada.erase(listaOrdenada.begin()+aleatorio);
        if(numCompConexas > 1)
            for(int i=0; i<listaOrdenada.size(); i++)
                listaOrdenada[i]->grafo->addLabel(grafo->edges, solucao->at(solucao->size()-1));

        iteracao++;
    }while(numCompConexas > 1);
 
    for(int i=0; i<listaOrdenada.size(); i++)
        delete listaOrdenada[i];

    return solucao;
}

vector<int>* GRASPReativo(Grafo* grafo, int tempoLimite, clock_t* tempoMelhorSolucao, float* tempoBuscaLocal, int* solucaoConstrutivo, GRBEnv* env, int custoOtimo, int* numIteracoes, int* numSolucoesRepetidas, float* melhorBeta, float* melhorTheta) {
    vector<int>* melhorSolucao = nullptr;
    vector<int>* solucao;
    clock_t tempo;
    clock_t auxTempoBuscaLocal[2];
    int tamanhoMelhorSolucao = INT_MAX;
    int auxSolucaoConstrutivo;

    float beta[8] = {0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7};
    float theta[4] = {0.1, 0.2, 0.3, 0.4};

    bool verifica;
    vector<vector<int>> solucoes;

    double mipGap;

    tempo = clock();
    *numIteracoes = 0;
    *tempoBuscaLocal = 0;
    *numSolucoesRepetidas = 0;
    int countIteracoes = 0;
    do {
        for(int j=0; j<8; j++) {
            for(int k=0; k<4; k++) {
                solucao = auxPMVCA(grafo, beta[j], theta[k]); 
                auxSolucaoConstrutivo = solucao->size();

                *melhorBeta = beta[j];
                *melhorTheta = theta[k];

                if(solucao->size() == custoOtimo) {
                    *tempoMelhorSolucao = clock();
                    *solucaoConstrutivo = auxSolucaoConstrutivo;
                    *numIteracoes += 1;

                    verifica = false;
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
                    }

                    return solucao;
                }

                auxTempoBuscaLocal[0] = clock();
                if(solucao->size() > 1)
                    buscaLocalMIP(grafo, solucao, env, &mipGap, 2);
                auxTempoBuscaLocal[1] = clock();
                *tempoBuscaLocal += (float)(auxTempoBuscaLocal[1] - auxTempoBuscaLocal[0]) / CLOCKS_PER_SEC;

                verifica = false;
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
                }
        
                if(solucao->size() < tamanhoMelhorSolucao) {
                    *tempoMelhorSolucao = clock();
                    if(melhorSolucao != nullptr)
                        delete melhorSolucao;
                    melhorSolucao = solucao;
                    *solucaoConstrutivo = auxSolucaoConstrutivo;

                    *melhorBeta = beta[j];
                    *melhorTheta = theta[k];

                    if(solucao->size() == custoOtimo) {
                        *numIteracoes += 1;
                        return melhorSolucao;
                    }

                    tamanhoMelhorSolucao = solucao->size();
                }
                else
                    delete solucao;
            }
        }
        countIteracoes++;
    } while((float)(clock() - tempo) / CLOCKS_PER_SEC < tempoLimite);

    delete env;

    cout << "Numero de Iteracoes: " << countIteracoes << endl;
    return melhorSolucao;
}

vector<int>* pMVCA(Grafo* grafo, int tempoLimite, clock_t* tempoMelhorSolucao) {
    vector<int>* melhorSolucao = nullptr;
    vector<int>* solucao;
    float beta[8] = {0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7};
    float theta[4] = {0.1, 0.2, 0.3, 0.4};
    float melhorBeta;
    float melhorTheta;
    int tamanhoMelhorSolucao = INT_MAX;
    int seed = 0;
    clock_t tempo[2];

    tempo[0] = clock();
    do {
        for(int j=0; j<8; j++) {
            for(int k=0; k<4; k++) {
                solucao = auxPMVCA(grafo, beta[j], theta[k]);
                buscaLocalExcedente(grafo, solucao);
                if(solucao->size() < tamanhoMelhorSolucao) {
                    if(tempoMelhorSolucao != nullptr)
                        *tempoMelhorSolucao = clock();
                    if(melhorSolucao != nullptr)
                        delete melhorSolucao;
                    melhorSolucao = solucao;
                    tamanhoMelhorSolucao = solucao->size();
                    melhorBeta = beta[j];
                    melhorTheta = theta[k];
                }
                else
                    delete solucao;
            }
        }
        seed++;
        tempo[1] = clock();
    }while((float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC < tempoLimite);

    cout << "Melhor Beta: " << melhorBeta << ", Melhor Theta: " << melhorTheta << endl;
    return melhorSolucao;
}

vector<int>* mip(Grafo* grafo, vector<int>* initialSolution, double* mipGap) {
    Grafo* solucao; //graph
    vector<int>* labelsSolucao; //labels in the solution
    stringstream ss;
    
    GRBEnv* env = 0;
    GRBVar* z;
    
    double* lb;
    double* ub;
    double* obj;
    char* type;
    string* variaveis;

    vector<int>* labels;
    int numLabels = grafo->edges.size(); //number of labels on the graph
    
    env = new GRBEnv();
    
    GRBModel model = GRBModel(*env);
    model.set(GRB_StringAttr_ModelName, "MLST");
    model.set(GRB_IntParam_OutputFlag, 0);
    model.set(GRB_DoubleParam_TimeLimit, 36000); //10h
    model.set(GRB_IntParam_LazyConstraints, 1);
    
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
    for(int i=0; i<initialSolution->size(); i++)
        z[initialSolution->at(i)].set(GRB_DoubleAttr_Start, 1);

    model.set(GRB_IntAttr_ModelSense, GRB_MINIMIZE);
    
    GRBLinExpr sum;

    //add the restriction about each node being present in the solution graph
    for(int i=0; i<grafo->roots.size(); i++) {
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
    model.addConstr(sum >= grafo->roots.size()-1, "numero minimo de arestas");

    mycallback cb = mycallback(numLabels, z, grafo);
    model.setCallback(&cb);

    model.optimize();    

    labelsSolucao = new vector<int>;

    for(int i=0; i<numLabels; i++) {
        if(z[i].get(GRB_DoubleAttr_X) >= 0.99)
            labelsSolucao->push_back(i);
    }
    cout << endl;

    *mipGap = model.get(GRB_DoubleAttr_MIPGap);
    
    delete []variaveis;
    delete []lb;
    delete []ub;
    delete []obj;
    delete []type;
    delete env;
    return labelsSolucao;
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
    for(int i=0; i<grafo->edges.size(); i++)
        espacoLabels[0]->push_back(new Label(i, grafo->numArestasLabels[i]));
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
                for(int j=0; j<grafo->edges.size(); j++)
                    if(!taNoVetor(&(closed[closed.size()-1]->solucao), j))
                        espacoLabels[camada]->push_back(new Label(j, grafo->numArestasLabels[j]));
            }
            novoNo = new No(closed[closed.size()-1]->solucao, espacoLabels[camada]->at(i)->label);
            open.push_back(novoNo);

            arvore = new Grafo(grafo->roots.size(), grafo->edges.size());
            for(int j=0; j<novoNo->solucao.size(); j++)
                arvore->addLabel(grafo->edges, novoNo->solucao[j]);
            removeCiclos(arvore);
            
            arestasNecessarias = grafo->roots.size() - 1 - arvore->numArestas;
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

int calculaDiferenca(vector<int>* solucao1, vector<int>* solucao2) {
    int diferenca = 0;
    bool achou;
    vector<bool> naoAchados;

    for(int i=0; i<solucao2->size(); i++)
        naoAchados.push_back(false);

    for(int i=0; i<solucao1->size(); i++) {
        achou = false;
        for(int j=0; j<solucao2->size(); j++) {
            if(solucao1->at(i) == solucao2->at(j)) {
                naoAchados[j] = true;
                achou = true;
                break;
            }
        }
        if(!achou)
            diferenca++;
    }
    
    for(int i=0; i<naoAchados.size(); i++)
        if(!naoAchados[i])
            diferenca++;

    return diferenca;
}

vector<int>* pertubacao(Grafo* grafo, vector<int>* solucao, float* tempoBuscaLocal, bool* valida, float alpha, GRBEnv* env) {
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
    vector<int>* vizinha = new vector<int>;
    vector<AuxiliaOrdenacao*> listaOrdenada;
    vector<Grafo*> possibilidades;
    vector<int> espacoLabels;
    vector<int> retirados;
    vector<int> possibilidadesNaoRemovidas;
    //float alphas[3] = {1.05, 1.15, 1.3};
    Grafo* grafoVizinha; 
    
    numLabels = grafo->edges.size();
    
    //REMOVE 20% DOS LABELS
    for(int i=0; i<solucao->size(); i++) { 
        if(rand() % 100 >= 80) //Com 70 a 200-0.2-200.2 foi
            retirados.push_back(solucao->at(i));
        else
            vizinha->push_back(solucao->at(i));
    }
    
    //FIM REMOVE 20% DOS LABELS
    //REMOVE 20% DOS LABELS, ONDE QUANTO MENOS ARESTAS O LABEL POSSUI, MAIOR A CHANCE DELE SER REMOVIDO
    /*
    totalArestas = 0;
    for(int i=0; i<solucao->size(); i++) {
        totalArestas += grafo->numArestasLabels[solucao->at(i)];
        auxRetirados[i] = true;
    }

    for(int i=0; i<ceil(solucao->size()*0.9); i++) {
        aleatorio = rand() % totalArestas;
        acumulada = 0;
        for(int j=0; j<solucao->size(); j++) {
            acumulada += grafo->numArestasLabels[solucao->at(j)];

            if(acumulada > aleatorio) {
                vizinha->push_back(solucao->at(j));
                //retirados.push_back(solucao->at(j));
                auxRetirados[j] = false;
                break;
            }
        }
    }

    for(int i=0; i<solucao->size(); i++)
        if(auxRetirados[i])
            retirados.push_back(solucao->at(i));
            //vizinha->push_back(solucao->at(i));
    */
    //FIM REMOVE 20% DOS LABELS, ONDE QUANTO MENOS ARESTAS O LABEL POSSUI, MAIOR A CHANCE DELE SER REMOVIDO
    
    count = 0;
    for(int i=0; i<numLabels; i++) { //ACHO QUE TA ERRADO, NÃO TA ADICIONANDO NOS GRAFOS OS LABELS QUE NÃO FORAM REMOVIDOS. TA CRIANDO OUTRAS SOLUÇÕES DO ZERO
        //if(!taNoVetor(vizinha, i) && !taNoVetor(&retirados, i)) {
        if(!taNoVetor(vizinha, i)) {
            possibilidades.push_back(new Grafo(grafo->roots.size(), numLabels));
            possibilidades[count]->addLabel(grafo->edges, i);
            for(int j=0; j<vizinha->size(); j++)
                possibilidades[count]->addLabel(grafo->edges, vizinha->at(j));
            espacoLabels.push_back(i);
            possibilidadesNaoRemovidas.push_back(count);
            count++;
        }
    }
    
    do {
        if(espacoLabels.empty()) {
            for(int i=0; i<possibilidades.size(); i++)
                delete possibilidades[i];

            *valida = false;
            return vizinha;
        }

        listaOrdenada.clear();
        for(int i=0; i<possibilidadesNaoRemovidas.size(); i++)
            listaOrdenada.push_back(new AuxiliaOrdenacao(possibilidades[possibilidadesNaoRemovidas[i]]->numCompConexas, i));  
        sort(listaOrdenada.begin(), listaOrdenada.end(), compara_sort_b);
        
        count = 0;
        for(int i=0; i<listaOrdenada.size(); i++) {
            //if(listaOrdenada[i]->numCompConexas > listaOrdenada[0]->numCompConexas*alphas[rand()%3])
            if(listaOrdenada[i]->numCompConexas > listaOrdenada[0]->numCompConexas*alpha)
                break;
            count++;
        }
        aleatorio = rand() % count;
        //aleatorio = rand() % (int)ceil(listaOrdenada.size()*(alpha-1));
        vizinha->push_back(espacoLabels[listaOrdenada[aleatorio]->posLabel]);
        espacoLabels.erase(espacoLabels.begin()+listaOrdenada[aleatorio]->posLabel);
        possibilidadesNaoRemovidas.erase(possibilidadesNaoRemovidas.begin()+listaOrdenada[aleatorio]->posLabel);
        
        numCompConexas = listaOrdenada[aleatorio]->numCompConexas;
        if(numCompConexas > 1)
            for(int i=0; i<possibilidadesNaoRemovidas.size(); i++)
                possibilidades[possibilidadesNaoRemovidas[i]]->addLabel(grafo->edges, vizinha->back());

        for(int i=0; i<listaOrdenada.size(); i++)
            delete listaOrdenada[i];
            
    }while(numCompConexas > 1);
    
    for(int i=0; i<possibilidades.size(); i++)
        delete possibilidades[i];
    
    /*
    for(int i=0; i<vizinha->size(); i++)
        cout << vizinha->at(i) << " ";
    cout << endl;*/

    tempo = clock();
    buscaLocalExcedente(grafo, vizinha);
    double mipGap;
    //buscaLocalMIP(grafo, vizinha, env, &mipGap, 2);
    *tempoBuscaLocal += ((float)(clock() - tempo));

    /*for(int i=0; i<vizinha->size(); i++)
        cout << vizinha->at(i) << " ";
    cout << endl;*/

    *valida = true;
    
    return vizinha;
    
}

vector<int>* SA(Grafo* grafo, vector<int>* initialSolution, double tempInicial, double tempFinal, int numIteracoes, double alpha, clock_t* tempoMelhorSolucao, float* tempoBuscaLocal, int custoOtimo, int* numSolucoes, int* numSolucoesRepetidas, GRBEnv* env) {
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
    float alphas[numAlphas] = {1.05, 1.1, 1.15, 1.2, 1.25, 1.3};
    float acumulada;
    int aleatorio;
    int indiceAlpha;
    float sumMediaCustoAlphas;
    vector<float> probAlphas;
    vector<int> countAlphas;
    vector<int> sumCustoSolucoes;

    for(int i=0; i<numAlphas; i++)
        probAlphas.push_back(100.0/numAlphas);

    
    vector<vector<int>> contabiliza;  
    *numSolucoesRepetidas = 0;
    *numSolucoes = 0;
    vector<int> a;
    for(int j=0; j<solucao->size(); j++)
        a.push_back(solucao->at(j));
    contabiliza.push_back(a);
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
        for(int i=0; i<numIteracoes; i++) {       
            if(*numSolucoes < numAlphas) {
                novaSolucao = pertubacao(grafo, solucao, tempoBuscaLocal, &valida, alphas[*numSolucoes], env);
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
                novaSolucao = pertubacao(grafo, solucao, tempoBuscaLocal, &valida, alphas[indiceAlpha], env);
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

            difQualidade = novaSolucao->size() - solucao->size();
            if(difQualidade <= 0) {
                //auxTempoBuscaLocal = clock();
                //buscaLocalDoisPorUm(grafo, novaSolucao);
                //*tempoBuscaLocal += (float)(clock() - auxTempoBuscaLocal);
                count++;
                if(!aux)
                    delete solucao;
                solucao = novaSolucao;

                /*if(difQualidade < 0) {
                    auxTempoBuscaLocal = clock();
                    buscaLocalDoisPorUm(grafo, solucao);
                    *tempoBuscaLocal += (float)(clock() - auxTempoBuscaLocal);
                }*/
                
                if(solucao->size() < melhorSolucao->size()) {
                    cout << "A: " << solucao->size() << " - " << melhorSolucao->size() << endl;
                    *tempoMelhorSolucao = clock();

                    auxTempoBuscaLocal = clock();
                    //buscaLocalDoisPorUm(grafo, solucao);
                    if(solucao->size() > 1)
                        buscaLocalMIP(grafo, solucao, env, &mipGap, 3);
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
                            delete env;

                            return melhorSolucao;
                        }
                    } else {
                        aux = true;
                        delete melhorSolucao;
                        melhorSolucao = solucao;
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
                    /*
                    auxTempoBuscaLocal = clock();
                    buscaLocalDoisPorUm(grafo, solucao);
                    *tempoBuscaLocal += (float)(clock() - auxTempoBuscaLocal);
                    if(solucao->size() < melhorSolucao->size()) {
                        *tempoMelhorSolucao = clock();
                        cout << "B: " << solucao->size() << " - " << melhorSolucao->size() << endl;
                        delete melhorSolucao;
                        melhorSolucao = solucao;
                        aux = true;
                    }*/
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
        auxTempoBuscaLocal = clock();
        //buscaLocalDoisPorUm(grafo, solucao);
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
                delete env;

                return melhorSolucao;
            }
        }
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
    delete env;

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

void cenarioUm() {
    Grafo* grafos[10];
    vector<int>* solucao;
    clock_t tempo[2];
    stringstream ss;
    float duracao;
    float mediaTempoMelhorSolucao;
    float br;
    const char* filePath;
    
    srand(0);
    for(int i=0; i<10; i++)
        grafos[i] = nullptr;
    
    filePath = "dataset/instances/g1/50/ld/";

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
            return;  
        }
    /*
    cout << "----- MVCA -----" << endl;
    tempo[0] = clock();
    br = 0;
    for(int i=0; i<10; i++) {
        solucao = MVCA(grafos[i]);
        br += solucao->size();
    }
    br /= 10;
    tempo[1] = clock();
    duracao = (float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC;
    cout << "Tamanho da Solucao Media (br): " << br;
    cout << ", Tempo: " << duracao << "s" << endl << endl;
    delete solucao;
    
    cout << "----- EXATO -----" << endl;
    br = 0;
    mediaTempoMelhorSolucao = 0;
    for(int i=0; i<10; i++) {
        cout << "Instancia: " << i << endl;
        tempo[0] = clock();
        solucao = exato(grafos[i]);
        tempo[1] = clock();
        mediaTempoMelhorSolucao += (float)(tempo[1] - tempo[0]);
        br += solucao->size();
        cout << mediaTempoMelhorSolucao / CLOCKS_PER_SEC << endl;
        
    }
    mediaTempoMelhorSolucao /= CLOCKS_PER_SEC;
    mediaTempoMelhorSolucao /= 10;
    br /= 10;
    cout << "Tamanho da Solucao Media (br): " << br;
    cout << ", Tempo: " << mediaTempoMelhorSolucao << "s" << endl << endl;
    delete solucao;
    
    cout << "----- MVCA RANDOMIZADO -----" << endl;
    tempo[0] = clock();
    br = 0;
    for(int i=0; i<10; i++) {
        solucao = MVCARandomizado(grafos[i], 0.1, 0);
        br += solucao->size();
        delete solucao;
    }
    br /= 10;
    tempo[1] = clock();
    duracao = (float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC;
    cout << "Tamanho da Solucao Media (br): " << br;
    cout << ", Tempo: " << duracao << "s" << endl << endl;
    */
    cout << "----- MVCA Parametrizado -----" << endl;
    tempo[0] = clock();
    br = 0;
    mediaTempoMelhorSolucao = 0;
    for(int i=0; i<10; i++) {
        cout << "Instancia: " << i << endl;
        tempo[0] = clock();
        solucao = pMVCA(grafos[i], 1, &tempo[1]);
        mediaTempoMelhorSolucao += (float)(tempo[1] - tempo[0]);
        br += solucao->size();
        cout << "Custo: " << solucao->size() << ", Tempo: " << (float)(tempo[1] - tempo[0]) / (1.657*CLOCKS_PER_SEC) << "s" << endl;
        delete solucao;
    }
    mediaTempoMelhorSolucao /= CLOCKS_PER_SEC;
    mediaTempoMelhorSolucao *= 1000;
    mediaTempoMelhorSolucao /= 10;
    br /= 10;
    cout << "Tamanho da Solucao Media (br): " << br;
    cout << ", Tempo: " << mediaTempoMelhorSolucao << "ms" << endl << endl;
    
    for(int i=0; i<10; i++)
        delete grafos[i];
}

void cenarioDois(string entrada, string saida, int numIteracoes, int tamanhoBloco, int seed) {
    FILE* file;
    Grafo* grafo;
    vector<int>* solucao;
    clock_t tempo[2];
    clock_t tempoMelhorSolucao;
    stringstream ss;
    float tempoBuscaLocal;
    float duracao;
    float duracaoTempoMelhorSolucao;
    float melhorAlpha;
    int solucaoConstrutivo;
    int iteracao;
    
    srand(seed);
    grafo = nullptr;
    /*
    cout << "----- INSTANCIAS: " << entrada << " -----" << endl << endl;
    cout << "Carregando Instancias" << endl;
    */
    tempo[0] = clock();
    grafo = carregaInstancias(entrada.c_str());
    tempo[1] = clock();

    duracao = (float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC;
    //cout << "Tempo para Carregar a Instancia: " << duracao << "s" << endl << endl;
    
    if(grafo == nullptr) {
        cout << "Grafo nulo" << endl;
        return;  
    }

    //cout << "----- MVCA REATIVO -----" << endl;
    
    tempo[0] = clock();
    solucao = MVCAReativo(grafo, numIteracoes, tamanhoBloco, &tempoBuscaLocal, &melhorAlpha, &iteracao, &solucaoConstrutivo, &tempoMelhorSolucao);
    tempo[1] = clock();

    duracao = (float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC;
    duracaoTempoMelhorSolucao = (float)(tempoMelhorSolucao - tempo[0]) / CLOCKS_PER_SEC;

    duracao *= 1000; //passando para ms
    duracaoTempoMelhorSolucao *= 1000;
    tempoBuscaLocal *= 1000; //passando para ms

    file = fopen(saida.c_str(), "a+");

    ss.str("");
    ss.clear();
    ss << "\n" << solucao->size() << ";" << solucaoConstrutivo << ";" << duracao << ";" << duracao - tempoBuscaLocal << ";" << tempoBuscaLocal << ";" << duracaoTempoMelhorSolucao << ";" << melhorAlpha << ";" << iteracao << ";" << seed;
    fputs(ss.str().c_str(), file);
    
    fclose(file);
    //cout << "Tamanho da Solucao: " << solucao->size();
    //cout << ", Tempo: " << duracao << "ms" << ", Tempo Construtor: " << duracao - tempoBuscaLocal << "ms" << ", Tempo Busca Local: " << tempoBuscaLocal << "ms" << endl << endl;

    delete solucao;
    delete grafo;
}

void cenarioTres(string entrada, string saida, int tempoLimite, int seed, int metodo) {
    FILE* file;
    Grafo* grafo;
    vector<int>* solucao;
    clock_t tempo[2];
    stringstream ss;
    float tempoBuscaLocal;
    float duracao;
    float tempoMelhorSolucao;
    int solucaoConstrutivo;
    int tamanhoBloco = 25;
    int custoOtimo;
    int numIteracoes;
    int numSolucoesRepetidas;
    
    srand(seed);
    GRBEnv* env = new GRBEnv();
    grafo = nullptr;
    /*
    cout << "----- INSTANCIAS: " << entrada << " -----" << endl << endl;
    cout << "Carregando Instancias" << endl;
    */
    
    tempo[0] = clock();
    grafo = carregaInstancias(entrada.c_str());
    tempo[1] = clock();
    duracao = (float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC;
    //cout << "Tempo para Carregar a Instancia: " << duracao << "s" << endl << endl;
    
    if(grafo == nullptr) {
        cout << "Grafo nulo" << endl;
        return;  
    }

    //cout << "----- GRASP -----" << endl;

    custoOtimo = custoSolucaoExata(entrada);
    
    tempo[0] = clock();
    if(metodo == 1)
        solucao = GRASP(grafo, tempoLimite, &tempo[1], &tempoBuscaLocal, &solucaoConstrutivo, env, custoOtimo, &numIteracoes, &numSolucoesRepetidas);
    else
        solucao = GRASPReativo(grafo, tempoLimite, tamanhoBloco, &tempo[1], &tempoBuscaLocal, &solucaoConstrutivo, env, custoOtimo, &numIteracoes, &numSolucoesRepetidas);
    duracao += (float)(clock() - tempo[0]) / CLOCKS_PER_SEC;
    tempoMelhorSolucao = (float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC;

    duracao *= 1000; //passando para ms
    tempoBuscaLocal *= 1000; //passando para ms
    tempoMelhorSolucao *= 1000;

    file = fopen(saida.c_str(), "a+");

    ss.str("");
    ss.clear();
    ss << "\n" << solucao->size() << ";" << solucaoConstrutivo << ";" << duracao << ";" << duracao - tempoBuscaLocal << ";" << tempoBuscaLocal << ";" << tempoLimite << ";" << tempoMelhorSolucao << ";" << numIteracoes << ";" << numIteracoes << ";" << numSolucoesRepetidas << ";" << seed;
    fputs(ss.str().c_str(), file);
    
    fclose(file);
    cout << "Tamanho da Solucao: " << solucao->size();
    cout << ", Tempo total: " << duracao << "ms" << ", Tempo Busca Local: " << tempoBuscaLocal << "ms" << ", Tempo Construtor: " << duracao - tempoBuscaLocal << "ms" << ", Tempo melhor solucao: " << tempoMelhorSolucao << "ms, Numero de iteracoes: " << numIteracoes << endl << endl;

    delete solucao;
    delete grafo;
}

void cenarioQuatro(string entrada, string saida) {
    FILE* file;
    Grafo* grafo;
    vector<int>* solucao;
    vector<int>* solucaoInicial;
    clock_t tempo[2];
    stringstream ss;
    float tempoSolucaoInicial;
    float duracao;
    float tempoBuscaLocal;
    float tempoLimiteGrasp;
    float limitesTempoGRASP[6][4] = {{0.01, 0.01, 0.02, 0.02}, {0.01, 0.03, 0.05, 0.1}, {0.03, 0.05, 0.1, 0.2}, 
    {0.03, 0.05, 0.1, 0.2}, {600, 600, 600, 600}, {600, 600, 600, 600}};
    double mipGap;
    int numIteracoesGrasp;
    int numSolucoesRepetidas;

    GRBEnv* env = new GRBEnv();
    grafo = nullptr;
    
    tempo[0] = clock();
    grafo = carregaInstancias(entrada.c_str());
    tempo[1] = clock();
    duracao = (float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC;
    
    if(grafo == nullptr) {
        cout << "Grafo nulo" << endl;
        return;  
    }
    
    if(grafo->roots.size() <= 50) {
        if(grafo->edges.size() <= 30)
            tempoLimiteGrasp = limitesTempoGRASP[0][0];
        else
            tempoLimiteGrasp = limitesTempoGRASP[0][2];
    } else {   
        switch(grafo->roots.size()) {
            case 100:
                switch(grafo->edges.size()) {
                    case 25: tempoLimiteGrasp = limitesTempoGRASP[1][0];
                    break;
                    case 50: tempoLimiteGrasp = limitesTempoGRASP[1][1];
                    break;
                    case 100: tempoLimiteGrasp = limitesTempoGRASP[1][2];
                    break;
                    case 125: tempoLimiteGrasp = limitesTempoGRASP[1][3];
                    break;
                }
            break;
            case 200:
                switch(grafo->edges.size()) {
                    case 50: tempoLimiteGrasp = limitesTempoGRASP[2][0];
                    break;
                    case 100: tempoLimiteGrasp = limitesTempoGRASP[2][1];
                    break;
                    case 200: tempoLimiteGrasp = limitesTempoGRASP[2][2];
                    break;
                    case 250: tempoLimiteGrasp = limitesTempoGRASP[2][3];
                    break;
                }
            break;
            case 400:
                switch(grafo->edges.size()) {
                    case 100: tempoLimiteGrasp = limitesTempoGRASP[3][0];
                    break;
                    case 200: tempoLimiteGrasp = limitesTempoGRASP[3][1];
                    break;
                    case 400: tempoLimiteGrasp = limitesTempoGRASP[3][2];
                    break;
                    case 500: tempoLimiteGrasp = limitesTempoGRASP[3][3];
                    break;
                }
            break;
            case 500:
                switch(grafo->edges.size()) {
                    case 125: tempoLimiteGrasp = limitesTempoGRASP[4][0];
                    break;
                    case 250: tempoLimiteGrasp = limitesTempoGRASP[4][1];
                    break;
                    case 500: tempoLimiteGrasp = limitesTempoGRASP[4][2];
                    break;
                    case 625: tempoLimiteGrasp = limitesTempoGRASP[4][3];
                    break;
                }
            break;
            case 1000:
                switch(grafo->edges.size()) {
                    case 250: tempoLimiteGrasp = limitesTempoGRASP[5][0];
                    break;
                    case 500: tempoLimiteGrasp = limitesTempoGRASP[5][1];
                    break;
                    case 1000: tempoLimiteGrasp = limitesTempoGRASP[5][2];
                    break;
                    case 1250: tempoLimiteGrasp = limitesTempoGRASP[5][3];
                    break;
                }
            break;
        }
    } 
    tempo[0] = clock();
    solucaoInicial = GRASP(grafo, tempoLimiteGrasp, nullptr, nullptr, nullptr, env, 0, &numIteracoesGrasp, &numSolucoesRepetidas);
    tempo[1] = clock();
    tempoSolucaoInicial = (float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC;
    tempoSolucaoInicial *= 1000;

    cout << "Solucao Inicial: " << solucaoInicial->size() << ", Tempo: " << tempoSolucaoInicial << "ms" << endl;

    tempo[0] = clock();
    solucao = mip(grafo, solucaoInicial, &mipGap);
    tempo[1] = clock();
    duracao += (float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC;
    duracao *= 1000; //passando para ms

    Grafo* novoGrafo = new Grafo(grafo->roots.size(), grafo->edges.size());
    for(int i=0; i<solucao->size(); i++) {
        novoGrafo->addLabel(grafo->edges, solucao->at(i));
        cout << solucao->at(i) << "-" << grafo->numArestasLabels[solucao->at(i)] << " ";
    }
    if(novoGrafo->numCompConexas == 1)
        cout << " - Viavel";
    else 
        cout << " - INVIAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAVEL: " << novoGrafo->numCompConexas;
    cout << endl;
    delete novoGrafo;

    file = fopen(saida.c_str(), "w+");
    
    ss.str("");
    ss.clear();
    ss << "\n" << solucao->size() << ";" << duracao + tempoSolucaoInicial << ";" << tempoSolucaoInicial << ";" << mipGap;
    fputs(ss.str().c_str(), file);
    
    fclose(file);
    delete solucao;
    delete solucaoInicial;
    delete grafo;
}

void cenarioCinco(string entrada, string saida, int numIteracoes, double alpha, double tempInicial, double tempFinal, float tempoLimite, int seed) {
    FILE* file;
    Grafo* grafo;
    vector<int>* solucaoInicial;
    vector<int>* solucaoSA;
    clock_t tempo[2];
    stringstream ss;
    int custoOtimo;
    int custoSolucaoInicial;
    int solucaoConstrutivo;
    int numIteracoesGrasp;
    int numSolucoesSA;
    int numSolucoesRepetidas;
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

    srand(seed);
    custoOtimo = custoSolucaoExata(entrada);
    GRBEnv* env = new GRBEnv();
    
    tempo[0] = clock();
    solucaoInicial = GRASP(grafo, tempoLimite, &tempo[1], nullptr, &solucaoConstrutivo, env, custoOtimo, &numIteracoesGrasp, &numSolucoesRepetidas);
    tempoSolucaoInicial = (float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC;
    tempoSolucaoInicial *= 1000;
    tempoMelhorSolucao = tempoSolucaoInicial;

    custoSolucaoInicial = solucaoInicial->size();
    cout << "Solucao Inicial: " << solucaoInicial->size() << ", Tempo: " << tempoSolucaoInicial << "ms" << endl;
    
    if(solucaoInicial->size() != custoOtimo) {
        tempo[0] = clock();
        solucaoSA = SA(grafo, solucaoInicial, tempInicial, tempFinal, numIteracoes, alpha, &tempo[1], &tempoBuscaLocal, custoOtimo, &numSolucoesSA, &numSolucoesRepetidas, env);
        tempoSA = (float)(clock() - tempo[0]) / CLOCKS_PER_SEC;
        if(solucaoSA->size() < custoSolucaoInicial) {
            tempoMelhorSolucao = (float)(tempo[1] - tempo[0]) / CLOCKS_PER_SEC;
            tempoMelhorSolucao *= 1000;
        }
        tempoSA *= 1000; //passando para ms
        tempoSA += tempoSolucaoInicial;
    } else {
        solucaoSA = solucaoInicial;
        tempoSA = 0;
        tempoBuscaLocal = 0;
    }

    file = fopen(saida.c_str(), "a+");

    ss.str("");
    ss.clear();
    ss << "\n" << solucaoSA->size() << ";" << custoSolucaoInicial << ";" << tempoSolucaoInicial << ";" << tempoSA << ";" << tempoSA - tempoBuscaLocal << ";" << tempoBuscaLocal << ";" << tempoMelhorSolucao << ";" << tempoLimite*1000 << ";" << numSolucoesSA << ";" << numSolucoesRepetidas << ";" << seed;
    fputs(ss.str().c_str(), file);
    
    fclose(file);
    cout << "Tamanho da Solucao: " << solucaoSA->size();
    cout << ", Tempo Solucao Inicial: " << tempoSolucaoInicial << "ms" << ", Tempo SA: " << tempoSA << "ms" << ", Tempo SA (Busca Local): " << tempoBuscaLocal << "ms" << ", Tempo SA (Construtivo): " << tempoSA - tempoBuscaLocal << "ms" << ", Tempo melhor solucao: " << tempoMelhorSolucao << "ms" << endl << endl;
    
    delete solucaoSA;
    delete grafo;
}

int main(int argc, char **argv) { 
    int metodo = stoi(argv[1]) ;
    if(argc < 2) {
        cout << "Parametro necessario:metodo(0 - Reativo, 1 - GRASP, 2 - MIP)" << endl;
        return 0;
    }
    if(metodo == 0) {
        if(argc < 7) {
            cout << "Parametros necessarios: arquivo de entrada, arquivo de saida, N, B, seed" << endl;
            return 0;
        }
        cenarioDois(argv[2], argv[3], stoi(argv[4]), stoi(argv[5]), stoi(argv[6]));
    } else if(metodo == 1 || metodo == 4){
        if(argc < 6) {
            cout << "Parametros necessarios: arquivo de entrada, arquivo de saida, tempo limite(s), seed" << endl;
            return 0;
        }
        cenarioTres(argv[2], argv[3], stoi(argv[4]), stoi(argv[5]), metodo);
    } else if(metodo == 2){
        if(argc < 4) {
            cout << "Parametros necessarios: arquivo de entrada, arquivo de saida" << endl;
            return 0;
        }
        cenarioQuatro(argv[2], argv[3]);
    } else if(metodo == 3){
        if(argc < 10) {
            cout << "Parametros necessarios: arquivo de entrada, arquivo de saida, numero de iteracoes, taxa de decaimento, temperatura inicial e final, tempo limite do GRASP, seed" << endl;
            return 0;
        }
        cenarioCinco(argv[2], argv[3], stoi(argv[4]), stof(argv[5]), stof(argv[6]), stof(argv[7]), stof(argv[8]), stoi(argv[9]));
    }
}