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
#include <sys/stat.h>
#include <libgen.h>
#include <pthread.h>

#define LISTEN 10
#define MAXTHREADS 5

#include "ftp_requete.h"

int sock;
struct addrinfo * addr;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_write = PTHREAD_COND_INITIALIZER;
char filenames[MAXTHREADS][1024];
int nbthreads=0;
int isrun[MAXTHREADS];

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

int attendre_si_ecriture(char* buff, int taille) {
  int j, k = -1;

  // Verification si fichier deja en ecriture
  pthread_mutex_lock(&mutex);
  do {
    for(j=0; j<MAXTHREADS; j++) {
      if(filenames[j][0] != '\0') {
	if(strncmp(buff, filenames[j], taille) == 0) {
	  printf("Fichier utilise, veuillez patienter...\n");
	  pthread_cond_wait(&cond_write, &mutex);	
	  break;
	}
      }
      else
	k = j;
    }
  } while(j<MAXTHREADS);
  // si non, ecrire le nom du fichier que l'on va ecrire dans une case vide
  strncpy(filenames[k], buff, taille);
  pthread_mutex_unlock(&mutex);

  return k;
}

int upload(int sock_client){
  char buff[1024];
  char * base;
  int i=0, k;
  int ret, taille, fd;
  char c;

  //Recuperation du nom du fichier.
  while((ret=recv(sock_client,&c,sizeof(char),0))>0){
    buff[i]=c;
    i++;
    if(c=='\0'){
      break;
    }
  }

  if(ret==-1){
    perror("recv");
    return -1;
  }
  
  // on verifie si quelqu un ecrit deja ce fichier
  if((k = attendre_si_ecriture(buff, i)) == -1)
    erreur("attendre_si_ecriture");

  //Ouverture du fichier
  base=basename(buff);
  printf("Writing %s...\n", base);
  if((fd=open(base,O_WRONLY | O_CREAT | O_TRUNC, 0666))==-1){
    perror("open");
    return -1;
  }

  //Recuperation de la taille du fichier
  if(recv(sock_client,&taille,sizeof(int),0)<0){
    close(fd);
    perror("recv");
    return -1;
  }
  taille=ntohl(taille);

  for(i=0;i<taille;i++){
    if(recv(sock_client,&c,sizeof(char),0)<0){
      perror("recv");
    }
    if(write(fd,&c,sizeof(char))<0){
      perror("write");
    }
  }

  if(close(fd)==-1){
    perror("close");
  }

  // on libere la case ou on a ecrit le nom de fichier en ecriture
  pthread_mutex_lock(&mutex);
  filenames[k][0] = '\0';
  pthread_cond_signal(&cond_write);
  pthread_mutex_unlock(&mutex);

  return 0;
}

int download(int sock_client){
  char buff[1024];
  int i=0;
  int ret, taille, fd;
  char c;
  struct stat st;
  char * base;

  //Recuperation du nom du fichier.
  while((ret=recv(sock_client,&c,sizeof(char),0))>0){
    buff[i]=c;
    i++;
    if(c=='\0'){
      break;
    }
  }

  if(ret==-1){
    perror("recv");
    return -1;
  }

  // on verifie si quelqu un ecrit deja ce fichier
  if(attendre_si_ecriture(buff, i) == -1)
    erreur("attendre_si_ecriture");

  base=basename(buff);
  printf("Reading %s...\n", base);
  //On recupere la structure stat du fichier, et on envoie la taille du fichier
  if(stat(base,&st)==-1){
    perror("stat");
    taille=htonl(-1);
    if(send(sock_client,&taille,sizeof(int),0)<0){
      perror("send");
    }
    return -1;
  }

  taille=htonl(st.st_size);
  if(send(sock_client,&taille,sizeof(int),0)<0){
    perror("send");
    return -1;
  }

  //Ouverture du fichier
  if((fd=open(base,O_RDONLY))==-1){
    perror("open");
  }

  for(i=0;i<st.st_size;i++){
    if(read(fd,&c,sizeof(char))<0){
      perror("read");
    }
    if(send(sock_client,&c,sizeof(char),0)<0){
      perror("send");
    }
  }

  if(close(fd)==-1){
    perror("close");
  }

  return 0;
}

int liste(int sock_client){
  DIR *pt_Dir;
  struct dirent* dirEnt;
  char c='\n';

  //On ouvre le dossier courant
  if((pt_Dir=opendir ("."))==NULL){
    perror ("opendir");
    return -1;
  }
	
  //On lit le répertoire
  while((dirEnt=readdir(pt_Dir))!=NULL){
    //On envoi le nom du fichier si ce n'est pas .. ou .
    if(strcmp(".",dirEnt->d_name)==0 || strcmp("..",dirEnt->d_name)==0){
      continue;
    }
    if(send(sock_client,(dirEnt->d_name),strlen(dirEnt->d_name),0)<0){
      perror("send");
    }
    if(send(sock_client,&c,sizeof(char),0)<0){
      perror("send");
    }
  }
  closedir (pt_Dir);

  c='\0';
  if(send(sock_client,&c,sizeof(char),0)<0){
    perror("send");
    return -1;
  }
  
  return 0;
}
void* handle_request(void* args){
  int ret,type,sock_client;
  sock_client = *((int*)args);
  //On envoie un entier au client pour lui indiquer qu'on le prend en charge.
  if(send(sock_client,&ret,sizeof(int),0)<0){
    perror("send");
  }

  while(1){
    //On recupere le type de l'opération
    if((ret=recv(sock_client, &type,sizeof(int),0))<0){
      perror("recv");
      break;
    }

    if(ret==0){
      break;
    }

    printf("Nouvelle requete recue !\n");

    type=ntohl(type);
    if(type==UPTYPE){
      ret=upload(sock_client);
      if(ret==-1){
	printf("L'upload ne s'est pas bien passe\n");
      }else{
	printf("L'upload s'est bien passee\n");
      }
    }else if(type==DOWNTYPE){
      ret=download(sock_client);
      if(ret==-1){
	printf("Le download ne s'est pas bien passe\n");
      }else{
	printf("Le download s'est bien passee\n");
      }
    }else if(type==LISTTYPE){
      ret=liste(sock_client);
      if(ret==-1){
	printf("L'envoi de la liste ne s'est pas bien passe\n");
      }else{
	printf("L'envoi de la liste s'est bien passee\n");
      }
    }else if(type==QUITTYPE){
      printf("Le client nous a quitte\n\n");
      break;
    }else{
      fprintf(stderr,"Type d'opération inconnue\n"); //Impossible
      break;
    }
  }
  pthread_mutex_lock(&mutex);
  nbthreads--;
  isrun[*((int*)(args+1))] = 0;
  if(nbthreads == MAXTHREADS-1)
    pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
  
  return NULL;
}

int main(int argc, char ** argv){
  int ret,sock_client;
  unsigned int len;
  int yes=1;
  int i=0;
  int thread_args[MAXTHREADS][2];
  pthread_t threads[MAXTHREADS];
  struct addrinfo indice, *aux;
  struct sigaction sigact;
  struct sockaddr addr_client;
  struct stat st;

  //Verification du nombre d'arguments
  if(argc!=3){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <port> <dir_path>\n",argv[0]);
    exit(EXIT_FAILURE);
  }

  //On verifie que le second argument est bien un repertoire.
  if(stat(argv[2],&st)==-1){
    erreur("stat");
  }

  if(!S_ISDIR(st.st_mode)){
    fprintf(stderr,"%s n'est pas un repertoire\n",argv[2]);
    exit(EXIT_FAILURE);
  }

  //On change le répertoire de travail 
  if(chdir(argv[2])==-1){
    erreur("chdir");
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

  // Initialisation des structures pour les threads
  memset(&isrun, 0, MAXTHREADS*sizeof(int));

  while(1){
    //On attend un nouveau client
    printf("<---- Serveur se met en attente d'un nouveau client---->\n");
    if((sock_client=accept(sock,&addr_client, &len))==-1){
      erreur("accept");
    }
    pthread_mutex_lock(&mutex);
    if(nbthreads == MAXTHREADS){
      pthread_cond_wait(&cond, &mutex);
      for(i=0; i<MAXTHREADS; i++) {
	if(!isrun[i])
	  break;
      }
    }
    nbthreads++;
    isrun[i]++;
    pthread_mutex_unlock(&mutex);
    thread_args[i][0] = sock_client;
    thread_args[i][1] = i;
    while(pthread_create(&threads[i], NULL, handle_request, (void*)thread_args) == -1) {
      perror("pthread_create");
      sleep(1);
    }
  }
  return EXIT_SUCCESS;
}  

