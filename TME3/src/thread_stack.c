#include "thread_stack.h"

char Pop() {
  char c;
  pthread_mutex_lock(&mutex_stack_size);
  while (!stack_size) {
    pthread_cond_wait(&empty_stack, &mutex_stack_size);
  }
  c = stack[--stack_size];
  if(stack_size == MAXSIZE-1)
    pthread_cond_broadcast(&full_stack);
  //Print_stack();
  pthread_mutex_unlock(&mutex_stack_size);
  return c;
}

void Push(char c) {
  pthread_mutex_lock(&mutex_stack_size);
  while (stack_size == MAXSIZE) {
    pthread_cond_wait(&full_stack, &mutex_stack_size);
  }
  stack[stack_size++] = c;
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
