#define SVID_SOURCE 1

#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <wait.h>
#include <time.h>

#define N 5
struct msgbuf {
  long mtype;
  int mtext;
};
int main(int argc, char ** argv){
  int i;
  pid_t pid;
  key_t key = ftok("/tmp",42);
  int mqdes;
  int somme=0;
  struct msgbuf msg2;
  
 
  if((mqdes=msgget(key,0666 | IPC_CREAT))==-1){
    perror("msgget");
    exit(EXIT_FAILURE);
  }

  for(i=0;i<N;i++){
    if((pid=fork())==-1){
      perror("fork");
      exit(EXIT_FAILURE);
    }else if(pid==0){
      struct msgbuf msg;
      srand(time(NULL)+i);
      msg.mtype=42;
      msg.mtext=(int)(10*(float)rand()/RAND_MAX);
      printf("Fils %d : Envoie de la valeur %d\n", i+1, msg.mtext);
      if(msgsnd(mqdes, &msg, sizeof(int),0)==-1){
	perror("msgsnd");
	exit(EXIT_FAILURE);
      }
      exit(EXIT_SUCCESS);
    } 
  }

  for(i=0;i<N;i++){
    wait(NULL);
  }

  for(i=0;i<N;i++){
    if(msgrcv(mqdes,&msg2,sizeof(int),0,0)==-1){
      perror("msgrcv");
      exit(EXIT_FAILURE);
    }
    somme+=msg2.mtext;
  }
  printf("Somme : %d\n",somme);

  if(msgctl(mqdes, IPC_RMID, 0)==-1){
    perror("msgctl");
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}
