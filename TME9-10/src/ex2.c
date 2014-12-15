#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <aio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

int main(int argc, char ** argv){
  if(argc!=3){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <nomfic> \"<chaine>\"\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  int ret,fd,fd2, clk_r;
  struct aiocb a;
  struct sigevent sigev;
  sigset_t set;
  struct timespec request, remain;

  sigemptyset(&set);
  sigaddset(&set,SIGRTMIN);
  sigprocmask(SIG_BLOCK,&set,NULL);

  char * buff=malloc((strlen(argv[2])+1)*sizeof(char));
  memset(buff,'\0',strlen(argv[2])+1);
  if((fd=open(argv[1],O_WRONLY | O_CREAT | O_TRUNC,0666))==-1){
    perror("open");
    exit(EXIT_FAILURE);
  }

  a.aio_fildes=fd;
  a.aio_offset=0;
  a.aio_buf=argv[2];
  a.aio_nbytes=strlen(argv[2]);
  a.aio_reqprio=0;

  sigev.sigev_notify=SIGEV_NONE;

  a.aio_sigevent=sigev;
  if(aio_write(&a)==-1){
    perror("aio_write");
    exit(EXIT_FAILURE);
  }

  if((fd2=open(argv[1],O_RDONLY))==-1){
    perror("open");
    exit(EXIT_FAILURE);
  }

  request.tv_sec = 0;
  request.tv_nsec = 50;
  
  while ((ret=aio_error(&a)) == EINPROGRESS) {
    while ((clk_r=clock_nanosleep(CLOCK_REALTIME, 0, &request, &remain)) != 0) {
      if(clk_r != EINTR) {
	perror("clock_nanosleep");
	exit(EXIT_FAILURE);
      }
      if(clk_r == EINTR) {
	request.tv_sec  = remain.tv_sec;
	request.tv_nsec = remain.tv_nsec;
      }
      else {
	request.tv_sec = 0;
	request.tv_nsec = 50;
      }
    }
  }
  
  if(ret > 0){
    errno=ret;
    perror("aio_error");
    exit(EXIT_FAILURE);
  }

  if((ret=aio_return(&a))==-1){
    perror("aio_return");
    exit(EXIT_FAILURE);
  }

  if(ret!=(strlen(argv[2]))){
    fprintf(stderr,"Nombre d'octets ecrits different du nombre voulu\n");
    exit(EXIT_FAILURE);
  }

  if(read(fd2,buff,strlen(argv[2]))==-1){
    perror("read");
    exit(EXIT_FAILURE);
  }

  printf("Contenu du fichier : %s\n", buff);
  free(buff);
  
  if(close(fd) < 0 || close(fd2) < 0) {
    perror("close");
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}
