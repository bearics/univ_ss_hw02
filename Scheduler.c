#include "Init.h"
#include "Thread.h"
#include "Scheduler.h"
#include "Run.h"
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>


int		RunScheduler( void )
{
	while(1)
	{
		
		//printQ();
		pthread_mutex_lock(&mainMutex);
		if(runStop != 0){
			pthread_cond_wait(&mainCond, &mainMutex);
		}
		__ContextSwitch(runTh, ReadyQHead);
		sleep(TIMESLICE);
		
	}
}

void    __ContextSwitch(Thread* pCurThread, Thread* pNewThread)
{
	// sleep current thread
	// nothing in current thread
	if(pNewThread == NULL)
		return;	// nothing to run
	if(pCurThread != NULL)
	{
		if(pCurThread->status != THREAD_STATUS_BLOCKED)
			pCurThread->status = THREAD_STATUS_READY;
			pthread_kill(pCurThread->tid, SIGUSR1);
			insertAtTail(READY_QUEUE, pCurThread);
	}
	// ready to run new th
	runTh = deleteAtFirst(READY_QUEUE);
	runTh->status = THREAD_STATUS_RUN;
	pthread_mutex_unlock(&mainMutex);
	__thread_wakeup(runTh);	
	return;
}


