#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <unistd.h>
#include <time.h>

#define N 3

void handler(int sig) {
  printf("%d : j'ai reçu ", getpid());
  switch(sig){
  case SIGUSR1:
    printf("SIGUSR1 !\n");
    break;
  case SIGCHLD:
    printf("SIGCHLD !\n");
    break;
  }
}

int main(int argc, char* argv[]){
  pid_t pids[N+1], son;
  int i;
  sigset_t mask, maskusr, maskchld;
  struct sigaction act;

  sigemptyset(&mask);
  sigfillset(&maskusr);
  sigfillset(&maskchld);
  sigdelset(&maskusr, SIGUSR1);
  sigdelset(&maskchld, SIGCHLD);
  sigaddset(&mask, SIGCHLD);
  sigaddset(&mask, SIGCONT);
  sigaddset(&mask, SIGUSR1);
  sigprocmask(SIG_SETMASK, &mask, NULL);

  act.sa_handler=handler;
  sigaction(SIGCHLD, &act, NULL);
  sigaction(SIGUSR1, &act, NULL);

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
    kill(getpid(), SIGSTOP);
    sigsuspend(&maskusr);
    exit(EXIT_SUCCESS);
  }
  else {
    sigsuspend(&maskchld);
    printf("Me : %d ; Father : %d ; Son : %d\n", pids[i-1], getppid(), son);
    if(i==1){
      printf("Tous les processus sont arretes.\n");
      kill(son, SIGCONT);
      sigsuspend(&maskchld);
      kill(son, SIGUSR1);
      sigsuspend(&maskchld);
    }
    else{
      kill(pids[i-1], SIGSTOP);
      kill(son, SIGCONT);
      sigsuspend(&maskchld);
      kill(son, SIGUSR1);
      sigsuspend(&maskchld);
      sigsuspend(&maskusr);
      exit(EXIT_SUCCESS);
    }
  }
  printf("Fin du programme !\n");
  return 0;
}


