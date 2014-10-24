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

int main(int argc, char ** argv){
  if(argc!=2){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <server_id>",argv[0]);
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}
