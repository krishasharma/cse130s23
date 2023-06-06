# CSE 130 Spring 2023 
## Programming Assignment 4: Multi-Threaded HTTP Server
### Short Description 
	This assignment provides you with experience managing concurrency through synchronization. This project will build a multi-threaded HTTP server. Namely, your server will add a thread-safe queue (Asgn 3) to an HTTP server (Asgn 2) so that the server can serve multiple clients simultaneously. While your server should process multiple clients simultaneously, it must ensure that its responses conform to a coherent and atomic linearization of the client requests. In effect, this means that an outside observer could not differentiate the behavior of your server from a server that uses only a single thread. Your server must create an audit log that identifies the linearization of your server.
### Build 
    	Type "make" on the command line, using the Makefile provided.
### Running 
        To run the program on the command line type ./httpserver [-t threads] <port>
### Errors
	The current code only passes the last test after running "make" followed with ./test_repo.sh 
### Cleaning 
        To clean please type make clean into command line
