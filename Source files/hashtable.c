#include "communication.h"
#include "hashtable.h"

int create_hash_node(hash_ptr* hash_addr, char *name, int amount) // creates a bank account in the right place of the hashtable(returns 0 for failure - 1 for success)
{
	while (*hash_addr != NULL) //traverse the hash bucket list
	{
		if (strcmp((*hash_addr)->name, name) == 0) // if the acount exists return 0
			return 0;
		hash_addr = &((*hash_addr)->next_overflow_bucket);
	}
	*hash_addr = malloc(sizeof(struct hash_node)); //create a new overflow bucket(bank account)
	strcpy((*hash_addr)->name, name);
	(*hash_addr)->amount = amount;
	(*hash_addr)->next_overflow_bucket = NULL;
	(*hash_addr)->account_list = NULL;
	//printf("Success : Created bank account with name %s and initial amount %d\n", (*hash_addr)->name, (*hash_addr)->amount);
	return 1;
}

int create_inner_account_node(account_ptr *addr, char *name, int amount) //returns 1 for success 0 for failure - this function  adds an account that interacts with a specific account in its account list
{
	while (*addr != NULL)
	{
		if (strcmp((*addr)->acc_name, name) == 0) // already exists
			return 0;
		addr=&((*addr)->next_account);
	}
	*addr=malloc(sizeof(struct account_node));
	strcpy((*addr)->acc_name, name);
	(*addr)->acc_amount=amount;
	(*addr)->next_account=NULL;
	return 1;
}


void delete_account_list(account_ptr* addr) // function to release memory for the list of accounts that an account interacts with
{
	account_ptr templist;
	while ((*addr) != NULL)
	{
		templist = *addr;
		(*addr) = (*addr)->next_account;
		free(templist);
	}
}

void delete_bucket_list(hash_ptr* hash_addr) // function to release memory for all the buckets of a hash_position in the hashtable
{
	hash_ptr templist = *hash_addr;
	hash_ptr h_next;
	while (templist != NULL)
	{
	   h_next = templist->next_overflow_bucket;
	   free(templist);
	   templist = h_next;
	}
	*hash_addr = NULL;
}

void release_structs(hash_ptr* hashtable, int size) // function to release memory for all the structures of the hashtable and the accounts
{
	int i;
	hash_ptr p1;
	for (i=0; i<size; i++)
	{
		p1 = hashtable[i];
		while (p1 != NULL)
		{
			delete_account_list(&(p1->account_list)); 
			p1 = p1->next_overflow_bucket;
		}
		delete_bucket_list(&hashtable[i]);
	}
}

int hash_function(char *name, int size) // A simple hash function that takes a string as input and returns a hash_key that indicates the position in the hashtable
{
	char c;
	int i, k, res = 0, len = strlen(name);
	for (i=0; i<len; i++)
	{
		c=name[i];
		k=c-'0';
		res=res+k;
	}
	return res%size; //returns the mod of the sum of the ASCII decimal values of all the characters of the given string and the size of the hashtable
}


void print_hashtable(hash_ptr* hashtable, int size)
{
	int i;
	hash_ptr p1;
	account_ptr a1;
	for (i=0; i<size; i++)
	{
		p1 = hashtable[i];
		while (p1 != NULL)
		{
			a1 = p1->account_list;
			printf("Bank account of %s has %d euros and interacts with bank accounts of : ", p1->name, p1->amount);
			while (a1 != NULL)
			{
				printf("%s(%d) ", a1->acc_name, a1->acc_amount);
				a1=a1->next_account;
			}
			printf("\n");
			p1=p1->next_overflow_bucket;
		}
	}
}

void transfer_amount(char *source_account, char *target_account, int amount, hash_ptr *hashtable, int size) //function to transfer amounts between bank accounts -- returns 0 for failure - 1 for success
{
	int i, s_pos, t_pos;
	hash_ptr p1, p2;
	s_pos=hash_function(source_account, size);
	t_pos=hash_function(target_account, size);
	p1=hashtable[s_pos];
	p2=hashtable[t_pos];
	while (p1 != NULL) // check if the transfer is possible
	{
		if (strcmp(p1->name, source_account)==0) // check if the transfer operation can be done
		{
			p1->amount=p1->amount-amount;
			break;
		}
		p1=p1->next_overflow_bucket;
	}
	while (p2 != NULL)
	{
		if (strcmp(p2->name, target_account) == 0) // found the target_account
		{
			create_inner_account_node(&(p2->account_list), source_account, amount);
			p2->amount=p2->amount+amount;
		}
		p2=p2->next_overflow_bucket;
	}
}

int check_transfer(char *source_account, int amount, hash_ptr* hashtable, int size)
{
	int i, s_pos;
	hash_ptr p1;
	s_pos=hash_function(source_account, size);
	p1=hashtable[s_pos];
	while (p1 != NULL) // check if the transfer is possible
	{
		if (strcmp(p1->name, source_account)==0) // check if the transfer operation can be done
		{
			if (p1->amount < amount)		
				return 0;
		}
		p1=p1->next_overflow_bucket;
	}
	return 1;
}

int check_account(hash_ptr* hashtable, int size, char *account) // function to check if an account exists, returns 1 if it exists or 0 if it doesn't
{
	int i = hash_function(account,size);
	hash_ptr p1 = hashtable[i];
	while (p1 != NULL)
	{
		if (strcmp(p1->name, account) == 0) //the account exists
			return 1;
		p1=p1->next_overflow_bucket;
	}
	return 0;
}


int account_balance(hash_ptr* hashtable, int size, char* name) // returns the amount of the bank account if successful , else -1
{
	int i;
	i=hash_function(name, size);
	hash_ptr p1;
	p1 = hashtable[i];
	while (p1 != NULL)
	{
		if (strcmp(p1->name, name) == 0)
		{
			//printf("Current amount of %s is %d\n", p1->name, p1->amount);
			return p1->amount;
		}
		p1=p1->next_overflow_bucket;
	}
	return -1;
}

int is_number(char *st) // function to check if a string is a number
{
	int k;
	for (k=0; k<strlen(st); k++)
	{
		 if(st[k] >= '0' && st[k] <= '9')
		 {
			continue; 	
		 }
		 else
		 {
		 	return 0; //is not a number
		 }
	}
	return 1; //is a number
}