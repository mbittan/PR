#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define N 10

//Fonction executee par tout les threads
void * thread_rand(void * i){
  printf("Numero d'ordre: %d; tid: %u\n", (*(int *)i),(unsigned int)pthread_self());
  *(unsigned int *)i=(unsigned int)pthread_self()*2;
  return i;
}

int main(){
  int i;
  pthread_t t[N];
  
  //Creation des threads
  for(i=0;i<N;i++){
    int * i2=malloc(sizeof(int));
    *i2=i;
    if(pthread_create(&(t[i]),NULL,thread_rand,i2)!=0){
      fprintf(stderr,"pthread_create error\n");
      exit(EXIT_FAILURE);
    }
  }

  //Attente de la fin des threads
  for(i=0;i<N;i++){
    unsigned int * ret;
    if(pthread_join(t[i],(void **)&ret)!=0){
      fprintf(stderr,"pthread_join error\n");
      exit(EXIT_FAILURE);
    }
    printf("Valeur de retour de %u : %u\n",(unsigned int)t[i],*ret);
    free(ret);
  }
  return EXIT_SUCCESS;
}
