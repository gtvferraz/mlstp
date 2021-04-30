#include <iostream>
#include <sstream>
#include <string.h>

using namespace std;

#define SEED 0
// Tenho que multiplicar meu tempo por 1,10616 para comparar com o do artigo
// g++ cpp/executaTodosGrupos.cpp -O3 -o executaTodosGrupos.out
// método(0 - Reativo, 1 - GRASP, 2 - MIP)
// Reativo: 0, grupo, numero de vertices, densidade, numero de labels, numero de execuções do reativo, N, B
// GRASP: 1, grupo, numero de vertices, densidade, numero de labels, numero de execuções do GRASP
// MIP: 2, grupo, numero de vertices, densidade, numero de labels
// SA: 3, grupo, numero de vertices, densidade, numero de labels, numero de execucoes do SA, numero de iteracoes, taxa de decaimento, temperatura inicial e final 
// GRASP Reativo: 4, grupo, numero de vertices, densidade, numero de labels, numero de execuções do GRASP
// ./executaTodosGrupos.out 0 1 20 0.8 12 10 200 50
// ./executaTodosGrupos.out 0 2 50 0.8 12 10 200 50
// ./executaTodosGrupos.out 1 1 20 0.8 12 10
// ./executaTodosGrupos.out 1 2 50 0.8 12 10
// ./executaTodosGrupos.out 2 1 20 0.8 12
// ./executaTodosGrupos.out 2 2 50 0.8 12
// ./executaTodosGrupos.out 3 1 20 0.8 12 10 50 0.9 300 0.001
// ./executaTodosGrupos.out 3 2 50 0.8 12 10 50 0.9 300 0.001
// ./executaTodosGrupos.out 4 1 20 0.8 12 10
// ./executaTodosGrupos.out 4 2 50 0.8 12 10

int main(int argc, char** argv) {
    FILE* file;
    char auxLinha[300];
    string path;
    string pathSaida;
    string linha;
    stringstream ss;

    int grupo;
    int numVertices;
    float densidade;
    int numLabels;
    int numExecucoes;
    int numIteracoes;
    int tamanhoBloco;
    int metodo;
    double taxaDecaimento;
    double tempInicial;
    double tempFinal;
    //int tempoLimiteLiteratura[6] = {1, 20, 60, 140, 300, 900};
    int tempoLimiteLiteratura[6] = {5, 25, 50, 140, 200, 800};
    float tempoLimiteGraspSA[6] = {0.001, 0.05, 0.1, 1, 1, 1};

    int n[4] = {20, 30, 40, 50};
    int n2[6] = {50, 100, 200, 400, 500, 1000};
    int label[6][4] = {{12, 25, 50, 62}, {25, 50, 100, 125}, {50, 100, 200, 250}, {100, 200, 400, 500}, {125, 250, 500, 625}, {250, 500, 1000, 1250}};
    string d[3] = {"hd", "md", "ld"};
    float auxD[3] = {0.8, 0.5, 0.2};

    if(argc < 2) {
        cout << "Parametro necessario:metodo(0 - Reativo, 1 - GRASP, 2 - MIP)" << endl;
        return 0;
    }
    metodo = stoi(argv[1]);
    if(metodo == 0) {
        if(argc < 9) {
            cout << "Parametros necessarios: grupo, numero de vertices, densidade, numero de labels, numero de execuçoes do reativo, N, B" << endl;
            return 0;
        }
        numExecucoes = atoi(argv[6]);
        numIteracoes = atoi(argv[7]);
        tamanhoBloco = atoi(argv[8]);
    } else if(metodo == 1 || metodo == 4){
        if(argc < 7) {
            cout << "Parametros necessarios: grupo, numero de vertices, densidade, numero de labels, numero de execuçoes do reativo" << endl;
            return 0;
        }
        numExecucoes = atoi(argv[6]);
    } else if(metodo == 2){
        if(argc < 6) {
            cout << "Parametros necessarios: grupo, numero de vertices, densidade, numero de labels" << endl;
            return 0;
        }
    } else if(metodo == 3){
        if(argc < 11) {
            cout << "Parametros necessarios: grupo, numero de vertices, densidade, numero de labels, numero de execucoes do SA, numero de iteracoes, taxa de decaimento, temperatura inicial e final" << endl;
            return 0;
        }
        numExecucoes = atoi(argv[6]);
        numIteracoes = atoi(argv[7]);
        taxaDecaimento = stof(argv[8]);
        tempInicial = stof(argv[9]);
        tempFinal = stof(argv[10]);
    }

    grupo = atoi(argv[2]);
    numVertices = atoi(argv[3]);
    densidade = atof(argv[4]);
    numLabels = atoi(argv[5]);
 
    if(densidade == (float)0.8) densidade = 0;
    else if(densidade == (float)0.5) densidade = 1;
    else if(densidade == (float)0.2) densidade = 2;

    if(grupo == 1) {
        path = "dataset/instances/g1/";
        if(metodo == 0)
            pathSaida = "saidasReativo/instances/g1/";
        else if(metodo == 1)
            pathSaida = "saidasGRASP/instances/g1/";
        else if(metodo == 2)
            pathSaida = "saidasMIP/instances/g1/";
        else if(metodo == 3)
            pathSaida = "saidasSA/instances/g1/";
        else if(metodo == 4)
            pathSaida = "saidasGRASPReativo/instances/g1/";
        for(int i=0; i<4; i++)
            if(n[i] == numVertices) {
                numVertices = i;
                break;
            }

        if(metodo == 0) {
            file = fopen("saidasReativo/saidaGrupo1.csv", "w+");
            fputs("Instância;Custo Médio Após BL;Custo Médio;Tempo Médio(ms);Tempo Médio Construtivo(ms);Tempo Médio BL(ms);Tempo Médio Melhor Solução(ms);Alpha=0.05;Alpha=0.10;Alpha=0.15;Alpha=0.20;Alpha=0.25;Alpha=0.30;Iteração Média da Melhor Solução;Semente;Menor Custo Médio;Tempo Médio do Menor Custo Médio(ms)\n", file);
        } else if(metodo == 1 || metodo == 4) {
            if(metodo == 1)
                file = fopen("saidasGRASP/saidaGrupo1.csv", "w+");
            else
                file = fopen("saidasGRASPReativo/saidaGrupo1.csv", "w+");
            fputs("Instância;Custo Médio Após BL;Custo Médio;Tempo Médio Total(ms);Tempo Médio Construtivo(ms);Tempo Médio BL(ms);Tempo Limite Médio(s);Tempo Médio(ms);Semente;Menor Custo Médio;Tempo Médio do Menor Custo Médio(ms);Media de Iteracoes;Número de Soluções;Número de Soluções Repetidas\n", file);
        } else if(metodo == 2) {
            file = fopen("saidasMIP/saidaGrupo1.csv", "w+");
            fputs("Instância;Custo Médio;Tempo Médio(ms);Tempo Médio Solução Inicial(ms);MIPGap\n", file);
        } else if(metodo == 3) {
            file = fopen("saidasSA/saidaGrupo1.csv", "w+");
            fputs("Instância;Custo Médio Após SA;Custo Médio;Tempo Médio da Solução Inicial(ms);Tempo Médio Total(ms);Tempo Médio Construtivo(ms);Tempo Médio BL(ms);Tempo Médio(ms);Tempo Limite do GRASP(ms);Semente;Menor Custo Médio;Tempo Médio do Menor Custo Médio(ms);Número de Soluções do GRASP;Número de Soluções Repetidas do GRASP;Número de Soluções do SA;Número de Soluções Repetidas do SA;Número de Soluções Parciais Repetidas\n", file);
        }
        fclose(file);

        int j;
        int countI = 0;
        for(int i=numVertices; i<4; i++) {
            if(countI == 0) j = densidade;
            else j= 0;
            for(; j<3; j++) {
                cout << "Grupo de Instancias: " << n[i] << "-" << auxD[j] << endl;
                ss.str("");
                ss.clear();
                ss << "./mediaGrupo.out " << metodo << " " << path << n[i] << "/" << d[j] << " "; 
                if(metodo == 0)
                    ss << numExecucoes << " " << numIteracoes << " " << tamanhoBloco << " " << SEED;
                else if(metodo == 1 || metodo == 4)
                    ss << numExecucoes << " " << tempoLimiteLiteratura[0] << " " << " " << SEED; 
                else if(metodo == 3)
                    ss << numExecucoes << " " << numIteracoes << " " << taxaDecaimento << " " << tempInicial << " " << tempFinal << " "  << tempoLimiteGraspSA[0] << " " << SEED; 

                int unusedIntReturn;
                unusedIntReturn = system(ss.str().c_str());

                ss.str("");
                ss.clear();
                ss << grupo << " " << n[i] << " " << auxD[j];

                if(metodo == 0)
                    file = fopen("saidasReativo/log.txt", "w+");
                else if(metodo == 1)
                    file = fopen("saidasGRASP/log.txt", "w+");
                else if(metodo == 2)
                    file = fopen("saidasMIP/log.txt", "w+");
                else if(metodo == 3)
                    file = fopen("saidasSA/log.txt", "w+");
                else if(metodo == 4)
                    file = fopen("saidasGRASPReativo/log.txt", "w+");
                fputs(ss.str().c_str(), file);
                fclose(file);

                ss.str("");
                ss.clear();
                ss << pathSaida << n[i] << "/" << d[j] << "/mediaGrupo.txt";
                file = fopen(ss.str().c_str(), "r");
                char* unusedReturn = fgets(auxLinha, 300, file);
                linha = auxLinha;
                fclose(file);

                ss.str("");
                ss.clear();
                ss << n[i] << "-" << auxD[j] << ";" << linha << "\n";

                if(metodo == 0)
                    file = fopen("saidasReativo/saidaGrupo1.csv", "a");
                else if(metodo == 1)
                    file = fopen("saidasGRASP/saidaGrupo1.csv", "a");
                else if(metodo == 2)
                    file = fopen("saidasMIP/saidaGrupo1.csv", "a");
                else if(metodo == 3)
                    file = fopen("saidasSA/saidaGrupo1.csv", "a");
                else if(metodo == 4)
                    file = fopen("saidasGRASPReativo/saidaGrupo1.csv", "a");
                fputs(ss.str().c_str(), file);
                fclose(file);
            }
            countI++;
        }
    } else if(grupo == 2) {
        path = "dataset/instances/g2/";
        if(metodo == 0)
            pathSaida = "saidasReativo/instances/g2/";
        else if(metodo == 1)
            pathSaida = "saidasGRASP/instances/g2/";
        else if(metodo == 2)
            pathSaida = "saidasMIP/instances/g2/";
        else if(metodo == 3)
            pathSaida = "saidasSA/instances/g2/";
        else if(metodo == 4)
            pathSaida = "saidasGRASPReativo/instances/g2/";
        for(int i=0; i<6; i++)
            if(n2[i] == numVertices) {
                numVertices = i;
                break;
            }
        for(int i=0; i<4; i++)
            if(label[numVertices][i] == numLabels) {
                numLabels = i;
                break;
            }

        if(metodo == 0) {
            file = fopen("saidasReativo/saidaGrupo2.csv", "w+");
            fputs("Instância;Custo Médio Após BL;Custo Médio;Tempo Médio(ms);Tempo Médio Construtivo(ms);Tempo Médio BL(ms);Tempo Médio Melhor Solução(ms);Alpha=0.05;Alpha=0.10;Alpha=0.15;Alpha=0.20;Alpha=0.25;Alpha=0.30;Iteração Média da Melhor Solução;Semente;Menor Custo Médio;Tempo Médio do Menor Custo Médio(ms)\n", file);
        } else if(metodo == 1 || metodo == 4) {
            if(metodo == 1)
                file = fopen("saidasGRASP/saidaGrupo2.csv", "w+");
            else
                file = fopen("saidasGRASPReativo/saidaGrupo2.csv", "w+");
            fputs("Instância;Custo Médio Após BL;Custo Médio;Tempo Médio Total(ms);Tempo Médio Construtivo(ms);Tempo Médio BL(ms);Tempo Limite Médio(s);Tempo Médio(ms);Semente;Menor Custo Médio;Tempo Médio do Menor Custo Médio(ms);Media de Iteracoes;Número de Soluções;Número de Soluções Repetidas\n", file);
        } else if(metodo == 2) {
            file = fopen("saidasMIP/saidaGrupo2.csv", "w+");
            fputs("Instância;Custo Médio;Tempo Médio(ms);Tempo Médio Solução Inicial(ms);MIPGap\n", file);
        } else if(metodo == 3) {
            file = fopen("saidasSA/saidaGrupo2.csv", "w+");
            fputs("Instância;Custo Médio Após SA;Custo Médio;Tempo Médio da Solução Inicial(ms);Tempo Médio Total(ms);Tempo Médio Construtivo(ms);Tempo Médio BL(ms);Tempo Médio(ms);Tempo Limite do GRASP(ms);Semente;Menor Custo Médio;Tempo Médio do Menor Custo Médio(ms);Número de Soluções do GRASP;Número de Soluções Repetidas do GRASP;Número de Soluções do SA;Número de Soluções Repetidas do SA;Número de Soluções Parciais Repetidas\n", file);
        }
        fclose(file);

        int k, j;
        int countI = 0;
        int countJ = 0;
        for(int i=numVertices; i<6; i++) {
            if(countI == 0) k = numLabels;
            else k = 0;
            for(; k<4; k++) {
                if(countJ == 0) j = densidade;
                else j = 0;
                for(; j<3; j++) {
                    cout << "Grupo de Instancias: " << n2[i] << "-" << auxD[j] << "-" << label[i][k] << endl;
                    ss.str("");
                    ss.clear();
                    ss << "./mediaGrupo.out " << metodo << " " << path << n2[i] << "/" << d[j] << "/" << label[i][k] << " "; 
                    if(metodo == 0)
                        ss << numExecucoes << " " << numIteracoes << " " << tamanhoBloco << " " << SEED;
                    else if(metodo == 1 || metodo == 4)
                        ss << numExecucoes << " " << tempoLimiteLiteratura[i] << " " << " " << SEED; 
                    else if(metodo == 3)
                        ss << numExecucoes << " " << numIteracoes << " " << taxaDecaimento << " " << tempInicial << " " << tempFinal << " " << tempoLimiteGraspSA[i] << " " << SEED; 

                    int unusedIntReturn;
                    unusedIntReturn = system(ss.str().c_str());

                    ss.str("");
                    ss.clear();
                    ss << grupo << " " << n2[i] << " " << auxD[j] << " " << label[i][k];
                    
                    if(metodo == 0)
                        file = fopen("saidasReativo/log.txt", "w+");
                    else if(metodo == 1)
                        file = fopen("saidasGRASP/log.txt", "w+");
                    else if(metodo == 2)
                        file = fopen("saidasMIP/log.txt", "w+");
                    else if(metodo == 3)
                        file = fopen("saidasSA/log.txt", "w+");
                    else if(metodo == 4)
                        file = fopen("saidasGRASPReativo/log.txt", "w+");
                    fputs(ss.str().c_str(), file);
                    fclose(file);
                 
                    ss.str("");
                    ss.clear();
                    
                    ss << pathSaida << n2[i] << "/" << d[j] << "/" << label[i][k] << "/mediaGrupo.txt";
                    file = fopen(ss.str().c_str(), "r");
                    char* unusedReturn = fgets(auxLinha, 300, file);
                    linha = auxLinha;
                    fclose(file);
                    
                    ss.str("");
                    ss.clear();
                    ss << n2[i] << "-" << auxD[j] << "-" << label[i][k] << ";" << linha << "\n";
                    
                    if(metodo == 0)
                        file = fopen("saidasReativo/saidaGrupo2.csv", "a");
                    else if(metodo == 1)
                        file = fopen("saidasGRASP/saidaGrupo2.csv", "a");
                    else if(metodo == 2)
                        file = fopen("saidasMIP/saidaGrupo2.csv", "a");
                    else if(metodo == 3)
                        file = fopen("saidasSA/saidaGrupo2.csv", "a");
                    else if(metodo == 4)
                        file = fopen("saidasGRASPReativo/saidaGrupo2.csv", "a");
                    fputs(ss.str().c_str(), file);
                    fclose(file);   
                }
                countJ++;
            }
            countI++;
        }
    }
}