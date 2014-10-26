#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include <semaphore.h>
   
#define N 5
pid_t * pid_first;
sem_t *semfirst, *semothers;

void wait_barrier(int n, int id) {
  int i;  
  if(id==0){
    for(i=0;i<(n-1);i++){
      sem_wait(semfirst);
    }    
    for(i=0;i<(n-1);i++){
      sem_post(semothers);
    }
  }else{
    sem_post(semfirst);
    sem_wait(semothers);
  }
}

void process (int NB_PCS, int id) {
  printf ("avant barrière\n");
  wait_barrier (NB_PCS,id);
  printf ("après barrière\n");
  exit (0);
}

int main() {
  int i;
  int shm_des;
  pid_t pid;

  //Creation des semaphores
  if((semfirst = sem_open("/sem_ex2_PR:42", O_CREAT | O_RDWR, 0666, 0))==SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }

  if((semothers = sem_open("/sem_ex2_PR:1337", O_CREAT | O_RDWR, 0666, 0))
     ==SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }
  
  //Creation du segment de memoire partage pour le pid du premier processus 
  //cree
  if((shm_des=shm_open("/shmex2:1",O_RDWR | O_CREAT, 0666))==-1){
    perror("shm_open");
    exit(EXIT_FAILURE);
  }

  if(ftruncate(shm_des,N*sizeof(pid_t))==-1){
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }

  if((pid_first=mmap(NULL,N*sizeof(pid_t),PROT_READ | PROT_WRITE, MAP_SHARED,shm_des,0))
     ==MAP_FAILED){
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  //Creation des N processus fils
  for(i=0;i<N;i++){
      pid=fork();
    if(pid==-1){
      perror("fork");
      exit(EXIT_FAILURE);
    }else if(pid==0){
      process(N,i);
      exit(EXIT_SUCCESS);
    }
  }  

  for(i=0;i<N;i++){
    wait(NULL);
  }

  //Suppression des semaphores
  if((sem_close(semfirst)==-1) || (sem_close(semothers)==-1)){
    perror("sem_close");
    exit(EXIT_FAILURE);
  }

  if((sem_unlink("/sem_ex2_PR:42")==-1) || (sem_unlink("/sem_ex2_PR:1337")==-1)){
    perror("sem_unlink");
    exit(EXIT_FAILURE);
  }

  //Suppression de la memoire partagee
  if(shm_unlink("/shmex2:1")==-1){
    perror("shm_unlink");
    exit(EXIT_FAILURE);
  }
  
  return EXIT_SUCCESS;
}
