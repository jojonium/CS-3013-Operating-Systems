#include "mailbox.h"
#include <stdio.h>

int SendMsg(int iTo, struct msg *pMsg) {
	if (iTo < 0 || iTo > inputThreads + 1) {
		printf("called SendMsg with invalid arguments\n");
		return 1;
	}
	
	sem_wait(semArray1[iTo]); // wait until the mailbox is empty

	postOffice[iTo] = pMsg;

	sem_post(semArray2[iTo]); // you have mail!

	return 0;
}

int RecvMsg(int iFrom, struct msg *pMsg) {
	sem_wait(semArray2[iFrom]); // wait for mail to arrive

	if (iFrom < 0 || iFrom > inputThreads + 1) {
		printf("called recvMsg with invalid arguments\n");
		return 1;
	}
	
	*pMsg = *postOffice[iFrom];

	sem_post(semArray1[iFrom]); // mark this mailbox empty
    
	return 0;
}
