#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define N 10
void * thread_rand(void * i){
  printf("Numero d'ordre: %d; tid: %u\n", (*(int *)i),(unsigned int)pthread_self());
  *(unsigned int *)i=(unsigned int)pthread_self()*2;
  // pthread_exit(i);
  return i;
}

int main(){
  int i;
  pthread_t t[N];
  
  for(i=0;i<N;i++){
    int * i2=malloc(sizeof(int));
    *i2=i;
    pthread_create(&(t[i]),NULL,thread_rand,i2);
  }

  for(i=0;i<N;i++){
    unsigned int * ret;
    pthread_join(t[i],(void **)&ret);
    printf("Valeur de retour de %u : %u\n",(unsigned int)t[i],*ret);
  }
  return EXIT_SUCCESS;
}
