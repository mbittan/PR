#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#define NB_THREAD 3
int cpt=1;
int nbargs;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

void * func_thread(void * arg){
  char ** names = (char **) arg;
  int i;
  FILE * fp1, * fp2;

  while(1){
    int c = 1; 
    pthread_mutex_lock(&mutex);
    if(cpt==nbargs){
      pthread_mutex_unlock(&mutex);
      return NULL;
    }
    i=cpt++;
    pthread_mutex_unlock(&mutex);
    fp1= fopen (names[i], "r");
    fp2= fopen (names[i], "r+");
    if ((fp1== NULL) || (fp2== NULL)) {
      perror ("fopen");
      exit (1);
    }
    
    while (c !=EOF) {
      c=fgetc(fp1);
      if (c!=EOF)
	fputc(toupper(c),fp2);
    }
    fclose (fp1);
    fclose (fp2);
  }
  return NULL;
}

int main (int argc, char ** argv){
  int i;
  pthread_t tid[NB_THREAD];
  nbargs=argc;  
  for(i=0;i<NB_THREAD;i++){
    if(pthread_create(&(tid[i]),NULL,func_thread, (void *)argv)!=0){
      fprintf(stderr, "pthread_create error");
      exit(EXIT_FAILURE);
    }  
  }

  for(i=0;i<NB_THREAD;i++){
    if(pthread_join(tid[i],NULL)!=0){ 
      fprintf(stderr, "pthread_join error");
      exit(EXIT_FAILURE);
    }
  }
  return EXIT_SUCCESS;
}
