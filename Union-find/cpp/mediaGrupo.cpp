#include <iostream>
#include <algorithm>
#include <climits>
#include <sstream>
#include <string.h>
#include <vector>

using namespace std;

#define NUMDADOSREATIVO 11
#define NUMDADOSGRASP 13
#define NUMDADOSMIP 4
#define NUMDADOSSA 16
#define NUMALPHAS 6

// g++ cpp/mediaGrupo.cpp -O3 -o mediaGrupo.out
// ./mediaGrupo.out 0 dataset/instances/g2/50/hd/50 10 200 50 0 0
// ./mediaGrupo.out 1 dataset/instances/g2/50/hd/50 10 10 0 0
// ./mediaGrupo.out 2 dataset/instances/g2/50/hd/50
// ./mediaGrupo.out 3 dataset/instances/g2/200/ld/100 10 50 0.9 300 0.001 1 0 0
// ./mediaGrupo.out 4 dataset/instances/g2/50/hd/50 10 10 0 0

int main(int argc, char** argv) {
    FILE* file;
    stringstream ss;
    stringstream auxEntrada;
    stringstream auxInstancia;
    string path;
    string entrada;
    int numExecucoes;
    int numIteracoes;
    int tamanhoBloco;
    int instancia;
    int seed;
    int metodo;
    
    char* split;
    char linha[300];
    int numLinha;
    int countAlpha[6];
    int numSolucoesSA = 0;
    int numSolucoesGrasp = 0;
    int numSolucoesRepetidasSA = 0;
    int numSolucoesRepetidasGrasp = 0;
    int numRepeticoesParciais = 0;
    float tempoLimite;
    float mediaCusto = 0;
    float mediaMenorTempo = 0;
    float mediaMenorCusto = 0;
    float mediaConstrutivo = 0;
    float mediaTempo = 0;
    float mediaTempoSolucaoInicial = 0;
    float mediaTempoConstrutivo = 0;
    float mediaTempoBuscaLocal = 0;
    float mediaTempoMelhorSolucao = 0;
    float mediaTempoTotal = 0;
    float mediaIteracao = 0;
    double mediaMipGap = 0;
    double taxaDecaimento;
    double tempInicial;
    double tempFinal;
    vector<float> dados;

    if(argc < 2) {
        cout << "Parametro necessario:metodo(0 - Reativo, 1 - GRASP, 2 - MIP)" << endl;
        return 0;
    }
    metodo = stoi(argv[1]);
    if(metodo == 0) {
        if(argc < 7) {
            cout << "Parametros necessarios: arquivo de entrada, numero de execuçoes do reativo, N, B, seed" << endl;
            return 0;
        }
        numExecucoes = atoi(argv[3]);
        numIteracoes = atoi(argv[4]);
        tamanhoBloco = atoi(argv[5]);
        instancia = atoi(argv[6]);
        seed = atoi(argv[7]);
        auxInstancia << "saidasReativo";

        for(int i=0; i<NUMALPHAS; i++)
            countAlpha[i] = 0;
    } else if(metodo == 1 || metodo == 4){
        if(argc < 6) {
            cout << "Parametros necessarios: arquivo de entrada, numero de execuçoes do GRASP, tempo limite(s), seed" << endl;
            return 0;
        }
        numExecucoes = atoi(argv[3]);
        tempoLimite = atoi(argv[4]);
        instancia = atoi(argv[5]);
        seed = atoi(argv[6]);
        if(metodo == 1)
            auxInstancia << "saidasGRASP";   
        else
            auxInstancia << "saidasGRASPReativo"; 
    } else if(metodo == 2){
        if(argc < 3) {
            cout << "Parametros necessarios: arquivo de entrada" << endl;
            return 0;
        }
        instancia = atoi(argv[3]);
        auxInstancia << "saidasMIP"; 
    } else if(metodo == 3){
        if(argc < 10) {
            cout << "Parametros necessarios: arquivo de entrada, numero de execucoes do SA, numero de iteracoes, taxa de decaimento, temperatura inicial e final, tempo limite do GRASP, seed" << endl;
            return 0;
        }
        numExecucoes = atoi(argv[3]);
        numIteracoes = atoi(argv[4]);
        taxaDecaimento = stof(argv[5]);
        tempInicial = stof(argv[6]);
        tempFinal = stof(argv[7]);
        
        tempoLimite = stof(argv[8]);
        instancia = atoi(argv[9]);
        seed = atoi(argv[10]);
        auxInstancia << "saidasSA";
    }
    entrada = argv[2];

    split = strtok((char*)entrada.c_str(), "/");
    while(split != NULL) {
        if(split == "")
            break;
        split = strtok(NULL, "/");
        auxInstancia << "/";
        auxInstancia << split;
    }
    path = auxInstancia.str();
    path.pop_back();

    entrada = argv[2];
    /*
    for(int i=instancia; i<10; i++) {
        ss.str("");
        ss.clear();
        auxEntrada.str("");
        auxEntrada.clear();
        auxInstancia.str("");
        auxInstancia.clear();
        cout << "Instancia: " << i << endl;
        
        auxInstancia << path << "/" << i << ".txt";
        auxEntrada << entrada << "/" << i << ".txt";
        if(metodo == 2) {
            ss << "./main.out " << metodo << " " << auxEntrada.str() << " " << auxInstancia.str() << " " << numExecucoes << " ";
        } else {
            ss << "./mediaAleatorio.out " << metodo << " " << auxInstancia.str() << " " << numExecucoes << " ";
            if(metodo == 0)
                ss << numIteracoes << " " << tamanhoBloco << " " << seed;
            else if(metodo == 1 || metodo == 4)
                ss << tempoLimite << " " << seed;
            else if(metodo == 3)
                ss << numIteracoes << " " << taxaDecaimento << " " << tempInicial << " " << tempFinal << " " << tempoLimite << " " << seed;
        }
        
        int unusedIntReturn = system(ss.str().c_str());
    }
    */
    for(int i=0; i<10; i++) {
        dados.clear();
        numLinha = 0;

        auxInstancia.str("");
        auxInstancia.clear();
        auxInstancia << path << "/" << i << ".txt";

        file = fopen(auxInstancia.str().c_str(), "r");
        char* unusedReturn;
        while(!feof(file)) {
            unusedReturn = fgets(linha, 300, file);
            if(numLinha == numExecucoes+1 || (metodo == 2 && numLinha == 1)) {        
                split = strtok(linha, ";");  
                if(metodo == 0)              
                    for(int j=0; j<NUMDADOSREATIVO; j++) {
                        dados.push_back(atof(split));
                        split = strtok(NULL, ";");
                    }
                else if(metodo == 1 || metodo == 4)
                    for(int j=0; j<NUMDADOSGRASP; j++) {
                        dados.push_back(atof(split));
                        split = strtok(NULL, ";");
                    }
                else if(metodo == 2)
                    for(int j=0; j<NUMDADOSMIP; j++) {
                        dados.push_back(atof(split));
                        split = strtok(NULL, ";");
                    }
                else if(metodo == 3)
                    for(int j=0; j<NUMDADOSSA; j++) {
                        dados.push_back(atof(split));
                        split = strtok(NULL, ";");
                    }

                mediaCusto += dados[0];
                if(metodo == 0) {
                    mediaConstrutivo += dados[1];
                    mediaTempo += dados[2];
                    mediaTempoConstrutivo += dados[3];
                    mediaTempoBuscaLocal += dados[4];
                    mediaTempoMelhorSolucao += dados[5];
                    countAlpha[(int)(dados[6]/0.05)-1]++;
                    mediaIteracao += dados[7];
                    mediaMenorCusto += dados[9];
                    mediaMenorTempo += dados[10];
                } else if(metodo == 1 || metodo == 4) {
                    mediaConstrutivo += dados[1];
                    mediaTempo += dados[2];
                    mediaTempoConstrutivo += dados[3];
                    mediaTempoBuscaLocal += dados[4];
                    mediaTempoMelhorSolucao += dados[6];
                    mediaMenorCusto += dados[8];
                    mediaMenorTempo += dados[9];
                    mediaIteracao += dados[10];
                    numSolucoesGrasp += dados[11];
                    numSolucoesRepetidasGrasp += dados[12];
                } else if(metodo == 2) {
                    mediaTempo += dados[1];
                    mediaTempoSolucaoInicial += dados[2];
                    mediaMipGap += dados[3];
                } else if(metodo == 3) {
                    mediaConstrutivo += dados[1];
                    mediaTempoSolucaoInicial += dados[2];
                    mediaTempoTotal += dados[3];
                    mediaTempoConstrutivo += dados[4];
                    mediaTempoBuscaLocal += dados[5];
                    mediaTempo += dados[6];
                    mediaMenorCusto += dados[9];
                    mediaMenorTempo += dados[10];
                    numSolucoesGrasp += dados[11];
                    numSolucoesRepetidasGrasp += dados[12];
                    numSolucoesSA += dados[13];
                    numSolucoesRepetidasSA += dados[14];
                    numRepeticoesParciais += dados[15];
                }
                
                break;
            }
            numLinha++;
        }
        seed = 0;
    }
    mediaCusto /= 10;
    mediaConstrutivo /= 10;
    mediaTempo /= 10;
    mediaTempoSolucaoInicial /= 10;
    mediaMipGap /= 10;
    mediaTempoConstrutivo /= 10;
    mediaTempoBuscaLocal /= 10;
    mediaTempoMelhorSolucao /= 10;
    mediaIteracao /= 10;
    mediaMenorCusto /= 10;
    mediaMenorTempo /= 10;
    mediaTempoTotal /= 10;

    ss.str("");
    ss.clear();
    ss << path << "/mediaGrupo.txt"; 
    file = fopen(ss.str().c_str(), "w+");

    fprintf(file, "%.4f;", mediaCusto);
    if(metodo == 0) {
        fprintf(file, "%.1f;%.4f;%.4f;%.4f;", mediaConstrutivo, mediaTempo, mediaTempoConstrutivo, mediaTempoBuscaLocal);
        fprintf(file, "%.4f;", mediaTempoMelhorSolucao);
        for(int i=0; i<NUMALPHAS; i++)
            fprintf(file, "%i;", countAlpha[i]);
        fprintf(file, "%.4f;", mediaIteracao);
        fprintf(file, "%i;%.1f;%.4f", seed, mediaMenorCusto, mediaMenorTempo);
    } else if(metodo == 1 || metodo == 4) {
        fprintf(file, "%.1f;%.4f;%.4f;%.4f;", mediaConstrutivo, mediaTempo, mediaTempoConstrutivo, mediaTempoBuscaLocal);
        fprintf(file, "%.2f;%.4f;", tempoLimite, mediaTempoMelhorSolucao);
        fprintf(file, "%i;%.1f;%.4f;%.2f;%i;%i", seed, mediaMenorCusto, mediaMenorTempo, mediaIteracao, numSolucoesGrasp, numSolucoesRepetidasGrasp);
    } else if(metodo == 2) {
        fprintf(file, "%.4f;%.4f;%.4f", mediaTempo, mediaTempoSolucaoInicial, mediaMipGap);
    } else if(metodo == 3) {
        fprintf(file, "%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;", mediaConstrutivo, mediaTempoSolucaoInicial, mediaTempoTotal, mediaTempoConstrutivo, mediaTempoBuscaLocal, mediaTempo);
        fprintf(file, "%i;%i;%.1f;%.4f;%i;%i;%i;%i;%i", (int)(tempoLimite*1000), seed, mediaMenorCusto, mediaMenorTempo, numSolucoesGrasp, numSolucoesRepetidasGrasp, numSolucoesSA, numSolucoesRepetidasSA, numRepeticoesParciais);
    }
    
    fclose(file);
}