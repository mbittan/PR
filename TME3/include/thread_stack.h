#ifndef THREAD_STACK_H
#define THREAD_STACK_H

#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAXSIZE 100

int stack_size;
char stack[MAXSIZE];

pthread_mutex_t mutex_stack_size;
pthread_cond_t empty_stack;
pthread_cond_t full_stack;

void Init_stack();

char Pop();

void Push(char c);

void Print_stack();

#endif
