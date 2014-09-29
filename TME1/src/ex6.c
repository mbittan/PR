#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <unistd.h>

#define N 3

void handler(int sig) {
  if(sig == SIGUSR1)
    return;
}

int main(int argc, char* argv[]){
  pid_t pids[N+1], son;
  int i;
  sigset_t mask, old_mask;
  struct sigaction act;
  
  act.sa_handler=handler;
  sigaction(SIGUSR1, &act, NULL);
  
  sigemptyset(&mask);
  sigaddset(&mask, SIGUSR1);
  sigprocmask(SIG_BLOCK, &mask, &old_mask);

  pids[0] = getpid();
	
  for(i=1; i<= N; i++){
    if((son = fork()) != 0)
      break;
    pids[i] = getpid();
  }
  if(i == N+1){
    for(i=0; i<N; i++)
      printf("%d ; ", pids[i]);
    printf("\n");
    kill(getppid(), SIGUSR1);
    exit(EXIT_SUCCESS);
  }
  else {
    sigsuspend(&old_mask);
    printf("Me : %d ; Father : %d ; Son : %d\n", pids[i-1], getppid(), son);
    if(i!=1){
      kill(getppid(), SIGUSR1);
      exit(EXIT_SUCCESS);
    }
  }

  return EXIT_SUCCESS;
}


