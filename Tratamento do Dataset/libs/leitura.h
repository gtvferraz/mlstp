#include <iostream>
#include <stdio.h>
#include <string.h>
#include "grafo.h"

using namespace std;

Grafo* carregaInstancias(const char* filePath) {
    int numVertices, numLabels;
    FILE* file;
    Grafo* grafo;
    
    file = fopen(filePath, "rt");
    if(file == NULL) {
        cout << "Arquivo inexistente" << endl;
        return nullptr;
    }

    char linha[10000];
    char* split;
    fgets(linha, 10000, file);
    split = strtok(linha, " ");
    numVertices = atoi(split);
    split = strtok(NULL, " ");
    numLabels = atoi(split);

    int i = 0;
    int j;
    int count = 0;
    int aux = 0;
    grafo = new Grafo(numVertices, numLabels);
    
    while(!feof(file)) {
        fgets(linha, 10000, file);
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

    
    fclose(file);
    return grafo;
}