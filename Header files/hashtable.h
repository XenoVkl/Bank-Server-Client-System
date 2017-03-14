#ifndef HASHTABLE_H
#define HASHTABLE_H
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
int is_number(char*);

#endif