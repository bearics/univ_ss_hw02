#include "Thread.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "MsgQueue.h"

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
	Qcb* qcb = qcbTblEntry[msqid].pQcb;
	struct mymsgbuf* msg = (struct mymsgbuf *)msgp;

	// make newmsgNode & add in Queue
	Message* newmsg = (Message*)malloc(sizeof(Message));
	newmsg->type = msg->mytype;
	strcpy(newmsg->data, msg->mtext);
	newmsg->size = msgsz;
	newmsg->pPrev = qcb->pMsgTail;
	newmsg->pNext = NULL;

	// update msghead, tail in qcb
	if(qcb->pMsgHead == NULL)
		qcb->pMsgHead = newmsg;
	qcb->pMsgTail = newmsg;

}

int	mymsgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg)
{
	Qcb* qcb = qcbTblEntry[msqid].pQcb;
	struct mymsgbuf* msg = (struct mymsgbuf *)msgp;

	Message* cur = qcb->pMsgHead;

	while(cur != NULL){
		if(cur->type == msgtyp)
			break;
		cur = cur->pNext;
	}

	if( cur == NULL) // no in msgQ
	{

	}

	// copy msg to user
	bzero(msg, 100);
	msg->mytype = cur->type;
	strcpy(msg->mtext, cur->data);
	

}


int 	mymsgctl(int msqid, int cmd, void* buf)
{


}

void printMS()
{
	printf("-------QCB TABLE!-------\n");
	for(int i=0; i<MAX_QCB_SIZE; i++)
		printf("  %2d |  %p     |\n", qcbTblEntry[i++].key,  qcbTblEntry[i++].pQcb);
	printf("-------QCB FIN!!!-------\n");

}