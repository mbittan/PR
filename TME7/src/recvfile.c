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

#define LISTEN 10

int sock;
struct addrinfo * addr;

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
  int ret,sock_client,fd;
  unsigned int len;
  int yes=1;
  struct addrinfo indice, *aux;
  struct sigaction sigact;
  struct sockaddr addr_client;
  char nomfic[256];
  char c;

  //Verification du nombre d'arguments
  if(argc!=2){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <port>\n",argv[0]);
    exit(EXIT_FAILURE);
  }

  //Adresse associée a la machine locale
  memset(&indice, '\0',sizeof(indice));
  indice.ai_family=AF_INET;
  indice.ai_socktype=SOCK_STREAM;
  indice.ai_flags = AI_PASSIVE; 
  if((ret=getaddrinfo(NULL,argv[1],&indice,&addr))!=0){
    fprintf(stderr, "getaddrinfo : %s\n",gai_strerror(ret));
    exit(EXIT_FAILURE);
  }

  //On parcourt les resultats renvoyés par getaddrinfo, et on s'arrete au
  //premier qui nous permet d'ouvrir une socket, et de la lier
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
    fprintf(stderr,"Impossible d'ouvrir une socket\n");
    exit(EXIT_FAILURE);
  }

  //Ecoute sur la socket
  if(listen(sock,LISTEN)==-1){
    erreur("listen");
  }

  //Redifinition de sigint pour la terminaison du programme
  sigact.sa_handler=handler;
  if(sigaction(SIGINT,&sigact,NULL)==-1){
    erreur("sigaction");
  }

  while(1){
    //On attend un nouveau client
    printf("<---- Serveur se met en attente d'un client---->\n");
    if((sock_client=accept(sock,&addr_client, &len))==-1){
      erreur("accept");
    }
    
    //On recupere le nom du fichier
    printf("Nouveau client !\n");
    if(recv(sock_client,nomfic,256*sizeof(char),MSG_WAITALL)<0){
      perror("recv");
      close(sock_client);
      continue;
    }
    
    //Ouverture du fichier
    printf("Nom du fichier : %s\n",nomfic);
    if((fd=open(nomfic,O_WRONLY | O_CREAT | O_TRUNC,0666))==-1){
      perror("open");
      close(sock_client);
      continue;
    }

    //On recupere le contenu du fichier caractere par caractere,
    //Et on l'ecrit dans le fichier ouvert.
    printf("Ecriture du fichier ...\n");
    while((ret=recv(sock_client,&c,sizeof(char),0))>0){
      if((ret=write(fd,&c,sizeof(char)))<=0){
	perror("write");
	close(fd);
	close(sock_client);
      }
    }
    if(ret==-1){
      perror("recv");
      close(fd);
      close(sock_client);
    }

    printf("<---- Ecriture terminee ! ---->\n");
    close(fd);
    close(sock_client);
  }
  return EXIT_SUCCESS;
}  

