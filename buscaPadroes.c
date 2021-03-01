#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "timer.h"

long long int size;      // Quantidade de elementos no arquivo de entrada
int _log;								 // Informa se o log deve ser impresso ou não
int tam_bloco;           // Valor N do trabalho, tamanho do bloco que será trabalhado pelas threads
int tam_buf;             // Valor M do trabalho, tamanho do buffer compartilhado pelas threads
FILE *fp;                // Arquivo binário que será lido
int num_lidos_total = 0; // Guarda quantos números já foram lidos do arquivo

long long int pos_max_repet = -1; // posição inicial da primeira maior sequencia de algarismos idênticos
long long int max_repet = 0;      // quantidade de repetições da mairo sequencia de algarismos idênticos
int valor_max_repet = -1;         // valor da primeira maior sequencia de algarismos idênticos

long long int count_trinca = 0;   // quantidade de trincas simples encontradas

long long int count_seq = 0;      // quantidade de sequencias <0, 1, 2, 3, 4, 5> encontradas

sem_t em, espVazio, espCheio, barreira; // Semáforos

// Estrutura que aramazena o iníco e fim (posições) do segmento do vetor a ser processado
typedef struct {
  int ini;
  int* data_set;
  int fim;
} info;

info *buffer; // Buffer que será utilizado pelas threads
int buffer_in = 0, buffer_out = 0, buffer_count = 0;  // Variaveis auxiliares para a fila (buffer)

// Função que verifica se o malloc deu certo ou não
void *errorcheck_malloc(size_t size) {
  void *res = malloc(size);
  if(res == NULL) {
    printf("Erro no malloc!\n");
    exit(-1);
  }
  return res;
}

// Função para começar a leitura de um bloco do buffer
// Espera para que tenha algum espaço cheio no buffer e, 
// caso as outras duas threads não tenham passado pela função ainda,
// chama um sem_post para não alterar o contador de espCheio
// A última thread que passa, reinicia a contagem e não chama sem_post
void entraLeitura() {

  // Variável estática contando quantas threads já passaram pela função
  static int passou = 0;

  // Espera até ter um espaço cheio
  sem_wait(&espCheio);

  sem_wait(&em);
  
  passou++;

  // Se todas as threads já passaram pela função, reinicia a contagem
  // Senão, envia um post para que continue contando um espaço cheio
  if(passou == 3) passou = 0;
  else sem_post(&espCheio);
  
  sem_post(&em);
}

// Função para terminar leitura de um bloco de um buffer
// Funciona como uma barreira para que um bloco só seja 
// removido do buffer depois que todas as 3 threads 
// tenham finalizado o processamento dele
void terminaLeitura() {
  sem_wait(&em);

  // Variável statica para contar quantas threads foram bloqueadas
  static int bloqueadas = 0;

  bloqueadas++;
  if(_log) printf("Uma thread entrou na barreira\n");

  // Se todas as threads ainda não chegaram na barreira, bloqueia a thread
  if (bloqueadas < 3) {
    sem_post(&em);
    sem_wait(&barreira);
    bloqueadas--;

    // Se não há mais threads bloqueadas, libera a exclusão mútua
    // Senão, desbloqueia outra thread
    if(bloqueadas == 0) {
      if(_log) printf("Todas as threads saíram da barreira\n");
      sem_post(&em);
    }  
    else sem_post(&barreira);

  }
  else {

    if(_log) printf("Todas as threads entraram da barreira\n");

    // A última thread que chega na barreira remove o bloco do buffer
    if(_log) printf("Desenfilou: ini:%d->fim:%d\n", buffer[buffer_out].ini, buffer[buffer_out].fim);
    free(buffer[buffer_out].data_set);
    buffer_out = (buffer_out+1) % tam_buf;
    buffer_count--;

    // Sinaliza que tem um novo espaço vazio no buffer
    sem_post(&espVazio);

    // Atualiza o nº de threads bloqueadas e libera outras threads para continuar
    bloqueadas--;
    sem_post(&barreira);
  }
}

void* enfila_info() {
  // Enquanto não leu todos os números do arquivo
  while(num_lidos_total < size) {
    
    // Espera até que tenha um espaço vazio no buffer
    if(_log) printf("Thread produtora esperando por um espaço vazio!\n");
    sem_wait(&espVazio);
    if(_log) printf("Thread produtora desbloqueada para inserir!\n");

    // Aloca um vetor para o bloco
    int *bloco = errorcheck_malloc(sizeof(int)*tam_bloco);

    // Lê os números do arquivo para o bloco e 
    // guarda quantos números foram lidos nessa operação
    int num_lidos = fread(bloco,sizeof(int),tam_bloco,fp);

    buffer[buffer_in].data_set = bloco; 
    buffer[buffer_in].ini = num_lidos_total + 1;				
    buffer[buffer_in].fim = num_lidos_total + num_lidos;
    buffer_in = (buffer_in+1) % tam_buf;

    sem_wait(&em);
    buffer_count++;
    num_lidos_total += num_lidos;
    sem_post(&em);

    if(_log) printf("Enfilou: ini:%d->fim:%d\n", num_lidos_total - num_lidos + 1, num_lidos_total);
    
    // Sinaliza que tem um novo espaço cheio no buffer
    sem_post(&espCheio);
  } 
  fclose(fp); // Fecha o arquivo

  pthread_exit(NULL); // Termina a thread
}

// Cada uma dessas funções tem um loop verificando se o arquivo ja terminou de ser verificado por essa thread e começa as operações tentando desenfilar alguma coisa do buffer
void* checaSeq () {
  int seq = 0;   //algarismo atualmente procurado para sequencia
  sem_wait(&em);
  while(num_lidos_total < size || buffer_count != 0) {
    sem_post(&em);

    entraLeitura();

    for (int i = 0; i < buffer[buffer_out].fim - buffer[buffer_out].ini + 1; i++){
      if (seq == buffer[buffer_out].data_set[i]) {
        if (seq == 5){seq = 0; count_seq++;} //se a sequencia chegar em 5 reseta e incrementa o contador de sequencias
        else seq++;
      }
      else if(buffer[buffer_out].data_set[i] == 0) seq = 1; // caso quebrada a sequencia reseta, mas se o valor atual for um procura o proximo
      else seq = 0;
    }
    
    if(_log) printf("Thread de Sequência executou no bloco da posição %d até a posição %d\n", buffer[buffer_out].ini, buffer[buffer_out].fim);

    terminaLeitura();

    sem_wait(&em);
  }
  sem_post(&em);

  pthread_exit(NULL); // Termina a thread
}

void* checaTrinca () {
  int ant = -1;   // guarda o último caracter lido por essa thread
  int repet = 0;  // guarda o numero de repetições de cada iteração
  sem_wait(&em);
  while(num_lidos_total < size || buffer_count != 0) {
    sem_post(&em);

    entraLeitura();
    
    for (int i = 0; i < buffer[buffer_out].fim - buffer[buffer_out].ini + 1; i++){
      if (ant == buffer[buffer_out].data_set[i]){
        repet++;
        if (repet == 3) { repet = 1; count_trinca++; } // caso repita três vezes incrementa trinca e reseta o contador local
      }
      else repet = 1; // reseta o contador na quebra da sequencia
      ant = buffer[buffer_out].data_set[i];
    }
    
    if(_log) printf("Thread de Trinca executou no bloco da posição %d até a posição %d\n", buffer[buffer_out].ini, buffer[buffer_out].fim);

    terminaLeitura();

    sem_wait(&em);
  }
  sem_post(&em);

  pthread_exit(NULL); // Termina a thread
}

void* checaRepet () {
  int ant = -1;                 // guarda o último caracter lido por essa thread
  int repet = 1;                // contador de repetição local
  int valor_repet = -1;         // valor da repetição local
  long long int pos_repet = -1; // posição da repetição local
  sem_wait(&em);
  while(num_lidos_total < size || buffer_count != 0) {
    sem_post(&em);

    entraLeitura();
    
    for (int i = 0; i < buffer[buffer_out].fim - buffer[buffer_out].ini + 1; i++){
      if (ant == buffer[buffer_out].data_set[i]){
        repet++;
        // na identificação da primeira repetição atualiza as variáveis de localização e valor
        if(repet == 2) { 
          valor_repet = buffer[buffer_out].data_set[i];
          pos_repet = buffer[buffer_out].ini + i - 1;
        }
      }
      else { repet = 1; valor_repet = -1; pos_repet = -1; } // no caso de quebra de repetição reseta os valores
      ant = buffer[buffer_out].data_set[i];
      
      // se o valor de repetição for o maior até então atualiza os indicadores globais
      if (repet > max_repet) {
        max_repet = repet;
        valor_max_repet = valor_repet;
        pos_max_repet = pos_repet;
      } 
    }
    
    if(_log) printf("Thread de Repetição executou no bloco da posição %d até a posição %d\n", buffer[buffer_out].ini, buffer[buffer_out].fim);

    terminaLeitura();

    sem_wait(&em);
  }
  sem_post(&em);

  pthread_exit(NULL); // Termina a thread
}

int main(int argc, char *argv[]){

  if(argc < 4) {
    printf("Uso do programa: ./%s <(0 ou 1) Define se imprime os logs de execução> <Tamanho de cada bloco> <Tamanho do buffer> <Nome do arquivo binário>\n", argv[0]);
    exit(1);
  }

  tam_bloco = atoi(argv[2]);
  tam_buf = atoi(argv[3]);
  _log = atoi(argv[1]);
  char* arq = argv[4];
  double ini, fim;
  pthread_t tid[4];

  // Inicializa os semáforos
  sem_init(&em, 0, 1); sem_init(&espVazio, 0, tam_buf); sem_init(&espCheio, 0, 0); sem_init(&barreira, 0, 0);

  // Inicializa o buffer
  buffer = (info *) errorcheck_malloc(sizeof(info) * tam_buf); 
  
  // Abre o arquivo
  fp = fopen( arq , "rb" );
  fread(&size,sizeof(size),1,fp);
 
  GET_TIME(ini);
  
  if(pthread_create(&tid[0], NULL, enfila_info, NULL)) exit(-1);
  if(pthread_create(&tid[1], NULL, checaSeq, NULL)) exit(-1);
  if(pthread_create(&tid[2], NULL, checaTrinca, NULL)) exit(-1);
  if(pthread_create(&tid[3], NULL, checaRepet, NULL)) exit(-1);
  
  for (int i=0; i<4; i++) {
    pthread_join(*(tid+i), NULL);
  }

  GET_TIME(fim);

  printf("Maior sequência de valores idênticos: %lld %lld %d\n", pos_max_repet, max_repet, valor_max_repet );
  printf("Quantidade de triplas: %lld\n", count_trinca);
  printf("Quantidade de ocorrências da sequência <012345>: %lld\n", count_seq);
  printf("Tempo de execução: %lfs\n", fim - ini);
  
  free(buffer);

}
