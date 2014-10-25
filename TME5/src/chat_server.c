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

#include "chat.h"

#define MAXCLIENTS 5

char *serverid;
char shmnames[MAXCLIENTS+1][BUFSZ+7];
shm_t *shm[MAXCLIENTS+1];
int shm_id;


void echo_loop() {
  int i;

  sem_wait(&(shm[0]->sem));
  
}


void sighandler(int sig){
  int i;

  /********  Send disconnect message to clients  *********/

  for(i=1; i<MAXCLIENTS+1; i++){
    if(shmnames[i][0] != '\0'){
      sem_wait(&shm[i]->sem);
      (shm[i]->msg).type = DISCONNECT;
      sem_post(&shm[i]->sem);
    }
  }

  /*****************  Closing semaphore  *****************/
  
  if(sem_destroy(&(shm[0]->sem)) == -1){
    perror("sem_destroy");
    exit(EXIT_FAILURE);
  }

  /**********  Closing shared memory segments  ***********/

  for(i=0; i<MAXCLIENTS+1; i++){
    if(shmnames[i][0] != '\0'){
      if(munmap(shm[i], sizeof(shm_t)) == -1){
	perror("munmap");
	exit(EXIT_FAILURE);
      }
      if(shm_unlink(shmnames[i]) == -1){
	perror("shm_unlink");
	exit(EXIT_FAILURE);
      }
    }
  }
}

int main(int argc, char ** argv){
  if(argc!=2){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <server_id>",argv[0]);
    exit(EXIT_FAILURE);
  }

  serverid=argv[1];
  // build shared memory name
  sprintf(servershmname,"/%s_shm:0",serverid);

  /***********  Initializing shared memory segment  ***********/
  
  if((shm_id = shm_open(servershmname, O_CREAT | O_RDWR, 0666)) == -1){
    perror("shm_open");
    exit(EXIT_FAILURE);
  }

  if(ftruncate(shm_id, sizeof(shm_t)) == -1){
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }

  if((shm[0] = mmap(NULL, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED,
		    shm_id, 0)) == MAP_FAILED){
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  /*****************  Initializing semaphore  *****************/

  if(sem_init(&(shm[0]->sem), 1, 1) == -1){
    perror("sem_init");
    exit(EXIT_FAILURE);
  }

  /******************  Changing signal handler  ***************/

  act.sa_handler = sighandler;
  if(sigaction(SIGINT, &act, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  /*********************  Infinite loop  **********************/

  echo_loop();


  return EXIT_SUCCESS;
}
