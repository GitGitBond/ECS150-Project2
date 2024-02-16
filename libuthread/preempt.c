#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100
struct sigaction new_action; // Structure to define the new action for a signal
struct sigaction old_action; // Structure to hold the previous action for SIGVTALRM
struct itimerval timer; // Structure to configure the timer

// Signal handler function for SIGVTALRM
void sighandler(int signum){

	// Gets called when the alarm rings
	if (signum == SIGVTALRM){
		uthread_yield(); // Yield to the next ready thread
	}
}

// Disable preemption by blocking SIGVTALRM signal
void preempt_disable(void)
{
	/* Disable preemption by blocking SIGVTALRM signal */
	sigset_t block_set;
	sigemptyset(&block_set);
	sigaddset(&block_set, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &block_set, NULL);
}

// Enable preemption by unblocking SIGVTALRM signal
void preempt_enable(void)
{
	/* Enable preemption by unblocking SIGVTALRM signal */
	sigset_t unblock_set;
	sigemptyset(&unblock_set);
	sigaddset(&unblock_set, SIGVTALRM);
	sigprocmask(SIG_UNBLOCK, &unblock_set, NULL);
}

// Start preemption
void preempt_start(bool do_preempt)
{
	/* Start preemption if do_preempt is true */
	if (!do_preempt){
		return; // Do nothing if do_preempt is false
	}
	
	// Set up signal handler for SIGVTALRM
    sigemptyset(&new_action.sa_mask);
	new_action.sa_handler = sighandler;
	new_action.sa_flags = 0;

	// Unblock SIGVTALRM signal
	sigset_t unblock_set;
	sigemptyset(&unblock_set);
	sigaddset(&unblock_set, SIGVTALRM);
	sigprocmask(SIG_UNBLOCK, &unblock_set, NULL);

	// Register the signal handler
	if (sigaction(SIGVTALRM, &new_action, &old_action) == -1) {
        perror("sigaction error\n");
        exit(1);
    }

	// Configure the timer for preemption
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_usec = 1000000 / HZ;
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_usec = 1000000 / HZ;

	// Set the timer
	if (setitimer(ITIMER_VIRTUAL, &timer, NULL) < 0){
		perror("setitimer error");
		exit(1);
	}
}

// Stop preemption
void preempt_stop(void)
{
	/* Stop preemption and restore the previous signal action */
    sigaction(SIGVTALRM, &old_action, NULL); // Restore previous signal action
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_VIRTUAL, &timer, NULL); // Disable the timer
}
