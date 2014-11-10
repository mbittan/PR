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
#include <sys/mman.h>
#include <errno.h>
int main(int argc, char ** argv){
  struct stat sfic1;
  int fd1,fd2;
  char *c;

  //On verifie le nombre d'arguments
  if(argc!=3){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"%s <fic1> <fic2>\n",argv[0]);
    exit(EXIT_FAILURE);
  }

  //Ouverture du fichier a copier
  if((fd1=open(argv[1], O_RDONLY))==-1){
    perror("open");
    exit(EXIT_FAILURE);
  }

  //Ouverture du nouveau fichier
  if((fd2=open(argv[2], O_WRONLY | O_CREAT | O_EXCL,0666))==-1){
    perror("open");
    exit(EXIT_FAILURE);
  }

  //On recupere les infos du fichier a copier
  if(fstat(fd1, &sfic1)==-1){
    perror("stat");
    exit(EXIT_FAILURE);
  }

  //On verifie que c'est un fichier regulier
  if(!S_ISREG(sfic1.st_mode)){
    fprintf(stderr,"Fichier %s n'est pas un fichier regulier\n",argv[1]);
    exit(EXIT_FAILURE);
  }

  //On mappe le fichier en memoire
  if((c=mmap(NULL,sfic1.st_size, PROT_READ, MAP_PRIVATE, fd1, 0))==MAP_FAILED){
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  //On ecrute tout le contenu du fichier dans le nouveau fichier
  if(write(fd2,c, sfic1.st_size)==-1){
    perror("write");
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
