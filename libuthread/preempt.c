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
#define INITIALIZE_VALUE 0

// Define static variables for managing preemption
static sigset_t block_alarm;           // Signal set for blocking the alarm signal
static sigset_t old_set;               // Old signal set
static struct itimerval timer;         // Timer for setting the interval for preemption
static struct itimerval old_timer;     // Old timer settings
static struct sigaction act;           // Signal action for setting up the timer handler
static struct sigaction old_act;       // Old signal action

// Signal handler for the timer
static void handle_timer(int signum)
{
	if (signum == SIGVTALRM)
		uthread_yield();  // Yield the current thread upon timer signal
}

// Function to start preemption
void preempt_start(bool preempt)
{
    if(preempt){
        act.sa_handler = handle_timer; 
        sigemptyset(&act.sa_mask); 
        act.sa_flags = INITIALIZE_VALUE;
        timer.it_interval.tv_sec = INITIALIZE_VALUE; 
        timer.it_value.tv_sec = INITIALIZE_VALUE;

        sigemptyset(&block_alarm);
        sigaddset(&block_alarm, SIGVTALRM); 

        timer.it_interval.tv_usec = (long int) (1.0 / HZ * 1000000);
        timer.it_value.tv_usec = (long int) (1.0 / HZ * 1000000);

        sigprocmask(SIG_SETMASK, NULL, &old_set); 
        setitimer(ITIMER_VIRTUAL, &timer, &old_timer);
        sigaction(SIGVTALRM, &act, &old_act);  
    }
}

// Function to stop preemption
void preempt_stop(void) {
	setitimer(SIGVTALRM, &old_timer, NULL);
	sigaction(SIGVTALRM, &old_act, NULL);
	sigprocmask(SIG_SETMASK, &old_set, NULL);
}

// Function to enable preemption
void preempt_enable(void) {
	sigprocmask(SIG_UNBLOCK, &block_alarm, NULL);
}

// Function to disable preemption
void preempt_disable(void) {
	sigprocmask(SIG_BLOCK, &block_alarm, NULL);
}