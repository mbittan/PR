#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>

void erreur(char* msg){
  perror(msg);
  exit(EXIT_FAILURE);
}

int main(int argc, char ** argv) {
  if(argc!=2){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <fic>\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  int fd1,fd2,i;
  struct stat st;
  char filename[1024];
  char c;
  //Ouverture du fichier a inverser
  if((fd1=open(argv[1],O_RDONLY,0666))==-1){
    erreur("open");
  }

  //Ouverture du nouveau fichier
  sprintf(filename,"%s.inv",argv[1]);
  if((fd2=open(filename,O_CREAT | O_EXCL | O_WRONLY,0666))==-1){
    erreur("open");
  }

  if(fstat(fd1,&st)==-1){
    erreur("stat");
  }

  for(i=st.st_size-1;i>=0;i--){
    if(pread(fd1,&c,sizeof(char),i)<0){
      erreur("read");
    }
    if(write(fd2,&c,sizeof(char))<0){
      erreur("write");
    }
  }
  return EXIT_SUCCESS;
}
