#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "queue.h"
#include "sem.h"
#include "private.h"
#include "uthread.h"

struct semaphore {
    size_t count;
    queue_t waiting_threads;
};



// void iterate_sem(queue_t queue, void* data) {
//     sem_t sem = (sem_t)data;
//     printf("%d\n",sem->count);
//     queue_iterate(sem->waiting_threads, iterate_uthread_tcb);
// }

sem_t sem_create(size_t count) {
    sem_t semaphore = (sem_t)malloc(sizeof(struct semaphore));
    if (semaphore == NULL) {
        return NULL;
    }

    semaphore->count = count;
    semaphore->waiting_threads = queue_create();
    if (semaphore->waiting_threads == NULL) {
        free(semaphore);
        return NULL;
    }

    return semaphore;
}

int sem_destroy(sem_t sem) {
    if (sem == NULL) {
        return -1;
    }

    if (queue_length(sem->waiting_threads) != 0) {
        return -1;
    }

    queue_destroy(sem->waiting_threads);
    free(sem);
    return 0;
}

int sem_down(sem_t sem) {
    
    if (sem == NULL) {
        return -1;
    }

    bool first_time = true;
    while (sem->count == 0) {
        if(first_time){
            queue_enqueue(sem->waiting_threads, (void*)uthread_current());
            first_time = false;
        }
        // Block the current thread until the semaphore becomes available
        uthread_block();
        // After unblocking, check if the semaphore is still not available
        if (sem->count == 0) {
            continue;
        }
    }

    // Decrement the semaphore count and return success
    sem->count--;
    return 0;
}

int sem_up(sem_t sem) {
    if (sem == NULL) {
        return -1;
    }

    sem->count++;
    // If there are waiting threads, unblock one
    if (queue_length(sem->waiting_threads) > 0) {
        struct uthread_tcb *waiting_thread;
        queue_dequeue(sem->waiting_threads, (void **)&waiting_thread);
        uthread_unblock(waiting_thread);
    }

    return 0;
}
