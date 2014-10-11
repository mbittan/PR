#include "thread_stack.h"

char Pop() {
  char c;
  pthread_mutex_lock(&mutex_stack_size);

  //Si la pile est vide, on attends qu'un producteur ecrive dedans
  while (!stack_size) {
    pthread_cond_wait(&empty_stack, &mutex_stack_size);
  }

  //Si ce n'est pas le cas, on recupere le caractere au sommet de la pile
  c = stack[--stack_size];

  //Si la taille de la pile est de MAXSIZE-1 (c'est a dire si la pile etait
  //pleine avant qu'on retire le caractere), alors on previent les producteurs
  //en attente qu'ils peuvent de nouveau ecrire sur la pile.
  if(stack_size == MAXSIZE-1)
    pthread_cond_broadcast(&full_stack);
  //Print_stack();
  pthread_mutex_unlock(&mutex_stack_size);
  return c;
}

void Push(char c) {
  pthread_mutex_lock(&mutex_stack_size);

  //Si la pile est pleine, on attends qu'elle se vide
  while (stack_size == MAXSIZE) {
    pthread_cond_wait(&full_stack, &mutex_stack_size);
  }

  //Sinon, on ecrit dans la pile et on augmente la taille de celle ci
  stack[stack_size++] = c;

  //Si la pile contient un element, ça veut dire qu'elle etait vide avant
  //qu'on rajoute notre caractere. On doit donc prevenir les consommateurs 
  //en attente que la pile n'est plus vide.
  if(stack_size == 1)
    pthread_cond_broadcast(&empty_stack);
  //Print_stack();
  pthread_mutex_unlock(&mutex_stack_size);
}

void Init_stack() {
  stack_size = 0;
  if(pthread_mutex_init(&mutex_stack_size, NULL)!=0 ||
     pthread_cond_init(&empty_stack, NULL)!=0 ||
     pthread_cond_init(&full_stack, NULL)!=0) {
    fprintf(stderr, "Init_stack error");
    exit(EXIT_FAILURE);
  }
}

void Print_stack() {
  int i;
  printf("STACK[%u] = |", (unsigned int)pthread_self());
  for(i=0; i<stack_size; i++)
    printf("%c|", stack[i]);
  printf(">\n");
}
