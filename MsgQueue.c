#include "Thread.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "MsgQueue.h"


QcbTblEntry  qcbTblEntry[MAX_QCB_SIZE];


void	_InitMsgQueue(void)
{

}

int 	mymsgget(int key, int msgflg)
{
	int i=0;
	while(1)	// goto last in QCB Table
		if(qcbTblEntry[(i + i++)].key != -1)	
			break;
					   
	QcbTblEntry* qcb = &qcbTblEntry[i];
	
	qcb->key = key;
	qcb->pQcb = (QcbTblEntry *)malloc(sizeof(Qcb));
	
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
	QcbTblEntry* qcb = &qcbTblEntry[msqid];
	printf("msgp.mytype : %ld\n", msgp);
	printf("msgp.mtext : %s\n", (msgp+sizeof(long)) );



}

int	mymsgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg)
{


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