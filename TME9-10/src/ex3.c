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
  int i, fd, random_val, sig, somme=0;
  int values[N];
  pid_t pid;
  struct aiocb a;
  struct sigevent event;
  sigset_t mask;

  if((fd = open(FILENAME, O_CREAT | O_RDWR | O_TRUNC, 0666)) == -1)
    erreur("open");

  a.aio_fildes=fd;
  a.aio_reqprio=10;
  
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
      if(aio_write(&a) < 0)
	erreur("aio_write");
      exit(EXIT_SUCCESS);
    }
  }

  for(i=0; i<N; i++){
    wait(NULL);
  }
 
  a.aio_reqprio=0;
  event.sigev_notify = SIGEV_SIGNAL;
  event.sigev_signo = SIGRTMIN;
  a.aio_sigevent = event;

  sigemptyset(&mask);
  sigaddset(&mask, SIGRTMIN);
  sigprocmask(SIG_BLOCK, &mask, NULL);

  /** pere **/
  for(i=0; i<N; i++){
    a.aio_offset=i*sizeof(int);
    a.aio_buf=values+i;
    a.aio_nbytes=sizeof(int);
    if(aio_read(&a) < 0)
      erreur("read");
  }

  for(i=0; i<N; i++) {
    sigwait(&mask, &sig);
  }
  for(i=0; i<N; i++) {
    somme += values[i];
    printf("Somme = %d : %d\n", i, values[i]);
  }

  if(close(fd) == -1)
    erreur("close");



  return EXIT_SUCCESS;
}
