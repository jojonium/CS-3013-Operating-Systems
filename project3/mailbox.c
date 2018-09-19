#include "mailbox.h"
#include <stdio.h>

int SendMsg(int iTo, struct msg *pMsg) {
	if (iTo < 0 || iTo > inputThreads + 1) {
		return 1;
	}
	postOffice[iTo] = pMsg;

	sem_post(semArray + iTo); // you have mail!

	return 0;
}

int RecvMsg(int iFrom, struct msg *pMsg) {
	printf("in RecvMsg\n");
	sem_wait(semArray + iFrom); // wait for mail

	if (iFrom < 0 || iFrom > inputThreads + 1) {
		return 1;
	}
	pMsg = postOffice[iFrom];
	return 0;
}
