#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/select.h>
#include <signal.h>

#define FILENAME "cxlog"
#define LISTEN 10

FILE * f;
int * sockets;
int tabsize=0;

void handler(int sig){
  int i;
  for(i=0;i<tabsize;i++){
    if(sockets[i]!=-1){
      if(close(sockets[i])==-1){
	perror("close");
      }
    }
  }
  free(sockets);
  fclose(f);
  exit(EXIT_SUCCESS);
}

int main(int argc, char **argv){
  if(argc==1){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <port1> [<port2> ...]\n",argv[0]);
    exit(EXIT_FAILURE);
  }

  int i,ret,yes=1;
  struct addrinfo indice, * addr, *aux;
  int maxfd=-1;
  fd_set ens;
  struct sigaction sigact;
  struct sockaddr_in addr_client;
  unsigned int client_len;
  char ipstr[INET_ADDRSTRLEN];

  //Tableau pour les descripteurs des sockets
  sockets = malloc((argc-1)*sizeof(int));
  tabsize=argc-1;

  //Ouverture du fichier log
  if((f=fopen(FILENAME,"w"))==NULL){
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  //On ouvre une socket pour chaque port passe en parametre
  for(i=0;i<(argc-1);i++){
    //On recupere la structure adresse associee a la machine
    memset(&indice, '\0',sizeof(indice));
    indice.ai_family=AF_INET;
    indice.ai_socktype=SOCK_STREAM;
    indice.ai_flags = AI_PASSIVE; 
    if((ret=getaddrinfo(NULL,argv[i+1],&indice,&addr))!=0){
      fprintf(stderr, "getaddrinfo : %s\n",gai_strerror(ret));
      exit(EXIT_FAILURE);
    }

    //On essaye d'ouvrir une socket avec le(s) resultat obtenu(s).
    //On s'arrete des qu'on a ouvert une socket correctement
    for(aux=addr;aux!=NULL;aux=aux->ai_next){
      if((sockets[i]=socket(aux->ai_family,aux->ai_socktype,0))==-1){
	perror("socket");
	continue;
      }
      
      if(setsockopt(sockets[i],SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))==-1){
	perror("setsockopt");
	close(sockets[i]);
	sockets[i]=-1;
	continue;
      }
      
      if(bind(sockets[i],aux->ai_addr,aux->ai_addrlen)==-1){
	perror("bind");
	close(sockets[i]);
	sockets[i]=-1;
	continue;
      }
      
      break;
    }
    
    if(aux==NULL){
      fprintf(stderr,"Impossible d'ouvrir une socket sur le port %s\n",argv[i+1]);
      continue;
    }

    //On calcule le max des descripteurs de fichiers (plus 1 !)
    if(maxfd<=sockets[i]){
      maxfd=sockets[i]+1;
    }
    
    if(listen(sockets[i],LISTEN)==-1){
      perror("listen");
      close(sockets[i]);
      sockets[i]=-1;
      continue;
    }
  }

  //On change le comportement par defaut de SIGINT
  sigact.sa_handler=handler;
  sigaction(SIGINT,&sigact,NULL);

  //Boucle principale
  while(1){
    FD_ZERO(&ens);
    for(i=0;i<tabsize;i++){
      if(sockets[i]!=-1){
	FD_SET(sockets[i],&ens);
      }
    }

    if((ret=select(maxfd,&ens,NULL,NULL,NULL))==-1){
      perror("select");
      continue;
    }    

    for(i=0;i<tabsize;i++){
      if(sockets[i]!=-1 && FD_ISSET(sockets[i],&ens)){
	memset(&addr_client,'\0',sizeof(addr_client));
	if(accept(sockets[i],(struct sockaddr *)&addr_client,&client_len)==-1){
	  perror("accept");
	  continue;
	}
	
	fprintf(f,"%s:%s\n",
		inet_ntop(AF_INET,(void *)&(addr_client.sin_addr),ipstr,INET_ADDRSTRLEN),
		argv[i+1]);
	printf("Connectserveur : Connexion recue de %s sur le port %s\n",
	       inet_ntop(AF_INET,(void *)&(addr_client.sin_addr),ipstr,INET_ADDRSTRLEN),
	       argv[i+1]);
      }
    }
  }
  return EXIT_SUCCESS;
}
