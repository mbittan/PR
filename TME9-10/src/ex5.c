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

int main(int argc,char ** argv){
  int i, sig;
  pid_t pid;
  sigset_t mask;

  sigemptyset(&mask);
  sigaddset(&mask, SIGRTMIN);
  sigprocmask(SIG_BLOCK, &mask, NULL);

  union sigval v;
  for(i=0;i<N;i++){
    if((pid=fork())==-1){
      perror("fork");
      exit(EXIT_FAILURE);
    }else if(pid==0){
      if(i==(N-1)){
	printf("Fils %d, pid %d\n", i+1, getpid());
	sigqueue(getppid(),SIGRTMIN,v);
	exit(EXIT_SUCCESS);
      }
    }else{
      sigwait(&mask,&sig);
      if(i==0){
	printf("Je suis le pere : %d\n", getpid());
	exit(EXIT_SUCCESS);
      }else{
	printf("Fils %d, pid %d\n",i,getpid());
	sigqueue(getppid(),SIGRTMIN,v);
	exit(EXIT_SUCCESS);
      }
      wait(NULL);
    }
  }

  return EXIT_SUCCESS;
}
