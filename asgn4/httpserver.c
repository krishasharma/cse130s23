#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_REQUEST_SIZE 1024
#define MAX_THREADS 4

typedef struct {
    int fd;
} Listener_Socket;

typedef struct {
    char oper[5];
    char uri[256];
    int status_code;
    int request_id;
} Audit_Log;

typedef struct {
    int client_socket;
    sem_t* log_semaphore;
} Worker_Args;

sem_t log_semaphore;
sem_t queue_semaphore;
sem_t worker_semaphore;
sem_t dispatcher_semaphore;
sem_t worker_exit_semaphore;
pthread_mutex_t queue_mutex;
pthread_mutex_t log_mutex;
pthread_mutex_t worker_count_mutex;
pthread_t worker_threads[MAX_THREADS];
int worker_count = 0;
Audit_Log audit_log[MAX_REQUEST_SIZE];
int audit_log_count = 0;

void* worker_thread_function(void* args);
void sigterm_handler(int signal);

int listener_init(Listener_Socket* sock, int port) {
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((sock->fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(sock->fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return -1;
    }

    if (listen(sock->fd, 3) < 0) {
        perror("listen failed");
        return -1;
    }

    return 0;
}

int listener_accept(Listener_Socket* sock) {
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    int client_socket = accept(sock->fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (client_socket < 0) {
        perror("accept failed");
        return -1;
    }

    return client_socket;
}

ssize_t read_until(int in, char buf[], size_t nbytes, char* string) {
    ssize_t total_bytes_read = 0;
    ssize_t bytes_read = 0;
    char* ptr = buf;

    while (total_bytes_read < nbytes) {
        bytes_read = read(in, ptr, 1);
        if (bytes_read <= 0) {
            if (errno == EINTR)
                continue;
            else
                return -1;
        }

        total_bytes_read++;
        if (strstr(buf, string) != NULL)
            return total_bytes_read;

        ptr++;
    }

    return total_bytes_read;
}

ssize_t write_all(int out, char buf[], size_t nbytes) {
    ssize_t total_bytes_written = 0;
    ssize_t bytes_written = 0;
    char* ptr = buf;

    while (total_bytes_written < nbytes) {
        bytes_written = write(out, ptr, nbytes - total_bytes_written);
        if (bytes_written <= 0) {
            if (errno == EINTR)
                continue;
            else
                return -1;
        }

        total_bytes_written += bytes_written;
        ptr += bytes_written;
    }

    return total_bytes_written;
}

ssize_t pass_bytes(int src, int dst, size_t nbytes) {
    char buf[MAX_REQUEST_SIZE];
    ssize_t total_bytes_passed = 0;
    ssize_t bytes_read = 0;
    ssize_t bytes_written = 0;

    while (total_bytes_passed < nbytes) {
        bytes_read = read(src, buf, sizeof(buf));
        if (bytes_read <= 0) {
            if (errno == EINTR)
                continue;
            else
                return -1;
        }

        bytes_written = write_all(dst, buf, bytes_read);
        if (bytes_written == -1)
            return -1;

        total_bytes_passed += bytes_written;
    }

    return total_bytes_passed;
}

void* worker_thread_function(void* args) {
    Worker_Args* worker_args = (Worker_Args*) args;
    int client_socket = worker_args->client_socket;

    // Read and process the request from the client
    char request[MAX_REQUEST_SIZE];
    ssize_t bytes_read = read_until(client_socket, request, sizeof(request), "\r\n\r\n");
    if (bytes_read <= 0) {
        // Error reading request
        close(client_socket);
        sem_post(worker_args->log_semaphore);
        sem_post(&worker_semaphore);
        return NULL;
    }

    // Process the request and generate the response
    char response[MAX_REQUEST_SIZE];
    // Implement request processing and response generation logic here
    // ...

    // Write the response to the client
    ssize_t bytes_written = write_all(client_socket, response, strlen(response));
    if (bytes_written <= 0) {
        // Error writing response
        close(client_socket);
        sem_post(worker_args->log_semaphore);
        sem_post(&worker_semaphore);
        return NULL;
    }

    // Add an entry to the audit log
    Audit_Log log_entry;
    strcpy(log_entry.oper, "PUT");
    strcpy(log_entry.uri, "/example");
    log_entry.status_code = 200;
    log_entry.request_id = worker_count;

    // Lock the audit log mutex
    pthread_mutex_lock(&log_mutex);
    audit_log[audit_log_count++] = log_entry;
    pthread_mutex_unlock(&log_mutex);

    // Cleanup and close the client socket
    close(client_socket);

    sem_post(worker_args->log_semaphore);
    sem_post(&worker_semaphore);
    return NULL;
}

void sigterm_handler(int signal) {
    // Cleanup and exit gracefully
    for (int i = 0; i < worker_count; i++) {
        sem_post(&worker_exit_semaphore);
    }

    for (int i = 0; i < worker_count; i++) {
        pthread_join(worker_threads[i], NULL);
    }

    sem_destroy(&queue_semaphore);
    sem_destroy(&worker_semaphore);
    sem_destroy(&dispatcher_semaphore);
    sem_destroy(&worker_exit_semaphore);
    pthread_mutex_destroy(&queue_mutex);
    pthread_mutex_destroy(&log_mutex);
    pthread_mutex_destroy(&worker_count_mutex);

    exit(0);
}

int main(int argc, char* argv[]) {
    int port = 8080;
    int num_threads = MAX_THREADS;

    if (argc >= 2) {
        port = atoi(argv[1]);
    }

    if (argc >= 3) {
        num_threads = atoi(argv[2]);
        if (num_threads < 1 || num_threads > MAX_THREADS) {
            fprintf(stderr, "Invalid number of threads. Defaulting to %d.\n", MAX_THREADS);
            num_threads = MAX_THREADS;
        }
    }

    // Initialize the listener socket
    Listener_Socket listener_socket;
    if (listener_init(&listener_socket, port) == -1) {
        perror("Failed to initialize listener socket");
        return 1;
    }

    // Initialize semaphores and mutexes
    sem_init(&log_semaphore, 0, 1);
    sem_init(&queue_semaphore, 0, 0);
    sem_init(&worker_semaphore, 0, num_threads);
    sem_init(&dispatcher_semaphore, 0, 1);
    sem_init(&worker_exit_semaphore, 0, 0);
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_mutex_init(&log_mutex, NULL);
    pthread_mutex_init(&worker_count_mutex, NULL);

    // Register SIGTERM signal handler
    signal(SIGTERM, sigterm_handler);

    // Start worker threads
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&worker_threads[i], NULL, worker_thread_function, NULL);
    }

    while (1) {
        // Accept a client connection
        int client_socket = listener_accept(&listener_socket);
        if (client_socket == -1) {
            perror("Failed to accept client connection");
            continue;
        }

        // Check if worker threads are available
        sem_wait(&worker_semaphore);

        // Create worker thread arguments
        Worker_Args worker_args;
        worker_args.client_socket = client_socket;
        worker_args.log_semaphore = &log_semaphore;

        // Lock the queue mutex
        pthread_mutex_lock(&queue_mutex);

        // Add the client socket to the queue
        // ...

        // Unlock the queue mutex
        pthread_mutex_unlock(&queue_mutex);

        // Signal the dispatcher thread
        sem_post(&queue_semaphore);
    }

    return 0;
}
