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
  int semncnt;
  struct sembuf bufsem[2];
  //On prend le mutex
  bufsem[0].sem_num = 0;
  bufsem[0].sem_op = -1;
  bufsem[0].sem_flg = 0;
  semop(semid, bufsem, 1);

  //On recupere le nombre de personnes endormies
  if((semncnt = semctl(semid, 1, GETNCNT)) == -1){
    perror("semctl ncnt");
    exit(EXIT_FAILURE);
  }

  //Si on est le dernier, on reveille les autres
  if(semncnt == n-1){
    bufsem[0].sem_op = 1;
    bufsem[1].sem_num = 1;
    bufsem[1].sem_op = n-1;
    bufsem[1].sem_flg = 0;
    semop(semid, bufsem, 2);
  }else{
    //Sinon on relache le mutex et on s'endort sur l'autre semaphore
    bufsem[0].sem_op=1;
    semop(semid, bufsem, 1);
    bufsem[1].sem_num = 1;
    bufsem[1].sem_op = -1;
    bufsem[1].sem_flg = 0;
    semop(semid, bufsem+1, 1);
  }
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

  //On cree deux semaphores :
  //0: Mutex
  //1: Compteur qui nous sert a savoir combien de processus sont bloques
  if((semid = semget(ftok("/tmp",42),2,0666 | IPC_CREAT))==-1){
    perror("semget");
    exit(EXIT_FAILURE);
  }

  //On met le mutex a 1
  semunion.val=1;
  if(semctl(semid,0,SETVAL,semunion)==-1){
    perror("semctl");
    exit(EXIT_FAILURE);
  }  

  //Et la seconde semaphore a 0
  semunion.val=0;
  if(semctl(semid,1,SETVAL,semunion)==-1){
    perror("semctl");
    exit(EXIT_FAILURE);
  }

  //Creation des fils
  for(i=0;i<N;i++){
    if((pid=fork())==-1){
      perror("fork");
      exit(EXIT_FAILURE);
    }else if(pid==0){
      process(N);
    }
  }

  //Attente de la fin des fils
  for(i=0;i<N;i++){
    wait(NULL);
  }
  return EXIT_SUCCESS;
}
