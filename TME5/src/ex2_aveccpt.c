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

sem_t *sem, *mutex;
int * cpt;

void wait_barrier(int n) {
  int i;
  sem_wait(mutex);
  if(--(*cpt) == 0){
    for(i=0; i<N-1; i++)
      sem_post(sem);
    sem_post(mutex);
    return;
  }
  sem_post(mutex);
  sem_wait(sem);
}

void process (int NB_PCS) {
    printf ("avant barrière\n");
    wait_barrier (NB_PCS);
    printf ("après barrière\n");
    exit (0);
}

int main() {
  int shm_des;
  int i;
  pid_t pid;

  /* INIT */

  if((sem = sem_open("/sem_ex2_PR:42", O_CREAT | O_RDWR, 0666, 0))==SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }

  if((mutex = sem_open("/sem_ex2_PR:1337", O_CREAT | O_RDWR, 0666, 1))==SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }

  if((shm_des=shm_open("/shmex2:0",O_RDWR | O_CREAT, 0666))==-1){
    perror("shm_open");
    exit(EXIT_FAILURE);
  }

  if(ftruncate(shm_des,sizeof(int))==-1){
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }

  if((cpt=mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED,shm_des,0))
     ==MAP_FAILED){
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  *cpt = N;

  /* BODY */

  for(i=0;i<N;i++){
    if((pid=fork())==-1){
      perror("fork");
      exit(EXIT_FAILURE);
    }else if(pid==0){
      process(N);
      exit(EXIT_SUCCESS);
    }
  }

  for(i=0;i<N;i++)
    wait(NULL);

  /* CLEAN */
  
  if((sem_close(sem)==-1) || (sem_close(mutex)==-1)){
    perror("sem_close");
    exit(EXIT_FAILURE);
  }

  if((sem_unlink("/sem_ex2_PR:42")==-1) || (sem_unlink("/sem_ex2_PR:1337")==-1)){
    perror("sem_unlink");
    exit(EXIT_FAILURE);
  }

  if(shm_unlink("/shmex2:0")==-1){
    perror("shm_unlink");
    exit(EXIT_FAILURE);
  }
  
  return EXIT_SUCCESS;
}
