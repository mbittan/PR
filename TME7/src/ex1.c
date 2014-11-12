#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h> 

#define N 5

void erreur(char * msg){
   perror(msg);
   exit(EXIT_FAILURE);
}

int main(int argc, char ** argv){
  int i, sock, r, sum=0, val=1;
  pid_t pid;
  struct sockaddr_un addr;
  socklen_t size_addr = sizeof(addr);
  memset(&addr,'\0',sizeof(struct sockaddr_un));
  addr.sun_family=AF_UNIX;
  strcpy(addr.sun_path,"./MySock");
 
  if((sock=socket(AF_UNIX,SOCK_DGRAM, 0))==-1){
    erreur("socket");
  }

  if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int))==-1){
    erreur("setsockopt");
  }
  
  if(bind(sock,(struct sockaddr *)&addr,sizeof(addr))==-1){
    erreur("bind");
  }
  
  for(i=0;i<N;i++){
    if((pid=fork())==-1){
      erreur("fork");
    }else if(pid==0){
      srand(getpid());

      //Generation de la valeur aleatoire
      r=(int)(10*(float)rand()/RAND_MAX);
      printf("Fils %d : Ecriture de la valeur %d\n", i+1, r);
      if(sendto(sock, &r, sizeof(r), 0, (struct sockaddr*) &addr, size_addr) == -1) {
	erreur("sendto");
      }
      close(sock);
      exit(EXIT_SUCCESS);
    }
  }

  for(i=0; i<N; i++){
    if(recvfrom(sock, &r, sizeof(r), 0, (struct sockaddr*) &addr, &size_addr) == -1){
      erreur("recvfrom");
    }
    sum+=r;
  }

  close(sock);
  unlink("./MySock");
  printf("Sum = %d\n", sum);

  return EXIT_SUCCESS;
}
