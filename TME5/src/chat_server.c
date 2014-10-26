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
#include <string.h>

#include "chat.h"

#define MAXCLIENTS 5

char *serverid;
char shmnames[MAXCLIENTS+1][BUFSZ+7];
shm_t *shm[MAXCLIENTS+1];


void echo_loop() {
  int i, full_shm_id;
  char string[BUFSZ+7];
  shm_t *full_shm;

  while(1){
    // checking messages
    sem_wait(&(shm[0]->sem));
    printf("bla\n");
    // if message to broadcast, search for receivers and send
    if((shm[0]->msg).type == DIFF){
      printf("Emitting : %s\n", (shm[0]->msg).content);
      for(i=1; i<MAXCLIENTS+1; i++){
	if(shmnames[i][0] != '\0') {
	  sem_wait(&(shm[i]->sem));
	  while((shm[i]->msg).type != EMPTY){
	    sem_post(&(shm[i]->sem));
	    //printf("yo\n");
	    sem_wait(&(shm[i]->sem));
	  }
	  printf("To : i=%d; %s\n", i, shmnames[i]);
	  (shm[i]->msg).type = DIFF;
	  strncpy((shm[i]->msg).content, (shm[0]->msg).content, BUFSZ);
	  sem_post(&(shm[i]->sem));
	}
      }
    }
    // else if connection message, add receiver and map to his shm
    else if((shm[0]->msg).type == CONNECT){
      for(i=1; i<MAXCLIENTS+1; i++){
	if(shmnames[i][0] == '\0') {
	  sprintf(shmnames[i],"/%s_shm:0",(shm[0]->msg).content);
	  if((full_shm_id = shm_open(shmnames[i], O_RDWR, 0666)) == -1){
	    perror("shm_open connect");
	    exit(EXIT_FAILURE);
	  }
	  if((shm[i] = mmap(NULL, sizeof(shm_t), PROT_READ | PROT_WRITE, 
			      MAP_SHARED, full_shm_id, 0)) == MAP_FAILED){
	    perror("mmap connect");
	    exit(EXIT_FAILURE);
	  }
	  break;
	}
      }
      // if too many clients already connected, send disconnect message
      if(i == MAXCLIENTS+1){
	printf("Cannot connect client\n");
	sprintf(string,"/%s_shm:0",(shm[0]->msg).content);
	if((full_shm_id = shm_open(string, O_RDWR, 0666)) == -1){
	  perror("shm_open connect full");
	  exit(EXIT_FAILURE);
	}
	if((full_shm = mmap(NULL, sizeof(shm_t), PROT_READ | PROT_WRITE, 
			    MAP_SHARED, full_shm_id, 0)) == MAP_FAILED){
	  perror("mmap connect full");
	  exit(EXIT_FAILURE);
	}
	sem_wait(&(full_shm->sem));
	while((full_shm->msg).type != EMPTY){
	  sem_post(&(full_shm->sem));
	  sem_wait(&(full_shm->sem));
	}
	(full_shm->msg).type = DISCONNECT;
	sem_post(&(full_shm->sem));
	if(munmap(full_shm, sizeof(shm_t)) == -1){
	    perror("munmap connect full");
	    exit(EXIT_FAILURE);
	  }
	  if(shm_unlink(string) == -1){
	    perror("shm_unlink connect full");
	    exit(EXIT_FAILURE);
	  }
      }
      else
	printf("Connected %s\n", shmnames[i]);
    }
    // else if disconnect message, remove from receivers and close shm
    else if((shm[0]->msg).type == DISCONNECT){
      for(i=1; i<MAXCLIENTS+1; i++){
	if(strncmp((shm[0]->msg).content, shmnames[i], BUFSZ) == 0){
	  printf("Disconnecting %s\n", (shm[0]->msg).content);
	  if(munmap(shm[i], sizeof(shm_t)) == -1){
	    perror("munmap");
	    exit(EXIT_FAILURE);
	  }
	  if(shm_unlink(shmnames[i]) == -1){
	    perror("shm_unlink");
	    exit(EXIT_FAILURE);
	  }
	  shmnames[i][0] = '\0';
	  break;
	}
      }
    }
    // message treated, mark it empty, sleep and go again
    (shm[0]->msg).type = EMPTY;
    sem_post(&(shm[0]->sem));
    sleep(1);
  }
  
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
  int shm_id, i;
  struct sigaction act;

  if(argc!=2){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <server_id>\n",argv[0]);
    exit(EXIT_FAILURE);
  }

  serverid=argv[1];
  // build shared memory name
  sprintf(shmnames[0],"/%s_shm:0",serverid);

  /***********  Initializing shared memory segment  ***********/
  
  if((shm_id = shm_open(shmnames[0], O_CREAT | O_RDWR, 0666)) == -1){
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

  (shm[0]->msg).type = EMPTY;
  (shm[0]->msg).content[0] = 'a';
  (shm[0]->msg).content[1] = 'e';
  (shm[0]->msg).content[2] = 'i';
  (shm[0]->msg).content[3] = '\0';
  for(i=1; i<MAXCLIENTS+1; i++)
    shmnames[i][0] = '\0';

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
