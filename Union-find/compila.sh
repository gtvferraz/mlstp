#!/bin/bash

g++ -m64 -g -O3 -o main.out cpp/main.cpp -I ~/../../opt/gurobi911/linux64/include/ -L ~/../../opt/gurobi911/linux64/lib/ -l gurobi_c++ -l gurobi91 -lm
g++ cpp/mediaAleatorio.cpp -O3 -o mediaAleatorio.out
g++ cpp/mediaGrupo.cpp -O3 -o mediaGrupo.out
g++ cpp/executaTodosGrupos.cpp -O3 -o executaTodosGrupos.out