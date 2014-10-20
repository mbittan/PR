#define SVID_SOURCE 1

#include <stack.h>

#define NB_PROD 3
#define NB_CONSO 2


void* Producteur(void* arg) {
    int c;
    while ((c = getchar()) != EOF) {
      if(c != '\n' && c != '\r') {
	Push(c);
      }
    }
    exit(EXIT_SUCCESS);
}

void* Consommateur(void* arg) {
    for (;;) {
        putchar(Pop());
        fflush(stdout);
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char ** argv){
  int i;
  Init_stack();
  for(i=0;i<NB_CONSO;i++){
    if(fork()==0){
      Consommateur(NULL);
    }
  }  

  for(i=0;i<NB_PROD;i++){
    if(fork()==0){
      Producteur(NULL);
    }
  }

  for(i=0;i<NB_CONSO+NB_PROD;i++){
    wait(NULL);
  }
  return EXIT_SUCCESS;
}
