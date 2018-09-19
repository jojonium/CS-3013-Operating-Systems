#ifndef MAILBOX_H_
#define MAILBOX_H_

#include <semaphore.h>

#define RANGE 1
#define ALLDONE 2

struct msg {
	int iSender; /* sender of the message (0 .. number-of-threads) */
	int type;    /* its type */
	int value1;  /* first value */
	int value2;  /* second value */
};

// global variables
struct msg **postOffice; // because it contains a bunch of mailboxes
pthread_t **rope;        // because it's made of threads
sem_t *semArray;         // couldn't think of a clever name for this one
int inputThreads;

int SendMsg(int iTo, struct msg *pMsg);
int RecvMsg(int iFrom, struct msg *pMsg);

#endif
