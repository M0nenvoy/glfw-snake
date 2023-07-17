#ifndef COLLECTIONS
#define COLLECTIONS

#ifdef COLLECTIONS_INTERNAL
typedef struct Queue {
	int front, rear, size;
	int capacity;
	int element_size;
	void *callback_context;
	char memory[];
} Queue;
#endif

#ifndef COLLECTIONS_INTERNAL
typedef void Queue;
#endif

/*
 *  Creates a queue that can store up to 'capacity' elements.
 *  Each element must be of size 'element_size' bytes. For example, if you would like to store integers,
 *  you pass 'sizeof(int)' as 'element_size'.
 */
Queue *collections_queue_create(int capacity, int element_size);

/*
 * Adds a new element to the queue.
 * The element should be of size 'element_size' that was specified when the queue was created.
 */
void collections_queue_add(Queue *q, const void *element);

/*
 * Return the number of elements stored in the queue.
 */
int collections_queue_size(Queue *q);

/*
 * Sets the callback context. It is a pointer that can be used by callback functions to store or modify context.
 */
void collections_queue_callback_context_set(Queue *q, void *context);

/*
 * Calls 'func' for each element in a queue.
 * Value of the current element can be received through the first argument of 'func'.
 * Callback context may be accessed via second argument of 'func'.
 */
void collections_queue_foreach(Queue *q, void func(void *arg, void *context));

/*
 * Gets the value of the last element in a queue.
 */
void collections_queue_peek_last(Queue *q, void *peek);

/*
 * Removes the least recent element in a queue.
 */
void collections_queue_pop_first(Queue *q);

/*
 * Removes all the elements in a queue.
 */
void collections_queue_empty(Queue *q);

/*
 * Destroys the queue.
 */
void collections_queue_destroy(Queue *q);

#endif
