# Project Report: User-level thread library
## Summary

The user-level thread library is designed to facilitate the creation and execution of independent threads within applications. It offers features such as semaphores, preemption, timers, and signal handlers to manage thread synchronization and prevent race conditions.

## Phase 1: queue API

The Queue API utilizes a linked list data structure to manage data in a FIFO (First In, First Out) manner. It comprises functions for creating, destroying, enqueueing, dequeuing, deleting elements, iterating over the queue, and determining the queue length.

The queue is represented by two main structures: queue_node and queue. The queue_node structure holds data and a pointer to the next node, while the queue structure manages pointers to the head and tail nodes, along with tracking the queue size.

The core functions of the Queue API include:

-   queue_create(): Dynamically allocates memory for a new queue, initializes pointers, and returns a pointer to the newly created queue.
-   queue_destroy(): Deallocates memory for a valid, empty queue, ensuring no memory leaks occur.
-   queue_enqueue(data): Adds an element to the end of the queue, updating pointers and increasing the size of the queue.
-   queue_dequeue(data): Removes the oldest element from the queue, storing its data, and updating pointers accordingly.
-   queue_delete(data): Deletes a specified element from the queue, handling various scenarios such as deletion from the head, tail, or middle of the queue.
-   queue_iterate(func): Iterates over each element of the queue, invoking a provided function for each node, facilitating custom operations.
-   queue_length(queue): Determines the length of the queue, providing an accurate count of elements within the queue or returning -1 for an invalid queue.

These functions collectively provide robust functionality for managing data within a queue, ensuring efficient and reliable data handling operations.

## Phase 2: uthread API

Our journey in implementing the Uthread API began with setting up essential structures and functionalities for effective thread management. We established the ready queue (rq) and the zombie queue (zq) to organize threads efficiently.

Building upon this foundation, we defined the struct uthread_tcb, encapsulating critical thread information. We implemented key functions like uthread_current() and uthread_yield() to manage thread execution seamlessly.

Next, we focused on thread creation and execution. The uthread_create() function was crafted to initialize new threads and enqueue them into the ready queue (rq). We orchestrated thread execution with the uthread_run() function, facilitating thread creation, initialization, and continuous execution.

As our journey progressed, we fortified the API with essential functionalities for managing thread states. Functions like uthread_exit() enabled graceful thread termination and cleanup, while uthread_block() and uthread_unblock() facilitated thread blocking and unblocking as needed.

With each component seamlessly integrated, our Uthread API emerged as a robust solution for efficient thread management, empowering a wide range of applications.

## Phase 3: semaphore API

In implementing the Semaphore API, we first designed a structure called struct semaphore to hold semaphore-related data. This structure comprises two crucial members: count, which tracks the available resources, and waiting_threads, responsible for maintaining a queue of threads waiting for resource access.

Subsequently, we proceeded to implement the creation and destruction operations for semaphores. Using the sem_create() function, we initialized a semaphore with a given initial count and created an empty waiting queue. Upon successful creation, a pointer to the semaphore is returned, or NULL is returned to indicate failure. The sem_destroy() function was developed to deallocate memory for the semaphore and its associated waiting queue, ensuring no waiting threads exist before destruction.

In the semaphore's down operation, when a thread attempts to acquire a resource using sem_down(), it enters a loop while the semaphore count is zero, indicating no available resources. If it's the first iteration of the loop, the current thread is enqueued into the semaphore's waiting queue. Subsequently, the thread blocks, waiting for the semaphore to become available, and unblocks only when the semaphore count increases.

Lastly, the semaphore's up operation is implemented through the sem_up() function. This function increments the semaphore count, signaling the release of a resource. If there are threads waiting in the queue, one waiting thread is dequeued and unblocked, allowing it to proceed. Preemption is disabled during semaphore operations to ensure atomicity and prevent race conditions.

## Phase 4: preemption

In the fourth phase, our focus was on implementing preemption to ensure fair CPU allocation among threads. We began by defining the necessary structures and variables to handle preemption effectively.

The first step was to establish a signal handler for the SIGVTALRM signal, responsible for triggering preemption. This signal handler, named sighandler(), simply invokes the uthread_yield() function to yield to the next ready thread when the alarm rings.

Next, we implemented functions to enable and disable preemption. The preempt_disable() function blocks the SIGVTALRM signal, effectively disabling preemption, while preempt_enable() unblocks the SIGVTALRM signal to enable preemption.

Subsequently, the preempt_start() function was developed to initiate preemption. It sets up the signal handler for SIGVTALRM, configures the timer to generate periodic signals, and registers the signal handler. This function ensures that preemption occurs at the specified frequency.

Finally, the preempt_stop() function was implemented to halt preemption. It restores the previous signal action for SIGVTALRM, disables the timer, and effectively stops preemption.


## queue_tester.c

-   test_queue_create(): This function tests the queue_create() function by verifying that it returns a non-null queue pointer.

-   test_simple_enqueue_dequeue(): It tests the basic enqueue and dequeue operations by enqueuing an integer value, dequeuing it, and asserting that the dequeued value matches the original enqueued value.

-   test_queue_length(): Verifies the correctness of the queue_length() function by enqueueing five items into the queue and checking if the length of the queue is 5.

-   test_queue_null_length(): Checks the behavior of queue_length() when the queue is NULL, ensuring it returns -1.

-   test_delete_item(): Tests the deletion of an item from the queue by enqueueing six items, deleting the second item, and verifying that the length of the queue becomes 5.

-   test_delete_item_not_found(): Verifies the behavior of queue_delete() when attempting to delete an item not present in the queue, ensuring it returns -1.

-   test_queue_destruction(): Tests the destruction of the queue by enqueueing and dequeueing three items, then destroying the queue and checking if it returns 0.

-   test_queue_destruction_not_empty(): Checks the behavior of queue_destroy() when attempting to destroy a non-empty queue, ensuring it returns -1.

-   test_dequeue_empty_queue(): Verifies the behavior of dequeue operation on an empty queue, ensuring it returns -1.

-   test_dequeue_string_array(): Tests the dequeue operation on a queue containing string array items, ensuring it dequeues the items correctly.

-   test_queue_iteration(): This function tests the iteration functionality of the queue. It creates a queue, enqueues integer items {0, 1, 2}, and then iterates over the queue using the queue_iterate() function. It verifies that the iteration works correctly by comparing the output with the expected output string "0 1 2 ".

## Debugging issues

-   Segmentation fault in uthread_hello.c: Faced a segmentation fault while testing uthread_hello.c, caused by inappropriate use of malloc() function in uthread_create().

-   No output in uthread_hello.c and uthread_yield.c tests: Tests didn't print any message due to missing application of queue_create() function to global queues (alive_queue and zombie_queue), resulting in an infinite loop when called, and setting the idle_thread's state to ready, causing it to get stuck in alive_queue and resulting in a program freeze.

## Reference

- [1] https://blog.csdn.net/weixin_48185168/article/details/133576463
- [2] https://www.gnu.org/software/libc/manual/2.36/html_mono/libc.html#Signal-Actions https://www.gnu.org/software/libc/manual/2.36/html_mono/libc.html#Setting-an-Ala