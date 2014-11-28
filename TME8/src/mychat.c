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

#define BLUE "\033[34m"
#define RED "\033[31m"
#define NORMAL "\033[39m"

struct addrinfo * addr_group, * myaddr;
int sock;
req_t requete,msg;
pthread_mutex_t mutex= PTHREAD_MUTEX_INITIALIZER;

void erreur(char * msg){
  perror(msg);
  exit(EXIT_FAILURE);
}

void handler(int sig){
  if(close(sock)==-1){
    erreur("close");
  }
  freeaddrinfo(addr_group);
  exit(EXIT_SUCCESS);
}
void * print_received_msg(void * args){
  int n=strlen("Entrez votre message : ");
  int i;
  while(1){
    if(recvfrom(sock,&msg,sizeof(req_t),0,NULL,NULL)==-1){
      perror("recvfrom");
      continue;
    }
    if(strcmp(msg.pseudo,requete.pseudo)!=0){
      pthread_mutex_lock(&mutex);
      for(i=0;i<n;i++){
	printf("\b");
      }
      i=printf(BLUE"%s"NORMAL" : %s",msg.pseudo, msg.msg);
      printf("\033[0K");
      printf("\nEntrez votre message : ");
      fflush(stdout);
      pthread_mutex_unlock(&mutex);
    }
  }
}

int main(int argc, char ** argv){ 
  int ret, yes=1;
  struct addrinfo indice, *aux;
  struct sigaction sigact;
  char buff[MESSAGE_SIZE];
  struct ip_mreq imr;
  pthread_t tid;

  //Verification des arguments
  if(argc!=4){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <addr> <port> <pseudo>\n",argv[0]);
    exit(EXIT_FAILURE);
  }

  //Verification de la longueur du pseudo 
  if(strlen(argv[3])>=PSEUDO_SIZE){
    fprintf(stderr,"Pseudo trop long. Le pseudo doit faire %d caracteres au maximum\n",PSEUDO_SIZE);
    exit(EXIT_FAILURE);
  }

  //Recopie du pseudo dans la requete
  strcpy(requete.pseudo,argv[3]);

  //On recupere les info associees a l'adresse du groupe multicast
  memset(&indice, '\0',sizeof(indice));
  indice.ai_family=AF_INET;
  indice.ai_socktype=SOCK_DGRAM;
  if((ret=getaddrinfo(argv[1],argv[2],&indice,&addr_group))!=0){
    fprintf(stderr, "getaddrinfo : %s\n",gai_strerror(ret));
    exit(EXIT_FAILURE);
  }

  //On recupere notre adresse
  memset(&indice, '\0',sizeof(indice));
  indice.ai_family=AF_INET;
  indice.ai_socktype=SOCK_DGRAM;
  indice.ai_flags=AI_PASSIVE;
  if((ret=getaddrinfo(NULL,argv[2],&indice,&myaddr))!=0){
    fprintf(stderr, "getaddrinfo : %s\n",gai_strerror(ret));
    exit(EXIT_FAILURE);
  }

  //On parcourt les resultats renvoyés par getaddrinfo, et on s'arrete au
  //premier qui nous permet d'ouvrir une socket
  for(aux=myaddr;aux!=NULL;aux=aux->ai_next){
    if((sock=socket(aux->ai_family,aux->ai_socktype,0))==-1){
      perror("socket");
      continue;
    }

    if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes))<0){
      close(sock);
      perror("setsockopt");
      continue;
    }

    //Multicast
    imr.imr_multiaddr.s_addr=(((struct sockaddr_in *)addr_group->ai_addr)->sin_addr.s_addr);
    imr.imr_interface.s_addr=INADDR_ANY;

    if(setsockopt(sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,&imr,sizeof(struct ip_mreq))<0){
      close(sock);
      perror("setsockopt");
      continue;
    }

    if(bind(sock,aux->ai_addr,aux->ai_addrlen)==-1){
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

  //Creation de la thread chargee d'afficher les messages
  if(pthread_create(&tid,NULL,print_received_msg,NULL)==-1){
    erreur("pthread_create");
  }

  printf("Bienvenue !\n");

  //Boucle principale
  while(1){
    pthread_mutex_lock(&mutex);
    printf("\nEntrez votre message : ");
    pthread_mutex_unlock(&mutex);
    fgets(buff,MESSAGE_SIZE*sizeof(char),stdin);

    pthread_mutex_lock(&mutex);
    printf("\033[F");
    buff[strlen(buff)-1]='\0';
    strcpy(requete.msg,buff);
    printf("\b\b\b\b");
    printf(RED"%s"NORMAL" : %s",requete.pseudo,requete.msg);
    printf("\033[0K");
    pthread_mutex_unlock(&mutex);
    if(sendto(sock,&requete,sizeof(req_t),0,addr_group->ai_addr,addr_group->ai_addrlen)==-1){
      perror("send");
      continue;  
    }

 
  }

  return EXIT_SUCCESS;
}
