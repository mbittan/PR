#define _XOPEN_SOURCE 777
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int nfork (int nb_fils){
  int i;
  pid_t pid;
  for(i=0;i<nb_fils;i++){
    if((pid=fork())==0){
      return 0;
    }else if(pid==-1){
      return (i==0)?-1:i;
    }
  }
  return i;
}

int main (int arg, char* argv []) {
  int p;
  int i=1; int N = 3;
  do {
    p = nfork (i) ;
    if (p != 0 )
      printf ("%d \n",p); 
  } while ((p ==0) && (++i<=N));
  printf ("FIN \n");      
  return EXIT_SUCCESS;
}
