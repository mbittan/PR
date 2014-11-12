#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>

void erreur(char * msg){
   perror(msg);
   exit(EXIT_FAILURE);
}

int main(int argc, char ** argv){
  int sock, port;
  struct sockaddr_in addr;
  socklen_t size_addr = sizeof(addr);

  if(argc!=2){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <port>\n",argv[0]);
    exit(EXIT_FAILURE);
  }

  port = atoi(argv[1]);

  /* creation socket */
  if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    erreur("socket");

  memset(&addr,'\0',sizeof(struct sockaddr_in));
  sin.sin_addr.s_addr=htonl(INADDR_ANY);
  sin.sin_port=htons(port);
  sin.sin_family=AF_INET; 
  
  return EXIT_SUCCESS;
}
