#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <wait.h>
#include <string.h>
#include <semaphore.h>
#include <errno.h>

#include "chat.h"

char *clientid;
char clientshmname[BUFSZ+7];
char servershmname[BUFSZ+7];
shm_t *shm;
shm_t *servershm;
char buf[BUFSZ];

void send_message(shm_t *dest, long type, char *src){
  int i;
  
  sem_wait(&(dest->sem));
  while((dest->msg).type != EMPTY){
    sem_post(&(dest->sem));
    sem_wait(&(dest->sem));
  }
  (dest->msg).type = type;
  for(i=0; i<BUFSZ-1 && src[i] != '\0' &&  src[i] != '\n'; i++)
    (dest->msg).content[i] = src[i];
  for( ; i<BUFSZ; i++)
    (dest->msg).content[i] = '\0';
  sem_post(&(dest->sem));
}

void echo_loop() {
  int c, flags;

  /*******  Make stdin non-blocking  *******/

  if(((flags = fcntl(STDIN_FILENO, F_GETFL, 0)) == -1) ||
     (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK)) == -1){
    perror("fcntl");
    exit(EXIT_FAILURE);
  }

  /**********  Connect to server ***********/

  send_message(servershm, CONNECT, clientid);

  /**********  Send/receive loop  **********/

  while (1) {
    // check stdin for message to send
    if(((c = read(STDIN_FILENO, buf, BUFSZ)) == -1) && errno != EAGAIN){
      perror("read");
      exit(EXIT_FAILURE);
    }
    if(c>0){
      send_message(servershm, DIFF, buf);
    }
    // check shm for message received and print or disconnect
    sem_wait(&(shm->sem));
    if((shm->msg).type == DIFF){
      printf("%s\n", (shm->msg).content);
      (shm->msg).type = EMPTY;
    }
    if((shm->msg).type == DISCONNECT){
      kill(getpid(), SIGUSR1);
    }
    sem_post(&(shm->sem));
    //sleep before re-check
    sleep(1);
  }
}

void sighandler(int sig){
  if(sig != SIGUSR1){
  /*********  Send disconnect message to server  *********/

    printf("Disconnecting from server...\n");
    send_message(servershm, DISCONNECT, clientshmname);
    printf("Disconnected from server.\n");
  }

  /*****************  Closing semaphore  *****************/
  
  if(sem_destroy(&(shm->sem)) == -1){
    perror("sem_destroy");
    exit(EXIT_FAILURE);
  }
  printf("Semaphore destroyed.\n");

  /***********  Closing shared memory segment  ***********/

  if((munmap(shm, sizeof(shm_t)) == -1) ||
     (munmap(servershm, sizeof(shm_t)) == -1)){
    perror("munmap");
    exit(EXIT_FAILURE);
  }
  printf("SHM unmapped.\n");

  if(shm_unlink(clientshmname) == -1){
    perror("shm_unlink");
    exit(EXIT_FAILURE);
  }
  printf("SHM unlinked.\n");

  exit(EXIT_SUCCESS);
}

int main(int argc, char ** argv){
  char *serverid;
  int shm_id, server_shm_id;
  struct sigaction act;

  if(argc!=3){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <client_id> <server_id>\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  
  clientid=argv[1];
  serverid=argv[2];
  // build shared memory names
  sprintf(clientshmname,"/%s_shm:0",clientid);
  sprintf(servershmname,"/%s_shm:0",serverid);

  /***********  Initializing shared memory segment  ***********/
  
  if((shm_id = shm_open(clientshmname, O_CREAT | O_RDWR, 0666)) == -1){
    perror("shm_open");
    exit(EXIT_FAILURE);
  }

  if(ftruncate(shm_id, sizeof(shm_t)) == -1){
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }

  if((shm = mmap(NULL, sizeof(shm_t), PROT_READ | PROT_WRITE, MAP_SHARED,
		     shm_id, 0)) == MAP_FAILED){
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  (shm->msg).type = EMPTY;

  /*****************  Initializing semaphore  *****************/

  if(sem_init(&(shm->sem), 1, 1) == -1){
    perror("sem_init");
    exit(EXIT_FAILURE);
  }

  /********************  Open server shm  *********************/

  if((server_shm_id = shm_open(servershmname, O_RDWR, 0666)) == -1){
    perror("shm_open server");
    exit(EXIT_FAILURE);
  }

  if((servershm = mmap(NULL, sizeof(req_t), PROT_WRITE, MAP_SHARED, 
		       server_shm_id, 0)) == MAP_FAILED){
    perror("mmap server");
    exit(EXIT_FAILURE);
  }

  /******************  Changing signal handler  ***************/

  act.sa_handler = sighandler;
  if((sigaction(SIGINT, &act, NULL) == -1) ||
     (sigaction(SIGUSR1, &act, NULL) == -1)) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }

  /*********************  Infinite loop  **********************/

  echo_loop();

  return EXIT_SUCCESS;
}

