# Bank-Server-Client-System
Implementation of a 'networking app' that represents a bank server/client system (Monitor Style - Networking Programming)

-------------------
Bank server program
-------------------
The bank server program is a program that manages a bank. It uses a hashtable(for quick data-access) to hold information for bank accounts. Each account has a list of all the accounts that interacts with and stores the amount of money that is receiving from them. When the bank server runs, it waits for external connections on a port(command line argument). Each connection(e.g socket descriptor that is returned from function accept() when a client is attempting to connect) is placed in a queue(the size of the queue is given from the command line). The placement to the queue is done from the main thread of the bank server which is called administrative/master thread. The bank server, also has a thread pool of workers(the size of the thread pool is given from the command line) and each worker-thread is responsible for receiving commands from the client and sending the commands' results back to the client through the socket. When a worker thread has done its task, it becomes available again, returns to the thread pool size and waits to obtain a new socket descriptor from the queue, therefore, serve a new client. If the queue is empty, the workers threads will wait and if there is a connection available from the queue, one of the workers threads will take over and do its task.

-------------------
Bank client program
-------------------
