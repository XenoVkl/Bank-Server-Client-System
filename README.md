# Bank-Server-Client-System
Implementation of a 'networking app' that represents a bank server/client system (Monitor Style - Networking Programming)

-------------------
Bank server program
-------------------
The bank server program is a program that manages a bank. It uses a hashtable(for quick data-access) to hold information for bank accounts. Each account has a list of all the accounts that interacts with and stores the amount of money that is receiving from them. When the bank server runs, it waits for external connections on a port(command line argument). Each connection(e.g socket descriptor that is returned from function accept() when a client is attempting to connect) is placed in a queue(the size of the queue is given from the command line). The placement to the queue is done from the main thread of the bank server which is called administrative/master thread. The bank server, also has a thread pool of workers(the size of the thread pool is given from the command line) and each worker-thread is responsible for receiving commands from the client and sending the commands' results back to the client through the socket. When a worker thread has done its task, it becomes available again, returns to the thread pool size and waits to obtain a new socket descriptor from the queue, therefore, serve a new client. If the queue is empty, the workers threads will wait and if there is a connection available from the queue, one of the workers threads will take over and do its task.

-------------------
Bank client program
-------------------
The bank client is connected with the bank server through an IP address and a port(both command line arguments). Then the bank client reads commands from a datafile or the stdin and prints the results that receives from the server(stdout).

Commands for the bank client:

- add_account [init_amount] [account_name] [delay]

  Request for a bank account creation with an initial amount of money

- add_transfer [amount] [src_name] [dest_name] [delay]

  Request for a ransaction between two bank accounts
  
- add_multi_transfer [amount] [src_name] [dst_name1] ... [dst_nameX] [delay]
 
  Request for multiple transactions between one bank account and others
  
- print_balance [name]
  
  Request to print account's balance
  
- print_multi_balance [name1] ... [nameX]

  Request to print the balance of many accounts
  
- sleep [time]

  The bank-client will wait X msecs before sending the next command to the bank server
  
- exit

  The program bank-client will be terminated
  

----------------------------------------------------------------------
Implementing the system using Networking and Monitor style programming 
----------------------------------------------------------------------
  
  

