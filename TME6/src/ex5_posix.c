#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>

#define N 3
void erreur(char* msg){
  perror(msg);
  exit(EXIT_FAILURE);
}

void fils(int fd){
  char c;
  pid_t pid=getpid();
  int ret;
  while((ret=read(fd,&c,sizeof(char)))>0){
    printf("%d : '%c'\n",pid,c);
  }
  if(ret==-1){
    erreur("read");
  }
  exit(EXIT_SUCCESS);
}
int main(int argc, char ** argv) {
  if(argc!=2){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <nom_fic>\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  int fd;
  pid_t pid;
  int i;

  if((fd=open(argv[1], O_RDONLY))==-1){
    erreur("open");
  }

  for(i=0;i<N;i++){
    if((pid=fork())==-1){
      erreur("fork");
    }else if (pid==0){
      fils(fd);
    }
  }

  for(i=0;i<N;i++){
    wait(NULL);
  }

  return EXIT_SUCCESS;
}
