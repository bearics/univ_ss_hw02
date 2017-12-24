#include "Init.h"
#include "Run.h"
#include <pthread.h>

void Init(){

	/* initialize main mutex and cond */
	pthread_mutex_init(&mainMutex, NULL);
	pthread_cond_init(&mainCond, NULL);

	runStop=0;

	/* for msgQ */
	_InitMsgQueue();
}

