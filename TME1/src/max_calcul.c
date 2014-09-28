#include "max_func.h"
#include <stdlib.h>

int main(int argc, char* argv[]){
  if(argc==1){
    fprintf(stderr,"Mauvaise utilisation\n");
    fprintf(stderr,"Usage : %s nb1 nb2 nb3 ...\n",argv[0]);
    exit(EXIT_FAILURE);
  }
  int vect[argc-1];
  int i;
  for(i=1; i<argc; i++)
    vect[i-1] = atoi(argv[i]);
  printf("Le maximum est : %d\n", max_func(vect, argc-1));
     
  return EXIT_SUCCESS;
}
  
