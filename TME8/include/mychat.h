#ifndef _MYCHAT_H_
#define _MYCHAT_H_

#define PSEUDO_SIZE 32
#define MESSAGE_SIZE 512

struct ip_mreq {
  struct in_addr imr_multiaddr; /* multicast group to join */
  struct in_addr imr_interface; /* interface to join one */
};

typedef struct _req {
  char pseudo[PSEUDO_SIZE];
  char msg[MESSAGE_SIZE];
} req_t;

#endif
