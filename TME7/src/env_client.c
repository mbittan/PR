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
  char buff[1024];

  //On recupere les info associees a l'adresse du serveur
  memset(&indice, '\0',sizeof(indice));
  indice.ai_family=AF_INET;
  indice.ai_socktype=SOCK_DGRAM;
  if((ret=getaddrinfo("127.0.0.1",PORTSERV,&indice,&addr))!=0){
    fprintf(stderr, "getaddrinfo : %s\n",gai_strerror(ret));
    exit(EXIT_FAILURE);
  }

  //On parcourt les resultats renvoyés par getaddrinfo, et on s'arrete au
  //premier qui nous permet d'ouvrir une socket
  for(aux=addr;aux!=NULL;aux=aux->ai_next){
    if((sock=socket(aux->ai_family,aux->ai_socktype,0))==-1){
      perror("socket");
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
    printf("Requete : ");
    fgets(buff,1024,stdin);
    //Gestion de set
    if((ret=sscanf(buff,"S %s %s", requete.variable, requete.valeur))>0){
      if(ret<2){
	fprintf(stderr,"Pas assez d'arguments pour S !\n\n");
	continue;
      }

      //Envoi de la requete
      printf("Envoi de la requete au serveur\n");
      requete.type=SETENV;
      if(sendto(sock,&requete,sizeof(req_t),0,aux->ai_addr,aux->ai_addrlen)!=sizeof(req_t)){
	perror("sendto");
	continue;
      }

      //Recuperation du resultat
      if(recvfrom(sock,&res,sizeof(result_t),0,NULL,NULL)!=sizeof(result_t)){
	perror("recvfrom");
	continue;
      }
      if(res.res==OK){
	printf("Operation S s'est bien passee !\n\n");
      }else if(res.res==VALINCORRECTE){
	fprintf(stderr,"Nom de variable incorrecte : %s\n\n",requete.variable);
      }else{
	fprintf(stderr, "Erreur inconnue de la part du serveur\n\n");
      }
    }else if((ret=sscanf(buff,"G %s",requete.variable))>0){
      //Gestion du GET
      //Envoi de la requete
      printf("Envoi de la requete au serveur\n");
      requete.type=GETENV;
      if(sendto(sock,&requete,sizeof(req_t),0,aux->ai_addr,aux->ai_addrlen)!=sizeof(req_t)){
	perror("sendto");
	continue;
      }
      
      //Recuperation du resultat
      if(recvfrom(sock,&res,sizeof(result_t),0,NULL,NULL)!=sizeof(result_t)){
	perror("recvfrom");
	continue;
      }
      if(res.res==OK){
	printf("L'operation G s'est bien passee. Variable %s = %s\n\n", 
	       requete.variable, res.valeur);
      }else if(res.res==EXISTEPAS){
	fprintf(stderr,"Variable %s n'existe pas chez le serveur\n\n",requete.variable);
      }else{
	fprintf(stderr, "Erreur inconnue de la part du serveur\n\n");
      }
    }else{
      fprintf(stderr,"Requete Invalide. Les requetes suivantes sont possibles :\n");
      fprintf(stderr,"S <variable> <valeur>\n");
      fprintf(stderr,"G <variable>\n\n");
    }
  }
  return EXIT_SUCCESS;
}
