#include <iostream>
#include <sstream>
#include <string.h>
#include <algorithm>
#include <climits>

using namespace std;

#define NUMDADOSREATIVO 9
#define NUMDADOSGRASP 11
#define NUMDADOSSA 14
#define NUMDADOSIG 20

// g++ cpp/mediaAleatorio.cpp -O3 -o mediaAleatorio.out
// ./mediaAleatorio.out 0 dataset/instances/g2/50/hd/50/0.txt 10 200 50 0
// ./mediaAleatorio.out 1 dataset/instances/g2/50/hd/50/0.txt 10 10 0
// ./mediaAleatorio.out 3 dataset/instances/g2/200/ld/100/4.txt 10 50 0.9 300 0.001 1 0
// ./mediaAleatorio.out 4 dataset/instances/g2/50/hd/50/0.txt 10 10 0
// ./mediaAleatorio.out 5 dataset/instances/g2/200/ld/100/4.txt 10 1000 1 0

void calculaMediaReativo(string entrada) {
    FILE* file;
    file = fopen(entrada.c_str(), "r");
    if(file == NULL) {
        cout << "Arquivo inexistente" << endl;
        return ;
    }

    char* split;
    char linha[300];
    int firstSeed;
    int i = 0;
    int menorCusto = INT_MAX;
    float menorTempo;
    float mediaCusto = 0;
    float melhorAlpha;
    float mediaConstrutivo = 0;
    float mediaTempo = 0;
    float mediaTempoConstrutivo = 0;
    float mediaTempoBuscaLocal = 0;
    float mediaTempoMelhorSolucao = 0;
    float mediaIteracao = 0;
    float dados[NUMDADOSREATIVO];

    char* unusedReturn;
    unusedReturn = fgets(linha, 300, file);
    while(!feof(file)) {
        unusedReturn = fgets(linha, 300, file);
        split = strtok(linha, ";");

        for(int j=0; j<NUMDADOSREATIVO; j++) {
            dados[j] = atof(split);
            split = strtok(NULL, ";");
        }

        if(dados[0] < menorCusto) {
            menorCusto = dados[0];
            menorTempo = dados[2];
            melhorAlpha = dados[6];
        }

        if(i == 0)
            firstSeed = dados[8];

        mediaCusto += dados[0];
        mediaConstrutivo += dados[1];
        mediaTempo += dados[2];
        mediaTempoConstrutivo += dados[3];
        mediaTempoBuscaLocal += dados[4];
        mediaTempoMelhorSolucao += dados[5];
        mediaIteracao += dados[7];

        i++;
    }

    mediaCusto /= i;
    mediaConstrutivo /= i;
    mediaTempo /= i;
    mediaTempoConstrutivo /= i;
    mediaTempoBuscaLocal /= i;
    mediaTempoMelhorSolucao /= i;
    mediaIteracao /= i;

    fclose(file);
    file = fopen(entrada.c_str(), "a");

    stringstream ss;
    ss.str("");
    ss.clear();
    ss << "\n" << mediaCusto << ";" << mediaConstrutivo << ";" << mediaTempo << ";" << mediaTempoConstrutivo << ";" << mediaTempoBuscaLocal << ";" << mediaTempoMelhorSolucao << ";" << melhorAlpha << ";" << mediaIteracao << ";" << firstSeed << ";" << menorCusto << ";" << menorTempo;
    fputs(ss.str().c_str(), file);

    fclose(file);
}

void calculaMediaGRASP(string entrada) {
    FILE* file;
    file = fopen(entrada.c_str(), "r");
    if(file == NULL) {
        cout << "Arquivo inexistente" << endl;
        return ;
    }

    char* split;
    char linha[300];
    int firstSeed;
    int i = 0;
    int menorCusto = INT_MAX;
    int tempoLimite;
    int numSolucoes = 0;
    int numSolucoesRepetidas = 0;
    float menorTempo = (float) INT_MAX;
    float mediaCusto = 0;
    float mediaConstrutivo = 0;
    float mediaTempoTotal = 0;
    float mediaTempoConstrutivo = 0;
    float mediaTempoBuscaLocal = 0;
    float mediaTempoMelhorSol = 0;
    float mediaNumIteracoes = 0;
    float dados[NUMDADOSGRASP];

    char* unusedReturn;
    unusedReturn = fgets(linha, 300, file);
    while(!feof(file)) {
        unusedReturn = fgets(linha, 300, file);
        split = strtok(linha, ";");

        for(int j=0; j<NUMDADOSGRASP; j++) {
            dados[j] = atof(split);
            split = strtok(NULL, ";");
        }

        if (dados[0] == menorCusto) {
            if(dados[6] <= menorTempo) {
                menorCusto = dados[0];
                menorTempo = dados[6];
                tempoLimite = dados[5];
            }
        }
        else if(dados[0] < menorCusto) {
            menorCusto = dados[0];
            menorTempo = dados[6];
            tempoLimite = dados[5];    
        }

        if(i == 0)
            firstSeed = dados[10];

        mediaCusto += dados[0];
        mediaConstrutivo += dados[1];
        mediaTempoTotal += dados[2];
        mediaTempoConstrutivo += dados[3];
        mediaTempoBuscaLocal += dados[4];
        mediaTempoMelhorSol += dados[6];
        mediaNumIteracoes += dados[7];
        numSolucoes += dados[8];
        numSolucoesRepetidas += dados[9];

        i++;
    }

    mediaCusto /= i;
    mediaConstrutivo /= i;
    mediaTempoTotal /= i;
    mediaTempoConstrutivo /= i;
    mediaTempoBuscaLocal /= i;
    mediaTempoMelhorSol /= i;
    mediaNumIteracoes /= i;

    fclose(file);
    file = fopen(entrada.c_str(), "a");

    stringstream ss;
    ss.str("");
    ss.clear();
    ss << "\n" << mediaCusto << ";" << mediaConstrutivo << ";" << mediaTempoTotal << ";" << mediaTempoConstrutivo << ";" << mediaTempoBuscaLocal << ";" << tempoLimite << ";" << mediaTempoMelhorSol << ";" << firstSeed << ";" << menorCusto << ";" << menorTempo << ";" << mediaNumIteracoes << ";" << numSolucoes << ";" << numSolucoesRepetidas;
    fputs(ss.str().c_str(), file);

    fclose(file);
}

void calculaMediaSA(string entrada) {
    FILE* file;
    file = fopen(entrada.c_str(), "r");
    
    if(file == NULL) {
        cout << "Arquivo inexistente" << endl;
        return ;
    }

    char* split;
    char linha[300];
    int firstSeed;
    int i = 0;
    int numMenorCusto = 0;
    int menorCusto = INT_MAX;
    int numSolucoesSA = 0;
    int numSolucoesGrasp = 0;
    int numSolucoesRepetidasSA = 0;
    int numSolucoesRepetidasGrasp = 0;
    int numRepeticoesParciais = 0;
    float tempoLimite;
    float mediaCusto = 0;
    float mediaConstrutivo = 0;
    float mediaTempo = 0;
    float mediaTempoMenorCusto = 0;
    float mediaTempoSolucaoInicial = 0;
    float mediaTempoBuscaLocal = 0;
    float mediaTempoConstrutivo = 0;
    float mediaTempoTotal = 0;
    float dados[NUMDADOSSA];

    char* unusedReturn;
    unusedReturn = fgets(linha, 300, file);
    while(!feof(file)) {
        unusedReturn = fgets(linha, 300, file);
        split = strtok(linha, ";");

        for(int j=0; j<NUMDADOSSA; j++) {
            dados[j] = atof(split);
            split = strtok(NULL, ";");
        }

        if(dados[0] <= menorCusto) {
            numMenorCusto++;
            menorCusto = dados[0];
            mediaTempoMenorCusto += dados[6];
        }

        if(i == 0) {
            firstSeed = dados[13];
            tempoLimite = dados[7];
        }

        mediaCusto += dados[0];
        mediaConstrutivo += dados[1];
        mediaTempoSolucaoInicial += dados[2];
        mediaTempoTotal += dados[3];
        mediaTempoConstrutivo += dados[4];
        mediaTempoBuscaLocal += dados[5];
        mediaTempo += dados[6];
        numSolucoesGrasp += dados[8];
        numSolucoesRepetidasGrasp += dados[9];
        numSolucoesSA += dados[10];
        numSolucoesRepetidasSA += dados[11];
        numRepeticoesParciais += dados[12];

        i++;
    }

    mediaCusto /= i;
    mediaConstrutivo /= i;
    mediaTempoSolucaoInicial /= i;
    mediaTempoTotal /= i;
    mediaTempoBuscaLocal /= i;
    mediaTempoConstrutivo /= i;
    mediaTempo /= i;
    mediaTempoMenorCusto /= numMenorCusto;

    fclose(file);
    file = fopen(entrada.c_str(), "a");

    stringstream ss;
    ss.str("");
    ss.clear();
    ss << "\n" << mediaCusto << ";" << mediaConstrutivo << ";" << mediaTempoSolucaoInicial << ";" << mediaTempoTotal << ";" << mediaTempoConstrutivo << ";" << mediaTempoBuscaLocal << ";" << mediaTempo << ";" << tempoLimite << ";" << firstSeed << ";" << menorCusto << ";" << mediaTempoMenorCusto << ";" << numSolucoesGrasp << ";" << numSolucoesRepetidasGrasp << ";" << numSolucoesSA << ";" << numSolucoesRepetidasSA << ";" << numRepeticoesParciais;
    fputs(ss.str().c_str(), file);

    fclose(file);
}

void calculaMediaIG(string entrada) {
    FILE* file;
    file = fopen(entrada.c_str(), "r");
    
    if(file == NULL) {
        cout << "Arquivo inexistente" << endl;
        return ;
    }

    char* split;
    char linha[300];
    int firstSeed;
    int i = 0;
    int countMelhoras = 0;
    int countUsoIG = 0;
    int countUsoIGDiff = 0;
    int numMenorCusto = 0;
    int menorCusto = INT_MAX;
    int numSolucoesIG = 0;
    int numSolucoesGrasp = 0;
    int numSolucoesRepetidasIG = 0;
    int numSolucoesRepetidasGrasp = 0;
    int numRepeticoesParciais = 0;
    float tempoLimite;
    float mediaCusto = 0;
    float mediaConstrutivo = 0;
    float mediaTempo = 0;
    float mediaTempoMenorCusto = 0;
    float mediaTempoSolucaoInicial = 0;
    float mediaTempoMip = 0;
    float mediaTempoConstrutivo = 0;
    float mediaTempoTotal = 0;
    float mediaItMelhora = 0;
    float minItMelhora = INT_MAX;
    float maxItMelhora = 0;
    float mediaNumSolIG = 0;
    float mediaNumCompConexas = 0;
    float minNumCompConexas = INT_MAX;
    float maxNumCompConexas = 0;
    float desvNumCompConexas = 0;
    float avgDiff = 0;
    float minDiff = (float) INT_MAX;
    float maxDiff = 0;
    float dados[NUMDADOSIG];

    char* unusedReturn;
    unusedReturn = fgets(linha, 300, file);
    while(!feof(file)) {
        unusedReturn = fgets(linha, 300, file);
        split = strtok(linha, ";");

        for(int j=0; j<NUMDADOSIG; j++) {
            dados[j] = atof(split);
            split = strtok(NULL, ";");
        }

        if(dados[0] <= menorCusto) {
            numMenorCusto++;
            menorCusto = dados[0];
            mediaTempoMenorCusto += dados[6];
        }

        if(i == 0) {
            firstSeed = dados[13];
            tempoLimite = dados[7];
        }

        mediaCusto += dados[0];
        mediaConstrutivo += dados[1];
        mediaTempoSolucaoInicial += dados[2];
        mediaTempoTotal += dados[3];
        mediaTempoConstrutivo += dados[4];
        mediaTempoMip += dados[5];
        mediaTempo += dados[6];
        if(dados[8] != 0) {
            mediaItMelhora += dados[8];
            countMelhoras++;
        }
        if(dados[9] != 0 && dados[9] < minItMelhora) {
            minItMelhora = dados[9];
        }
        if(dados[10] > maxItMelhora) {
            maxItMelhora = dados[9];
        }
        numSolucoesGrasp += dados[11];
        if(dados[13] != 0) {
            countUsoIG++;
            if(dados[13] > 1) {
                countUsoIGDiff++;
                if(dados[17] < minDiff)
                    minDiff = dados[17];
                if(dados[18] > maxDiff)
                    maxDiff = dados[18];
            }
        }
        numSolucoesRepetidasGrasp += dados[12];
        numSolucoesIG += dados[13];
        numSolucoesRepetidasIG += dados[14];
        numRepeticoesParciais += dados[15];
        avgDiff += dados[16];

        i++;
    }

    mediaCusto /= i;
    mediaConstrutivo /= i;
    mediaTempoSolucaoInicial /= i;
    mediaTempoTotal /= i;
    mediaTempoMip /= i;
    mediaTempoConstrutivo /= i;
    mediaTempo /= i;

    if(countUsoIG > 0) {
        mediaNumSolIG = numSolucoesIG/countUsoIG;
        if(countUsoIGDiff > 0)
            avgDiff /= countUsoIGDiff;
    } else {
        minDiff = 0;
    }

    if(countMelhoras > 0)
        mediaItMelhora /= countMelhoras;
    else {
        minItMelhora = 0;
    }
    mediaTempoMenorCusto /= numMenorCusto;

    fclose(file);
    file = fopen(entrada.c_str(), "a");

    stringstream ss;
    ss.str("");
    ss.clear();
    ss << "\n" << mediaCusto << ";" << mediaConstrutivo << ";" << mediaTempoSolucaoInicial << ";" << mediaTempoTotal << ";" << mediaTempoConstrutivo << ";" << mediaTempoMip << ";" << mediaTempo << ";" << tempoLimite << ";" << firstSeed << ";" << menorCusto << ";" << mediaTempoMenorCusto << ";" << mediaItMelhora << ";" << minItMelhora << ";" << maxItMelhora << ";" << mediaNumSolIG << ";" << numSolucoesGrasp << ";" << numSolucoesRepetidasGrasp << ";" << numSolucoesIG << ";" << numSolucoesRepetidasIG << ";" << numRepeticoesParciais << ";" << avgDiff << ";" << minDiff << ";" << maxDiff;
    fputs(ss.str().c_str(), file);

    fclose(file);
}

int main(int argc, char** argv) {
    FILE* file;
    char* entrada;
    int numExecucoes;
    int numIteracoes;
    int tamanhoBloco;
    int seed;
    int metodo;
    float tempoLimite;
    double taxaDecaimento;
    double tempInicial;
    double tempFinal;

    char* saida;
    stringstream ss;
    stringstream ss2;
    string str;
    string str2;
    
    if(argc < 2) {
        cout << "Parametro necessario:metodo(0 - Reativo, 1 - GRASP, 2 - SA)" << endl;
        return 0;
    }
    metodo = atoi(argv[1]);
    if(metodo == 0) {
        if(argc < 7) {
            cout << "Parametros necessarios: arquivo de entrada, numero de execuçoes do reativo, N, B, seed" << endl;
            return 0;
        }
        ss << "saidasReativo";
        
        numIteracoes = atoi(argv[4]);
        tamanhoBloco = atoi(argv[5]);
        seed = atoi(argv[6]);
    } else if(metodo == 1 || metodo == 4){
        if(argc < 6) {
            cout << "Parametros necessarios: arquivo de entrada, numero de execuçoes do GRASP, tempo limite(s), seed" << endl;
            return 0;
        }
        if(metodo == 1)
            ss << "saidasGRASP";
        else
            ss << "saidasGRASPReativo";
        tempoLimite = atoi(argv[4]);
        seed = atoi(argv[5]);
    } else if(metodo == 3){
        if(argc < 10) {
            cout << "Parametros necessarios: arquivo de entrada, numero de execucoes do SA, numero de iteracoes, taxa de decaimento, temperatura inicial e final, tempo limite do GRASP, seed" << endl;
            return 0;
        }
        ss << "saidasSA";
        numIteracoes = atoi(argv[4]);
        taxaDecaimento = stof(argv[5]);
        tempInicial = stof(argv[6]);
        tempFinal = stof(argv[7]);
        tempoLimite = stof(argv[8]);
        seed = atoi(argv[9]);
    } else if(metodo == 5){
        if(argc < 7) {
            cout << "Parametros necessarios: arquivo de entrada, numero de execucoes do IG, numero de iteracoes, tempo limite do GRASP, seed" << endl;
            return 0;
        }
        ss << "saidasIG";
        numIteracoes = atoi(argv[4]);
        tempoLimite = stof(argv[5]);
        seed = atoi(argv[6]);
    }
    entrada = argv[2];
    numExecucoes = atoi(argv[3]);
    saida = strtok(entrada, "/");
    ss2 << "dataset";
    
    while(saida != NULL) {
        if(saida == "")
            break;
        saida = strtok(NULL, "/");
        ss << "/";
        ss2 << "/";
        ss << saida;
        ss2 << saida;
    }

    str = ss.str();
    str2 = ss2.str();
    str.pop_back();
    str2.pop_back();

    if(seed == 0) {
        file = fopen(str.c_str(), "w+");
        fclose(file);
    }
    
    int unusedIntReturn;
    for(int i=seed; i<numExecucoes; i++) {
        ss.str("");
        ss.clear();
        ss << "./main.out " << metodo << " " << str2 << " " << str << " ";
        if(metodo == 0)
            ss << numIteracoes << " " << tamanhoBloco << " " << i;
        else if(metodo == 1 || metodo == 4)
            ss << tempoLimite << " " << i;
        else if(metodo == 3)
            ss << numIteracoes << " " << taxaDecaimento << " " << tempInicial << " " << tempFinal << " " << tempoLimite << " " << i;
        else if(metodo == 5)
            ss << numIteracoes << " " << tempoLimite << " " << i;
        unusedIntReturn = system(ss.str().c_str());    
    }
    
    if(metodo == 0)
        calculaMediaReativo(str);
    else if(metodo == 1 || metodo == 4)
        calculaMediaGRASP(str);
    else if(metodo == 3)
        calculaMediaSA(str);
    else if(metodo == 5)
        calculaMediaIG(str);
}