#include "Thread.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "MsgQueue.h"
#include "run.h"

struct mymsgbuf{
        long mytype;
        char mtext[100];
};

void	_InitMsgQueue(void)
{
	for(int i=0; i<MAX_QCB_SIZE; i++)
		qcbTblEntry[i].key = -1;
}

int 	mymsgget(int key, int msgflg)
{
	int i=0;
	while(1){// goto last in QCB Table
		if(qcbTblEntry[(i)].key == key)
			return i;
		if(qcbTblEntry[(i)].key == -1)
			break;
		i++;
	}	
					   
	QcbTblEntry* qcb = &qcbTblEntry[i];
	
	qcb->key = key;
	qcb->pQcb = (Qcb*)malloc(sizeof(Qcb));
	
	// init QCB
	(qcb->pQcb)->msgCount = 0;
	(qcb->pQcb)->pMsgHead = NULL;
	(qcb->pQcb)->pMsgTail = NULL;
	(qcb->pQcb)->waitThreadCount = 0;
	(qcb->pQcb)->pThreadHead = NULL;
	(qcb->pQcb)->pThreadTail = NULL;	

	return i;
}


int 	mymsgsnd(int msqid, const void *msgp, int msgsz, int msgflg)
{
	runStop++;
	pthread_mutex_lock(&mainMutex);


	Qcb* qcb = qcbTblEntry[msqid].pQcb;
	struct mymsgbuf* msg = (struct mymsgbuf *)msgp;


	// make newmsgNode & add in Queue
	Message* newmsg = (Message*)malloc(sizeof(Message));
	newmsg->type = msg->mytype;
	strcpy(newmsg->data, msg->mtext);
	newmsg->size = strlen(msg->mtext);
	newmsg->pPrev = qcb->pMsgTail;
	newmsg->pNext = NULL;

	// update msghead, tail in qcb
	if(qcb->pMsgHead == NULL)
		qcb->pMsgHead = newmsg;
	else
		(qcb->pMsgTail)->pNext = newmsg;
	qcb->pMsgTail = newmsg;
	qcb->msgCount++;

	// find msg in waiting Queue
	Thread* cur = qcb->pThreadHead;
	while(cur != NULL)
	{
		if(cur->type == msg->mytype)
		{
			if(cur->pPrev == NULL)
			{
				if(cur->pNext == NULL)
				{
					qcb->pThreadHead = NULL;
					qcb->pThreadTail = NULL;
				}
				else
				{
					qcb->pThreadHead = cur->pNext;
					(cur->pNext)->pPrev = NULL;
				}
			}
			else
			{
				if(cur->pNext == NULL)
				{
					(cur->pPrev)->pNext = NULL;
					qcb->pThreadTail = cur->pPrev;
				}
				else
				{
					(cur->pPrev)->pNext = cur->pNext;
					(cur->pNext)->pPrev = cur->pPrev;
				}
			}
			cur->status = THREAD_STATUS_READY;
			insertAtTail(READY_QUEUE, cur);
			qcb->waitThreadCount--;
			break;
		}
		// not equal type
		cur = cur->pNext;
	}

	runResume();
	return strlen(msg->mtext);	
}

int	mymsgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg)
{
	runStop++;
	pthread_mutex_lock(&mainMutex);

	Qcb* qcb = qcbTblEntry[msqid].pQcb;
	struct mymsgbuf* msg = (struct mymsgbuf *)msgp;

	Message* cur = qcb->pMsgHead;

	while(cur != NULL)
	{
		if(cur->type == msgtyp)
			break;
		cur = cur->pNext;
	}

	if( cur == NULL) // no in msgQ
	{
		if(qcb->pThreadHead == NULL)
		{	// add thread
			qcb->pThreadHead = runTh;
			qcb->pThreadTail = runTh;
		}
		else
		{	// add thread
			(qcb->pThreadTail)->pNext = runTh;
			runTh->pPrev = qcb->pThreadTail;
			qcb->pThreadTail = runTh;
		}
		qcb->waitThreadCount++;

		runTh->type = msgtyp;
		runTh->status = THREAD_STATUS_BLOCKED;


		// sleep thread

		
		Thread* pTh = runTh;
		runTh=NULL;
		pTh->bRunnable = FALSE;

		runResume();
		pthread_mutex_lock(&(pTh->readyMutex));
		waitCreate=1;
		pthread_cond_wait(&(pTh->readyCond), &(pTh->readyMutex));
		pthread_mutex_unlock(&(pTh->readyMutex));


		//		__thread_wait_handler(0);

		runStop++;
		pthread_mutex_lock(&mainMutex);
		cur = qcb->pMsgHead;
		while( cur != NULL )
		{
			if(cur->type == msgtyp)
				break;
			cur = cur->pNext;
		}
		
	}

	if(cur == NULL)
	{
		runResume();
		return -1;
	}

	// delete msg
	if(cur->pPrev == NULL)
	{
		if(cur->pNext == NULL)
		{
			qcb->pMsgHead = NULL;
			qcb->pMsgTail = NULL;
		}
		else
		{
			qcb->pMsgHead = cur->pNext;
			(cur->pNext)->pPrev = NULL;
		}
	}
	else
	{
		if(cur->pNext == NULL)
		{
			(cur->pPrev)->pNext = NULL;
			qcb->pMsgTail = cur->pPrev;
		}
		else
		{
			(cur->pPrev)->pNext = cur->pNext;
			(cur->pNext)->pPrev = cur->pPrev;
		}
	}

	bzero(msg, 100);
	msg->mytype = cur->type;
	strcpy(msg->mtext, cur->data);
	qcb->msgCount--;

	runResume();

	return strlen(msg->mtext);
}


int 	mymsgctl(int msqid, int cmd, void* buf)
{
	runStop++;
	pthread_mutex_lock(&mainMutex);

	Qcb* qcb = qcbTblEntry[msqid].pQcb;
	Message* pM = qcb->pMsgHead;
	Thread* pT = qcb->pThreadHead;

	if(pM != NULL )
	{	// if there is msg, return
		runResume();
		return -1;
	}

	qcbTblEntry[msqid].key=-1;
	// delete msg
	
	while(pM != NULL)
	{
		Message* del=pM;
		pM=pM->pNext;
		free(del);
	}

	// delete thread
	
	while(pT != NULL)
	{
		insertAtTail(READY_QUEUE, pT);
		pT=pT->pNext;
	}

	free(qcb);

	runResume();

	return 0;
}

void printMS()
{
	
	printf("-------QCB TABLE!-------\n");
	for(int i=0; i<3; i++) {
		printf("  %2d |  %p  |\n", qcbTblEntry[i].key,  qcbTblEntry[i].pQcb);
		if( qcbTblEntry[i].pQcb != NULL)
		{
			Message* pM =(qcbTblEntry[i].pQcb)->pMsgHead;
			while(pM != NULL)
			{
				printf("\t\>> type:%ld  , text: %s\n",pM->type, pM->data);
				pM = pM->pNext;
			}
			printf("\t--------------------------------\n");
			Thread* pTh = (qcbTblEntry[i].pQcb)->pThreadHead;
			while(pTh != NULL)
			{
				printf("\t>> type : %ld , tid:%u\n", pTh->type, (unsigned int)(pTh->tid));
				printf("\t *  Prev : %p,  \tNext : %p,     \ttid: %u\n",  pTh->pPrev, pTh->pNext, (unsigned int)(pTh->tid));
				printf("\t *  status : %d,  \tbRunnable : %d   ptid : %u\n", pTh->status, pTh->bRunnable, (unsigned int)(pTh->parentTid));
				pTh = pTh->pNext;
				sleep(1);
			}
		}
	}

	printf("-------QCB FIN!!!-------\n");
	//printQ();	

}