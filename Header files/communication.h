#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>	     /* sockets */
#include <sys/types.h>	     /* sockets */
#include <sys/socket.h>	     /* sockets */
#include <netinet/in.h>	     /* internet sockets */
#include <netdb.h>	         /* gethostbyaddr */
#include <stdlib.h>	         /* exit */
#include <signal.h>          /* signal */
#include <pthread.h>
#include <unistd.h>


typedef struct {
	int *q_array;
	int start;
	int end;
	int count;
	int max_size;
} queue_t;

pthread_mutex_t mut;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;
pthread_mutex_t mut_array[129];
queue_t queue;

void *consumer(void*);
int obtain(queue_t*);
void place(queue_t*, int);
void initialize(queue_t*, int); 

#endif