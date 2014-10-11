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
  int * i;
  pthread_mutex_lock(&mutex);
  i = (int*)arg;

  //Si on est pas la derniere thread, on cree un nouveau thread
  if(*i != NB_THREAD){
    if(pthread_create(&(tid[*i-1]),NULL,func_thread, i)){
      fprintf(stderr, "pthread_create error");
      exit(EXIT_FAILURE);
    }     
    (*i)++;
    
    //On attends alors que la thread principale nous donne l'ordre 
    //de se terminer 
    pthread_cond_wait(&fwait,&mutex);
  }
  else{
    //Si on est la derniere thread, on previent la thread principale
    //que toutes les threads ont ete crees.
    pthread_cond_signal(&mwait);

    //on attends alors que la thread principale nous ordonne de se terminer
    pthread_cond_wait(&fwait,&mutex);
  }
  pthread_mutex_unlock(&mutex);
  pthread_exit(NULL);
}

int main (int argc, char ** argv){
  int i=1;
  sigset_t sig;
  struct sigaction action;

  //On masque tout les signaux
  sigfillset(&sig);
  sigprocmask(SIG_SETMASK,&sig, NULL);

  //On redefinit le comportement par defaut du SIGINT
  action.sa_handler = &handler;
  sigaction(SIGINT, &action, NULL);

  pthread_mutex_lock(&mutex);
  
  //On cree le premier thread 
  if(pthread_create(&(tid[i]),NULL,func_thread, &i)){
    fprintf(stderr, "pthread_create error");
    exit(EXIT_FAILURE);
  }  

  //On attends que tout les threads aient ete crees
  pthread_cond_wait(&mwait, &mutex);
  printf("Tout mes fils sont créés\n");

  //On attends un SIGINT
  sigdelset(&sig, SIGINT);
  sigsuspend(&sig);

  //On a recu un SIGINT, on donne l'ordre aux threads de se terminer
  pthread_cond_broadcast(&fwait);
  pthread_mutex_unlock(&mutex);

  //On attends la terminaison des threads
  for(i=0; i < NB_THREAD; i++)
    pthread_join(tid[i], NULL);

  printf("Tout mes descendants se sont terminés\n");

  return EXIT_SUCCESS;
}











