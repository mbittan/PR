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

req_t *message;
sem_t *sem;
char buf[BUFSZ];

void echo_loop() {
  int c;
  while (1) {
    if((c = read(STDIN_FILENO, buf, BUFSZ)) == -1){
      perror("read");
      exit(EXIT_FAILURE);
    }
    if(c>0)
      sendMessageToServer();
    if(buf[0] != '\0'){
      printf("%s\n", buf);
      buf[0] = '\0';
    }
    usleep(100000);
  }
}

int main(int argc, char ** argv){
  char *clientid=argv[1];
  char *serverid=argv[2];
  char clientshmname[1031];
  char clientsemname[1031];
  int shm_id;

  if(argc!=3){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <client_id> <server_id>",argv[0]);
    exit(EXIT_FAILURE);
  }

  // build shared memory and semaphore names
  sprintf(clientshmname,"/%s_shm:0",clientid);
  sprintf(clientsemname,"/%s_sem:0",clientid);

  /*****************  Initializing semaphore  *****************/

  if((sem = sem_open(clientsemname, O_CREAT | O_RDWR, 0666, 1)) == SEM_FAILED){
    perror("sem_open");
    exit(EXIT_FAILURE);
  }

  /***********  Initializing shared memory segment  ***********/
  
  if((shm_id = shm_open(clientshmname, O_CREAT | O_RDWR, 0666)) == -1){
    perror("shm_open");
    exit(EXIT_FAILURE);
  }

  if(ftruncate(shm_id, sizeof(req_t)) == -1){
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }

  if((message = mmap(NULL, sizeof(req_t), PROT_READ | PROT_WRITE, MAP_SHARED,
		     shm_id, 0)) == MAP_FAILED){
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  /*******************  

  /*******************  Infinite loop  *******************/

  echo_loop();

  /***********  Closing shared memory segment  ***********/

  if(munmap(message, sizeof(req_t)) == -1){
    perror("munmap");
    exit(EXIT_FAILURE);
  }

  if(shm_unlink(clientshmname) == -1){
    perror("shm_unlink");
    exit(EXIT_FAILURE);
  }

  /*****************  Closing semaphore  *****************/
  
  if(sem_close(sem) == -1){
    perror("sem_close");
    exit(EXIT_FAILURE);
  }

  if(sem_unlink(clientsemname) == -1){
    perror("sem_unlink");
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}

