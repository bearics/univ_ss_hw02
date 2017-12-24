#include "Thread.h"
#include "Init.h"
#include "Scheduler.h"
#include "Run.h"
#include <stdlib.h>
#include <signal.h>

int 	thread_create(thread_t *thread, thread_attr_t *attr, void *(*start_routine) (void *), void *arg)
{
	Thread* pTh = NULL;
	runStop++;
	waitCreate = 0;
	pthread_mutex_lock(&mainMutex);
	WrapperArg* wrapperArg=(WrapperArg*)malloc(sizeof(WrapperArg));
	wrapperArg->funcPtr = start_routine;
	wrapperArg->funcArg = arg;
	pthread_create(thread, attr, __wrapperFunc,wrapperArg);
	pTh = insertAtTail(READY_QUEUE, createNode(*thread));	// insert readyQ
	while(waitCreate != 1) {}
	pthread_mutex_lock(&(pTh->readyMutex));
	pthread_mutex_unlock(&(pTh->readyMutex));
	runResume();
}


int 	thread_join(thread_t thread, void **retval)
{
	Thread* pth = NULL;
	Thread* cth = NULL;
	runStop++;
	pthread_mutex_lock(&mainMutex);

	if( runTh != NULL )
		pth = runTh;
	else {
		pth = searchQueue(READY_QUEUE, pthread_self());
		deleteNode(READY_QUEUE, pthread_self());
	}

	cth = searchQueue(WAITING_QUEUE, thread);
	if(cth != NULL)
	{
		if(cth->status == THREAD_STATUS_ZOMBIE)
		{	// reaping zombie at waiting Q
			*retval = cth->pExitCode;
			free(deleteNode(WAITING_QUEUE, thread)); // delete zombie
			runResume();
			return 0;	// success
		}
	}
	// there is no zombie(thread)

	pth->status = THREAD_STATUS_BLOCKED;
	insertAtTail(WAITING_QUEUE, pth);
	runTh = NULL;
	do
	{
		runResume();
		__thread_wait_handler(0);
		runStop++;
		pthread_mutex_lock(&mainMutex);
		cth = searchQueue(WAITING_QUEUE, thread);
		if( cth != NULL)
			if(cth->status == THREAD_STATUS_ZOMBIE)
				break;
			else
				cth = NULL;
	} while (cth == NULL);
	// now cth is zombie's node pointer
	*retval = cth->pExitCode;
	free(deleteNode(WAITING_QUEUE, thread)); // delete zombie
	pth->status = THREAD_STATUS_READY;
	insertAtTail(READY_QUEUE, deleteNode(WAITING_QUEUE, pth->tid));
	runResume();
	return 0;	// success
}


int 	thread_suspend(thread_t tid)
{
	if(searchQueue(READY_QUEUE, tid) == NULL)
		return -1; // no tid

	runStop++;
	pthread_mutex_lock(&mainMutex);

	insertAtTail(WAITING_QUEUE, deleteNode(READY_QUEUE, tid));
	
	Thread* pth = searchQueue(WAITING_QUEUE, tid);	// find TCB in waiting queue

	pth->status = THREAD_STATUS_BLOCKED;

	runResume();
	return 0;	// success!

}


int	thread_resume(thread_t tid)
{
	if(searchQueue(WAITING_QUEUE, tid) == NULL)
		return -1; // no tid

	runStop++;
	pthread_mutex_lock(&mainMutex);

	insertAtTail(READY_QUEUE, deleteNode(WAITING_QUEUE, tid));

	Thread* pth = searchQueue(READY_QUEUE, tid);	// find TCB in waiting queue

	pth->status = THREAD_STATUS_READY;

	runResume();
	return 0;	// success!
}

int thread_exit(void* retval)
{
	Thread* pth=NULL;
	Thread* cth=NULL;
	runStop++;
	pthread_mutex_lock(&mainMutex);

	cth = runTh;

	cth->pExitCode = retval;
	cth->status = THREAD_STATUS_ZOMBIE;
	insertAtTail(WAITING_QUEUE, cth);
	runTh = NULL;
	if((pth = searchQueue(WAITING_QUEUE, cth->parentTid)) != NULL){
		__thread_wakeup(pth);
	}
	runResume();
	return 0;
}

thread_t	thread_self()
{
	return pthread_self();
}	
