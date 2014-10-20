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
  int i,j;
  pid_t pid;
  key_t key;
  int mqdes;
  int somme=0;
  struct msgbuf msg, msg2;
  
  //Ouverture de la file de messages
  key = ftok("/tmp",42);
  if((mqdes=msgget(key, 0666 | IPC_CREAT))==-1){
    perror("msgget");
    exit(EXIT_FAILURE);
  }
  
  for(i=0;i<N;i++){
    if((pid=fork())==-1){
      perror("fork");
      exit(EXIT_FAILURE);
    }else if(pid==0){
      srand(time(NULL)+i);
      msg.mtype=i+1;
      
      //On genere une valeur aleatoire pour le nombre de messages voulu
      msg.mtext=(int)((N*((float)rand()/RAND_MAX))+1);
      printf("Fils %d : Envoie de la valeur %d a son pere\n", i+1, msg.mtext);
      
      //On envoi le nombre de messages voulu au pere
      if(msgsnd(mqdes, &msg, sizeof(int),0)==-1){
	perror("msgsnd fils");
	exit(EXIT_FAILURE);
      }

      //On lit les messages du pere et on fait la somme
      for(j=msg.mtext; j>0; j--){
	if(msgrcv(mqdes, &msg2, sizeof(int),i+1+N,0)==-1){
	  perror("msgrcv fils");
	  exit(EXIT_FAILURE);
	}
	somme+=msg2.mtext;
      }
      printf("Fils %d : somme = %d\n", i+1, somme);
      exit(EXIT_SUCCESS);
    }
  }

  srand(time(NULL));
  for(i=0;i<N;i++){
    //On recupere le message du i+1eme fils
    if(msgrcv(mqdes,&msg2,sizeof(int),i+1,0)==-1){
      perror("msgrcv pere");
      exit(EXIT_FAILURE);
    }
    msg.mtype=msg2.mtype+N;
    printf("Fils %ld veut %d messages \n", msg2.mtype, msg2.mtext);
    
    //On envoie le nombre de messages demande
    for(j=0; j<msg2.mtext; j++){
      msg.mtext = (int)(100*(float)rand()/RAND_MAX);
      printf("Pere envoie %d a fils %ld\n", msg.mtext, msg2.mtype);
      if(msgsnd(mqdes, &msg, sizeof(int),0)==-1){
	perror("msgsnd pere");
	exit(EXIT_FAILURE);
      }
    }
  }
  
  //attente des fils
  for(i=0;i<N;i++){
    wait(NULL);
  }

  //Destruction de la file
  if(msgctl(mqdes, IPC_RMID, 0)==-1){
    perror("msgctl");
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}
