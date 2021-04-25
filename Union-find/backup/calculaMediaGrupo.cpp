#include <iostream>
#include <algorithm>
#include <climits>
#include <string.h>
#include <sstream>

using namespace std;

#define NUMDADOS 8

int main(int argc, char **argv) {
    if(argc < 2) {
        cout << "Parametro necessario: arquivo de entrada" << endl;
        return 0;
    }

    FILE* file;
    file = fopen(argv[1], "r");
    if(file == NULL) {
        cout << "Arquivo inexistente" << endl;
        return 0;
    }

    char* split;
    char linha[300];
    int firstSeed;
    int i = 0;
    int menorCusto = INT_MAX;
    float menorTempo;
    float mediaCusto=0;
    float melhorAlpha;
    float mediaConstrutivo=0;
    float mediaTempo=0;
    float mediaTempoConstrutivo=0;
    float mediaTempoBuscaLocal=0;
    float mediaIteracao=0;
    float dados[NUMDADOS];
    fgets(linha, 300, file);
    while(!feof(file)) {
        fgets(linha, 300, file);
        split = strtok(linha, ";");

        for(int i=0; i<NUMDADOS; i++) {
            dados[i] = atof(split);
            split = strtok(NULL, ";");
        }

        if(dados[0] < menorCusto) {
            menorCusto = dados[0];
            menorTempo = dados[2];
            melhorAlpha = dados[5];
        }

        if(i == 0)
            firstSeed = dados[7];

        mediaCusto += dados[0];
        mediaConstrutivo += dados[1];
        mediaTempo += dados[2];
        mediaTempoConstrutivo += dados[3];
        mediaTempoBuscaLocal += dados[4];
        mediaIteracao += dados[6];

        i++;
    }

    mediaCusto /= i;
    mediaConstrutivo /= i;
    mediaTempo /= i;
    mediaTempoConstrutivo /= i;
    mediaTempoBuscaLocal /= i;
    mediaIteracao /= i;

    fclose(file);
    file = fopen(argv[1], "a");

    stringstream ss;
    ss.str("");
    ss.clear();
    ss << "\n" << mediaCusto << ";" << mediaConstrutivo << ";" << mediaTempo << ";" << mediaTempoConstrutivo << ";" << mediaTempoBuscaLocal << ";" << melhorAlpha << ";" << mediaIteracao << ";" << firstSeed << ";" << menorCusto << ";" << menorTempo;
    fputs(ss.str().c_str(), file);

    fclose(file);
}