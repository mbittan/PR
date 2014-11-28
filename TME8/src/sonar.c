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
    if(recvfrom(sock,buff,4*sizeof(char),0,(struct sockaddr *)&ad,&len)!=4){
      perror("recvfrom");
    }
    printf("Sonar : PONG recu de la part de %s\n",inet_ntop(AF_INET,(void *)&ad,ipstr,INET_ADDRSTRLEN));
  }
  return NULL;
}

int main(int argc, char ** argv){ 
  struct sockaddr_in addr; 
  int broadcast = 1;
  pthread_t tid;

  if((sock = socket(AF_INET, SOCK_DGRAM, 0))==-1){
    perror("socket");
    exit(EXIT_FAILURE);
  }

  if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast,sizeof(broadcast))==-1){
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  memset(&addr,'\0',sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORTSERV); 
  addr.sin_addr.s_addr = INADDR_BROADCAST;

  if(pthread_create(&tid, NULL, reponse_thread, NULL)==-1){
    perror("pthread_create");
    exit(EXIT_FAILURE);
  }
  while(1){
    if(sendto(sock,"PING",4*sizeof(char),0,(struct sockaddr *)&addr,sizeof(addr))==-1){
      perror("sendto");
    }
    printf("Sonar : Envoi d'un PING\n");
    sleep(3);
  }
  return EXIT_SUCCESS;
}
