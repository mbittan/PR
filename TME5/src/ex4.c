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
#include <unistd.h>

#define N 10

typedef struct {
  int baguettes[N];
  sem_t sem_baguette[N];
  sem_t mutex;
}shm_t;

shm_t * shm;
int cpt=0;

void penser(int i){
  printf("Philosophe %d pense\n",i);
}

void manger(int i){
  printf("Philosophe %d mange\n",i);
}

void philosophe(int i){
  while(1){
    penser(i);

   
    sem_wait(&(shm->mutex));
    if(shm->baguettes[i]==1 && shm->baguettes[(i+1)%N]==1){
      shm->baguettes[i]=0;
      printf("Philosophe %d prend la baguette %d\n",i,i);
      shm->baguettes[(i+1)%N]=0;
      printf("Philosophe %d prend la baguette %d\n",i,(i+1)%N);
      sem_post(&(shm->sem_baguette[i]));
    }
    sem_post(&(shm->mutex));
    sem_wait(&(shm->sem_baguette[i]));
    
     
    manger(i);
    cpt++;

    sem_wait(&(shm->mutex));
    shm->baguettes[i]=1;
    shm->baguettes[(i+1)%N]=1;
    printf("Philosophe %d pose les baguettes %d et %d\n",i,i,(i+1)%N);

    if(shm->baguettes[(i+N-1)%N]==1 && shm->baguettes[i]==1){
      shm->baguettes[(i-1+N)%N]=0;
      printf("Philosophe %d prend la baguette %d\n",(i-1+N)%N,(i-1+N)%N);
      shm->baguettes[i]=0;
      printf("Philosophe %d prend la baguette %d\n",(i-1+N)%N,i);
      sem_post(&(shm->sem_baguette[(i-1+N)%N]));
    }  
    if(shm->baguettes[(i+1)%N]==1 && shm->baguettes[(i+2)%N]==1){
      shm->baguettes[(i+1)%N]=0;
      printf("Philosophe %d prend la baguette %d\n",(i+1)%N,(i+1)%N);
      shm->baguettes[(i+2)%N]=0;
      printf("Philosophe %d prend la baguette %d\n",(i+1)%N,(i+2)%N);
      sem_post(&(shm->sem_baguette[(i+1)%N]));
    }
    sem_post(&(shm->mutex));
  }
}
int main(int argc, char ** argv){
  int shm_des;
  int i;
  pid_t pid;
  //Ouverture de la memoire partagee 
  if((shm_des=shm_open("/shmex4:0",O_RDWR | O_CREAT, 0666))==-1){
    perror("shm_open");
    exit(EXIT_FAILURE);
  }

  if(ftruncate(shm_des,sizeof(shm_t))==-1){
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }

  if((shm=mmap(NULL,sizeof(shm_t),PROT_READ | PROT_WRITE, MAP_SHARED,shm_des,0))
     ==MAP_FAILED){
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  //Initialisation du tableau et creation des semaphores
  for(i=0;i<N;i++){
    shm->baguettes[i]=1;
    if(sem_init(&(shm->sem_baguette[i]),1,0)==-1){
      perror("sem_init");
      exit(EXIT_FAILURE);
    }
  }
  if(sem_init(&(shm->mutex),1,1)==-1){
    perror("sem_init");
    exit(EXIT_FAILURE);
  }
  //Creation des philosophes
  for(i=0;i<N;i++){
    if((pid=fork())==-1){
      perror("fork");
      exit(EXIT_FAILURE);
    }else if(pid==0){
      srand(getpid());
      philosophe(i);
      exit(EXIT_SUCCESS);
    }
  }

  //Attente de la fin des fils
  for(i=0;i<N;i++){
    wait(NULL);
  } 
  if(shm_unlink("/shmex4:0")==-1){
    perror("shm_unlink");
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}
