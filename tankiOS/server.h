#ifndef __SERVER_H__
#define __SERVER_H__
#include <pthread.h>
#include <sys/types.h>
#define PORT 8888
extern int exit_status;
extern int move_player(int key, struct sockaddr_in addr);
void* udp_server(void *none);
#endif
