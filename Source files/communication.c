#include "communication.h"
#include "hashtable.h"

void initialize(queue_t * queue, int size) //Initialize queue that we will store socket descriptors
{
	queue->start = 0;
	queue->end = -1;
	queue->count = 0;
	queue->max_size=size;
	queue->q_array=malloc(size*sizeof(int));
}

void place(queue_t * queue, int sock_des)  // place a socket descript in the queue
{
	pthread_mutex_lock(&mut); // lock mutex
	while (queue->count >= queue->max_size) //wait while queue is full
	{
		printf(">> Queue is Full \n");
		pthread_cond_wait(&cond_nonfull, &mut); //use the condition variable to wait
	}
	queue->end = (queue->end + 1) % queue->max_size;
	queue->q_array[queue->end] = sock_des;
	queue->count++;
	pthread_mutex_unlock(&mut); //unlock mutex
}

int obtain(queue_t * queue) // obtain a socket descriptor from the queue
{
	int item = 0;
	pthread_mutex_lock(&mut);
	while (queue->count <= 0)  // wait if the queue is empty
	{
		printf("Worker thread (%ld) - Waiting to obtain a socket descriptor from the connections' queue\n", pthread_self());
		pthread_cond_wait(&cond_nonempty, &mut); //use the condition variable to wait
	}
	item = queue->q_array[queue->start];
	printf("Worker thread (%ld) obtained connection(socket descriptor %d)\n", pthread_self(), item);

	queue->start = (queue->start + 1) % queue->max_size;
	queue->count--;
	pthread_mutex_unlock(&mut); // unlock mutex
	return item;
}


void *consumer(void * ptr) //thread function - in a few words, this function takes a socket descriptor from the queue and starts reading commands from it
{

	char str[1024], temp_str[1024], ret[1024], temp[11], *s, *token, *t_array[20]; 
	int k, size, c, d, i , pos, newsock_des, msec, entries = 129, j = 0, option = 0, flag = 0, t, *arr, swap;

	for (i=0; i<20; i++)
	    	t_array[i]=malloc(30*sizeof(char));
	while (1) 	//worker thread will run forever, each time trying to obtain a socket descriptor and then start reading from it
	{
		t=0;
		memset(str, 0, sizeof(str));
		memset(temp_str, 0, sizeof(temp_str));
		newsock_des=obtain(&queue);					//take a socket descript from the queue
		//printf("consumer %d\n", newsock_des);
		for (i=0; i<20; i++)
			memset(t_array[i], 0 , sizeof(t_array[i]));
	    while(read(newsock_des, str, sizeof(str)) > 0) 
	    {
	    	j=0;
	    	option=0;
	    	msec=0;
	        //printf("I received the string : %s", str);
	        /*Parse the received message */
	        strcpy(temp_str, str);
	        token = strtok_r(str, " \n", &s);
			while (token != NULL) //parse the string and go to the right case
			{
				if (j == 0)
				{
					if (strcmp(token, "add_account") == 0)
					{
						option = 1;
					}
					else if (strcmp(token, "add_transfer") == 0)
					{
						option = 2;
					}
					else if (strcmp(token, "add_multi_transfer") == 0)
					{
						option = 3;
					}
					else if (strcmp(token, "print_balance") == 0)
					{
						option = 4;
					}
					else if (strcmp(token, "print_multi_balance") == 0)
					{
						option = 5;
					}
					else	//other command(unknown)
					{
						option = 6;
						break;
					}
				}
				else if (j != 0)
				{
					strcpy(t_array[j], token); //store the tokens in an array of strings
				}
				token = strtok_r(NULL, " \n", &s);
				j++;
			}
			switch(option)
			{
				case 1:	//add_account
					if (j>3) //if there is a delay in the command
					{
						pos=hash_function(t_array[2],entries);
						pthread_mutex_lock(&mut_array[pos]); //lock the mutex of a specific hashtable's bucket and enter the critical section
						msec=atoi(t_array[3]); //apply delay to the command
						usleep(msec*1000);
						t=create_hash_node(&hash_table[pos], t_array[2], atoi(t_array[1])); //create the account
						pthread_mutex_unlock(&mut_array[pos]); //unlock the mutex of the bucket
    						if (t == 1) //if the account creation was successful
    						{
							strcpy(ret, "Success. Account creation(");
							strcat(ret, t_array[2]);
							strcat(ret, ":");
							strcat(ret, t_array[1]);
							strcat(ret, "[:");
							strcat(ret, t_array[3]);
							strcat(ret, "])");
							if (write(newsock_des, ret, sizeof(ret)) < 0) //write back to client through the socket
							perror_exit("write");
    						}
						else	//if the account creation failed for some reason
						{
							strcpy(ret, "Error. Account creation failed(");
							strcat(ret, t_array[2]);
							strcat(ret, ":");
							strcat(ret, t_array[1]);
							strcat(ret, "[:");
							strcat(ret, t_array[3]);
							strcat(ret, "])");
							if (write(newsock_des, ret, sizeof(ret)) < 0) //write back to client through the socket
							perror_exit("write");
						}
    					
					}
					else //if there is no delay in the command
					{
						pos=hash_function(t_array[2],entries);
						pthread_mutex_lock(&mut_array[pos]); //lock mutex of the bucket
    						t=create_hash_node(&hash_table[pos], t_array[2], atoi(t_array[1])); //create account
    						pthread_mutex_unlock(&mut_array[pos]); //unlock mutex of the bucket
						if (t == 1) //account creation was successful
						{
							strcpy(ret, "success. Account creation(");
							strcat(ret, t_array[2]);
							strcat(ret, ":");
							strcat(ret, t_array[1]);
							strcat(ret, ")");
							fflush(stdout);
							//printf("I ll send %s from the socket\n", ret);
							if (write(newsock_des, ret, sizeof(ret)) < 0) //write back to client through the socket
							perror_exit("write");
						}
						else //account creation failed
						{
							strcpy(ret, "Error. Account creation failed(");
							strcat(ret, t_array[2]);
							strcat(ret, ":");
							strcat(ret, t_array[1]);
							strcat(ret, ")");
							if (write(newsock_des, ret, sizeof(ret)) < 0) //write back to client through the socket
							perror_exit("write");
						}
    					
					}
					break;
				case 2: //transfer command
					c=hash_function(t_array[2], entries); //get the position of the buckets in the hashtable
					d=hash_function(t_array[2], entries);
					if (c == d) //if they are the same we'll need to lock the mutex only once
						size=1;
					else
						size=2;
					arr=malloc(size*sizeof(int));
					if (size == 2) // if they are not the same , put them in an array and sort it (we'll avoid deadlocks by using that sorting method)
					{
						if (arr[0] > arr[1])
						{
							t=arr[0];
							arr[0]=arr[1];
							arr[1]=t;
						}
					}
					if (j>4)	//if the command has a delay
					{
						for (i=0; i<size; i++)
							pthread_mutex_lock(&mut_array[arr[i]]); //lock the  mutexes of the buckets that invlove those accounts in the right order 
						msec=atoi(t_array[4]); //apply delay
						usleep(msec*1000);
						t=check_account(hash_table, entries, t_array[2]);
						if (t==0) //if the first account of the transfer command does not exist
						{
							strcpy(ret, "Error. Transfer addition failed(");
							strcat(ret, t_array[2]);
							strcat(ret, ":");
							strcat(ret, t_array[3]);
							strcat(ret, ":");
							strcat(ret, t_array[1]);
							strcat(ret, "[:");
							strcat(ret, t_array[4]);
							strcat(ret, "])");
							for (i=0; i<size; i++)	//unlock the mutexes of the buckets that involve those accounts in the right order
									pthread_mutex_unlock(&mut_array[arr[i]]);
							if (write(newsock_des, ret, sizeof(ret)) < 0) //write back to the client through the socket
								perror_exit("write");
	            					free(arr);
							break;
						}
						t=check_account(hash_table, entries, t_array[3]);
						if (t==0) //if the second account of the transfer command does not exist
						{
							strcpy(ret, "Error. Transfer addition failed(");
							strcat(ret, t_array[2]);
							strcat(ret, ":");
							strcat(ret, t_array[3]);
							strcat(ret, ":");
							strcat(ret, t_array[1]);
							strcat(ret, "[:");
							strcat(ret, t_array[4]);
							strcat(ret, "])");
							for (i=0; i<size; i++)	//unlock the mutexes of the buckets that involve those accounts in the right order
								pthread_mutex_unlock(&mut_array[arr[i]]);
							if (write(newsock_des, ret, sizeof(ret)) < 0) //write back to the client through the socket
								perror_exit("write");
							free(arr);
							break;
						}
						t=check_transfer(t_array[2], atoi(t_array[1]), hash_table, entries);
						if (t==0)
						{
							strcpy(ret, "Error. Transfer addition failed(");
							strcat(ret, t_array[2]);
							strcat(ret, ":");
							strcat(ret, t_array[3]);
							strcat(ret, ":");
							strcat(ret, t_array[1]);
							strcat(ret, "[:");
							strcat(ret, t_array[4]);
							strcat(ret, "])");
    							for (i=0; i<size; i++) //unlock the mutexes of the buckets that involve those accounts in the right order
								pthread_mutex_unlock(&mut_array[arr[i]]);
    							if (write(newsock_des, ret, sizeof(ret)) < 0) //write back to the client through the socket
	            						perror_exit("write");
	            					free(arr);
							break;
						}
						transfer_amount(t_array[2], t_array[3], atoi(t_array[1]), hash_table, entries);
						strcpy(ret, "Success. Transfer addition(");
						strcat(ret, t_array[2]);
						strcat(ret, ":");
						strcat(ret, t_array[3]);
						strcat(ret, ":");
						strcat(ret, t_array[1]);
						strcat(ret, "[:");
						strcat(ret, t_array[4]);
						strcat(ret, "])");
						for (i=0; i<size; i++) //unlock the mutexes of the buckets that involve those accounts in the right order
								pthread_mutex_unlock(&mut_array[arr[i]]);
						if (write(newsock_des, ret, sizeof(ret)) < 0) //write back to the client through the socket
            					perror_exit("write");
            					free(arr);
					}
					else //if there is not a delay
					{
						for (i=0; i<size; i++)
								pthread_mutex_lock(&mut_array[arr[i]]);
						t=check_account(hash_table, entries, t_array[2]);
						if (t==0) //account does not exist
						{
							strcpy(ret, "Error. Transfer addition failed(");
							strcat(ret, t_array[2]);
							strcat(ret, ":");
							strcat(ret, t_array[3]);
							strcat(ret, ":");
							strcat(ret, t_array[1]);
							strcat(ret, ")");
							for (i=0; i<size; i++)	//unlock the mutexes of the buckets that involve those accounts in the right order
									pthread_mutex_unlock(&mut_array[arr[i]]);
    							if (write(newsock_des, ret, sizeof(ret)) < 0) //write back to the client through the socket
	            						perror_exit("write");
	            					free(arr);
							break;
						}
						t=check_account(hash_table, entries, t_array[3]);
						if (t==0) //account does not exist
						{
							strcpy(ret, "Error. Transfer addition failed(");
							strcat(ret, t_array[2]);
							strcat(ret, ":");
							strcat(ret, t_array[3]);
							strcat(ret, ":");
							strcat(ret, t_array[1]);
							strcat(ret, ")");
							for (i=0; i<size; i++)
									pthread_mutex_unlock(&mut_array[arr[i]]);
							if (write(newsock_des, ret, sizeof(ret)) < 0)
	            						perror_exit("write");
	            					free(arr);
							break;
						}
						t=check_transfer(t_array[2], atoi(t_array[1]), hash_table, entries);
						if (t==0)
						{
							strcpy(ret, "Error. Transfer addition failed(");
							strcat(ret, t_array[2]);
							strcat(ret, ":");
							strcat(ret, t_array[3]);
							strcat(ret, ":");
							strcat(ret, t_array[1]);
							strcat(ret, ")");
							for (i=0; i<size; i++) //unlock the mutexes of the buckets that involve those accounts in the right order
								pthread_mutex_unlock(&mut_array[arr[i]]);
    							if (write(newsock_des, ret, sizeof(ret)) < 0)  //write back to the client through the socket
	            						perror_exit("write");
	            					free(arr);
							break;
						}
						transfer_amount(t_array[2], t_array[3], atoi(t_array[1]), hash_table, entries);
						strcpy(ret, "Success. Transfer addition(");
						strcat(ret, t_array[2]);
						strcat(ret, ":");
						strcat(ret, t_array[3]);
						strcat(ret, ":");
						strcat(ret, t_array[1]);
						strcat(ret, ")");
						for (i=0; i<size; i++)
								pthread_mutex_unlock(&mut_array[arr[i]]); //unlock the mutexes of the buckets that involve those accounts in the right order
						if (write(newsock_des, ret, sizeof(ret)) < 0)   //write back to the client through the socket
            						perror_exit("write");
            					free(arr);
					}
					break;
				case 3://multitransfer command
					k=is_number(t_array[j-1]); // check if the last string of the command is a number(delay)
					if (k == 1) //if it is, define the right size
					{
						size=j-3;
						arr=malloc(size*sizeof(int)); //create an array that will have the position of the buckets(involved in the transfer command)
						for (i=2; i<j-1; i++)
						{
							arr[i-2]=hash_function(t_array[i], entries);
						}
						
					}
					else //if it is not, define the right size
					{
						size=j-2;
						arr=malloc(size*sizeof(int)); //create an array that will have the position of the buckets(involved in the transfer command)
						for (i=2; i<=j-1; i++)
						{
							arr[i-2]=hash_function(t_array[i], entries);
						}
					}

					//sort array
					for (c = 0 ; c < ( size - 1 ); c++)
					{
						for (d = 0 ; d < size - c - 1; d++)
					    {
					    	if (arr[d] > arr[d+1])
					        {
					        	swap       = arr[d];
					        	arr[d]   = arr[d+1];
					        	arr[d+1] = swap;
					        }
					    }
					}
					for (i=0; i<size; i++)
					{
						if (arr[i] != arr[i+1] && i!=size-1)
						{
							//printf("lock mutex %d\n", arr[i]);
							pthread_mutex_lock(&mut_array[arr[i]]);	//lock mutexes using the sorted array and skip the same mutexes(don't lock twice)
						}
					}
					if (k==1) //apply delay
					{	
						msec=atoi(t_array[j-1]);
						usleep(msec*1000);
					}

					for (i=2; i<j ;i++)
					{
						t=check_account(hash_table, entries, t_array[i]);
						if (t == 0) //does not exist
						{
							if (k == 0) // there is no delay but the account does not exist
							{
								strcpy(ret, "Error. Multi-Transfer addition failed(");
								strcat(ret, t_array[2]);
								strcat(ret, ":");
								strcat(ret, t_array[1]);
								strcat(ret, ")");
								if (write(newsock_des, ret, sizeof(ret)) < 0)
	            					perror_exit("write");
	            				flag=1;
	            				break;
	            			}
							else if (k==1 && i!=(j-1))// there is not a delay since the last string is an account
							{
								strcpy(ret, "Error. Multi-Transfer addition failed(");
								strcat(ret, t_array[2]);
								strcat(ret, ":");
								strcat(ret, t_array[1]);
								strcat(ret, "[:");
								strcat(ret, t_array[j-1]);
								strcat(ret, "])");
								if (write(newsock_des, ret, sizeof(ret)) < 0)
	            					perror_exit("write");
	            				flag=1;
	            				break;
							}
							else if (k==1 && i==(j-1))
								break;
						}
					}
					if (flag == 1) //time to exit the switch-case
					{
						flag=0;
						for (i=0; i<size; i++)
						{
							if (arr[i] != arr[i+1] && i!=size-1)
							{
								//printf("Unlock mutex %d\n", arr[i]);
								pthread_mutex_unlock(&mut_array[arr[i]]);	//unlock mutexes of the right buckets
							}
						}
						free(arr);
						break;

					}
					//store the amount that needs to be transferred in the variable i
					if (k == 1) //if there is a delay
						i=j-4;
					else	// if not
						i=j-3;
					t=check_transfer(t_array[2], i*atoi(t_array[1]), hash_table, entries); //check the transfer
					if (t == 0 && k == 1) // cannot transfer and there is a delay
					{
						strcpy(ret, "Error. Multi-Transfer addition failed(");
						strcat(ret, t_array[2]);
						strcat(ret, ":");
						strcat(ret, t_array[1]);
						strcat(ret, "[:");
						strcat(ret, t_array[j-1]);
						strcat(ret, "])");
						for (i=0; i<size; i++)
						{
							if (arr[i] != arr[i+1] && i!=size-1)
							{
								//printf("Unlock mutex %d\n", arr[i]);
								pthread_mutex_unlock(&mut_array[arr[i]]); //unlock the right mutexes
							}
						}	
						free(arr);
						if (write(newsock_des, ret, sizeof(ret)) < 0)
        					perror_exit("write");
					}
					else if (t== 0 && k == 0) //cannot transfer and there is no delay
					{
						strcpy(ret, "Error. Multi-Transfer addition failed(");
						strcat(ret, t_array[2]);
						strcat(ret, ":");
						strcat(ret, t_array[1]);
						strcat(ret, "])");
						for (i=0; i<size; i++)
						{
							if (arr[i] != arr[i+1] && i!=size-1)
							{
								//printf("Unlock mutex %d\n", arr[i]);
								pthread_mutex_unlock(&mut_array[arr[i]]); //unlock the right mutexes of the buckets
							}
						}
						free(arr);
						if (write(newsock_des, ret, sizeof(ret)) < 0)
        					perror_exit("write");
					}
					else if (t == 1 && k == 0) //account exists and there is no delay
					{
						strcpy(ret, "Success. Multi-Transfer addition(");
						strcat(ret, t_array[2]);
						strcat(ret, ":");
						strcat(ret, t_array[1]);
						strcat(ret, "])");
						for (i=3; i<j; i++)
						{
							transfer_amount(t_array[2], t_array[i], atoi(t_array[1]), hash_table, entries);
						}
						for (i=0; i<size; i++)
						{
							if (arr[i] != arr[i+1] && i!=size-1)
							{
								//printf("Unlock mutex %d\n", arr[i]);
								pthread_mutex_unlock(&mut_array[arr[i]]);	//unlock mutexes
							}
						}
						free(arr);
						if (write(newsock_des, ret, sizeof(ret)) < 0)
        					perror_exit("write");
					}
					else if (t == 1 && k == 1) //account exists and there is a delay
					{
						strcpy(ret, "Success. Multi-Transfer addition(");
						strcat(ret, t_array[2]);
						strcat(ret, ":");
						strcat(ret, t_array[1]);
						strcat(ret, "[:");
						strcat(ret, t_array[j-1]);
						strcat(ret, "])");
						for (i=3; i<j-1; i++)
						{
							transfer_amount(t_array[2], t_array[i], atoi(t_array[1]), hash_table, entries); //time to transfer amounts
						}
						for (i=0; i<size; i++)
						{
							if (arr[i] != arr[i+1] && i!=size-1)
							{
								//printf("Unlock mutex %d\n", arr[i]);
								pthread_mutex_unlock(&mut_array[arr[i]]);
							}
						}
						free(arr);
						if (write(newsock_des, ret, sizeof(ret)) < 0) //write back to client through the socket
        					perror_exit("write");
					}
					break;
				case 4: //balance command
					/*Critical Section */
					pos=hash_function(t_array[1], entries);
					pthread_mutex_lock(&mut_array[pos]); //lock the mutex of the right position of the hashtable(bucket)
					t=check_account(hash_table, entries, t_array[1]);
					if (t==0) //account does not exist
					{
						strcpy(ret, "Error. Balance(");
						strcat(ret, t_array[1]);
						strcat(ret, ")");
						if (write(newsock_des, ret, sizeof(ret)) < 0)	//write back
            				perror_exit("write");
            			pthread_mutex_unlock(&mut_array[pos]); //unlock mutex
            			break;
					}
					t=account_balance(hash_table, entries, t_array[1]); //get amount
					strcpy(ret, "Success. Balance(");
					strcat(ret, t_array[1]);
					strcat(ret, ":");
					sprintf(temp, "%d", t);
					strcat(ret, temp);
					strcat(ret, ")");
					if (write(newsock_des, ret, sizeof(ret)) < 0) //write back
        				perror_exit("write");
        			pthread_mutex_unlock(&mut_array[pos]); //unlock mutex
					break;
				case 5://multibalance command
					size=j-1;
					arr=malloc(size*sizeof(int)); //create an array
					for (i=1; i<j; i++) //store the positions of the accounts in the hashtable(buckets) to the array
					{
						arr[i-1]=hash_function(t_array[i], entries);
					}
					for (c = 0 ; c < ( size - 1 ); c++) //sort the array
					{
						for (d = 0 ; d < size - c - 1; d++)
					    {
					    	if (arr[d] > arr[d+1]) 
					        {
					        	swap       = arr[d];
					        	arr[d]   = arr[d+1];
					        	arr[d+1] = swap;
					        }
					    }
					}
					for (i=0; i<size; i++)
					{
						if (arr[i] != arr[i+1] && i!=size-1) //lock the right mutexes (sorting order) and ignore the same mutexes(we don't want to lock twice)
						{
							//printf("Lock mutex %d\n", arr[i]);
							pthread_mutex_lock(&mut_array[arr[i]]);
						}
					}
						
					for (i=1; i<j; i++)
					{
						t=check_account(hash_table, entries, t_array[i]); 
						if (t==0) //account exists
						{
							strcpy(ret, "Error. Multi-Balance(");
							flag=1;
							break;
						}
					}
					if (flag==1) //return failure
					{
						for (i=1; i<j; i++)
						{
							strcat(ret, t_array[i]);
							if (i != j-1)
								strcat(ret, ":");
						}
						strcat(ret, ")");
						if (write(newsock_des, ret, sizeof(ret)) < 0) //write back to client through the socket
            					perror_exit("write");
						flag=0;
						for (i=0; i<size; i++)
						{
							if (arr[i] != arr[i+1] && i!=size-1)
							{
								//printf("Unlock mutex %d\n", arr[i]);
								pthread_mutex_unlock(&mut_array[arr[i]]); //unlock the right mutexes
							}
						}
						free(arr);
						break;
					}
					strcpy(ret, "Success. Multi-Balance(");
					for (i=1; i<j; i++)
					{
						t=account_balance(hash_table, entries, t_array[i]); //get amount
						strcat(ret, t_array[i]);
						strcat(ret, "/");
						sprintf(temp, "%d", t);
						strcat(ret, temp);
						if (i != j-1)
							strcat(ret, ":");
					}
					strcat(ret, ")");
					if (write(newsock_des, ret, sizeof(ret)) < 0) //write back
        					perror_exit("write");
        			for (i=0; i<size; i++)
					{
						if (arr[i] != arr[i+1] && i!=size-1)
						{
							//printf("Unlock mutex %d\n", arr[i]);
							pthread_mutex_unlock(&mut_array[arr[i]]); //unlock mutexes
						}
					}
					free(arr);
					break;
				case 6: //unknown command
						strcpy(str, "Unknown Command");
	        			if (write(newsock_des, str, sizeof(str)) < 0)
	            			perror_exit("write");
					break;
				default:
					strcpy(str, "success");
	        		if (write(newsock_des, str, sizeof(str)) < 0)
	            		perror_exit("write");
					break;
			}
	        
	    }
	    
		printf("Closing connection(%d)\n", newsock_des);
	    close(newsock_des);   
		pthread_cond_signal(&cond_nonfull);
		usleep(500000);
	}
	for (i=0; i<20; i++)
	    	free(t_array[i]);
	//pthread_exit(0);
}
