#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]){
  int _log = atoi(argv[1]);
  long long int size = atoll(argv[2]);
  FILE *fp;
  srand(time(0));
  
  fp = fopen( "teste.bin" , "wb" );
  
  fwrite(&size, sizeof(size), 1, fp);
  
  for (long long int i = 0; i < size; i++){
    int r = (rand() % (5 + 1));
    if(_log) printf("%d ", r);
    fwrite(&r, sizeof(r), 1, fp);
  }
  if(_log) printf("\n");
  
  return 0;
}
