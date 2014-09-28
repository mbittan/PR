#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
void sighandler(int sig){
  if(sig==SIGUSR1){
    printf("Processus P3 créé\n");
  }else if(sig==SIGUSR2){
    printf("Processus P3 terminé\n");
  }
}
int main(){
  pid_t pidgp=getpid();
  sigset_t mask;
  sigemptyset(&mask);
  struct sigaction sigact;
  sigact.sa_handler=sighandler;
  sigaction(SIGUSR1, &sigact,NULL);
  sigaction(SIGUSR2, &sigact,NULL);
  if(fork()==0){
    if(fork()==0){
      kill(pidgp,SIGUSR1);
      exit(EXIT_SUCCESS);
    }else{
      wait(NULL);
      kill(pidgp,SIGUSR2);
      exit(EXIT_SUCCESS);
    }
  }else{
    sigsuspend(&mask);
    sigsuspend(&mask);
    wait(NULL);
    printf("Processus P2 terminé\n");
  }

  return 0;
}
