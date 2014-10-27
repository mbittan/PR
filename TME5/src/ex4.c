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
#define PENSE 0
#define AFAIM 1
#define MANGE 2

typedef struct {
  int baguettes[N];
  int etat[N];
  sem_t sem_philo[N];
  sem_t mutex;
}shm_t;

shm_t * shm;
int cpt=0;
void sighandler(int sig){
  printf("\nDestruction de la memoire partagée\n");
  if(shm_unlink("/shmex4:0")==-1){
    perror("shm_unlink");
    exit(EXIT_FAILURE);
  }
}
void penser(int i){
  struct timespec ts;
  ts.tv_sec=0;
  ts.tv_nsec=50000000*(i+1);
  printf("Philosophe %d pense\n",i);
  nanosleep(&ts,NULL);
}

void manger(int i){
  struct timespec ts;
  ts.tv_sec=0;
  ts.tv_nsec=50000000*(i+1);
  printf("Philosophe %d mange\n",i);
  nanosleep(&ts,NULL);
}

void philosophe(int i){
  while(1){
    //philosophe pense
    penser(i);

    //Philosophe a faim.
    shm->etat[i]=AFAIM;   

    //Si les philosophes a cote de i ne mangent pas, on peut manger
    //Sinon, on attend qu'ils aient termine
    sem_wait(&(shm->mutex));
    if(shm->etat[(i-1+N)%N]!=MANGE && shm->etat[(i+1)%N]!=MANGE){
      shm->baguettes[i]=0;
      printf("Philosophe %d prend la baguette %d\n",i,i);

      shm->baguettes[(i+1)%N]=0;
      printf("Philosophe %d prend la baguette %d\n",i,(i+1)%N);

      shm->etat[i]=MANGE;
      sem_post(&(shm->sem_philo[i]));
    }
    sem_post(&(shm->mutex));
    sem_wait(&(shm->sem_philo[i]));
     
    manger(i);

    sem_wait(&(shm->mutex));
    
    //Philosophe rend les baguettes
    shm->baguettes[i]=1;
    shm->baguettes[(i+1)%N]=1;
    shm->etat[i]=PENSE;
    printf("Philosophe %d pose les baguettes %d et %d\n",i,i,(i+1)%N);

    //Si le philosophe i-1 veut manger et que ses baguettes sont dispo, on lui 
    //fait prendre les baguettes
    if(shm->etat[(i+N-1)%N]==AFAIM && shm->etat[(i-2+N)%N]!=MANGE){
      shm->baguettes[(i-1+N)%N]=0;
      printf("Philosophe %d prend la baguette %d\n",(i-1+N)%N,(i-1+N)%N);
      shm->baguettes[i]=0;
      printf("Philosophe %d prend la baguette %d\n",(i-1+N)%N,i);
      shm->etat[(i-1+N)%N]=MANGE;
      sem_post(&(shm->sem_philo[(i-1+N)%N]));
    }  

    //De meme pour i+1
    if(shm->etat[(i+1)%N]==AFAIM && shm->etat[(i+2)%N]!=MANGE){
      shm->baguettes[(i+1)%N]=0;
      printf("Philosophe %d prend la baguette %d\n",(i+1)%N,(i+1)%N);
      shm->baguettes[(i+2)%N]=0;
      printf("Philosophe %d prend la baguette %d\n",(i+1)%N,(i+2)%N);
      shm->etat[(i+1)%N]=MANGE;
      sem_post(&(shm->sem_philo[(i+1)%N]));
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

  //Initialisation des tableaux et creation des semaphores
  for(i=0;i<N;i++){
    shm->baguettes[i]=1;
    shm->etat[i]=PENSE;
    if(sem_init(&(shm->sem_philo[i]),1,0)==-1){
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

  //Changement du comportement par defaut de sigint
  struct sigaction sigact;
  sigact.sa_handler=sighandler;
  sigaction(SIGINT, &sigact, NULL);

  //Attente de la fin des fils
  for(i=0;i<N;i++){
    wait(NULL);
  } 

  return EXIT_SUCCESS;
}
