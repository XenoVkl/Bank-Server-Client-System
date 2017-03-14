OBJS 	= server.o functions.o
OBJS2   = client.o
SOURCE	= server.c client.c functions.c
HEADER  = functions.h
OUT  	= server client
CC	= gcc
FLAGS   = -g -c 
# -g option enables debugging mode 
# -c flag generates object code for separate files

all: server client

# create/compile the individual files >>separately<< 
server.o: server.c
	$(CC) $(FLAGS) server.c

client.o: client.c
	$(CC) $(FLAGS) client.c

functions.o: functions.c
	$(CC) $(FLAGS) functions.c

server: $(OBJS)
	$(CC) -g $(OBJS) -o server -lpthread
	rm $(OBJS)
client: client.o
	$(CC) -g client.o -o client
	rm client.o
# clean house
clean:
	rm -f $(OBJS) $(OBJS2) $(OUT)
