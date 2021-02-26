#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "timer.h"

long long int size;     // Quantidade de elementos no arquivo de entrada
int _log;								// Informa se o log deve ser impresso ou não
int tam_bloco;          // Valor N do trabalho, tamanho do bloco que será trabalhado pelas threads
int tam_buf;            // Valor M do trabalho, tamanho do buffer compartilhado pelas threads
FILE *fp;               // Arquivo binário que será lido
sem_t em, espVazio, espCheio; // Semáforos

// Estrutura que aramazena o iníco e fim (posições) do segmento do vetor a ser trabalhado
typedef struct{
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
    printf("Erro no malloc!");
    exit(-1);
  }
  return res;
}

// TODO copiei do meu outro trabalho tem q rever ainda, tava pensando se a gente deveria usar semaforo
void* enfila_info(){

  int num_lidos_total = 0; // Guarda quantos números já foram lidos do arquivo

  // Enquanto não leu todos os números do arquivo
  while(num_lidos_total < size) {

    // Aloca um vetor para o bloco
    int *bloco = errorcheck_malloc(sizeof(int)*tam_bloco);

    // Lê os números do arquivo para o bloco e 
    // guarda quantos números foram lidos nessa operação
    int num_lidos = fread(&bloco,sizeof(int),tam_bloco,fp);
    
    // Espera até que tenha um espaço vazio no buffer
    if(_log) printf("Thread produtora esperando por um espaço vazio!\n");
    sem_wait(&espVazio);
    if(_log) printf("Thread produtora desbloqueada para inserir!\n");

    /*
    pthread_mutex_lock(&mutex);
    // Não permite enfilar se a fila tiver cheia
    while(buffer_count == tam_buf) {
      if(_log)printf("Thread produtora bloqueada ao inserir!\n");
      pthread_cond_wait(&cond_livre, &mutex);
      if(_log)printf("Thread produtora desbloqueada para inserir!\n");
    }
    pthread_mutex_unlock(&mutex);
    */

    buffer[buffer_in].data_set = bloco; 
    buffer[buffer_in].ini = num_lidos_total + 1;				
    buffer[buffer_in].fim = num_lidos_total + num_lidos;
    buffer_in = (buffer_in+1) % tam_buf;
    buffer_count++;
    num_lidos_total += num_lidos;

    if(_log) printf("enfilou: ini:%d->fim:%d\n", num_lidos_total + 1, num_lidos_total + num_lidos);
    
    // Sinaliza que tem um novo espaço cheio no buffer
    sem_post(&espCheio);
    
    //pthread_cond_signal(&cond_populado); // como um elemento foi inserido sinaliza que o vetor está populado
  } 
  fclose(fp); // Fecha o arquivo
}

// Cada uma dessas funções tem um loop verificando se o arquivo ja terminou de ser verificado por essa thread e começa as operações tentando desenfilar alguma coisa do buffer
void* checaSeq (){};
void* checaTrinca (){};
void* checaRepet (){};

int main(int argc, char *argv[]){

  if(argc < 4) {
    printf("Uso do programa: ./%s <(0 ou 1) Define se imprime os logs de execução> <Tamanho de cada bloco> <Tamanho do buffer>\n", argv[0]);
    exit(1);
  }

  tam_bloco = atoi(argv[2]);
  tam_buf = atoi(argv[3]);
  _log = atoi(argv[1]);
  double ini, fim;
  pthread_t tid[4];

  sem_init(&em, 0, 1); sem_init(&espCheio, 0, 0); sem_init(&espVazio, 0, tam_buf);

  buffer = (info *) errorcheck_malloc(sizeof(info) * tam_buf); // Inicializa o buffer
  
  fp = fopen( "teste.bin" , "rb" );
  fread(&size,sizeof(size),1,fp);
  
  // int arquivo[size];
  // fread(&arquivo,sizeof(arquivo),1,fp);
  // for(long long int i = 0; i<size; i++) printf("%u ", arquivo[i]); printf("\n"); // checando validade da leitura
  
  GET_TIME(ini);
  
  // eu acho que cada thread poderia retornar um struct com os resultados finais? me pareceu uma boa ideia ao
  // invés de criar um monte de varíavel global mas não sei fazer isso :p
  if(pthread_create(&tid[0], NULL, enfila_info, NULL)) exit(-1);
  if(pthread_create(&tid[1], NULL, checaSeq, NULL)) exit(-1);
  if(pthread_create(&tid[2], NULL, checaTrinca, NULL)) exit(-1);
  if(pthread_create(&tid[3], NULL, checaRepet, NULL)) exit(-1);
  
  for (int i=0; i<4; i++) {
    pthread_join(*(tid+i), NULL);
  }

  GET_TIME(fim);
  
}
