#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define N 8

int sum;
pthread_mutex_t mutex;

void * thread_rand(void * i){
  //Tirage aleatoire de la valeur
  int random_val = (int) (10*((double)rand())/ RAND_MAX);
  printf("tid: %u;\trandom_val: %d\n",(unsigned int)pthread_self(), random_val);

  //Ajout de la valeur dans la variable globale
  pthread_mutex_lock(&mutex);
  sum += random_val;
  pthread_mutex_unlock(&mutex);
  return NULL;
}

int main(){
  int i;
  pthread_t t[N];
  
  //Initialisation du mutex
  pthread_mutex_init(&mutex, NULL);

  //de la variable globale
  sum = 0;

  //et du generateur de nombre aleatoire
  srand(time(NULL));
  
  //creation des threads
  for(i=0;i<N;i++){
    if(pthread_create(&(t[i]),NULL,thread_rand,NULL) != 0){
      fprintf(stderr, "pthread creation failed.\n");
      return EXIT_FAILURE;
    }
  }

  //Attente de la fin des threads
  for(i=0;i<N;i++){
    if(pthread_join(t[i],NULL) != 0){
      fprintf(stderr, "pthread join failed.\n");
      return EXIT_FAILURE;
    }
  }

  //Affichage de la somme
  printf("Sum = %d\n", sum);
  return EXIT_SUCCESS;
}
