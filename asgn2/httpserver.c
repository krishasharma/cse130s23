#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>

#include "asgn2_helper_funcs.h"
#include "asgn2_helper_funcs.a"

#define BUFSIZE 1024
#define port 8080
#define SA struct sockaddr 
#define REQUEST_SIZE 2048
#define URI_LEN 64 


// initializiations ------------------------------------------------------------------------

Listener_Socket* server_socket;
Listener_Socket* client_socket;

// helper funcs ----------------------------------------------------------------------

/*
// driver funcs -----------------------------------------------------------------------

int main(int argc, char **argv) {
	int portnum;

	// check if the port is valid
	if (argv[1]) {
		portnum = atoi(argv[1]); // parse the command line argument to get the port number
	}
	
	// if no port was given, throw error
        // if the port passed to httpserver is invalid (not an integer between 1 and 65535)
        // or if httpserver cannot bind to the provided port, then your httpserver should
        // produce the message “‘Invalid Port\n”’ to stderr and exit with a return code of 1.
	if (portnum < 0 || portnum > 65535 || !argv[1]) {  
		fprintf(stderr, "Invalid Port\n"); // no port provided
		exit(1); // exit with a return code of negative one
	}
	
	int server_sockfd = listener_init(server_socket, portnum); // listen into the server_socket

	// bind the socket to the port
	bind(server_sockfd, (SA *) &server_socket, sizeof(server_socket));

	// accept connections made by clients to the port
	// get the client socket  
	int client_sockfd = listener_accept(client_socket);

	handle_client(client_sockfd);
	
	close(client_sockfd);
	close(server_sockfd);
}
*/

int handle_client(int client_sockfd) {
	int read = 0;
        int write = 0;

	char request[REQUEST_SIZE];
	char method[9];
	char uri[65];
	char version[9]; 

	// receive the request from the client 
	read = read_until(client_sockfd, request, sizeof(request) - 1, 0);
	if (read == -1) {
		fprintf(stderr, "error reciving request");
		return 1; 
	}
	request[read] = '\0'; // at the end of the read request for the bytes stored
				    // put a NULL character
	// parse the request line 
	// parse the request and extract method, uri, and version 
	char* token = strtok(request, " \r\n");
	if (token == NULL) {
		// if there is an invalid request format 
		// send an appropriate response back to the client 
		char response[29] = "HTTP/1.1 400 Bad Request\r\n\r\n";
		write = write_all(client_sockfd, response, strlen(response));
		if (write == -1) {
			fprintf(stderr, "write invalid\n");
			return 1;
		}
		return 0; 
	} 
	strncpy(method, token, sizeof(method) - 1); // copy the size of method number of bytes 
						    // from token to method 
	method[sizeof(method) - 1] = '\0'; // at the end of the method request and after reading 
					   // the bytes put a NULL character

	token = strtok(NULL, " \r\n");
	if (token == NULL) {
		// if there is an invalid request format 
                // send an appropriate response back to the client 
                char response[29] = "HTTP/1.1 400 Bad Request\r\n\r\n";
		write = write_all(client_sockfd, response, strlen(response));
                if (write == -1) {
                        fprintf(stderr, "write invalid\n");
                        return 1;
                }
		return 0;
	} 
	strncpy(uri, token, sizeof(uri) - 1);
	uri[sizeof(uri) - 1] = '\0';

	token = strtok(NULL, " \r\n");
	if (token == NULL) {
		// if there is an invalid request format 
                // send an appropriate response back to the client 
                char response[29] = "HTTP/1.1 400 Bad Request\r\n\r\n";
                write = write_all(client_sockfd, response, strlen(response));
                if (write == -1) {
                        fprintf(stderr, "write invalide\n");
                        return 1;
                }
		return 0;
	}
	strncpy(version, token, sizeof(version) - 1);
	version[sizeof(version) - 1] = '\0';

	// check if the request is valid 
	// handle get and put requests 
	if (strcmp(method, "GET") != 0 && strcmp(method, "PUT") != 0) {
		// invalid method 
		// send an appropriate response back to the client 
		char response[29] = "HTTP/1.1 400 Bad Request\r\n\r\n";
		write = write_all(client_sockfd, response, strlen(response));
		if (write == -1) {
                        fprintf(stderr, "write invalide\n");
                        return 1;
                }
                return 0;
	}

	if (strcmp(version, "HTTP/1.1") != 0) {
		// invalid version 
		// send an appropriate response back to the client 
		char response[44] = "HTTP/1.1 505 HTTP Version Not Supported\r\n\r\n";
		write = write_all(client_sockfd, response, strlen(response));
		if (write == -1) {
                        fprintf(stderr, "write invalide\n");
                        return 1;
                }
                return 0;
	}

	// process the request based on the method and uri 
	if (strcmp(method, "GET") == 0) {
		// handle GET request 
		// open and read the file specified by the uri 
		// send the file content as the response body 
		int file_fd = open(uri + 1, O_RDONLY);
		if (file_fd == -1) {
			// if the file is not found
			// send a 404 not found respnse back to the client
			char response[29] = "HTTP/1.1 404 Not Found\r\n\r\n";
			write = write_all(client_sockfd, response, strlen(response)); 
			if (write == -1) {
                        	fprintf(stderr, "write invalide\n");
                        	return 1;
                	}
                	return 0;
		}
		
		// read the file and send it as the response body 
		char response[REQUEST_SIZE]; 
		while ((read = read_until(file_fd, response, sizeof(response), 0)) > 0) {
			write = write_all(client_sockfd, response, read);
		}
		close(file_fd);
	} 
	else if (strcmp(method, "PUT") == 0) {
		// handle PUT request 
		int file_fd = open(uri + 1, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (file_fd == -1) {
			// error creating or opening file 
			// send 500 internal server error response back to the client 
			char response[39] = "HTTP/1.1 500 Internal Server Error\r\n\r\n";
			write = write_all(client_sockfd, response, strlen(response));
			if (write == -1) {
                                fprintf(stderr, "write invalide\n");
                                return 1;
                        }
                        return 0;
		}

		// recive the message body and write it to the file 
		token = strtok(NULL, "\r\n");
		while (token != NULL) {
			write = write_all(file_fd, token, strlen(token));
			write = write_all(file_fd, "\n", 1);
			token = strtok(NULL, "\r\n");

		}
		// send a 200 ok response back to the client 
		char response[20] = "HTTP/1.1 200 OK\r\n\r\n";
		write = write_all(client_sockfd, response, strlen(response));

		close(file_fd);
	}
	return 0;
}


// driver funcs -----------------------------------------------------------------------

int main(int argc, char **argv) {
        int portnum;
	
	if (argc < 1) {
		fprintf(stderr, "Error, no port specified\n");
		return 1;
	}

        // check if the port is valid
        portnum = atoi(argv[1]); // parse the command line argument to get the port number

        // if no port was given, throw error
        // if the port passed to httpserver is invalid (not an integer between 1 and 65535)
        // or if httpserver cannot bind to the provided port, then your httpserver should
        // produce the message “‘Invalid Port\n”’ to stderr and exit with a return code of 1.
        if (portnum < 0 || portnum > 65535 || !argv[1]) {
                fprintf(stderr, "Invalid Port\n"); // no port provided
                exit(1); // exit with a return code of negative one
        }

        int server_sockfd = listener_init(server_socket, portnum); // listen into the server_socket

        // bind the socket to the port
        bind(server_sockfd, (SA *) &server_socket, sizeof(server_socket));

        // accept connections made by clients to the port
        // get the client socket  
        int client_sockfd = listener_accept(client_socket);

        handle_client(client_sockfd);

        close(client_sockfd);
        close(server_sockfd);
}


















