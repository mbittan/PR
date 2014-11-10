#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define N 3

void erreur(char* msg){
  perror(msg);
  exit(EXIT_FAILURE);
}
void fils(FILE * f){
  char c;
  pid_t pid=getpid();
  while((c=fgetc(f))!=EOF){
    printf("%d : %c\n",pid,c);
  }
  exit(EXIT_SUCCESS);
}
int main(int argc, char ** argv) {
  if(argc!=2){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <nom_fic>\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  FILE * f;
  pid_t pid;
  int i;
  if((f=fopen(argv[1],"r"))==NULL){
    erreur("fopen");
  }

  if(setvbuf(f,NULL,_IONBF,0)!=0){
    erreur("setvbuf");
  }

  for(i=0;i<N;i++){
    if((pid=fork())==-1){
      erreur("fork");
    }else if (pid==0){
      fils(f);
    }
  }

  for(i=0;i<N;i++){
    wait(NULL);
  }

  return EXIT_SUCCESS;
}
