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

#define N 500

pid_t pid[N];
sigset_t mask;

void wait_barrier(int n, int id){
  int i,sig;
  union sigval v; 
  if(id==0){
    siginfo_t * infos = malloc((n-1)*sizeof(siginfo_t));
    for(i=0;i<n-1;i++){
      sigwaitinfo(&mask,&(infos[i]));
    }   
    
    for(i=0;i<n-1;i++){
      sigqueue(infos[i].si_pid,SIGRTMIN,v);
    }
  }else{
    sigqueue(pid[0],SIGRTMIN,v);
    sigwait(&mask,&sig);
  }
}

void process (int NB_PCS, int id) {
    printf ("avant barrière\n");
    wait_barrier (NB_PCS,id);
    printf ("après barrière\n");
    exit (EXIT_SUCCESS);
}

int main(int argc,char ** argv){
  int i;

  sigemptyset(&mask);
  sigaddset(&mask,SIGRTMIN);
  sigprocmask(SIG_BLOCK,&mask,NULL);
  for(i=0;i<N;i++){
    if((pid[i]=fork())==-1){
      perror("fork");
      exit(EXIT_FAILURE);
    }
    if(pid[i]==0){
      process(N,i);
    }
  }

  for(i=0;i<N;i++){
    wait(NULL);
  }
  return EXIT_SUCCESS;
}
