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


void perror_exit(char *message) 
{
    perror(message);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	int port, sock_des, i, n, msec;
	char command_file[256], hostname[64], *s, temp_str[1024], str[1024], *token;
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*)&server;
    struct hostent *rem;
    FILE *ifp;
	/* Some error-handling for the arguments */
	if (argc != 7)
	{
		printf("Number of arguments must be 7\n");
		exit(1);
	}
	if ((strcmp(argv[1], "-p") != 0) && (strcmp(argv[1], "-h") != 0) && (strcmp(argv[1], "-i") != 0))
	{
		printf("Flag-arguments error\n");
		exit(1);
	}
	if ((strcmp(argv[2], "-p") == 0) || (strcmp(argv[2], "-h") == 0) || (strcmp(argv[2], "-i") == 0))
	{
		printf("Flag-arguments error\n");
		exit(1);
	}
	if ((strcmp(argv[4], "-p") == 0) || (strcmp(argv[4], "-h") == 0) || (strcmp(argv[4], "-i") == 0))
	{
		printf("Flag-arguments error\n");
		exit(1);
	}
	if ((strcmp(argv[6], "-p") == 0) || (strcmp(argv[6], "-h") == 0) || (strcmp(argv[6], "-i") == 0))
	{
		printf("Flag-arguments error\n");
		exit(1);
	}
	/* ---- */

//some more error handling techinques 
	if (strcmp(argv[1], "-p") == 0)
    {
    	port=atoi(argv[2]);
    	if (strcmp(argv[3], "-h") == 0)
    	{
    		strcpy(hostname, argv[4]);
    		strcpy(command_file, argv[6]);
    	}
    	else if (strcmp(argv[5], "-h") == 0)
    	{
    		strcpy(hostname, argv[6]);
    		strcpy(command_file, argv[4]);
    	}
    }
    else if (strcmp(argv[3], "-p") == 0)
    {
    	port=atoi(argv[4]);
    	if (strcmp(argv[1], "-h") == 0)
    	{
    		strcpy(hostname, argv[2]);
    		strcpy(command_file, argv[6]);
    	}
    	else if (strcmp(argv[5], "-h") == 0)
    	{
    		strcpy(hostname, argv[6]);
    		strcpy(command_file, argv[2]);
    	}
    }
    else if (strcmp(argv[5], "-p") == 0)
    {
    	port=atoi(argv[6]);
    	if (strcmp(argv[1], "-h") == 0)
    	{
    		strcpy(hostname, argv[2]);
    		strcpy(command_file, argv[4]);
    	}
    	else if (strcmp(argv[3], "-h") == 0)
    	{
    		strcpy(hostname, argv[4]);
    		strcpy(command_file, argv[2]);
    	}
    }
    //printf("port %d hostname %s command_file %s\n", port, hostname, command_file);

    /* Create socket */
    if ((sock_des = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    	perror_exit("socket error");
    //printf("sock_des=%d\n", sock_des);
	/* Find server address */
    if ((rem = gethostbyname(hostname)) == NULL) {	
	   herror("gethostbyname error"); exit(1);
    }
    server.sin_family = AF_INET;       /* Internet domain */
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port);         /* Server port */
    if (connect(sock_des, serverptr, sizeof(server)) < 0)
		perror_exit("connect error");
    printf("Connecting to %s port %d\n", hostname, port);

    /*Start writing to socket by reading commands from the file */

    if ((ifp=fopen(command_file, "rb")) == NULL) //open command file
    {
        perror("fopen-source file");
        exit(0);
    }
    while(1) //read from the file in this loop line by line till EOF
    {
        if (fgets(str, sizeof(str), ifp) == NULL)
            break;
        //s=str;
        strcpy(temp_str, str); // some error-message handling again
        token = strtok_r(temp_str, " \n", &s);
        if (strcmp(str, "exit\n") == 0) //exit command for client
        {
            close(sock_des);
            return 0;
        }
        if (strcmp(token, "sleep") == 0) // sleep command for client
        {
            token = strtok_r(NULL, " \n", &s);
            msec=atoi(token);
            //printf("client sleeping for %d msec\n", msec);
            usleep(msec*1000);
            //printf("client woke up\n");
            continue;
        }
        if (write(sock_des, str, sizeof(str)) < 0)
               perror_exit("write");
        if (read(sock_des, str, sizeof(str)) < 0)
                perror_exit("read"); 
        printf("%s\n", str);
    } 
    fclose(ifp);

    /*Start writing to socket by reading commands from the stdin*/
    while(1)
    {
    	if (fgets(str, sizeof(str), stdin) == NULL)
            break;
        s=str;
        strcpy(temp_str, str); // some error-message handling again
        token = strtok_r(temp_str, " \n", &s);
        if (strcmp(str, "exit\n") == 0) //exit command for client
        {
            close(sock_des);
            return 0;
        }
        if (strcmp(token, "sleep") == 0) // sleep command for client
        {
            token = strtok_r(NULL, " \n", &s);
            msec=atoi(token);
            usleep(msec*1000);
            continue;
        }
    	if (write(sock_des, str, sizeof(str)) < 0)
        	   perror_exit("write");
        if (read(sock_des, str, sizeof(str)) < 0)
        	    perror_exit("read");        
        printf("%s\n", str);
    }
    close(sock_des);
}