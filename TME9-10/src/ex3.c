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

#define N 5
#define FILENAME "abitbol"

void erreur(char* msg){
  perror(msg);
  exit(EXIT_FAILURE);
}

int main() {
  int i, fd, random_val, sig, somme=0,ret;
  int values[N];
  pid_t pid;
  struct aiocb a;
  struct sigevent event;
  sigset_t mask;

  sigemptyset(&mask);
  sigaddset(&mask, SIGRTMIN);
  sigprocmask(SIG_BLOCK, &mask, NULL);

  if((fd = open(FILENAME, O_CREAT | O_RDWR | O_TRUNC, 0666)) == -1)
    erreur("open");

  a.aio_fildes=fd;
  a.aio_reqprio=0;
  
  for(i=0; i<N; i++){
    if((pid = fork()) == -1)
      erreur("fork");
    /** fils **/
    if(pid == 0){
      srand(getpid());
      random_val = (int) (10*(float)rand()/ RAND_MAX);
      printf("son %d : %d\n", i, random_val);
      a.aio_offset=i*sizeof(int);
      a.aio_buf=&random_val;
      a.aio_nbytes=sizeof(int);

      event.sigev_notify = SIGEV_SIGNAL;
      event.sigev_signo = SIGRTMIN;
      a.aio_sigevent=event;

      if(aio_write(&a) < 0)
	erreur("aio_write");

      sigwait(&mask, &sig);
      
      if((ret=aio_error(&a))>0){
	errno=ret;
	perror("aio_error");
	exit(EXIT_FAILURE);
      }

      if((ret=aio_return(&a))==-1){
	perror("aio_return");
	exit(EXIT_FAILURE);
      }

      if(ret!=sizeof(int)){
	fprintf(stderr,"Nombre d'octets ecrits different du nombre voulu\n");
	exit(EXIT_FAILURE);
      }

      exit(EXIT_SUCCESS);
    }
  }

  for(i=0; i<N; i++){
    wait(NULL);
  }
 
  a.aio_reqprio=0;
  a.aio_sigevent = event;
  event.sigev_notify = SIGEV_SIGNAL;
  event.sigev_signo = SIGRTMIN;
  a.aio_sigevent=event;

  /** pere **/
  for(i=0; i<N; i++){
    a.aio_offset=i*sizeof(int);
    a.aio_buf=&(values[i]);
    a.aio_nbytes=sizeof(int);
    if(aio_read(&a) < 0)
      erreur("read");
    sigwait(&mask, &sig);
    
    if((ret=aio_error(&a))>0){
      errno=ret;
      perror("aio_error");
      exit(EXIT_FAILURE);
    }
    
    if((ret=aio_return(&a))==-1){
      perror("aio_return");
      exit(EXIT_FAILURE);
    }
    
    if(ret!=sizeof(int)){
      fprintf(stderr,"Nombre d'octets lus different du nombre voulu\n");
      exit(EXIT_FAILURE);
    }
    somme += values[i];
   
  }

  if(close(fd) == -1)
    erreur("close");

  printf("Somme = %d\n", somme);

  return EXIT_SUCCESS;
}
