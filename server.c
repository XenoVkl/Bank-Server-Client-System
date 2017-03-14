#include "functions.h"

/* Wait for all dead child processes */
void sigchld_handler (int sig)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void perror_exit(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}


int main(int argc, char** argv)
{

	int pos, sock_des, newsock_des, port, thread_pool_size, queue_size, entries = 129, k;
	hash_table=malloc(entries*sizeof(hash_ptr)); //create hashtable
	socklen_t clientlen;
	struct sockaddr_in server, client;
    struct sockaddr *serverptr=(struct sockaddr *)&server;
    struct sockaddr *clientptr=(struct sockaddr *)&client;
    struct hostent *rem;

	/* Some error-handling for the arguments */
	if (argc != 7)
	{
		printf("Number of arguments must be 7\n");
		exit(1);
	}
	if ((strcmp(argv[1], "-p") != 0) && (strcmp(argv[1], "-s") != 0) && (strcmp(argv[1], "-q") != 0))
	{
		printf("Flag-arguments error\n");
		exit(1);
	}
	if ((strcmp(argv[2], "-p") == 0) || (strcmp(argv[2], "-s") == 0) && (strcmp(argv[2], "-q") == 0))
	{
		printf("Flag-arguments error\n");
		exit(1);
	}
	if ((strcmp(argv[4], "-p") == 0) || (strcmp(argv[4], "-s") == 0) && (strcmp(argv[4], "-q") == 0))
	{
		printf("Flag-arguments error\n");
		exit(1);
	}
	if ((strcmp(argv[6], "-p") == 0) || (strcmp(argv[6], "-s") == 0) && (strcmp(argv[6], "-q") == 0))
	{
		printf("Flag-arguments error\n");
		exit(1);
	}
    if (strcmp(argv[1], "-p") == 0)
    {
    	port=atoi(argv[2]);
    	if (strcmp(argv[3], "-s") == 0)
    	{
    		thread_pool_size=atoi(argv[4]);
    		queue_size=atoi(argv[6]);
    	}
    	else if (strcmp(argv[5], "-s") == 0)
    	{
    		thread_pool_size=atoi(argv[6]);
    		queue_size=atoi(argv[4]);
    	}
    }
    else if (strcmp(argv[3], "-p") == 0)
    {
    	port=atoi(argv[4]);
    	if (strcmp(argv[1], "-s") == 0)
    	{
    		thread_pool_size=atoi(argv[2]);
    		queue_size=atoi(argv[6]);
    	}
    	else if (strcmp(argv[5], "-s") == 0)
    	{
    		thread_pool_size=atoi(argv[6]);
    		queue_size=atoi(argv[2]);
    	}
    }
    else if (strcmp(argv[5], "-p") == 0)
    {
    	port=atoi(argv[6]);
    	if (strcmp(argv[1], "-s") == 0)
    	{
    		thread_pool_size=atoi(argv[2]);
    		queue_size=atoi(argv[4]);
    	}
    	else if (strcmp(argv[3], "-s") == 0)
    	{
    		thread_pool_size=atoi(argv[4]);
    		queue_size=atoi(argv[2]);
    	}
    }
    printf("port : %d\nWorker threads that are used : %d\nSize of connections' queue : %d\n", port, thread_pool_size, queue_size);
    signal(SIGCHLD, sigchld_handler); 						/* Reap dead children asynchronously */
    if ((sock_des = socket(PF_INET, SOCK_STREAM, 0)) < 0)  /* Create socket */
        perror_exit("socket error");
    server.sin_family = AF_INET;      					 /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);     					 /* The given port */
    if (bind(sock_des, serverptr, sizeof(server)) < 0) 	 /* Bind socket to address */
        perror_exit("bind");
    if (listen(sock_des, 5) < 0)  /* Listen for connections */
    	perror_exit("listen");
    printf("Listening for connections to port %d\n", port);

    /*Worker Thread Creation & initialization condition variables*/
    pthread_t worker[thread_pool_size];
    initialize(&queue, queue_size);
    pthread_mutex_init(&mut, 0);
    pthread_cond_init(&cond_nonempty, 0);
    pthread_cond_init(&cond_nonfull, 0);
    for (k=0; k<thread_pool_size; k++)
        pthread_create(&worker[k], 0, consumer, 0);
    for (k=0; k<129; k++)
    {
        pthread_mutex_init(&mut_array[k], 0);
    }

    while(1)
    {
        clientptr=(struct sockaddr *)&client;
        clientlen=sizeof client;
        //printf("sock_des=%d\n", sock_des);s
        /*Start accepting connections */
        if ((newsock_des = accept(sock_des, clientptr, &clientlen)) < 0) 
        {
            printf("newsock_des=%d\n", newsock_des);
            perror_exit("accept");
        }
        printf("Accepted connection(socket descriptor : %d)\n", newsock_des);
        place(&queue, newsock_des); //place the socket descriptor to the connections' queue
        pthread_cond_signal(&cond_nonempty);
        usleep(0);


        
    }
    for (k=0; k<thread_pool_size; k++)
        pthread_join(worker[k], 0);
    pthread_cond_destroy(&cond_nonempty);
    pthread_cond_destroy(&cond_nonfull);
    pthread_mutex_destroy(&mut);
    for (k=0; k<129; k++)
        pthread_mutex_destroy(&mut_array[k]);
    free(hash_table);
	return 0;
}

