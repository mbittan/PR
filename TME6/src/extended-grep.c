#define _XOPEN_SOURCE 777

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

void erreur(char* msg){
  perror(msg);
  exit(EXIT_FAILURE);
}
int main(int argc, char ** argv) {
  if(argc!=3){
    fprintf(stderr,"Nombre d'arguments incorrect\n");
    fprintf(stderr,"Usage : %s <expr> <chemin>\n",argv[0]);
    exit(EXIT_FAILURE);
  }

  DIR * pt_dir;
  struct dirent * dirEnt;
  struct stat st;
  char * buff;
  int fd;
  int nb=0;
  char filename[1024];

  //Ouverture du dossier
  if((pt_dir=opendir(argv[2]))==NULL){
    erreur("opendir");
  }

  //On enleve le '/' a la fin du nom du dossier, s'il y en a un
  if(argv[2][strlen(argv[2])-1]=='/'){
    argv[2][strlen(argv[2])-1]='\0';
  }

  while((dirEnt=readdir(pt_dir))!=NULL) {
    //Nom complet du fichier
    sprintf(filename,"%s/%s",argv[2],dirEnt->d_name);

    //On recupere la structure stat du fichier
    if(stat(filename, &st)==-1){
      erreur("stat");
    }

    //Si ce n'est pas un fichier regulier, on passe au fichier suivant
    if(!S_ISREG(st.st_mode)){
      continue;
    }

    //On ouvre le fichier
    if((fd=open(filename,O_RDONLY,0666))==-1){
      erreur("open");
    }

    //On lit tout le contenu
    buff=malloc(st.st_size*sizeof(char));
    if(read(fd,buff,st.st_size)==-1){
      erreur("read");
    }

    //On cherche l'expression passee en parametre
    if(strstr(buff,argv[1])!=NULL){
      printf("%s trouvé dans le fichier %s/%s\n",argv[1],argv[2],dirEnt->d_name);
      nb++;
    }

    //Desallocation de la memoire allouée et fermeture du fichier
    free(buff);
    if(close(fd)==-1){
      erreur("close");
    }
  }

  if(nb==0){
    printf("Aucun fichier valide\n");
  }

  if(closedir(pt_dir)==-1){
    erreur("closedir");
  }
  return EXIT_SUCCESS;
}
