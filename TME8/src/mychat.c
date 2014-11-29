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
//Mutex pour l'affichage, afin que le thread principal et le thread charge
//d'afficher les messages recus n'ecrivent pas en meme temps sur stdout
pthread_mutex_t mutex= PTHREAD_MUTEX_INITIALIZER;
pthread_t tid;
struct ip_mreq imr;

void erreur(char * msg){
  perror(msg);
  exit(EXIT_FAILURE);
}

void handler(int sig){
  if(close(sock)==-1){
    erreur("close");
  }
  freeaddrinfo(addr_group);
  freeaddrinfo(myaddr);

  ///On envoi un signal au thread pour qu'il se termine
  pthread_kill(tid,SIGTERM);
  exit(EXIT_SUCCESS);    
}

void * print_received_msg(void * args){
  int n=strlen("Entrez votre message : ");
  int i;
  while(1){
    //Reception d'un message
    if(recvfrom(sock,&msg,sizeof(req_t),0,NULL,NULL)==-1){
      perror("recvfrom");
      continue;
    }

    //Si le message recu provient d'une autre personne, on l'affiche
    if(strcmp(msg.pseudo,requete.pseudo)!=0){
      pthread_mutex_lock(&mutex);
      //On deplace le curseur du terminal au debut de la ligne
      for(i=0;i<n;i++){
      	printf("\b");
      }

      //On affiche le message
      printf(BLUE"%s"NORMAL" : %s",msg.pseudo, msg.msg);

      //On efface les caracteres restants s'il y en a
      printf("\033[0K");

      //On reaffiche la demande de message
      printf("\nEntrez votre message : ");

      //On force l'affichage.
      fflush(stdout);
      pthread_mutex_unlock(&mutex);
    }
  }
  return NULL;
}

int main(int argc, char ** argv){ 
  int ret, yes=1;
  struct addrinfo indice, *aux;
  struct sigaction sigact;
  char buff[MESSAGE_SIZE];

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

    //Attente d'un message de la part de l'utilisateur
    fgets(buff,MESSAGE_SIZE*sizeof(char),stdin);

    pthread_mutex_lock(&mutex);
    
    //Retour a la ligne precedente
    printf("\033[F");
    
    //On enleve le saut de ligne du message
    buff[strlen(buff)-1]='\0';

    //On copie le message dans la structure que l'on va envoyer aux autres
    //membre du groupe de multicast
    strcpy(requete.msg,buff);

    //On affiche notre message
    printf(RED"%s"NORMAL" : %s",requete.pseudo,requete.msg);

    //On efface les autres caracteres
    printf("\033[0K");

    pthread_mutex_unlock(&mutex);

    //On envoie le message
    if(sendto(sock,&requete,sizeof(req_t),0,addr_group->ai_addr,addr_group->ai_addrlen)==-1){
      perror("send");
      continue;  
    }

 
  }

  return EXIT_SUCCESS;
}
