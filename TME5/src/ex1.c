#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
   
#define N 5

int main(int argc, char ** argv){
  int *tab;
  int shm_des;
  int i;
  pid_t pid;
  int somme=0;

  if((shm_des=shm_open("/shmex1:0",O_RDWR | O_CREAT, 0666))==-1){
    perror("shm_open");
    exit(EXIT_FAILURE);
  }

  if(ftruncate(shm_des,N*sizeof(int))==-1){
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }

  if((tab=mmap(NULL,N*sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED,shm_des,0))
     ==MAP_FAILED){
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  for(i=0;i<N;i++){
    if((pid=fork())==-1){
      perror("fork");
      exit(EXIT_FAILURE);
    }else if(pid==0){
      srand(getpid());
      tab[i]=(int) (10*(float)rand()/ RAND_MAX);
      printf("Fils %d : %d\n",i,tab[i]);
      exit(EXIT_SUCCESS);
    }
  }

  for(i=0;i<N;i++){
    wait(NULL);
  }

  for(i=0;i<N;i++){
    somme+=tab[i];
  }

  printf("Pere : Somme = %d\n",somme);
  if(shm_unlink("/shmex1:0")==-1){
    perror("shm_unlink");
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}
