/*
    Autores: Kaiky Ferreira, Rodrigo Belarmino, Rodrigo Cesar
    //////////////////// CSV Reader /////////////////////////

*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct Dados {
  long int data;
  int codCidade;
  int codEstado;
  int novosCasos;
  struct Dados *proximo;
} Dados;

char *timeConverter(long int tempo);
int verificaExistencia(Dados *inicio, int codCidade, int codEstado);
int addFinal(Dados item);
void lerArquivo(char nomeArquivo[], Dados *inicio, int opcao);
int consolidarDadosCSV(Dados *inicio);
void historicoDadosCSV(Dados dado);
int totalEstado(Dados *inicio, int codEstado);
void imprimirCasosPorEstado(Dados *inicio, int codEstado);
int verificaExistenciaEstado(Dados *inicio, int codEstado);
void verificarConsolidadosExistente(Dados *inicio);

Dados *lista = NULL;

char *timeConverter(long int tempo) {
  time_t epoch_time = tempo;
  struct tm *tm_info = localtime(&epoch_time);
  char *data = calloc(20, sizeof(char));
  strftime(data, 20, "%d/%m/%Y", tm_info);
  return data;
}

int verificaExistencia(Dados *inicio, int codCidade, int codEstado) {
  Dados *noAtual = inicio;

  while (noAtual != NULL) {
    if (noAtual->codCidade == codCidade && noAtual->codEstado == codEstado) {
      return 1;
    }
    noAtual = noAtual->proximo;
  }

  return 0;
}

int addFinal(Dados item) {
  Dados *novoNo = malloc(sizeof(Dados));

  if (novoNo != NULL) {
    *novoNo = item;
    novoNo->proximo = NULL;

    if (lista == NULL) {
      lista = novoNo;
    } else {
      Dados *noAtual = NULL;

      noAtual = lista;
      while (noAtual->proximo != NULL) {
        noAtual = noAtual->proximo;
      }

      noAtual->proximo = novoNo;
    }

    return 1;
  } else {
    return 0;
  }
}

void lerArquivo(char nomeArquivo[], Dados *inicio, int opcao) {
  FILE *leitura = fopen(nomeArquivo, "r");
  Dados dado;
  Dados *noAtual = inicio;

  if (leitura != NULL) {
    while (!feof(leitura)) {
      if (opcao == 1) {
        fscanf(leitura, "%ld;%d;%d;%d\n", &dado.data, &dado.codCidade,
               &dado.codEstado, &dado.novosCasos);
      } else {
        fscanf(leitura, "%d;%d;%d;%ld\n", &dado.codCidade, &dado.codEstado,
               &dado.novosCasos, &dado.data);
      }
      historicoDadosCSV(dado);
      if (verificaExistencia(inicio, dado.codCidade, dado.codEstado) == 1) {
        noAtual = inicio;
        while (noAtual != NULL) {
          if (noAtual->codEstado == dado.codEstado &&
              noAtual->codCidade == dado.codCidade) {
            noAtual->novosCasos += dado.novosCasos;
            if (dado.data > noAtual->data) {
              noAtual->data = dado.data;
            }
          }
          noAtual = noAtual->proximo;
        }
      } else {
        addFinal(dado);
      }
    }
    if (opcao == 1)
      printf("Arquivo lido com sucesso.\n");
    sleep(2);
    fclose(leitura);
  } else {
    if (opcao == 1)
      printf("Arquivo não encontrado ou não existe.\n");
    sleep(2);
  }

}

int consolidarDadosCSV(Dados *inicio) {
  FILE *consolidar = fopen("dados_consolidados.csv", "w+");
  Dados *novoNo = inicio;

  if (consolidar != NULL) {
    while (novoNo != NULL) {
      fprintf(consolidar, "%d;%d;%d;%li\n", novoNo->codCidade,
              novoNo->codEstado, novoNo->novosCasos, novoNo->data);
      novoNo = novoNo->proximo;
    }
    return 1;
  } else {
    return 0;
  }

  fclose(consolidar);
}

void historicoDadosCSV(Dados dado) {
  FILE *historico = fopen("historico_cargas.csv", "a");
  long int tempo = time(NULL);

  if (historico != NULL) {
    fprintf(historico, "%li;%d;%d;%d;%li\n", tempo, dado.codCidade,
            dado.codEstado, dado.novosCasos, dado.data);
  }
  fclose(historico);
}

int totalEstado(Dados *inicio, int codEstado) {
  int total = 0;
  Dados *noAtual = inicio;

  while (noAtual != NULL) {
    if (noAtual->codEstado == codEstado) {
      total += noAtual->novosCasos;
    }
    noAtual = noAtual->proximo;
  }

  return total;
}

int verificaExistenciaEstado(Dados *inicio, int codEstado) {
  Dados *noAtual = inicio;
  int count = 0;

  while (noAtual != NULL) {
    if (noAtual->codEstado == codEstado)
      count += 1;
    noAtual = noAtual->proximo;
  }

  if (count < 1)
    return 0;

  return 1;
}

void imprimirCasosPorEstado(Dados *inicio, int codEstado) {
  Dados *noAtual = inicio;

  printf("Total de casos no estado: %d casos\n",
         totalEstado(inicio, codEstado));
  printf("Dados por cidade:\n");
  printf("CIDADE \t QTD CASOS \t ULTIMA ATUALIZACAO\n");
  printf("----------------------------------------------------\n");
  while (noAtual != NULL) {
    if (noAtual->codEstado == codEstado) {
      char *data = timeConverter(noAtual->data);
      printf("%5d %12d  %20s\n", noAtual->codCidade, noAtual->novosCasos, data);
      free(data);
    }
    noAtual = noAtual->proximo;
  }
}

void verificarConsolidadosExistente(Dados *inicio) {
  if (inicio == NULL) {
    lerArquivo("dados_consolidados.csv", inicio, 0);
  }
}

int menu() {
  int opcao = -1;

  printf("\n----- Sistema de Consolidação de Dados -----\n");
  printf("1 - Para fazer uma carga de dados.\n");
  printf("2 - Para imprimir relatório estadual.\n");
  printf("0 - Sair.\n");
  printf("Digite sua opção: ");
  scanf("%d", &opcao);

  return opcao;
}

int main() {
  int opcao = -1;
  int codEstado = 0;
  char arquivo[30];
  verificarConsolidadosExistente(lista);

  while (opcao != 0) {
    opcao = menu();
    switch (opcao) {
    case 1:
      printf("Digite o caminho relativo do arquivo: ");
      scanf(" %s", arquivo);
      lerArquivo(arquivo, lista, 1);
      consolidarDadosCSV(lista);
      break;
    case 2:
      printf("Digite o código do estado: ");
      scanf("%d", &codEstado);
      if (verificaExistenciaEstado(lista, codEstado) == 0) {
        printf("\nX - Estado ainda não foi inserido para a lista de dados "
               "consolidados\n");
      } else {
        imprimirCasosPorEstado(lista, codEstado);
      }

      break;
    case 0:
      printf("Saindo...\n");
      sleep(2);
      break;
    default:
      printf("\nX - Digite uma opção válida!\n");
      break;
    }
  }
  return EXIT_SUCCESS;
}