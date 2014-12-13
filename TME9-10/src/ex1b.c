#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <aio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


int main(int argc, char ** argv){
  if(argc!=3){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <nomfic> \"<chaine>\"\n",argv[0]);
    exit(EXIT_FAILURE);
  }

  int ret,fd,fd2;
  struct aiocb  a;
  struct sigevent sigev;

  char * buff=malloc((strlen(argv[2])+1)*sizeof(char));
  memset(buff,'\0',strlen(argv[2])+1);
  if((fd=open(argv[1],O_WRONLY | O_CREAT | O_TRUNC,0666))==-1){
    perror("open");
    exit(EXIT_FAILURE);
  }

  //Preparation pour l'ecriture
  a.aio_fildes=fd;
  a.aio_offset=0;
  a.aio_buf=argv[2];
  a.aio_nbytes=strlen(argv[2]);
  a.aio_reqprio=0;

  sigev.sigev_notify=SIGEV_NONE;
  a.aio_sigevent=sigev;
  if(aio_write(&a)==-1){
    perror("aio_write");
    exit(EXIT_FAILURE);
  }

  //Ouverture du second descripteur de fichier
  if((fd2=open(argv[1],O_RDONLY))==-1){
    perror("open");
    exit(EXIT_FAILURE);
  }

  const struct aiocb * const list = {&a};
  //Attente de la fin de l'ecriture
  if(aio_suspend(&list,1,NULL)==-1){
    perror("aio_suspend");
    exit(EXIT_FAILURE);
  }
  

  //Check errors
  if((ret=aio_error(&a))>0){
    errno=ret;
    perror("aio_error");
    exit(EXIT_FAILURE);
  }

  if((ret=aio_return(&a))==-1){
    perror("aio_return");
    exit(EXIT_FAILURE);
  }

  if(ret!=(strlen(argv[2]))){
    fprintf(stderr,"Nombre d'octets ecrits different du nombre voulu\n");
    exit(EXIT_FAILURE);
  }

  //Preparation pour la lecture
  a.aio_fildes=fd2;
  a.aio_offset=0;
  a.aio_buf=buff;
  a.aio_nbytes=strlen(argv[2]);
  a.aio_reqprio=0;

  sigev.sigev_notify=SIGEV_NONE;
  a.aio_sigevent=sigev;
  if(aio_read(&a)==-1){
    perror("read");
    exit(EXIT_FAILURE);
  }

  //Attente de la fin de l'ecriture
  if(aio_suspend(&list,1,NULL)==-1){
    perror("aio_suspend");
    exit(EXIT_FAILURE);
  }
  

  //Check errors
  if((ret=aio_error(&a))>0){
    errno=ret;
    perror("aio_error");
    exit(EXIT_FAILURE);
  }

  if((ret=aio_return(&a))==-1){
    perror("aio_return");
    exit(EXIT_FAILURE);
  }

  if(ret!=(strlen(argv[2]))){
    fprintf(stderr,"Nombre d'octets lus different du nombre voulu\n");
    exit(EXIT_FAILURE);
  }

  printf("Contenu du fichier : %s\n", buff);
  free(buff);
  
  if(close(fd) < 0 || close(fd2) < 0) {
    perror("close");
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}
