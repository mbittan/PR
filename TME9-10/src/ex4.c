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
#include <aio.h>
#include <signal.h>

void erreur(char* msg){
  perror(msg);
  exit(EXIT_FAILURE);
}

int main(int argc, char ** argv) {
  if(argc!=2){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <nom_fic>\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  char buff[10];
  char newfile[1024];
  int i=0, j, fdl, fde,ret=1;
  struct aiocb ** a= malloc(10*sizeof(struct aiocb *));
  for(j=0;j<10;j++){
    a[j]=malloc(sizeof(struct aiocb));
  }
  sprintf(newfile,"%s.inv",argv[1]);

  if((fdl=open(argv[1],O_RDONLY,0666))==-1){
    erreur("open");
  }

  if((fde=open(newfile,O_WRONLY | O_TRUNC | O_CREAT,0666))==-1){
    erreur("open");
  }
  while((ret=read(fdl,buff,10*sizeof(char)))>0){
    for(j=ret-1;j>=0;j--){
      a[j]->aio_fildes=fde;
      a[j]->aio_offset=i+ret-j-1;
      a[j]->aio_buf=&(buff[j]);
      a[j]->aio_nbytes=sizeof(char);
      a[j]->aio_reqprio=0;
      a[j]->aio_sigevent.sigev_notify=SIGEV_NONE;
      a[j]->aio_lio_opcode=LIO_WRITE;
    }
    if(lio_listio(LIO_WAIT,a,10,NULL)==-1){
      erreur("lio_listio");
    }
    i+=10;
  }

  if(close(fde)==-1 || close(fdl)==-1){
    erreur("close");
  }  
  for(j=0;j<10;j++){
    free(a[j]);
  }
  free(a);
  return EXIT_SUCCESS;
}
