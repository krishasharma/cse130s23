#include "queue.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>

// structs ------------------------------------------------------------------------------------------

typedef struct queue{
	void **buffer; // array to hold the elements
	int size; // maximum size of the buffer
	int count; // number of elements currently in the buffer
	int in; // index to enqueue elements 
	int out; // index to dequeue elements
	pthread_mutex_t mutex; // mutex for thread saftey 
	pthread_cond_t not_full; // condition variable for blocking push
	pthread_cond_t not_empty; // condition variable for blocking pop
} queue_t;

/*struct queue {

}*/

// Constructors-Destructors ------------------------------------------------------------------------

struct queue *queue_new(int size) {
	// the constructor for the queue 
	// queue_new will allocate the required memory and initialize necessary variables for a queue
	// can hold at most size elements 
	queue_t *q = (queue_t *)malloc(sizeof(queue_t));
	q->buffer = (void **)malloc(size * sizeof (void *));
	q->size = size;
	q->count = 0;
	q->in = 0;
	q->out = 0;
	pthread_mutex_init(&q->mutex, NULL); // initialize the mutex 
	pthread_cond_init(&q->not_full, NULL); // initialize the condition variable for blocking push
	pthread_cond_init(&q->not_empty, NULL); // initialize the condition variable for blocking pop
	return q;
}

void queue_delete(queue_t **q) {
	// destructor for the queue 
	// queue_delete will free any allocated memory 
	// sets the passed in pointer to NULL 
	if (q == NULL || *q == NULL) {
		return;
	}
	queue_t *ptr = *q;
	free(ptr->buffer);
	pthread_mutex_destroy(&ptr->mutex); // destroy the mutex
	pthread_cond_destroy(&ptr->not_full); // destroy the condition variable for blocking push 
	pthread_cond_destroy(&ptr->not_empty); // destroy the condition variable for blocking pop
	free(ptr); // free the pointer 
	*q = NULL;
}


// Access functions ---------------------------------------------------------------------------------

bool queue_push(queue_t *q, void *elem){
	if (q == NULL || elem == NULL) {
		return false;
	}
	pthread_mutex_lock(&q->mutex); // acquire the lock on the mutex
	while (q->count == q->size) { // while the buffer is not full 
		pthread_cond_wait(&q->not_full, &q->mutex);
	} 
	q->buffer[q->in] = elem; // enequeue the element 
	q->in = (q->in + 1) % q->size;
	q->count += 1;
	pthread_cond_signal(&q->not_empty); // signal that the buffer is not empty 
	pthread_mutex_unlock(&q->mutex); // release the lock on the mutex 
	return true;	
}

bool queue_pop(queue_t *q, void **elem) {
	if (q == NULL || elem == NULL) {
		return false;
	}
	pthread_mutex_lock(&q->mutex); // acquire the lock on the mutex 
	while (q->count == 0) { // whle the buffer is not empty 
		pthread_cond_wait(&q->not_empty, &q->mutex);
	}
	*elem = q->buffer[q->out]; // dequeue the element 
	q->out = (q->out + 1) % q->size;
	q->count -= 1;
	pthread_cond_signal(&q->not_full); // signal that the buffer is not full
	pthread_mutex_unlock(&q->mutex); // release the lock on the mutex
	return true;	
}


