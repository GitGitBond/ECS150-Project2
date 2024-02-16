#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

queue_t rq; // Queue for ready threads
queue_t zq; // Queue for terminated threads
struct uthread_tcb *ct; // Pointer to the currently executing thread
struct uthread_tcb *it; // Pointer to the idle thread

// Enum representing thread states
typedef enum state
{
	running, // Thread is currently running
	ready, // Thread is ready to run
	blocked, // Thread is blocked
	zombie // Thread has terminated
} state_t;

struct uthread_tcb
{
	state_t state; // State of the thread
	void *stk; // Pointer to the thread's stack
	uthread_ctx_t *ctx; // Pointer to the thread's context
};

// Function to get the currently executing thread
struct uthread_tcb *uthread_current(void)
{
	return ct;
}

// Function to yield the CPU to the next ready thread
void uthread_yield(void)
{
	// Disable preemption
	preempt_disable();
	struct uthread_tcb *nt;
	struct uthread_tcb *current = uthread_current();

	// Enqueue the current thread to the ready queue
	queue_enqueue(rq, ct);
	// Dequeue the next thread from the ready queue
	queue_dequeue(rq, (void **)&nt);

	if (nt == NULL)
		perror("queue_dequeue");

	// Update states of current and next threads
	ct->state = ready;
	nt->state = running;

	ct = nt;
	// Perform a context switch
	uthread_ctx_switch(current->ctx, nt->ctx);
	// Enable preemption
	preempt_enable();
}

// Function to terminate the currently executing thread
void uthread_exit(void)
{
	ct->state = zombie;
	// Destroy the stack of the terminated thread
	uthread_ctx_destroy_stack(ct->stk);
	// Enqueue the terminated thread to the zombie queue
	queue_enqueue(zq, uthread_current());
	struct uthread_tcb *nt;
	queue_dequeue(rq, (void **)&nt);

	struct uthread_tcb *curr = ct;
	ct = nt;
	ct->state = running;
	uthread_ctx_switch(curr->ctx, nt->ctx);
}

// Function to create a new thread
int uthread_create(uthread_func_t func, void *arg)
{
	struct uthread_tcb *nt = malloc(sizeof(struct uthread_tcb));
	if (nt == NULL)
		return -1;

	// Initialize the new thread
	nt->state = ready;
	nt->stk = uthread_ctx_alloc_stack();

	if (nt->stk == NULL)
		return -1;

	nt->ctx = malloc(sizeof(uthread_ctx_t));
	if (nt->ctx == NULL)
		return -1;

	int ret = uthread_ctx_init(nt->ctx, nt->stk, func, arg);
	if (ret == -1)
		return -1;

	// Enqueue the new thread to the ready queue
	queue_enqueue(rq, nt);
	return 0;
}

// Function to run the threads
int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
	if (preempt)
		preempt_start(preempt);

	// Create the ready and zombie queues
	rq = queue_create();
	zq = queue_create();

	if (rq == NULL || zq == NULL)
		return -1;

	it = malloc(sizeof(struct uthread_tcb));
	if (it == NULL)
		return -1;
	it->state = running;
	it->ctx = malloc(sizeof(uthread_ctx_t));
	if (it->state != running || it->ctx == NULL)
		return -1;

	ct = it;

	// Create the initial thread
	if (uthread_create(func, arg))
		return -1;

	while (1)
	{
		// Check for completed threads
		while (queue_length(zq) > 0)
		{
			// Deallocate the terminated threads
			struct uthread_tcb *et;
			queue_dequeue(zq, (void **)&et);
			free(et);
		}

		// Check if all threads are completed
		if (queue_length(rq) <= 0)
			break;

		uthread_yield();
	}

	if (preempt)
		preempt_stop();

	return 0;
}

// Function to block the currently executing thread
void uthread_block(void)
{
	ct->state = blocked;

	// Get the next thread from the ready queue
	struct uthread_tcb *nt;
	queue_dequeue(rq, (void **)&nt);

	if (nt == NULL)
		perror("queue_dequeue");

	// Prepare the next thread
	nt->state = running;

	// Swap contexts
	struct uthread_tcb *curr = ct;
	ct = nt;
	uthread_ctx_switch(curr->ctx, nt->ctx);
}

// Function to unblock a thread
void uthread_unblock(struct uthread_tcb *uthread)
{
	if (uthread == NULL)
		return;

	// Set the thread to ready and enqueue it to the ready queue
	uthread->state = ready;
	queue_enqueue(rq, uthread);

	// Yield to the next thread
	uthread_yield();
}
