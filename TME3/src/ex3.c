#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include "thread_stack.h"

#define NB_CONSO 1
#define NB_PROD 1


void* Producteur(void* arg) {
    int c;
    while ((c = getchar()) != EOF && c != '\n') {
        Push(c);
    }
    return NULL;
}

void* Consommmateur(void* arg) {
    for (;;) {
        putchar(Pop());
        fflush(stdout);
    }
    return NULL;
}

int main() {
  pthread_t consos[NB_CONSO];
  pthread_t prods[NB_PROD];
  int i;

  Init_stack();

  for(i=0;i<NB_CONSO;i++){
    if(pthread_create(&(consos[i]),NULL,Consommmateur, NULL)!=0){
      fprintf(stderr, "pthread_create error");
      exit(EXIT_FAILURE);
    }  
  }

  for(i=0;i<NB_PROD;i++){
    if(pthread_create(&(prods[i]),NULL,Producteur, NULL)!=0){
      fprintf(stderr, "pthread_create error");
      exit(EXIT_FAILURE);
    }  
  }

  for(i=0;i<NB_CONSO;i++){
    if(pthread_join(consos[i],NULL)!=0){ 
      fprintf(stderr, "pthread_join error");
      exit(EXIT_FAILURE);
    }
  }
  for(i=0;i<NB_PROD;i++){
    if(pthread_join(prods[i],NULL)!=0){ 
      fprintf(stderr, "pthread_join error");
      exit(EXIT_FAILURE);
    }
  }

  return EXIT_SUCCESS;
}
