#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/select.h>
#include <pthread.h>
#include "mychat.h"

#define PORTSERV 9999

int sock;

void * reponse_thread(void * args){
  char buff[4];
  char ipstr[INET_ADDRSTRLEN];
  struct sockaddr_in ad;
  unsigned int len;
  while(1){
    //On attend une reponse
    if(recvfrom(sock,buff,4*sizeof(char),0,(struct sockaddr *)&ad,&len)!=4){
      perror("recvfrom");
    }

    //On affiche l'identite de la personne qui a envoye le PONG
    printf("Sonar : PONG recu de la part de %s\n",
	   inet_ntop(AF_INET,(void *)&ad,ipstr,INET_ADDRSTRLEN));
  }
  return NULL;
}

int main(int argc, char ** argv){ 
  struct sockaddr_in addr; 
  int broadcast = 1;
  pthread_t tid;

  //On recupere le un descripteur de socket
  if((sock = socket(AF_INET, SOCK_DGRAM, 0))==-1){
    perror("socket");
    exit(EXIT_FAILURE);
  }

  //On met l'option SO_BROADCAST sur la socket
  if(setsockopt(sock,SOL_SOCKET,SO_BROADCAST,&broadcast,sizeof(broadcast))==-1){
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  //On initialise l'adresse
  memset(&addr,'\0',sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORTSERV); 
  addr.sin_addr.s_addr = INADDR_BROADCAST;

  //On cree le thread qui va attendre les reponses
  if(pthread_create(&tid, NULL, reponse_thread, NULL)==-1){
    perror("pthread_create");
    exit(EXIT_FAILURE);
  }

  while(1){
    //Envoi du PING
    if(sendto(sock,"PING",4*sizeof(char),0,
	      (struct sockaddr *)&addr,sizeof(addr))==-1){
      perror("sendto");
    }
    printf("Sonar : Envoi d'un PING\n");
    sleep(3);
  }
  return EXIT_SUCCESS;
}
