#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NB_THREADS 2

pthread_cond_t cond;
pthread_mutex_t mutex;
int cpt;
int flag=0;

void wait_barrier(int N) {
  pthread_mutex_lock(&mutex);
  if(++cpt == N){
    flag=1;
    pthread_cond_broadcast(&cond);
  }
  else{
    while(!flag){
      pthread_cond_wait(&cond, &mutex);
    }
  }
  pthread_mutex_unlock(&mutex);
}

void* thread_func (void *arg) {
    printf ("avant barriere\n");
    wait_barrier (NB_THREADS);
    printf ("après barriere\n");
    pthread_exit ( (void*)0);
}

int main() {
  int i;
  pthread_t t[NB_THREADS];

  cpt = 0;
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond,NULL);

  for(i=0;i<NB_THREADS;i++){
    pthread_create(&(t[i]),NULL,thread_func,NULL);
  }

  for(i=0;i<NB_THREADS;i++){
    pthread_join(t[i],NULL);
  }

  return EXIT_SUCCESS;
}
