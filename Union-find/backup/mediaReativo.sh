#!/bin/bash

# ./mediaReativo.sh dataset/instances/g2/100/hd/50/0.txt 10 200 50 0

if [ "$#" -ne 5 ]; then
  echo "Parametros necessarios: arquivo de entrada, numero de execuçoes do reativo, N, B, seed"
  exit 1
fi

entrada=$1
numExecucoes=$2
numIteracoes=$3
tamanhoBloco=$4
seed=$5

saida=${entrada##*/}

i=0
while [ $i -lt $numExecucoes ];
do
  ./main.out $entrada $saida $numIteracoes $tamanhoBloco $seed
  ((seed=$seed+1))
  ((i=$i+1))
  echo $i
done