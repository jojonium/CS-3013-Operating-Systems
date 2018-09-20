#ifndef BOARD_H_
#define BOARD_H_

#include <semaphore.h>

#define RANGE 1
#define ALLDONE 2
#define GO 3
#define GENDONE 4 // Generation Done
#define MAXTHREAD 10
#define MAXGRID 40
#define ON '1'
#define OFF '0'

struct msg {
	int iSender; /* sender of the message (0 .. number-of-threads) */
	int type;    /* its type */
	int value1;  /* first value */
	int value2;  /* second value */
};

// global variables
char **A; // board 1
char **B; // board 2
char **C; // board 3
struct msg **postOffice; // because it contains a bucolsh of mailboxes
pthread_t **rope;        // because it's made of threads
sem_t **semArray1;
sem_t **semArray2;
char **boards[3];
int inputThreads, rows, cols;
long iteration, gens;
void *playT(void *args);
void printGrid(char **board, long iteration);
void playOne(char *old[], char *new[]);
int checkDone(char *A[], char *B[], char *C[]);
int SendMsg(int iTo, struct msg *pMsg);
int RecvMsg(int iFrom, struct msg *pMsg);

#endif
