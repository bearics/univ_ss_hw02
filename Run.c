#include "Run.h"
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>


void *__wrapperFunc(void* arg)
{
	void* ret;
	WrapperArg* pArg = (WrapperArg*)arg;
	
	int retSig;
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	signal(SIGUSR1, __thread_wait_handler);
	while(searchQueue(READY_QUEUE, pthread_self()) == NULL) {} 
	__thread_wait_handler(0);
	ret = (*(pArg->funcPtr))(pArg->funcArg);
	free(arg);
	return ret;
}

void __thread_wakeup(Thread* pTh)
{
	pthread_mutex_lock(&(pTh->readyMutex));
	pTh->bRunnable = TRUE;
	pthread_cond_signal(&(pTh->readyCond));
	pthread_mutex_unlock(&(pTh->readyMutex));
}

void __thread_wait_handler(int signo)
{
	Thread* pTh;
	if((pTh = searchQueue(READY_QUEUE, pthread_self())) == NULL){
		if((pTh = searchQueue(WAITING_QUEUE, pthread_self())) == NULL){ 
			pTh = runTh;
		}
	}
	pTh->bRunnable = FALSE;
	pthread_mutex_lock(&(pTh->readyMutex));
	waitCreate=1;
	while (pTh->bRunnable == FALSE){
		pthread_cond_wait(&(pTh->readyCond), &(pTh->readyMutex));
	}
	pthread_mutex_unlock(&(pTh->readyMutex));
}


/* doubly linked list functions */
Thread**	selectQHead(Queue queue)
{
	if( queue == READY_QUEUE )
		return &ReadyQHead;
	else if(queue == WAITING_QUEUE)
		return &WaitQHead;
}

Thread**	selectQTail(Queue queue)
{
	if( queue == READY_QUEUE )
		return &ReadyQTail;
	else if(queue == WAITING_QUEUE)
		return &WaitQTail;
}

Thread* createNode(thread_t tid)
{
	Thread* newNode = (Thread*)malloc(sizeof(Thread));

	/* initialize Thread struct */
	newNode->status = THREAD_STATUS_READY;
	newNode->pExitCode = (void*)0;
	newNode->tid = tid;
	newNode->parentTid = pthread_self();
	pthread_cond_init(&(newNode->readyCond), NULL);
	newNode->bRunnable = FALSE;
	pthread_mutex_init(&(newNode->readyMutex), NULL);
	newNode->pNext = NULL;
	newNode->pPrev = NULL;
	newNode->type = -1;

	return newNode;
}

Thread*	insertAtTail(Queue queue, Thread* pth)
{
	Thread** pHead = selectQHead(queue);
	Thread** pTail = selectQTail(queue);
	
	pth->pNext = NULL;
	pth->pPrev = NULL;

	if(*pHead == NULL)
	{
		*pHead = pth;
		*pTail = pth;
		return pth;
	}
	
	(*pTail)->pNext = pth;
	pth->pPrev = *pTail;
	*pTail = pth;
	
	return pth;
}


Thread*	deleteAtFirst(Queue queue)
{
	Thread** pHead = selectQHead(queue);
	Thread** pTail = selectQTail(queue);
	Thread* del = *pHead;

	if(*pHead == NULL)
		return NULL;	// nothing to delete
	if(del->pNext == NULL)
	{
		*pHead = NULL;
		*pTail = NULL;
	}
	else
	{
		*pHead = del->pNext;
		(*pHead)->pPrev = NULL;
	}

	del->pNext = NULL;
	del->pPrev = NULL;
	return del;
}

Thread* deleteNode(Queue queue, thread_t tid)
{
	Thread** pHead = selectQHead(queue);
	Thread** pTail = selectQTail(queue);
	Thread* del = searchQueue(queue, tid);
	Thread* tmp = NULL;

	if( *pHead == NULL || del == NULL)
		return NULL;

	if( *pHead == del )
		*pHead = del->pNext;

	if( del->pNext != NULL )
		del->pNext->pPrev = del->pPrev;

	if( del->pPrev != NULL)
		del->pPrev->pNext = del->pNext;

	if(*pHead == NULL)
		*pTail = NULL;
	else
	{
		tmp = *pHead;	
		while(tmp->pNext != NULL )
		{
			tmp = tmp->pNext;
		}
		*pTail = tmp;
	}
	
	del->pNext = NULL;
	del->pPrev = NULL;

	return del;
}

Thread* searchQueue(Queue queue, thread_t tid)
{
	Thread** pHead = selectQHead(queue);
	Thread* temp = *pHead;
	while(temp != NULL)
	{
		if(temp->tid == tid)
			break;
		temp=temp->pNext;
	}
	return temp;
}

void printRunningTh()
{
	if(runTh == NULL)
	{
		printf("\n---------------------------No Running Th--------------------------\n");
		return;
	}
	printf("\n----------------------------------Running Th----------------------\n");
	printf(" *  Prev : %p,  \tNext : %p,     \ttid: %u\n",  runTh->pPrev, runTh->pNext, (unsigned int)(runTh->tid));
	printf(" *  status : %d,  \tbRunnable : %d   ptid : %u\n", runTh->status, runTh->bRunnable, (unsigned int)(runTh->parentTid));
	printf("\n------------------------------------------------------------------\n");
}

void print(Queue queue)
{
	Thread** pHead = selectQHead(queue);
	Thread** pTail = selectQTail(queue);
	Thread* temp = *pHead;
	printf("\n----------------------------QUEUE LIST----------------------------\n");
	printf("\n\tHead(%p)\t\t Tail(%p)\n", *pHead, *pTail);
	int i=0;
	while(temp != NULL)
	{
		printf("\nnode%2d(%p) ------------------------------------------------\n",i, temp);
		printf(" *  Prev : %p,  \tNext : %p,     \ttid: %u\n",  temp->pPrev, temp->pNext, (unsigned int)(temp->tid));
		printf(" *  status : %d,  \tbRunnable : %d   ptid : %u\n", temp->status, temp->bRunnable, (unsigned int)(temp->parentTid));
		temp = temp->pNext;
		i++;
		if(i>10)
			break;
	}
	printf("\n");
}

void runResume()
{
	if(--runStop < 0)
		runStop=0;
	pthread_cond_signal(&mainCond);
	pthread_mutex_unlock(&mainMutex);
	printf("self=%u, run=%d\n",pthread_self(), runStop);
	return;
}
void printQ()
{
	printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^READY_QUEUE^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
	print(READY_QUEUE);
	printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^WAITING_QUEUE^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
	print(WAITING_QUEUE);
	printRunningTh();	
}