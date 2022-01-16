#include <iostream>
#include <stdio.h>
#include <string.h>
#include "grafo_listaAdj.h"

using namespace std;

GrafoListaAdj* carregaInstancias(const char* filePath) {
    int numVertices, numLabels;
    char *unusedReturn;
    FILE* file;
    GrafoListaAdj* grafo;
    
    file = fopen(filePath, "rt");
    
    if(file == NULL) {
        cout << "Arquivo inexistente" << endl;
        return nullptr;
    }

    char linha[3000];
    char* split;
    unusedReturn = fgets(linha, 50, file);
    split = strtok(linha, " ");
    numVertices = atoi(split);
    split = strtok(NULL, " ");
    numLabels = atoi(split);

    int i = 0;
    int j;
    int count = 0;
    
    grafo = new GrafoListaAdj(numVertices, numLabels);
    while(!feof(file)) {
        unusedReturn = fgets(linha, 3000, file);
        split = strtok(linha, " ");
        for(j = i+1; j<numVertices; j++) {
            if(atoi(split) < numLabels) {
                grafo->addAresta(i, j, atoi(split));
                count++;
            }
            split = strtok(NULL, " ");
            atoi(split);
        }
        i++;
    }
    //cout << "Quantidade de Arestas Adicionadas: " << count << endl;
    fclose(file);
    return grafo;
}