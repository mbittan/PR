#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define N 10

int sum;
int cpt=0;
int flag_print=0;
pthread_mutex_t mutex;
pthread_cond_t cond_fin;


void * thread_rand(void * i){
  //Tirage aleatoire
  int random_val = (int) (10*((double)rand())/ RAND_MAX);
  printf("tid: %u;\trandom_val: %d\n",(unsigned int)pthread_self(), random_val);
  
  //Ajout de la valeur dans la variable globale
  pthread_mutex_lock(&mutex);
  sum += random_val;
  cpt++; 

  //Si on est le dernier thread a ajouter la valeur, on signale la condition
  //pour reveiller print_thread, et on met flag_print a 1
  if(cpt==N){
    flag_print=1;
    pthread_cond_signal(&cond_fin);
  }
  pthread_mutex_unlock(&mutex);
  return NULL;
}

void * print_thread(void * arg){  
  pthread_mutex_lock(&mutex);
  while(!flag_print){
    pthread_cond_wait(&cond_fin,&mutex);
  }
  printf("Somme : %d\n",sum);
  pthread_mutex_unlock(&mutex);
  return NULL;
}

int main(){
  int i;
  pthread_t t[N+1];

  //Initialisations  
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond_fin,NULL);
  sum = 0;
  srand(time(NULL));

  //Creation de print_thread
  if(pthread_create(&(t[N]),NULL,print_thread,NULL) != 0){
    fprintf(stderr, "pthread_create failed.\n");
    return EXIT_FAILURE;
  }

  //Creation des N threads
  for(i=0;i<N;i++){
    if(pthread_create(&(t[i]),NULL,thread_rand,NULL) != 0){
      fprintf(stderr, "pthread creation failed.\n");
      return EXIT_FAILURE;
    }
  }

  //Attente de la fin de toutes les threads
  for(i=0;i<=N;i++){
    if(pthread_join(t[i],NULL) != 0){
      fprintf(stderr, "pthread join failed.\n");
      return EXIT_FAILURE;
    }
  }

  
  return EXIT_SUCCESS;
}
