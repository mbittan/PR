#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define N 8

int sum;
int cpt=0;

pthread_mutex_t mutex, mutex_fin;
pthread_cond_t cond_print, cond_fin;


void * thread_rand(void * i){
  int random_val = (int) (10*((double)rand())/ RAND_MAX);
  printf("tid: %u;\trandom_val: %d\n",(unsigned int)pthread_self(), random_val);
  pthread_mutex_lock(&mutex);
  sum += random_val;
  cpt++; 
  if(cpt==N){
    pthread_cond_signal(&cond_print);
  }
  pthread_mutex_unlock(&mutex);
  return NULL;
}

void * print_thread(void * arg){
  pthread_cond_wait(&cond_print,&mutex);
  printf("Somme : %d\n",sum);
  pthread_mutex_unlock(&mutex);
  pthread_mutex_lock(&mutex_fin);
  pthread_cond_signal(&cond_fin);
  pthread_mutex_unlock(&mutex_fin);
  return NULL;
}

int main(){
  int i;
  pthread_t t[N+1];
  
  pthread_mutex_init(&mutex, NULL);
  pthread_mutex_init(&mutex_fin, NULL);

  sum = 0;
  pthread_mutex_lock(&mutex);
  pthread_mutex_lock(&mutex_fin);
  if((pthread_create(&(t[N]),NULL,print_thread,NULL) != 0) || (pthread_detach(t[N]) != 0)){
    fprintf(stderr, "pthread creation or detach failed.\n");
    return EXIT_FAILURE;
  }
  srand(time(NULL));
  
  for(i=0;i<N;i++){
    if((pthread_create(&(t[i]),NULL,thread_rand,NULL) != 0) || (pthread_detach(t[i]) != 0)){
      fprintf(stderr, "pthread creation or detach failed.\n");
      return EXIT_FAILURE;
    }
  }

  pthread_cond_wait(&cond_fin,&mutex_fin);
  pthread_mutex_unlock(&mutex_fin);
  
  return EXIT_SUCCESS;
}
