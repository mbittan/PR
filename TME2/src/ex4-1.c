#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define N 15

int sum;
int cpt=0;
int print_flag=0, fin_flag=0;
pthread_mutex_t mutex, mutex_fin;
pthread_cond_t cond_print, cond_fin;


void * thread_rand(void * i){
  //Tirage aleatoire
  int random_val = (int) (10*((double)rand())/ RAND_MAX);
  printf("tid: %u;\trandom_val: %d\n",(unsigned int)pthread_self(), random_val);

  //Ajout de la valeur dans la variable globale
  pthread_mutex_lock(&mutex);
  sum += random_val;
  cpt++; 
  
  //Si on est le dernier thread a ajouter la valeur dans la variable globale
  //On met print_flag a 1 et on signale la condition pour reveiller
  //print_thread
  if(cpt==N){
    print_flag=1;
    pthread_cond_signal(&cond_print);
  }
  pthread_mutex_unlock(&mutex);
  return NULL;
}

void * print_thread(void * arg){
  //On attends que tout les threads aient ajoute leurs valeurs
  pthread_mutex_lock(&mutex);
  while(!print_flag){
    pthread_cond_wait(&cond_print,&mutex);
  }
 
  //On affiche la somme
  printf("Somme : %d\n",sum);
  pthread_mutex_unlock(&mutex);

  //On met fin_flag a 1 et on signale la condition
  //Pour que la thread principale se reveille et que le programme 
  //puisse se terminer
  pthread_mutex_lock(&mutex_fin);
  fin_flag=1;
  pthread_cond_signal(&cond_fin);
  pthread_mutex_unlock(&mutex_fin);
  return NULL;
}

int main(){
  int i;
  pthread_t t[N+1];
  
  //Initialisations
  pthread_mutex_init(&mutex, NULL);
  pthread_mutex_init(&mutex_fin, NULL);
  pthread_cond_init(&cond_print,NULL);
  pthread_cond_init(&cond_fin,NULL);
  sum = 0;
  srand(time(NULL));

  //Creation et detachement de print_thread
  if((pthread_create(&(t[N]),NULL,print_thread,NULL) != 0) || (pthread_detach(t[N]) != 0)){
    fprintf(stderr, "pthread creation or detach failed.\n");
    return EXIT_FAILURE;
  }
  
  //Creation et detachement des N threads
  for(i=0;i<N;i++){
    if((pthread_create(&(t[i]),NULL,thread_rand,NULL) != 0) || (pthread_detach(t[i]) != 0)){
      fprintf(stderr, "pthread creation or detach failed.\n");
      return EXIT_FAILURE;
    }
  }

  //on attends que tout les threads se terminent
  pthread_mutex_lock(&mutex_fin);
  while(!fin_flag){
    pthread_cond_wait(&cond_fin,&mutex_fin);
  }
  pthread_mutex_unlock(&mutex_fin);
  
  return EXIT_SUCCESS;
}
