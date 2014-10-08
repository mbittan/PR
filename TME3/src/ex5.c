#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <signal.h>

#define NB_THREAD 3
pthread_t tid[NB_THREAD];
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fwait =PTHREAD_COND_INITIALIZER;
pthread_cond_t mwait =PTHREAD_COND_INITIALIZER;

void handler(int sig){}


void * func_thread(void * arg){  
  int *i;
  pthread_mutex_lock(&mutex);
  i = (int*)arg;
  if(*i != NB_THREAD){
    (*i)++;
    if(pthread_create(&(tid[*i-1]),NULL,func_thread, i)){
      fprintf(stderr, "pthread_create error");
      exit(EXIT_FAILURE);
    }     
    pthread_cond_wait(&fwait,&mutex);
  }
  else{
    pthread_cond_signal(&mwait);
    pthread_cond_wait(&fwait,&mutex);
  }
  pthread_mutex_unlock(&mutex);
  pthread_exit(0);
}

int main (int argc, char ** argv){
  int i;
  sigset_t sig;
  struct sigaction action;


  sigfillset(&sig);
  sigprocmask(SIG_SETMASK,&sig, NULL);
  action.sa_handler = &handler;

  sigaction(SIGINT, &action, NULL);

  pthread_mutex_lock(&mutex);
  i = 0;
  if(pthread_create(&(tid[i]),NULL,func_thread, &i)){
      fprintf(stderr, "pthread_create error");
      exit(EXIT_FAILURE);
    }  
    pthread_cond_wait(&mwait, &mutex);
    printf("Tout mes fils sont créés\n");

    sigdelset(&sig, SIGINT);
    sigsuspend(&sig);
    pthread_cond_broadcast(&fwait);
    pthread_mutex_unlock(&mutex);
    printf("llll\n");
    for(i=0; i < NB_THREAD; i++)
      pthread_join(tid[i], NULL);

    printf("Tout mes descendants se sont terminés\n");

    return EXIT_SUCCESS;
}











