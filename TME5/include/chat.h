#ifndef CHAT_H_
#define CHAT_H_

typedef struct request {
  long type;
  char content[1024];
} req_t;

#endif
