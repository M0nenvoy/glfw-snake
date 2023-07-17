#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define COLLECTIONS_INTERNAL
#include "collections.h"

Queue *collections_queue_create(int capacity, int element_size)
{
	Queue *queue = malloc(sizeof(*queue) + capacity * element_size);
	queue->element_size = element_size;
	queue->capacity = capacity;
	queue->size = 0;
	queue->front = 0;
	queue->rear = -1;
	queue->callback_context = NULL;
	return queue;
}

void collections_queue_add(Queue *q, const void *element)
{
	assert(q->size < q->capacity);
	q->rear = (q->rear + 1) % q->capacity;
	q->size += 1;
	memcpy(&q->memory[q->rear * q->element_size], element, q->element_size);
}

void collections_queue_callback_context_set(Queue *q, void *context)
{
	q->callback_context = context;
}

void collections_queue_foreach(Queue *q, void func(void *arg, void *context))
{
	int qoffs = q->front % q->capacity;
	for (int i = 0; i < collections_queue_size(q); ++i) {
		func(&q->memory[qoffs * q->element_size], q->callback_context);
		qoffs = (qoffs + 1) % q->capacity;
	}
}

int collections_queue_size(Queue *q)
{
	return q->size;
}

void collections_queue_peek_last(Queue *q, void *pop)
{
	assert(q->size > 0);
	memcpy(pop, &q->memory[q->rear * q->element_size], q->element_size);
}

void collections_queue_pop_first(Queue *q)
{
	q->front = (q->front + 1) % q->capacity;
	q->size--;
}

void collections_queue_empty(Queue *q)
{
	while (collections_queue_size(q) > 0) {
		collections_queue_pop_first(q);
	}
}

void collections_queue_destroy(Queue *q)
{
	free(q);
}
