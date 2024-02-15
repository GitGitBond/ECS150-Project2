#include <assert.h>
#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"



typedef unsigned short uthread_t;
// All possible thread states.
#define READY 0
#define RUNNING 1
#define BLOCKED 2
#define ZOMBIE 3

// Queued threads to be dealt with.
static queue_t ready_processes;
static queue_t zombie_processes;
static queue_t blocked_processes;

static struct uthread_tcb* running;
// Initial conditions: preemption disabled, no instantiated threads.
static int preempt_required = 0;
static uthread_t num_processes = 0;

struct uthread_tcb {
	uthread_t tid;
	int tid_blocking;
	uthread_ctx_t* context;
	int state;
	void* stack;
	int already_joined;
};

// void iterate_uthread_tcb(queue_t queue, void* data) {
// 	struct uthread_tcb* tcb = (struct uthread_tcb*)data;
// 	printf("tid:%d tid_blocking:%d state:%d", tcb->tid, tcb->tid_blocking, tcb->state);
// }

static int manage_thread_library(struct uthread_tcb** myThread, int is_main) {
	
	*myThread = (struct uthread_tcb*)malloc(sizeof(struct uthread_tcb));
	// If failure, send error.
	if (!*myThread) {
		if (is_main) {
			// Free unused queues.
			queue_destroy(zombie_processes);
			queue_destroy(ready_processes);
		}

		return EXIT_FAILURE;
	}

	(*myThread)->context = (ucontext_t*)malloc(sizeof(ucontext_t));
	if (!(*myThread)->context) {
		if (is_main) {
			queue_destroy(ready_processes);
			queue_destroy(zombie_processes);
		}
		free(myThread);

		return EXIT_FAILURE;
	}

	// Adequately describe the newly created thread.
	(*myThread)->tid = num_processes;
	(*myThread)->state = is_main ? RUNNING : READY;
	(*myThread)->stack = NULL;
	(*myThread)->tid_blocking = -1;

	return EXIT_SUCCESS;
}

struct uthread_tcb *uthread_current(void)
{
	return running;
}

void uthread_yield(void) {
	preempt_disable();
	// Add running process back to the ready queue.
	if (running->state == RUNNING) {
		running->state = READY;
		queue_enqueue(ready_processes, (void*)running);
	} else if (running->state == BLOCKED) {
		queue_enqueue(blocked_processes, (void*)running);
	} 

	// Run the next ready process.
	struct uthread_tcb* current_process = running;
	queue_dequeue(ready_processes, (void**)&running);
	running->state = RUNNING;
	// Call context switch anytime we wish to switch processes.
	uthread_ctx_switch(current_process->context, running->context);
	preempt_enable();
}

void uthread_exit(void) {
	preempt_disable();
	running->state = ZOMBIE;
	queue_enqueue(zombie_processes, (void*)running);
	uthread_yield();
}

int uthread_create(uthread_func_t func, void *arg) {
	preempt_disable();
	// If thread overflow, send error.
	if(num_processes >= USHRT_MAX){
		return -1;
	}

	num_processes++;
	struct uthread_tcb* myThread = NULL;
	if (manage_thread_library(&myThread, 0)) {
		num_processes--;

		return -1;
	}

	myThread->stack = uthread_ctx_alloc_stack();
	if (!myThread->stack) {
		free(myThread->context);
		free(myThread);
		
		return -1;
	}

	int context_init_error = uthread_ctx_init(myThread->context, myThread->stack, func, arg);

	if (context_init_error) {
		uthread_ctx_destroy_stack(myThread->stack);
		free(myThread->context);
		free(myThread);

		return -1;
	}

	preempt_disable();

	if (queue_enqueue(ready_processes, (void*)myThread) == -1) {
		uthread_ctx_destroy_stack(myThread->stack);
		free(myThread->context);
		free(myThread);

		return -1;
	}
	
	preempt_enable();

	return myThread->tid;
}

int uthread_run(bool preempt, uthread_func_t func, void *arg)
{
	preempt_required = preempt;

	// Initialize the queues
	ready_processes = queue_create();
	zombie_processes = queue_create();
	blocked_processes = queue_create();
	if (!ready_processes || !zombie_processes || !blocked_processes) {
		return -1;
	}

	// Create the main thread
	struct uthread_tcb* main_thread;
	if (manage_thread_library(&main_thread, 1) != EXIT_SUCCESS) {
		return -1;
	}

	// Set the running thread to the main thread
	running = main_thread;

	// Create the initial thread and start the scheduler loop
	int thread_id = uthread_create(func, arg);
	if (thread_id == -1) {
		free(main_thread->context);
		free(main_thread);
		queue_destroy(ready_processes);
		queue_destroy(zombie_processes);
		return -1;
	}

	while (queue_length(ready_processes) > 0) {
		uthread_yield();
	}

	free(main_thread->context);
	free(main_thread);
	queue_destroy(ready_processes);
	queue_destroy(zombie_processes);

	return 0;
}

void uthread_block(void)
{
	preempt_disable();
	running->state = BLOCKED;
	queue_enqueue(blocked_processes, (void*)running);
	uthread_yield();
}

void uthread_unblock(struct uthread_tcb *uthread)
{
	if (uthread != NULL && uthread->state == BLOCKED) {
		uthread->state = READY;
		queue_delete(blocked_processes, (void*)uthread);
		queue_enqueue(ready_processes, (void*)uthread);
	}
}

int uthread_start(int preempt)
{
	ready_processes = queue_create();
	// If failure, send error.
	if (!ready_processes) {
		return -1;
	}	

	zombie_processes = queue_create();
	// If failure, clean readies. Send error.
	if (!zombie_processes) {
		queue_destroy(ready_processes);

		return -1;
	}

	blocked_processes = queue_create();
	// If failure, clean readies. Send error.
	if (!blocked_processes) {
		queue_destroy(ready_processes);
		queue_destroy(zombie_processes);

		return -1;
	}
	
	if(manage_thread_library(&running, 1)) {
		return -1;
	}
	
	if (preempt) {
		preempt_required = preempt;
		preempt_start(true);
	}

	return 0;
}
