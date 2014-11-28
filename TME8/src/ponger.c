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

#define PORTSERV "9999"


int main(int argc, char ** argv){ 
  struct sockaddr_in addr; 
  unsigned int len=sizeof(addr);
  int sock;
  char buff[4];
  struct addrinfo indice, * res, *aux;
  int ret;

  memset(&indice, 0, sizeof indice);
  indice.ai_family = AF_INET;
  indice.ai_socktype = SOCK_DGRAM;
  indice.ai_flags = AI_PASSIVE;

  if((ret=getaddrinfo(NULL, PORTSERV, &indice, &res))!=0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
    return 1;
  }

  for(aux = res; aux != NULL; aux = aux->ai_next) {
    if((sock=socket(aux->ai_family, aux->ai_socktype,aux->ai_protocol))==-1){
      perror("socket");
      continue;
    }

    if(bind(sock, aux->ai_addr, aux->ai_addrlen)==-1){
      close(sock);
      perror("bind");
      continue;
    }
    break;
  }

  if(aux==NULL){
    fprintf(stderr,"Impossible d'ouvrir une socket\n");
    exit(EXIT_FAILURE);
  }
  memset(&addr,'\0',sizeof(addr));
  while(1){
    if(recvfrom(sock,buff,4*sizeof(char),0,(struct sockaddr *)&addr,&len)<0){
      perror("sendto");
      continue;
    }
    printf("Ponger : PING Recu\n");
    if(sendto(sock,"PONG",4*sizeof(char),0,(struct sockaddr *)&addr,len)<4){
      perror("recvfrom");
      continue;
    }
    printf("Ponger : PONG envoye\n");
  }
  return EXIT_SUCCESS;
}
