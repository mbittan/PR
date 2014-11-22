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
#include <libgen.h>

void erreur(char * msg){
  perror(msg);
  exit(EXIT_FAILURE);
}

int main(int argc, char ** argv){
  int ret,sock,fd;
  int total = 0;
  struct addrinfo indice, *aux, *addr;
  char nomfic[256];
  char * base;
  char c;

  //Verification du nombre d'arguments
  if(argc!=4){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <addr> <port> <filename>\n",argv[0]);
    exit(EXIT_FAILURE);
  }

  //On ouvre le fichier
  if((fd=open(argv[3],O_RDONLY,0666))==-1){
    erreur("open");
  }

  //On recupere le nom du fichier sans le chemin
  base = basename(argv[3]);
  strcpy(nomfic,base);  

  //On recupere les info associees a l'adresse passee en parametre
  memset(&indice, '\0',sizeof(indice));
  indice.ai_family=AF_INET;
  indice.ai_socktype=SOCK_STREAM;
  if((ret=getaddrinfo(argv[1],argv[2],&indice,&addr))!=0){
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

    if(connect(sock,aux->ai_addr,aux->ai_addrlen)==-1){
      close(sock);
      close(fd);
      perror("connect");
      continue;
    }

    break;
  }
  if(aux==NULL){
    fprintf(stderr,"Impossible d'ouvrir une socket\n");
    exit(EXIT_FAILURE);
  }

  //Envoie du nom du fichier. Comme un send peut ne pas envoyer la totalite de
  //Ce qu'on lui donne en parametre, on procede potentielement a plusieurs
  //envois.
  printf("Envoi du nom du fichier\n");
  while(total!=256){
    ret=send(sock,nomfic+total,(256-total)*sizeof(char),0);
    if(ret==-1){
      close(sock);
      close(fd);
      erreur("send");
    }
    total+=ret;
  }

  //Envoie du contenu du fichier.
  printf("Envoi du fichier\n");
  while((ret=read(fd,&c,sizeof(char)))>0){
    if((ret=send(sock,&c,sizeof(char),0))<0){
      close(sock);
      close(fd);
      erreur("send");
    }
    if(ret==0){
      fprintf(stderr,"Le serveur a ferme la socket avant la fin du transfert\n");
      close(fd);
      close(sock);
      exit(EXIT_FAILURE);
    }
  }

  if(ret==-1){
    close(sock);
    close(fd);
    erreur("write");
  }
  printf("Fichier envoye avec succes !!!!!\n");
  if(close(sock)==-1){
    erreur("close");
  }  
  if(close(fd)==-1){
    erreur("close");
  }
  freeaddrinfo(addr);
  return EXIT_SUCCESS;
}
