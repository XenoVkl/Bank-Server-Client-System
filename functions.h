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

typedef struct hash_node *hash_ptr;
typedef struct account_node *account_ptr;
typedef struct waiting_node *waiting_ptr;

struct hash_node
{
	char name[50];
	int amount;
	hash_ptr next_overflow_bucket;
	account_ptr account_list;
};

struct account_node
{
	char acc_name[50];
	int acc_amount;
	account_ptr next_account;
};

struct waiting_node
{
	int sd;
	waiting_ptr next_node;
};

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
hash_ptr* hash_table;


int create_hash_node(hash_ptr*, char*, int);
int create_inner_account_node(account_ptr*, char*, int);
void release_structs(hash_ptr*, int);
void delete_account_list(account_ptr*);
void delete_bucket_list(hash_ptr*);
void print_hashtable(hash_ptr*, int);
void transfer_amount(char*, char*, int, hash_ptr*, int);
int account_balance(hash_ptr*, int, char*);
int check_transfer(char*, int, hash_ptr*, int);
int check_account(hash_ptr*, int, char*);
void *consumer(void*);
int obtain(queue_t*);
void place(queue_t*, int);
void initialize(queue_t*, int); 
int is_number(char*);
