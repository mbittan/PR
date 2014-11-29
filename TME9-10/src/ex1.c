#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <aio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

void interrupt_signal(int signo,siginfo_t *si,void *context){
  printf("Mtoto\n");
}

int main(int argc, char ** argv){
  if(argc!=3){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <nomfic> \"<chaine>\"\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  int ret,fd,fd2,sig;
  struct aiocb a;
  struct sigevent sigev;
  struct sigaction action;
  sigset_t set;

  action.sa_sigaction=interrupt_signal;
  action.sa_flags=SA_SIGINFO;
  sigaction(SIGRTMIN,&action,NULL);

  sigemptyset(&set);
  sigaddset(&set,SIGRTMIN);
  sigprocmask(SIG_BLOCK,&set,NULL);

  char * buff=malloc((strlen(argv[2])+1)*sizeof(char));
  if((fd=open(argv[1],O_WRONLY | O_CREAT | O_TRUNC,0666))==-1){
    perror("open");
    exit(EXIT_FAILURE);
  }

  a.aio_fildes=fd;
  a.aio_offset=0;
  a.aio_buf=argv[2];
  a.aio_nbytes=strlen(argv[2])+1;
  a.aio_reqprio=0;

  sigev.sigev_notify=SIGEV_SIGNAL;
  sigev.sigev_signo=SIGRTMIN;

  a.aio_sigevent=sigev;
  if(aio_write(&a)==-1){
    perror("aio_write");
    exit(EXIT_FAILURE);
  }

  if((fd2=open(argv[1],O_RDONLY))==-1){
    perror("open");
    exit(EXIT_FAILURE);
  }

  sigwait(&set,&sig);  
  
  if((ret=aio_error(&a))>0){
    errno=ret;
    perror("aio_error");
    exit(EXIT_FAILURE);
  }

  if((ret=aio_return(&a))==-1){
    perror("aio_return");
    exit(EXIT_FAILURE);
  }

  if(ret!=(strlen(argv[2])+1)){
    fprintf(stderr,"Nombre d'octets ecrits different du nombre voulu\n");
    exit(EXIT_FAILURE);
  }

  if(read(fd2,buff,strlen(argv[2])+1)==-1){
    perror("read");
    exit(EXIT_FAILURE);
  }

  printf("Contenu du fichier : %s\n", buff);
  free(buff);
  return EXIT_SUCCESS;
}
