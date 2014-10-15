#define SVID_SOURCE 1

#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <wait.h>
#include <time.h>

#define N 3

union semun {
  int              val;    /* Valeur pour SETVAL */
  struct semid_ds *buf;    /* Tampon pour IPC_STAT, IPC_SET */
  unsigned short  *array;  /* Tableau pour GETALL, SETALL */
  struct seminfo  *__buf;  /* Tampon pour IPC_INFO
			      (spécifique à Linux) */
};

int semid;

void wait_barrier(int n){
  int zcnt;
  struct sembuf bufsem;
  bufsem.sem_num = 0;
  bufsem.sem_op = 0;
  bufsem.sem_flg = 0;
  if((zcnt = semctl(semid, 0, GETZCNT)) == -1){
    perror("semctl zcnt");
    exit(EXIT_FAILURE);
  }
  if(zcnt == n-1)
    bufsem.sem_op = -1;
  
  semop(semid, &bufsem, 1);
}

void process (int NB_PCS) {
  printf("avant barrière\n");
  wait_barrier(NB_PCS);
  printf("après barrière\n");
  exit(0);
}

int main(int argc, char ** argv){
  int i;
  pid_t pid;

  union semun semunion;
  if((semid = semget(ftok("/tmp",42),1,0666 | IPC_CREAT))==-1){
    perror("semget");
    exit(EXIT_FAILURE);
  }
  semunion.val=1;
  if(semctl(semid,0,SETVAL,semunion)==-1){
    perror("semctl");
    exit(EXIT_FAILURE);
  }

  for(i=0;i<N;i++){
    if((pid=fork())==-1){
      perror("fork");
      exit(EXIT_FAILURE);
    }else if(pid==0){
      process(N);
    }
  }

  for(i=0;i<N;i++){
    wait(NULL);
  }
  return EXIT_SUCCESS;
}
