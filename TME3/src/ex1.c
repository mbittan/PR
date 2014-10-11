#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>


void * func_thread(void * arg){
  char * name = (char *) arg;
  FILE * fp1, * fp2;
  int c = 1; 

  //Ouverture du fichier
  fp1= fopen (name, "r");
  fp2= fopen (name, "r+");
  if ((fp1== NULL) || (fp2== NULL)) {
    perror ("fopen");
    exit (1);
  }
  
  //On transforme toutes les lettres minuscules en majuscules
  while (c !=EOF) {
    c=fgetc(fp1);
    if (c!=EOF)
      fputc(toupper(c),fp2);
  }

  //Fermeture des fichiers
  fclose(fp1);
  fclose(fp2);
  return NULL;
}

int main (int argc, char ** argv){
  int i;
  //Tableau pour les ids de chaque thread
  pthread_t * tid=malloc((argc-1)*sizeof(pthread_t));

  //Creation des threads
  for(i=1;i<argc;i++){
    if(pthread_create(&(tid[i-1]),NULL,func_thread, (void *)argv[i])!=0){
      fprintf(stderr, "pthread_create error");
      exit(EXIT_FAILURE);
    }  
  }

  //Attente de la terminaison de tout les threads
  for(i=0;i<argc-1;i++){
    if(pthread_join(tid[i],NULL)!=0){ 
      fprintf(stderr, "pthread_join error");
      exit(EXIT_FAILURE);
    }
  }

  return EXIT_SUCCESS;
}
