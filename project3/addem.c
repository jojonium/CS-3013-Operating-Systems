#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include "mailbox.h"

#define MAXTHREAD 10

void *addit(void *arg);

int main(int argc, char *argv[]) {
	int i, target, step, temp, result;
	struct msg *received;
	struct msg **mailToSend;

	// check arguments
	if (argc < 3) {
		printf("ERROR: too few arguments\n");
		printf("Expected the form;\n\t./addem numThreads number\n");
		exit(1);
	}

	inputThreads = atoi(argv[1]);
	target = atoi(argv[2]);

	if (inputThreads < 1) {
		printf("Input number of threads too small, defaulting to 1\n");
		inputThreads = 1;
	} else if (inputThreads > MAXTHREAD) {
		printf("Input number of threads too high, defaulting to %d\n", MAXTHREAD);
		inputThreads = MAXTHREAD;
	}

	if (target < 1) {
		printf("Argument 2 too small, defaulting to 100\n");
		target = 100;
	}

	// allocate memory
	postOffice = (struct msg **)malloc((inputThreads + 1) * sizeof(struct msg *));
	mailToSend = (struct msg **)malloc(inputThreads * sizeof(struct msg *));
	semArray1 = (sem_t **)malloc((inputThreads + 1) * sizeof(sem_t *));
	semArray2 = (sem_t **)malloc((inputThreads + 1) * sizeof(sem_t *));
	rope = (pthread_t **)malloc(inputThreads * sizeof(pthread_t *));
	received = (struct msg *)malloc(sizeof(struct msg));

	// make the mailboxes
	for (i = 0; i <= inputThreads; i++) {
		// allocate more memory
		semArray1[i] = (sem_t *)malloc(sizeof(sem_t));
		semArray2[i] = (sem_t *)malloc(sizeof(sem_t));
		sem_init(semArray1[i], 0, 1); //psem
		sem_init(semArray2[i], 0, 0); //csem
	}

	// make messages to send
	step = target / inputThreads;
	temp = 0;
	for (i = 0; i < inputThreads; i++) {
		mailToSend[i] = (struct msg *)malloc(sizeof(struct msg *));
		mailToSend[i]->iSender = 0;
		mailToSend[i]->type = RANGE;
		mailToSend[i]->value1 = temp + 1;
		temp += step;
		// last message needs to be different to account for rounding errors
		mailToSend[i]->value2 = (i == inputThreads - 1) ? target : temp;
	}
	
	// send 'em
	for (i = 0; i < inputThreads; i++) {
		SendMsg(i + 1, mailToSend[i]);
	}

	// make the threads
	for (i = 0; i < inputThreads; i++) {
		rope[i] = (pthread_t *)malloc(sizeof(pthread_t *));
		if (pthread_create(rope[i], NULL, &addit, (void *)(intptr_t)(i + 1))) {
			printf("error creating thread %d\n", i + 1);
		}
	}

	// receive 'em
	result = 0;
	for (i = 1; i <= inputThreads; i++) {
		RecvMsg(0, received);
		if (received->type != ALLDONE) {
			printf("Child sent wrong type message\n");
		}
		result += received->value1;
	}
	
	printf("The total for 1 to %d using %d threads is %d.\n", target, inputThreads, result);

	return 0;

}

// waits for a message from the parent, then adds the sum of integers 
// between message->value1 and message->value2 inclusive
void *addit(void *arg) {
	int index, out, i;
	struct msg *message;

	index = (int)(intptr_t)arg;
	
	message = (struct msg *)malloc(sizeof(struct msg));

	RecvMsg(index, message); // wait for mail from parent
	
	if (message->type != RANGE) {
		//printf("Thread received wrong type of message\n");
	}

	// rewrite message and send it back
	message->iSender = index;
	message->type = ALLDONE;

	out = 0;
	for (i = message->value1; i <= message->value2; i++) {
		out += i;
	}

	message->value1 = out;

	SendMsg(0, message); // send the sum back

	return (void *)0;
}
