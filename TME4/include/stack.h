#ifndef STACK_H
#define STACK_H

#define SVID_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <wait.h>
#include <time.h>

#define MAXSIZE 100

int shmid;
int semid;

typedef struct _stack {
  int stack_size;
  char stack[MAXSIZE];
} stack;

union semun {
  int              val;    /* Valeur pour SETVAL */
  struct semid_ds *buf;    /* Tampon pour IPC_STAT, IPC_SET */
  unsigned short  *array;  /* Tableau pour GETALL, SETALL */
  struct seminfo  *__buf;  /* Tampon pour IPC_INFO
			      (spécifique à Linux) */
};

stack * t;

void Init_stack();

char Pop();

void Push(char c);

void Print_stack();

#endif
