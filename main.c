#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define NUM_SYMBOL 256

typedef struct {
  char simbolo;
  unsigned int ocorrencia;
  float prob;
  float cum_prob;
} info_simbolo;

typedef struct {
  unsigned int low;
  unsigned int high;
} range;

// Protótipo das funções
void calculaInformacoesSimbolos(FILE *inputfile, info_simbolo symbols[NUM_SYMBOL], int *tamanho_string, range *range_atual, unsigned int low, unsigned int high);
int compareSymbolInfo(const void *a, const void *b);
void compressao(FILE *inputfile, FILE *outputFile, unsigned int low, unsigned int high);

int main(int argc, char *argv[]) {
  // Verifica se foram passados argumentos suficientes
  if (argc < 4) {
    fprintf(stderr, "Use: %s arquivo.txt low high\n", argv[0]);
    return 1;
  }

  // Lê os argumentos da linha de comando
  char *inputFilename = argv[1];
  unsigned int low = atoi(argv[2]);
  unsigned int high = atoi(argv[3]);

  // Abre o arquivo de entrada
  FILE *inputFile = fopen(inputFilename, "r");
  if (inputFile == NULL) {
    printf("Erro ao abrir o arquivo");
    return 0;
  }

  // Abre o arquivo de saída
  char *compressedOutputname = "codificado.txt";
  FILE *compressedOutputFile = fopen(compressedOutputname, "w");

  // Realiza a compressão
  compressao(inputFile, compressedOutputFile, low, high);
  
  // Fecha os arquivos
  fclose(inputFile);
  fclose(compressedOutputFile);

  return 0;
}

// Função para calcular as informações dos símbolos
void calculaInformacoesSimbolos(FILE *inputfile, info_simbolo symbols[NUM_SYMBOL], int *tamanho_string, range *range_atual, unsigned int low, unsigned int high){
  // Inicializa as estruturas de dados
  memset(symbols, 0, sizeof(info_simbolo) * NUM_SYMBOL);
  *tamanho_string = 0;

  // Conta a ocorrência de cada símbolo no arquivo de entrada
  int j;
  while ((j = fgetc(inputfile)) != EOF) {
    symbols[j].ocorrencia++; // Incrementa a contagem de ocorrência do símbolo atual
    (*tamanho_string)++; // Incrementa o tamanho total da string
  }

  // Calcula as probabilidades de ocorrência de cada símbolo
  float cum_prob = 0.0;
  for (int i = 0; i < NUM_SYMBOL; i++) {
    if (symbols[i].ocorrencia > 0) { // Verifica se o símbolo ocorreu pelo menos uma vez
      symbols[i].simbolo = (char)i; // Define o símbolo atual
      symbols[i].prob = (float)symbols[i].ocorrencia / (*tamanho_string); // Calcula a probabilidade do símbolo
      cum_prob += symbols[i].prob; // Atualiza a probabilidade acumulativa
      symbols[i].cum_prob = cum_prob; // Define a probabilidade acumulativa para o símbolo atual
    }
  }

  // Ordena os símbolos por ocorrência
  qsort(symbols, NUM_SYMBOL, sizeof(info_simbolo), compareSymbolInfo);

  // Atualiza as probabilidades acumulativas
  cum_prob = 0.0;
  for (int i = 0; i < NUM_SYMBOL; i++) {
    if (symbols[i].ocorrencia > 0) { // Verifica se o símbolo ocorreu pelo menos uma vez
      cum_prob += symbols[i].prob; // Atualiza a probabilidade acumulativa
      symbols[i].cum_prob = cum_prob; // Define a probabilidade acumulativa para o símbolo atual
    }
  }

  // Inicializa o intervalo com os valores fornecidos
  range_atual->low = low;
  range_atual->high = high;
}

// Função de comparação para a ordenação dos símbolos
int compareSymbolInfo(const void *a, const void *b){
  // Converte os ponteiros genéricos para ponteiros do tipo info_simbolo
  info_simbolo *symbolA = (info_simbolo *)a;
  info_simbolo *symbolB = (info_simbolo *)b;

  // Compara as ocorrências dos símbolos
  if (symbolA->ocorrencia < symbolB->ocorrencia) {
    return 1; // Retorna 1 se a ocorrência de symbolA for menor que a ocorrência de symbolB
  } else if (symbolA->ocorrencia > symbolB->ocorrencia) {
    return -1; // Retorna -1 se a ocorrência de symbolA for maior que a ocorrência de symbolB
  } else {
    // Se as ocorrências forem iguais, compara os símbolos
    return (symbolA->simbolo < symbolB->simbolo) ? -1 : 1; // Retorna -1 se o símboloA for menor que o símboloB, caso contrário, retorna 1
  }
}

// Função de compressão
void compressao(FILE *inputfile, FILE *outputFile, unsigned int low, unsigned int high) {
  // Declarações das estruturas de dados
  info_simbolo symbols[NUM_SYMBOL]; // Array de estruturas info_simbolo
  int tamanho_string; // Variável para armazenar o tamanho da string de entrada
  range range_atual; // Estrutura para armazenar o intervalo atual

  // Calcula as informações dos símbolos presentes no arquivo de entrada
  calculaInformacoesSimbolos(inputfile, symbols, &tamanho_string, &range_atual, low, high);

  // Calcula o número de dígitos em high para formatar a saída
  int digitos_high = snprintf( NULL, 0, "%u", range_atual.high);
  // Define o divisor usado para o underflow
  int divisor = pow(10, digitos_high - 1);

  // Imprime informações iniciais na saída padrão
  printf("---------------------------------------");
  printf("\nLow: %0*u \nHigh: %0*u\n", digitos_high, range_atual.low, digitos_high, range_atual.high);
  printf("---------------------------------------\n");

  // Imprime as informações dos símbolos no arquivo de saída
  for (int i = 0; i < NUM_SYMBOL; i++) {
    if (symbols[i].ocorrencia > 0) {
      printf("%c\t%*d\t%.4lf\t%.4lf\n", symbols[i].simbolo, digitos_high,
             symbols[i].ocorrencia, symbols[i].prob, symbols[i].cum_prob);
    }
  }
  printf("---------------------------------------\n");
  printf("\n");

  // Define o ponteiro de leitura para o início do arquivo
  fseek(inputfile, 0, SEEK_SET);
  bool first_symbol = true; // Variável indicando se é o primeiro símbolo
  info_simbolo *previous_symbol = NULL; // Ponteiro para o símbolo anterior
  int last; // Último valor codificado
  int j; // Variável para leitura dos caracteres do arquivo de entrada

  // Loop de compressão
  while ((j = fgetc(inputfile)) != EOF) {
    char symbol = (char)j; // Lê um caractere do arquivo
    info_simbolo *Info_simbolo = NULL;

    // Verifica se o caractere está na tabela de símbolos
    for (int i = 0; i < NUM_SYMBOL; i++) {
      // Verifica se o símbolo atual na tabela de símbolos é o mesmo que o símbolo atual do arquivo
      if (symbols[i].simbolo == symbol) {
        // Se forem iguais, encontramos o símbolo correspondente
        Info_simbolo = &symbols[i]; // Guarda um ponteiro para a entrada da tabela que corresponde ao símbolo atual
        previous_symbol = &symbols[i - 1]; // Guarda um ponteiro para a entrada anterior da tabela
        break; // Sai do loop, pois encontramos o símbolo desejado
      }
    }

    // Verifica se o símbolo foi encontrado na tabela
    if (Info_simbolo == NULL) {
      fprintf(stderr, "Erro: Símbolo não encontrado na tabela.\n");
      exit(EXIT_FAILURE);
    }

    unsigned int NewLow;
    // Calcula o novo valor de low
    if (first_symbol == true) {
      NewLow = 0000; // Valor inicial para o primeiro símbolo
      first_symbol = false; // Define que não estamos mais no primeiro símbolo
    } else {
      if (previous_symbol == NULL) { // Verifica se há um símbolo anterior
        fprintf(stderr, "Erro: Não há símbolo anterior.\n"); // Mensagem de erro se não houver símbolo anterior
        exit(EXIT_FAILURE); // Encerra o programa com código de falha
      }
      // Calcula o novo valor de low baseado no intervalo atual e na probabilidade acumulada do símbolo anterior
      NewLow = range_atual.low + (unsigned int)((double)(range_atual.high - range_atual.low + 1) * previous_symbol->cum_prob);
    }

    // Calcula o novo valor de high
    unsigned int NewHigh = range_atual.low + (unsigned int)((double)(range_atual.high - range_atual.low + 1) * Info_simbolo->cum_prob) - 1;

    // Verifica se o intervalo precisa ser ajustado para evitar underflow
    if (NewLow > NewHigh) {
      NewLow = range_atual.low;
    }

    unsigned int oldLow = NewLow;

    // Reduz o intervalo se necessário para evitar valores maiores que high
    while (NewHigh > high || NewLow > high){
      if (NewHigh > high)
        NewHigh /= 10;
      if (NewLow > high)
        NewLow /= 10;
    }

    // Escreve o valor codificado no arquivo de saída
    if ((NewLow / divisor) == (NewHigh / divisor)) {
      // Se o intervalo não precisa ser dividido, imprime o símbolo e o próximo valor codificado
      printf("%c \tNewLow: %0*u \tNewhigh: %0*u\t saida: %d\n", Info_simbolo->simbolo, digitos_high, NewLow, digitos_high, NewHigh, (oldLow / divisor));
      // Escreve o próximo valor codificado no arquivo de saída
      fprintf(outputFile, "%d", (oldLow / divisor));
    } else {
      // Se o intervalo precisa ser dividido, imprime o símbolo sem escrever o próximo valor codificado
      printf("%c \tNewLow: %0*u \tNewhigh: %0*u\t saida: \n", Info_simbolo->simbolo, digitos_high, NewLow, digitos_high, NewHigh);
    }

    // Shift left
    while ((NewLow / divisor) == (NewHigh / divisor)) {
      // Shift left: multiplica o intervalo atual por 10 e ajusta os valores
      NewLow = (NewLow - (NewLow / divisor) * divisor) * 10;
      NewHigh = (NewHigh - (NewHigh / divisor) * divisor) * 10 + 9;

      // Verifica se o intervalo ainda precisa ser dividido
      if ((NewLow / divisor) == (NewHigh / divisor)) {
        // Imprime e escreve o próximo valor codificado no arquivo de saída
        printf("\tNewLow: %0*u \tNewhigh: %0*u\t saida: %d\n", digitos_high, NewLow, digitos_high, NewHigh, (NewLow / divisor));
        fprintf(outputFile, "%d", (NewLow / divisor));
      } else {
        // Imprime o intervalo atual (sem escrever o próximo valor codificado)
        printf("\tNewLow: %0*u \tNewhigh: %0*u\t saida: \n", digitos_high, NewLow, digitos_high, NewHigh);
      }
    }

    // Atualiza o intervalo
    range_atual.low = NewLow;
    range_atual.high = NewHigh;

    last = range_atual.low / 100; // Atualiza o último valor codificado
  }

  // Escreve o último valor codificado no arquivo de saída
  fprintf(outputFile, "%d", last);
}