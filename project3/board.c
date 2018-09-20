#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void printGrid(char **board, long iteration) {
	int i, j;

	printf("=== Generation %ld ===\n", iteration);
	for (j = 0; j < rows; j++) {
		for (i = 0; i < cols; i++) {
			if (board[i][j] == ON)
				printf("\x1B[32mX\x1B[0m ");
			else
				printf("  ");
		}
		printf("\n");
	}
	printf("\n");
}

int getNeighbors(unsigned x, unsigned y, char *b[]) {
	return  ((y > 0 && x > 0) ? (b[x - 1][y - 1] == ON) : 0) +               // top left
		((y > 0) ? (b[x][y - 1] == ON) : 0) +                            // top
		((y > 0 && x < cols - 1) ? (b[x + 1][y - 1] == ON) : 0) +        // top right
		((x < cols - 1) ? (b[x + 1][y] == ON) : 0) +                     // right
		((y < rows - 1 && x < cols - 1) ? (b[x + 1][y + 1] == ON) : 0) + // bot right
		((y < rows - 1) ? (b[x][y + 1] == ON) : 0) +                     // bottom
		((y < rows - 1 && x > 0) ? (b[x - 1][y + 1] == ON) : 0) +        // bot left
		((x > 0) ? (b[x - 1][y] == ON) : 0);                             // left
}

// waits for a message from master thread, then plays the grid for the given rows (icolslusive)
// then passes a new message back to the master
void *playT(void *arg) {
	int bot, top, index, i, j, neighbors;
	long curgen;
	struct msg *message;

	index = (int)(intptr_t)arg;

	message = (struct msg *)malloc(sizeof(struct msg));

	RecvMsg(index, message); // wait for message from master
	if (message->type != RANGE) {
		printf("Thread %d expected a RANGE\n", index);
	}
	bot = message->value1;
	top = message->value2;

	for (curgen = 0; curgen <= gens; curgen++) {
		RecvMsg(index, message); // wait for message from master

		if (message->type != GO) {
			printf("Thread %d expected a GO\n", index);
		}		

		// play one generation of the game
		for (j = bot; j <= top; j++) {
			for (i = 0; i < cols; i++) {
				if (((neighbors = getNeighbors(i, j, boards[curgen % 3])) == 3) ||
				    (neighbors == 2 && boards[curgen % 3][i][j] == ON)) {
					boards[(curgen + 1) % 3][i][j] = ON;
				} else {
					boards[(curgen + 1) % 3][i][j] = OFF;
				}
			}
		}

		// make a GENDONE message
		message->iSender = index;
		message->type = GENDONE;
		
		SendMsg(0, message); // send it back
	}

	// send ALLDONE message
	message->iSender = index;
	message->type = ALLDONE;

	SendMsg(0, message);

	return (void *)0;
}

// returns 1 if equal, 0 otherwise
int arrayEquals(char *A[], char *B[]) {
	int i, j;
	for (j = 0; j < rows; j++) {
		for (i = 0; i < cols; i++) {
			if (A[i][j] != B[i][j])
				return 0;
		}
	}
	return 1;
}

// returns 0 for not done, 1 for loop, 2 for oscillate
int checkDone(char *A[], char *B[], char *C[]) {
	if (arrayEquals(A, B))
		return 1;
	else if (arrayEquals(C, B))
		return 2;
	else
		return 0;
}

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
