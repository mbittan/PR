#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <wait.h>
#include <unistd.h>
#include <time.h>

#define N 3

int main(int argc, char* argv[]){
  pid_t pids[N+1], son;
  int i, status;

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
    srand(time(NULL));
    exit((int)(rand () /(((double) RAND_MAX +1) /100)));
  }
  else {
    wait(&status);
    printf("Me : %d ; Father : %d ; Son : %d\n", pids[i-1], getppid(), son);
    if(i==1)
      printf("I'm %d, random value = %d\n", getpid(), WEXITSTATUS(status));
    else
      exit(WEXITSTATUS(status));
  }

  return 0;
}


