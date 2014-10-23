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
   
#define NBMSG 10

typedef struct request {
    long type;
    char content[1024];
} req_t;

req_t *message;

int main(int argc, char ** argv){
  char * clientid=argv[1];
  char * serverid=argv[2];
  char clientshmname[1031];
  int shm_id;

  if(argc!=3){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <client_id> <server_id>",argv[0]);
    exit(EXIT_FAILURE);
  }

  // build shared memory name
  sprintf(clientshmname,"/%s_shm:0",clientid);

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

  /*******************  Infinite loop  *******************/

  

  /***********  Closing shared memory segment  ***********/

  if(munmap(message, sizeof(req_t)) == -1){
    perror("munmap");
    exit(EXIT_FAILURE);
  }

  if(shm_unlink(clientshmname) == -1){
    perror("shm_unlink");
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}
