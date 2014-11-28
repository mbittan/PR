#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include <signal.h>
#include <ctype.h>
#include <sys/stat.h>

#include "ftp_requete.h"

int sock;

void erreur(char * msg){
  perror(msg);
  exit(EXIT_FAILURE);
}

void handler(int sig){
  int type=htonl(QUITTYPE);
  if(send(sock,&type,sizeof(int),0)<0){
    perror("send");
  }

  if(close(sock)==-1){
    erreur("close");
  }

  printf("\n\nProgramme terminé avec sucess !\n");
  exit(EXIT_SUCCESS);
}

int upload(char * buff){
  struct stat st;
  int ret,fd,taille;
  int i=strlen("UPLOAD");
  int type=htonl(UPTYPE);
  char c;

  while(isspace(buff[i])){
    i++;
  }
  buff[strlen(buff)-1]='\0';
  buff=buff+i;

  printf("Envoi du fichier %s ...\n",buff);

  //On recupere la structure stat associee au fichier
  if(stat(buff,&st)==-1){
    perror("stat");
    return -1;
  }

  //On envoie le type d'operation que l'on veut effectuer au serveur
  //ainsi que le nom et la taille du fichier que l'on va envoyer.
  if(send(sock,&type,sizeof(int),0)<0){
    perror("send");
    return -1;
  }

  if(send(sock,buff,strlen(buff)+1,0)<0){
    perror("send");
    return -1;
  }

  if((fd=open(buff,O_RDONLY))==-1){
    perror("open");
    return -1;
  }

  taille=htonl(st.st_size);
  if(send(sock,&taille,sizeof(int),0)<0){
    perror("send");
    close(fd);
    return -1;
  }

  //Envoi du fichier
  while((ret=read(fd,&c,sizeof(char)))>0){
    if(send(sock,&c,sizeof(char),0)<0){
      perror("send");
    }
  }

  if(ret==-1){
    perror("read");
    close(fd);
    return -1;
  }

  if(close(fd)==-1){
    perror("close");
  }
  return 0;
}

int download(char * buff){
  int i=strlen("DOWNLOAD"), taille, fd;
  int type = htonl(DOWNTYPE);
  char * c;

  while(isspace(buff[i])){
    i++;
  }
  buff[strlen(buff)-1]='\0';
  buff=buff+i;
  
  printf("Telechargement du fichier %s ...\n",buff);

  //Envoi du type d'operation
  if(send(sock,&type,sizeof(int),0)<0){
    perror("send");
    return -1;
  }

  //Envoi du nom du fichier
  if(send(sock,buff,strlen(buff)+1,0)<0){
    perror("send");
    return -1;
  }

  //Recuperation de la taille du fichier.
  if(recv(sock,&taille,sizeof(int),0)<0){
    perror("recv");
    return -1;
  }

  taille=ntohl(taille);
  if(taille==-1){
    printf("Erreur : fichier %s n'existe pas chez le serveur\n",buff);
    return -1;
  }

  //Ouverture du fichier
  if((fd=open(buff, O_WRONLY | O_CREAT | O_TRUNC, 0666))==-1){
    perror("open");
    return -1;
  }

  //Ecriture du fichier
  for(i=0;i<taille;i++){
    if(recv(sock,&c,sizeof(char),0)<0){
      perror("recv");
    }
    if(write(fd,&c,sizeof(char))<0){
      perror("write");
    }
  }
  
  if(close(fd)==-1){
    perror("close");
  }

  return 0;
}

int liste(){
  int type=htonl(LISTTYPE);
  char c;
  //Envoi du type
  if(send(sock,&type,sizeof(int),0)<0){
    perror("send");
    return -1;
  }

  printf("Liste des fichiers :\n");

  while(1){
    if(recv(sock,&c,sizeof(char),0)<0){
      perror("recv");
    }
    if(c=='\0'){
      break;
    }
    printf("%c",c);
  }
  return 0;
}

int main(int argc, char ** argv){
  int ret;
  struct addrinfo indice, *aux, *addr;
  struct sigaction sigact;
  char buff[1024];
  
  //Verification du nombre d'arguments
  if(argc!=3){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <addr> <port>\n",argv[0]);
    exit(EXIT_FAILURE);
  }

  //On recupere les info associees a l'adresse du serveur
  memset(&indice, '\0',sizeof(indice));
  indice.ai_family=AF_INET;
  indice.ai_socktype=SOCK_STREAM;
  if((ret=getaddrinfo(argv[1],argv[2],&indice,&addr))!=0){
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

    if(connect(sock,aux->ai_addr,aux->ai_addrlen)==-1){
      close(sock);
      perror("connect");
      continue;
    }

    break;
  }
  if(aux==NULL){
    fprintf(stderr,"Impossible d'ouvrir une socket\n");
    exit(EXIT_FAILURE);
  }

  //Changement du comportement par defaut de SIGINT
  sigact.sa_handler=handler;
  sigaction(SIGINT,&sigact,NULL);

  //On lit un entier de la part du serveur, pour savoir si on a la main
  printf("Attente de la prise en charge par le serveur ...\n");
  if(recv(sock,&ret,sizeof(int),0)<0){
    perror("recv");
  }

  printf("Connection etablie avec le serveur !\n");
  printf("Les commandes sont les suivantes :\n");
  printf("UPLOAD <fichier>\n");
  printf("DOWNLOAD <fichier>\n");
  printf("LIST\n\n");

  //Boucle principale
  while(1){
    printf("Requete : ");
    fgets(buff,1024*sizeof(char),stdin);
    if(strncmp("UPLOAD",buff,strlen("UPLOAD"))==0){
      ret=upload(buff);
      if(ret==-1){
	printf("Erreur lors de l'envoi du fichier !\n");
      }else{
	printf("Le fichier a bien ete envoye !!\n\n");
      }
    }else if(strncmp("DOWNLOAD",buff,strlen("DOWNLOAD"))==0){    
      ret=download(buff);
      if(ret==-1){
	printf("Erreur lors du telechargement du fichier !\n");
      }else{
	printf("Le fichier a bien ete telecharge !!\n\n");
      }
    }else if(strncmp("LIST",buff,strlen("LIST"))==0){
      ret=liste();
      if(ret==-1){
	printf("Erreur lors de la demande de la liste des ficheirs");
      }
    }else{
      printf("Operation inconnue.\n\n");
    }
  }

  return EXIT_SUCCESS;
}
