#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <netinet/in.h>

#define MAX_REQUEST_LEN 1024
#define MAX_URI_LEN     256
#define MAX_LOG_LEN     512
#define DEFAULT_THREADS 4

typedef struct {
    int client_socket;
    char request[MAX_REQUEST_LEN];
} Request;

typedef struct {
    char operation[5];
    char uri[MAX_URI_LEN];
    int status_code;
    char request_id[10];
} LogEntry;

pthread_mutex_t log_mutex;
pthread_mutex_t queue_mutex;
pthread_cond_t queue_empty_cond;

sem_t thread_semaphore;

int num_threads;
int num_active_threads;
int num_requests;
int current_request_id;

Request *request_queue;
LogEntry *audit_log;

void log_entry(const char *operation, const char *uri, int status_code, const char *request_id) {
    pthread_mutex_lock(&log_mutex);

    LogEntry entry;
    strcpy(entry.operation, operation);
    strcpy(entry.uri, uri);
    entry.status_code = status_code;
    strcpy(entry.request_id, request_id);

    int log_index = num_requests++;
    audit_log[log_index] = entry;

    fprintf(
        stderr, "%s,%s,%d,%s\n", entry.operation, entry.uri, entry.status_code, entry.request_id);

    pthread_mutex_unlock(&log_mutex);
}

void process_request(Request *request) {
    // Process the request and generate a response
    // For simplicity, let's assume a simple echo server

    char response[MAX_REQUEST_LEN];
    strcpy(response, request->request);

    // Send the response back to the client
    write(request->client_socket, response, strlen(response));

    // Log the request
    log_entry("PUT", request->request, 200, request->request);
}

void *worker_thread(void *arg) {
    (void) arg; // Cast arg
    while (1) {
        sem_wait(&thread_semaphore);
        pthread_mutex_lock(&queue_mutex);

        Request *request = &request_queue[num_requests - 1];
        num_active_threads++;

        pthread_mutex_unlock(&queue_mutex);

        process_request(request);

        pthread_mutex_lock(&queue_mutex);

        num_active_threads--;
        if (num_active_threads == 0) {
            pthread_cond_signal(&queue_empty_cond);
        }

        pthread_mutex_unlock(&queue_mutex);
    }

    return NULL;
}

void *dispatcher_thread(void *arg) {
    int server_socket = *(int *) arg;
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);

    while (1) {
        int client_socket
            = accept(server_socket, (struct sockaddr *) &client_address, &client_address_len);

        pthread_mutex_lock(&queue_mutex);

        Request *request = &request_queue[num_requests++];
        request->client_socket = client_socket;

        pthread_mutex_unlock(&queue_mutex);

        sem_post(&thread_semaphore);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: ./httpserver [-t threads] <port>\n");
        return 1;
    }

    int port;
    int opt;
    num_threads = DEFAULT_THREADS;

    while ((opt = getopt(argc, argv, "t:")) != -1) {
        switch (opt) {
        case 't': num_threads = atoi(optarg); break;
        default: fprintf(stderr, "Usage: ./httpserver [-t threads] <port>\n"); return 1;
        }
    }

    port = atoi(argv[optind]);

    request_queue = (Request *) malloc(sizeof(Request) * MAX_REQUEST_LEN);
    audit_log = (LogEntry *) malloc(sizeof(LogEntry) * MAX_REQUEST_LEN);

    pthread_mutex_init(&log_mutex, NULL);
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_cond_init(&queue_empty_cond, NULL);

    sem_init(&thread_semaphore, 0, 0);

    pthread_t *worker_threads = (pthread_t *) malloc(sizeof(pthread_t) * num_threads);

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) == -1) {
        perror("bind");
        return 1;
    }

    if (listen(server_socket, num_threads) == -1) {
        perror("listen");
        return 1;
    }

    pthread_create(&worker_threads[0], NULL, dispatcher_thread, &server_socket);
    for (int i = 1; i <= num_threads; i++) {
        pthread_create(&worker_threads[i], NULL, worker_thread, NULL);
    }

    for (int i = 0; i <= num_threads; i++) {
        pthread_join(worker_threads[i], NULL);
    }

    free(worker_threads);
    free(request_queue);
    free(audit_log);

    return 0;
}
