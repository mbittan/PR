#ifndef CHAT_H_
#define CHAT_H_

#define BUFSZ 1024

#define CONNECT 0
#define DIFF 1
#define DISCONNECT 2

typedef struct request {
  long type;
  char content[BUFSZ];
} req_t;

typedef struct shm {
  req_t msg;
  sem_t sem;
} shm_t;

#endif
