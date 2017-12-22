#ifndef _RUN_H_
#define _RUN_H_

#include "Thread.h"

#define FALSE 0
#define TRUE 1

typedef enum{
	WAITING_QUEUE = 0,
	READY_QUEUE = 1,
}Queue;

/* main mutex and cond */
int runStop;
int waitCreate;
pthread_mutex_t mainMutex;
pthread_cond_t mainCond;

/* running thread pointer */
Thread* runTh;

/* wait and wakeup funtion */
void *__wrapperFunc(void* arg);
void __thread_wakeup(Thread* pTh);
void __thread_wait_handler(int signo);

/* node funtions */
Thread* createNode(thread_t tid);
Thread**	selectQueue(Queue queue);
Thread*	insertAtTail(Queue queue, Thread* pth);
Thread*	deleteAtFirst(Queue queue);
Thread* deleteNode(Queue queue, thread_t tid);
Thread* searchQueue(Queue queue, thread_t tid);
void	print(Queue queue);
void printRunningTh();
void runResume();
void printQ();


#endif /* _RUN_H_ */



