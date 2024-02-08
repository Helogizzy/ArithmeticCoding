# ArithmeticCoding
> Trabalho de Processamento de Imagens Digitais

## Sobre
O algoritmo de codificação aritmética consiste em representar a probabilidade de ocorrência de cada carácter de acordo com esses intervalos. Parte-se do intervalo [ 0 , 1 ) e nele identifica-se o sub-intervalo ao qual corresponde o primeiro símbolo lido do arquivo. Para cada símbolo subsequente, subdivide-se o intervalo atual em sub-intervalos proporcionais às probabilidades da tabela de intervalos, e encontra-se novamente o intervalo que corresponde ao próximo símbolo. Ao final do processo, teremos um intervalo que corresponde a probabilidade da ocorrência de todos os símbolos na ordem correta.

## Como usar
- Compilar:
```gcc main.c -o main -lm```

- Executar:
```./main entrada.txt 0000 9999```

O arquivo entrada.txt deve estar na mesma pasta do arquivo ```main```

## Informações
- Linguagem: C
- Versão GCC: ```8.1.0```
- SOs testados: ```Windows 10 - Visual Studio Code 1.85.2``` e ```WSL - Ubuntu 20.04.4 LTS```
