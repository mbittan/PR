#define SVID_SOURCE 1

#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <wait.h>
#include <time.h>

#define N 5

int main(int argc, char ** argv){
  int i;
  pid_t pid;
  key_t key = ftok("/tmp",42);
  int shmid;
  int somme=0;
  int r;
  int* shm;
 
  //Creation de la memoire partagee
  if((shmid=shmget(key, N*sizeof(int), 0666 | IPC_CREAT))==-1){
    perror("shmget");
    exit(EXIT_FAILURE);
  }
  if((shm = (int*)shmat(shmid, NULL, 0666)) == (void*)-1){
    perror("shmat");
    exit(EXIT_FAILURE);
  }

  for(i=0;i<N;i++){
    if((pid=fork())==-1){
      perror("fork");
      exit(EXIT_FAILURE);
    }else if(pid==0){
      srand(time(NULL)+i);

      //Generation de la valeur aleatoire
      r=(int)(1000*(float)rand()/RAND_MAX);
      printf("Fils %d : Ecriture de la valeur %d\n", i+1, r);
      
      //Ecriture de la valeur dans la memoire partagee
      shm[i] = r;
      shmdt(shm);
      exit(EXIT_SUCCESS);
    } 
  }

  //Attente de la fin de tout les fils
  for(i=0;i<N;i++){
    wait(NULL);
  }

  //Somme
  for(i=0;i<N;i++){
    somme+=shm[i];
  }
  printf("Somme : %d\n",somme);

  //Suppression de la memoire partagee
  shmdt(shm);
  if(shmctl(shmid, IPC_RMID, 0)==-1){
    perror("shmctl");
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}
