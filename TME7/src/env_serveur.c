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
#include <dirent.h>
#include <fcntl.h>

#include "requete.h"

#define PORTSERV "4567"

struct addrinfo * addr;
int sock;

void erreur(char * msg){
  perror(msg);
  exit(EXIT_FAILURE);
}

void handler(int sig){
  if(close(sock)==-1){
    erreur("close");
  }
  freeaddrinfo(addr);
  exit(EXIT_SUCCESS);
}

int main(int argc, char ** argv){
  int ret;
  struct addrinfo indice, *aux;
  struct sigaction sigact;
  req_t requete;
  result_t res;
  struct sockaddr addr_client;
  unsigned int len_addr_client;
  char * buff;

  //On recupere les info associees a l'adresse du serveur
  memset(&indice, '\0',sizeof(indice));
  indice.ai_family=AF_INET;
  indice.ai_socktype=SOCK_DGRAM;
  indice.ai_flags=AI_PASSIVE;
  if((ret=getaddrinfo(NULL,PORTSERV,&indice,&addr))!=0){
    fprintf(stderr, "getaddrinfo : %s\n",gai_strerror(ret));
    exit(EXIT_FAILURE);
  }

  //On parcourt les resultats renvoyés par getaddrinfo, et on s'arrete au
  //premier qui nous permet d'ouvrir une socket et de la lier
  for(aux=addr;aux!=NULL;aux=aux->ai_next){
    if((sock=socket(aux->ai_family,aux->ai_socktype,0))==-1){
      perror("socket");
      continue;
    }
    if(bind(sock, aux->ai_addr, aux->ai_addrlen) == -1){
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

  //On redefinit le comportement de SIGINT
  sigact.sa_handler=handler;
  sigaction(SIGINT,&sigact,NULL);

  //Boucle principale
  while(1){
    //On recupere une requete
    printf("-------- Attente d'une requete --------\n");
    if(recvfrom(sock,&requete,sizeof(req_t),0,&addr_client, &len_addr_client)!=sizeof(req_t)){
      perror("recvfrom");
      continue;
    }

    //operation SET
    if(requete.type==SETENV){
      printf("Requete SET recue\n");
      if(setenv(requete.variable,requete.valeur,1)==-1){
	res.res=VALINCORRECTE;
      }else{
	res.res=OK;
      }
    }else{//Operation GET
      printf("Requete GET recue\n");
      if((buff=getenv(requete.variable))==NULL){
	res.res=EXISTEPAS;
      }else{
	res.res=OK;
	strncpy(res.valeur,buff,TAILLE_MAX-1);
	res.valeur[TAILLE_MAX-1]='\0';
      }
    }

    printf("Envoi du resultat\n");
    if(sendto(sock,&res,sizeof(result_t),0,&addr_client,len_addr_client)!=sizeof(result_t)){
      perror("sendto");
      continue;
    }
  }
  return EXIT_SUCCESS;
}
