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

#define BUFSIZE 1024
#define PORT 8080

int handle_request(int connfd);


int main(int argc, char *argv[]){
    int sockfd, connfd, portno;
    socklen_t clilen;
    struct sockaddr_in, serv_addr, cli_addr;
    // if no port was given, throw error
    // if the port passed to httpserver is invalid (not an integer between 1 and 65535)
    // or if httpserver cannot bind to the provided port, then your httpserver should
    // produce the message “‘Invalid Port\n”’ to stderr and exit with a return code of 1.
    if (argc < 2) {
        fprintf(stderr,"Invalid Port\n"); // no port provided
        exit(1);
    }

    // create a socket STEP 2
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // if the socket can not be opened, throw error
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    // setting up the address structure
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // if the port passed to httpserver is invalid (not an integer between 1 and 65535)
    // or if httpserver cannot bind to the provided port, then your httpserver should
    // produce the message “‘Invalid Port\n”’ to stderr and exit with a return code of 1
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Invalid Port\n"); // could not bind
        exit(1);
    } else {
        // bind the socket to the address
        bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr);
    }

    // listen for incomming connections
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while (1) {
        // accept incomming connections
        connfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (connfd < 0) {
            perror("ERROR on accept");
            continue;
        }
        if (handle_request(connfd) < 0) {
            perror("ERROR handling request");
        }
        close(connfd);
    }

    close(sockfd);
    return 0;
}



int handle_request(int connfd){
    char buf[BUFSIZE];
    char method[BUFSIZE], uri[BUFSIZE], version[BUFSIZE];
    char filepath[BUFSIZE];
    struct stat st;
    int fd, n;

    n = read(connfd, buf, BUFSIZE);
    if (n < 0) {
        perror("ERROR reading from socket");
        return -1;
    }
    sscanf(buf, "%s %s %s", method, uri, version);

    if (strcmp(method, "GET") == 0) {
        sprintf(filepath, ".%s", uri);
        if (stat(filepath, &st) < 0) {
            snprintf(buf, BUFSIZE, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
            write(connfd, buf, strlen(buf));
            return 0;
        }
        if ((fd = open(filepath, O_RDONLY)) < 0) {
            perror("ERROR opening file");
            return -1;
        }
        snprintf(buf, BUFSIZE, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\n\r\n", st.st_size);
        write(connfd, buf, strlen(buf));
        while ((n = read(fd, buf, BUFSIZE)) > 0) {
            write(connfd, buf, n);
        }
        close(fd);
    }
    else if (strcmp(method, "PUT") == 0) {
        sprintf(filepath, ".%s", uri);
        if ((fd = open(filepath, O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0) {
            perror("ERROR opening file");
            return -1;
        }
        while ((n = read(connfd, buf, BUFSIZE)) > 0) {
            write(fd, buf, n);
        }
        // closing connection,
        close(fd);
    }
}
