#define _XOPEN_SOURCE 777
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
int main(int argc, char **argv) {
  pid_t pid;
  int  j=0; int i = 0;
  while (i < 2) {
    i ++;
    if ((pid = fork ()) == -1) {
      perror ("fork");
      exit (1);
    }
    else if (pid == 0) j=i;
  }
  if (j == 2) {
    /* char *tab[]={"/bin/sleep","2", NULL}; */
    /* execv("/bin/sleep", tab); */
    execl("/bin/sleep","/bin/sleep","2",NULL);
    printf ("sans fils \n");
  } else {
    printf ("%d fils \n", (i-j) );
    while (j < i) {
      j++;
      wait (NULL);
    } 
  }
  return EXIT_SUCCESS;
 }  
