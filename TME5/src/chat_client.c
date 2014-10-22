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

req_t messages[NBMSG];
int curs;


int main(int argc, char ** argv){
  if(argc!=3){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <client_id> <server_id>",argv[0]);
    exit(EXIT_FAILURE);
  }

  char * clientid=argv[1];
  char * serverid=argv[2];
  char clientshmname[1031];
  int shm_id;

  sprintf(clientshmname,"/%s_shm:0",clientid);

  curs = 0;

  if((shm_id = shm_open(clientshmname, O_CREAT | O_RDWR, 0666)) == -1){
    perror("shm_open");
    exit(EXIT_FAILURE);
  }

  if(ftruncate(shm_id, NBMSG*sizeof(req_t)) == -1){
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }

  if(

  return EXIT_SUCCESS;
}
