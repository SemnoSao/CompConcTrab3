#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]){
  long long int size = atoll(argv[1]);
  FILE *fp;
  srand(time(0));
  
  fp = fopen( "teste.bin" , "wb" );
  
  fwrite(&size, sizeof(size), 1, fp);
  
  for (long long int i = 0; i < size; i++){
    int r = (rand() % (5 + 1));
    //printf("%d ", r);
    fwrite(&r, sizeof(r), 1, fp);
  }
  //printf("\n");
  
  return 0;
}
