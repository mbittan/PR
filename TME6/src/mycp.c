#define _XOPEN_SOURCE 777
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <sys/stat.h>
#include <errno.h>

int main(int argc, char ** argv){
  struct stat sfic1,sfic2;
  int fd1,fd2;
  char c;
  int ret;

  if(argc!=3){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"%s <fic1> <fic2>\n",argv[0]);
    exit(EXIT_FAILURE);
  }

  if(stat(argv[1], &sfic1)==-1){
    perror("stat");
    exit(EXIT_FAILURE);
  }

  if(stat(argv[2], &sfic2)!=-1 || errno!=ENOENT){
    fprintf(stderr,"Fichier %s existe deja\n",argv[2]);
    exit(EXIT_FAILURE);
  }

  if(!S_ISREG(sfic1.st_mode)){
    fprintf(stderr,"Fichier %s n'est pas un fichier regulier\n",argv[1]);
    exit(EXIT_FAILURE);
  }

  if((fd1=open(argv[1], O_RDONLY))==-1){
    perror("open");
    exit(EXIT_FAILURE);
  }

  if((fd2=open(argv[2], O_WRONLY | O_CREAT,0666))==-1){
    perror("open");
    exit(EXIT_FAILURE);
  }

  while((ret=read(fd1,&c,sizeof(char)))>0){
    if(write(fd2,&c,sizeof(char))==-1){
      perror("write");
      exit(EXIT_FAILURE);
    }
  }

  if(ret==-1){
    perror("read");
    exit(EXIT_FAILURE);
  }

  if(close(fd1)==-1){
    perror("close");
    exit(EXIT_FAILURE);
  }

  if(close(fd2)==-1){
    perror("close");
    exit(EXIT_FAILURE);
  }
  return EXIT_SUCCESS;
}
