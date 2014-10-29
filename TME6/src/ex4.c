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

#define N 5
#define FILENAME "abitbol"

void erreur(char* msg){
  perror(msg);
  exit(EXIT_FAILURE);
}

int main() {
  int i, fd, random_val, somme=0;
  pid_t pid;

  if((fd = open(FILENAME, O_CREAT | O_RDWR | O_TRUNC, 0666)) == -1)
    erreur("open");
  
  for(i=0; i<N; i++){
    if((pid = fork()) == -1)
      erreur("fork");
    /** fils **/
    if(pid == 0){
      srand(getpid());
      random_val = (int) (10*(float)rand()/ RAND_MAX);
      printf("son %d : %d\n", i, random_val);
      if(pwrite(fd, &random_val, sizeof(int), i*sizeof(int)) < 0)
	erreur("write");
      exit(EXIT_SUCCESS);
    }
  }
  
  /** pere **/
  for(i=0; i<N; i++){
    wait(NULL);
  }
  for(i=0; i<N; i++){
    if(pread(fd, &random_val, sizeof(int), i*sizeof(int)) < 0)
      erreur("read");
    somme += random_val;
  }
  printf("Somme = %d\n", somme);

  if(close(fd) == -1)
    erreur("close");

  return EXIT_SUCCESS;
}
