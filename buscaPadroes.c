#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

// estrutura que aramazena o iniíco e fim (posições) do segmento do vetor a ser trabalhado
typedef struct{
    int ini;
    int* data_set;
    int fim;
} info;

info buffer[MAX_BUFFER_SIZE]; // buffer que será utilizado pelas threads
size_t buffer_lot = 0;				// quantos elementos estão no buffer atualmente
