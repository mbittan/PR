#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <wait.h>
#include <arpa/inet.h>

#define N 5
#define PORT "4567"

void erreur(char * msg){
   perror(msg);
   exit(EXIT_FAILURE);
}

int main(int argc, char ** argv){
  int sock,ret,i,val,somme=0;
  int yes = 1;
  pid_t pid;
  struct addrinfo indice, *addr,*aux;

  //On recupere l'adresse associé a cette machine
  memset(&indice, '\0',sizeof(indice));
  indice.ai_family=AF_INET;
  indice.ai_socktype=SOCK_DGRAM;
  indice.ai_flags=AI_PASSIVE;
  if((ret=getaddrinfo(NULL,PORT,&indice,&addr))!=0){
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

    if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))==-1){
      erreur("setsockopt");
    }

    if(bind(sock,aux->ai_addr,aux->ai_addrlen)==-1){
      perror("bind");
      close(sock);
      continue;
    }

    break;
  }
  if(aux==NULL){
    erreur("socket");
  }
  
  //Creation des fils
  for(i=0;i<N;i++){
    if((pid=fork())==-1){
      erreur("fork");
    }else if(pid==0){
      //Creation de la valeur aleatoire
      srand(getpid());
      val=(int)(10*(float)rand()/RAND_MAX);
      printf("Fils %d : Envoie de la valeur %d\n",i+1,val);
      val=htonl(val);

      if(sendto(sock,&val,sizeof(int),0,aux->ai_addr,aux->ai_addrlen)<=0){
	erreur("sendto");
      }

      if(close(sock)==-1){
	erreur("close");
      }

      exit(EXIT_SUCCESS);
    }
  }

  //Attente de la fin des fils
  for(i=0;i<N;i++){
    wait(NULL);
  }

  //Lecture des nombres par le pere
  for(i=0;i<N;i++){
    if(recvfrom(sock,&val,sizeof(int),0,NULL,NULL)<0){
      erreur("recvfrom");
    }
    val=ntohl(val);
    somme+=val;
  }

  printf("Pere : somme = %d\n",somme);

  //Fermeture socket
  if(close(sock)==-1){
    erreur("close");
  }
  freeaddrinfo(addr);
  return EXIT_SUCCESS;
}
