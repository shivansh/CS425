# Computer Networks (CS425) - Assignment 1

## Directory Structure
```
├── include
│   └── standard.h
├── Makefile
├── obj
├── README.md
├── serve
│   ├── binary
│   └── file.txt
└── src
    ├── client.c
    └── server.c
```

## Running
To compile and run -
```
make
```
To start the server -
```
./server <port_number>
```
**NOTE:** *port_number* should be greater than 1024.

To start the client -
```
./client username:password@<server_ip> <port_number>
```
The port_number should be the same as that of the server specified above.  
This will led to a prompt asking for the filename. For a valid filename, the client will successfully finish execution and the requested file will be placed in the location from where the client was invoked.  
The files which will be served by the server are located in the directory `serve`.

### Testing
For testing, run the following commands -
```
make
./server 1098
## Invoke a new shell process, and run the following from inside it
./client shivansh:rai@127.0.0.1 1098
## When prompted for a filename, enter either "binary" or "file.txt".
```
For quickly checking the integrity of the transferred files, run -
```
sha1sum <filename> && sha1sum serve/<filename>
```

## Implementation Details
* The server runs in an infinite loop listening for connections.
* The maximum number of connections which can be handled concurrently is set to be **5** (an arbitrary value).
* For each connection, the server forks() a new child process which independently handles communication with the connecting client. 
* If the size of the requested file is more than the main memory size, then it will not be possible to load the entire file at once in RAM. This case is handled by sending the file in chunks of 1024 bytes sequentially.  
**NOTE:** 1024 bytes is just an arbitrary value. To increase the speed of transfers, it can be set to atmost (slightly lesser than) the main memory size. The disadvantage of using a very small buffer size is the increased I/O operations.

## Limitations
* Port collisions
  For avoiding port collisions, the port number 0 can be used which basically handles the responsibility of assigning a free port number to the Operating System.
* For looking up the pair (username, password), a linear search is done resulting in a `O(n)` running time. This can improved by loading the file `users.txt` in memory and creating a hash-set from its values. This will reduce the average search time to `O(1)`.
* The server **might** be vulnerable to buffer overflow attacks, although an attempt has been made to prevent them.
