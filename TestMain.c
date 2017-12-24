#include <stdio.h>
#include <stdlib.h>

#include "TestCase1.h"
#include "TestCase2.h"
#include "TestCase3.h"
#include "TestCase4.h"


#include "Init.h"
#include "Scheduler.h"
#include "Thread.h"
#include "MsgQueue.h"

/* have to delete */
#include <string.h>
void hyounglin(void)
{
	struct mymsgbuf{
        long mytype;
        char mtext[100];
	};

	struct mymsgbuf mb, mb2;

	mb.mytype = 2;
	strcpy(mb.mtext, "hello");

	int id = mymsgget(1234, 0);
	mymsgsnd(id, (void*)&mb, 100, 1);
	printf("send fin \n");
	sleep(2);

	mymsgrcv(id, (void *)&mb2, 100, 2,0);
	printf("rcv gogo \n");
	printf("type : %ld", mb2.mytype);
	printf(", text : %s\n", mb2.mtext);
	sleep(10);
}

/* have to delete */
// #include "Task.h"
// #include "init.h"
// #include "ObjMgr.h"
// #include "Scheduler.h"

void main(int argc, char* argv[])
{
	int TcNum;
	thread_t tid1,tid2,tid3,tid4;

	if(argc != 2)
	{
		perror("Input TestCase Number!");
		exit(0);
	}
	
	Init();
	
	TcNum = atoi(argv[1]);
	
	switch(TcNum)
	{
		case 0:
			thread_create(&tid4, NULL,(void*)hyounglin, 0);
			break;
		case 1:
		    thread_create(&tid1, NULL, (void*)TestCase1, 0);
			break;
		case 2:
			thread_create(&tid2, NULL, (void*)TestCase2, 0);
		    break;
		case 3:
			thread_create(&tid3, NULL, (void*)TestCase3, 0);
		   	break;
		case 4:
			thread_create(&tid4, NULL,(void*)TestCase4, 0);
			break;
		
	}

	RunScheduler();
	while(1){}
}

