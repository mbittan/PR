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
  int i, somme=0;
  pid_t pid;
  sigset_t mask;
  int random_val;
  siginfo_t info;

  sigemptyset(&mask);
  sigaddset(&mask, SIGRTMIN);
  sigprocmask(SIG_BLOCK, &mask, NULL);

  union sigval v;
  for(i=0;i<N;i++){
    if((pid=fork())==-1){
      perror("fork");
      exit(EXIT_FAILURE);
    }
    if(pid==0){
      srand(getpid());
      random_val = (int) (10*(float)rand()/ RAND_MAX);
      v.sival_int=random_val;
      printf("Fils %d : %d\n",i+1,random_val);
      sigqueue(getppid(),SIGRTMIN,v);
      exit(EXIT_SUCCESS);
    }
  }

  for(i=0;i<N;i++){
    sigwaitinfo(&mask,&info);
    somme+=info.si_value.sival_int;
  }

  for(i=0;i<N;i++){
    wait(NULL);
  }
  printf("Pere : Somme = %d\n",somme);
  return EXIT_SUCCESS;
}
