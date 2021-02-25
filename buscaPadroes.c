#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

long long int size;     // qauntidade de elementos no arquivo de entrada
int _log;								//informa se o log deve ser impresso ou não
int tam_bloco;          // Valor N do trabalho, tamanho do bloco que será trabalhado pelas threads
int tam_buf;            // Valor M do trabalho, tamanho do buffer compartilhado pelas threads

// estrutura que aramazena o iniíco e fim (posições) do segmento do vetor a ser trabalhado
typedef struct{
  int ini;
  int* data_set;
  int fim;
} info;

info buffer*;           // buffer que será utilizado pelas threads
size_t buffer_lot = 0;  // quantos elementos estão no buffer atualmente

// TODO copiei do meu outro trabalho tem q rever ainda, tava pensando se a gente deveria usar semaforo
void* enfila_info(int* vet, int ini){

  // TODO while não chegou ao fim do arquivo

	if(_log)printf("enfilou: ini:%d->fim:%d\n", id, ini, fim);
	pthread_mutex_lock(&mutex);
	
	//não permite enfilar se a fila tiver cheia
	while(buffer_lot+1 >= tam_bloco) {
		if(_log)printf("Thread produtora bloqueada ao inserir!\n", id);
		pthread_cond_wait(&cond_livre, &mutex);
		if(_log)printf("Thread produtora desbloqueada para inserir!\n", id);
	}
	
	buffer[buffer_lot].data_set = vet; 
	buffer[buffer_lot].ini = ini;				
	buffer[buffer_lot].fim = ini + tam_bloco;
	buffer_lot++;
	pthread_cond_signal(&cond_populado); // como um elemento foi inserido sinaliza que o vetor está populado
	pthread_mutex_unlock(&mutex);
}

void* checaSeq (){};
void* checaTrinca (){};
void* checaRepet (){};

int main(int argc, char *argv[]){
  tam_bloco = atoi(argv[2]);
  tam_buf = atoi(argv[3]);
  _log = atoi(argv[1]);
  FILE *fp;
  double ini, fim;
  pthread_t tid[4];
  
  fp = fopen( "teste.bin" , "rb" );
  fread(&size,sizeof(size),1,fp);
  
  int arquivo[size];
  fread(&arquivo,sizeof(arquivo),1,fp);
  //for(long long int i = 0; i<size; i++) printf("%u ", arquivo[i]); printf("\n"); // checando validade da leitura
  
  GET_TIME(ini);
  
  // eu acho que cada thread poderia retornar um struct com os resultados finais? me pareceu uma boa ideia ao
  // invés de criar um monte de varíavel global mas não sei fazer isso :p
  if(pthread_create(&tid[0], NULL, enfila_info, (void *)i+1)) exit(-1);
  if(pthread_create(&tid[1], NULL, checaSeq, (void *)i+1)) exit(-1);
  if(pthread_create(&tid[2], NULL, checaTrinca, (void *)i+1)) exit(-1);
  if(pthread_create(&tid[3], NULL, checaRepet, (void *)i+1)) exit(-1);
  
  for (int i=0; i<4; i++) {
    pthread_join(*(tid+i), NULL);
  }
  GET_TIME(fim);
  
}
